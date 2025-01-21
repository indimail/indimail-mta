/*
 * $Id: qmail-rm.c,v 1.29 2025-01-22 00:30:35+05:30 Cprogrammer Exp mbhangui $
 *
 * COPYRIGHT INFORMATION - DO NOT REMOVE
 *
 * "Portions Copyright (c) 2000-2001 LinuxMagic Inc. All Rights Reserved.
 * This file contains Original Code and/or Modifications of Original Code as
 * defined in and that are subject to the Wizard Software License Version
 * 1.0 (the 'License'). You may not use this file except in compliance with
 * the License. Please obtain a copy of the License at:
 *
 * http://www.linuxmagic.com/opensource/licensing/GPL-2.text
 *
 * and read it before using this file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND LINUXMAGIC HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT. Please see
 * the License for the specific language governing rights and limitations
 * under the License."
 *
 * Please read the terms of this license carefully. By using or downloading
 * this software or file, you are accepting and agreeing to the terms of this
 * license with LinuxMagic Inc. If you are agreeing to this license on behalf
 * of a company, you represent that you are authorized to bind the company to
 * such a license. If you do not meet this criterion or you do not agree to
 * any of the terms of this license, do NOT download, distribute, use or alter
 * this software or file in any way.
 *
 * This program is loosely based on the python script which
 * does a similar thing and can be found at:
 *
 * http://www.redwoodsoft.com/~dru/programs/
 *
 *
 * NOTES:
 *
 *     trash directory is relative to queue directory unless an
 *     absolute path is specified (begins with /) and assumed to
 *     exist.
 *
 *     queue directory should be absolute path
 *
 *     originally tested on Linux and OpenBSD
 *
 * TODO:
 *
 * cleanup the path generation routines
 *
 * Modified: 12/23/2003 Clint Martin ( c.martin*earthlink.net )
 *  I have added two additional parameters. -X and -x  both are intended to aid in forcing the expiration
 *  of messages in the qmail queue to happen in a different time frame than the default Qmail time frame.
 *  this is done by modifying the time stamps on the files in the "INFO" directory of the queue.
 *
 *  -X takes a parameter of the number of seconds to offset the creation/modification time of matching
 *      messages.  positive values cause the dates to go back in time, essentially making the message "OLDER"
 *      when passing through the Queue, qmail will bounce any messages that are TOO old.
 *      negative values essentially make the message YOUNGER. thereby lengthening the time they spend in the queue
 *      Passing the value 0 will offset into the past all matching files by exactly 7 days.
 *
 *  -x takes a complete date/timestamp as it's parameter.  All matching messages are re-stamped to this exact value
 *      The format of the parameter is the same as the output of the date(8) command. ie:
 *      "Tue Dec 23 11:42:29 PST 2003"
 *      This makes it easy to use the date command to change time stamps relative to "today" see examples below for details
 *
 *
 *  I think the -X parameter is relatively self explanatory... however:
 *
 *      Make all messages matching Foo@bar.com 1 day older.
 *
 *      qmail-remove -p Foo@bar.com -X 86400
 *
 *      Newer?
 *
 *      qmail-remove -p Foo@bar.com -X -86400
 *
 *  Some examples of the -x option:
 *
 *      Make all messages matching Foo@bar.com look like they went into the Queue Today
 *
 *      qmail-remove -p Foo@bar.com -x "`date`"
 *
 *      Yesterday?
 *
 *      qmail-remove -p Foo@bar.com -x "`date -v1d`"
 *
 *      Tomorrow?
 *
 *      qmail-remove -p Foo@bar.com -x "`date -v+1d`"
 *
 * I think you get the idea.
 *
 * Notes:
 *
 *      I think the -X option may be a little confusing. perhaps I should change it so that positive values
 *      offset into the future, whereas negative values offset into the past?  Thoughts?
 *
 *      The X, x and R options are mutually exclusive. Currently R will take precedence
 *
 *      Using the "X" option causes us to change the file time for all matching files
 *      to be OLDER than the queue expiration time.
 *
 *      This is intended as an easy way to clean out crap from the queue without having to shut down qmail, to do it.
 *      By changing the file timestamp, qmail will expire and clean out the queue for us.. of course we don't get to keep
 *      the files when this happens
 *
 *      This can also be used to keep a message in the queue for a longer period of time.  Using a negative offset, or a
 *      date in the future will accomplish this.
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <limits.h>
#include <regex.h>
#include <stdlib.h>
#include <string.h>
#define _XOPEN_SOURCE
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <fts.h>
#include <stralloc.h>
#include <substdio.h>
#include <lock.h>
#include <error.h>
#include <open.h>
#include <fmt.h>
#include <env.h>
#include <sgetopt.h>
#include <qprintf.h>
#include <noreturn.h>
#include "control.h"
#include "auto_split.h"
#include "getEnvConfig.h"
#include "hasrenameat.h"
#include "hasunlinkat.h"
#include "auto_qmail.h"

/*- many linux fcntl.h's seem to be broken */
#ifndef O_NOFOLLOW
#define O_NOFOLLOW  0400000
#endif

#ifndef QUEUE_COUNT
#define QUEUE_COUNT 10
#endif

/*- prototypes */
void            usage(void);
int             search_file(const char *, const char *);
int             remove_file(int, int, const char *);
int             delete_file(int, const char *);
int             expire_file(const char *);
char           *read_file(const char *);
int             find_files(char *, char *[], const char *);
unsigned long   digits(unsigned long);
char           *mk_nohashpath(char *, int);
char           *mk_hashpath(const char *, int);
char           *mk_newpath(const char *, int);
#ifdef HASRENAMEAT
int             renameat(int, const char *, int, const char *);
#else
int             rename(const char *, const char *);
#endif
char           *strptime(const char *, const char *, struct tm *);

typedef const char c_char;
/*- globals */
extern const char *__progname;
static const char     *default_pattern = ".*";
static unsigned long   read_bytes = 0;
static int      regex_flags = 0, verbosity = 0, conf_split, remove_files = 0, delete_files = 0;
static c_char  *yank_dir = "trash";

/*- if the eXpire option is specified on the command line, this will reflect that */
static int      expire_files = 0;
/*- one week in seconds -- this can be changed in the future by a parameter passed to us */
static time_t   expire_offset = 60 * 60 * 24 * 7;
/*- if specified, this is the timestamp the file will be stamped with */
static time_t   expire_date = 0;
static char     sserrbuf[512];
static char     ssoutbuf[512];
static char     strnum[FMT_ULONG];
static substdio ssout = SUBSTDIO_FDBUF((ssize_t (*)(int,  char *, size_t)) write, 1, ssoutbuf, sizeof(ssoutbuf));
static substdio sserr = SUBSTDIO_FDBUF((ssize_t (*)(int,  char *, size_t)) write, 2, sserrbuf, sizeof(sserrbuf));
static stralloc QueueBase = { 0 };
static stralloc Queuedir = { 0 };

/*
 * queues i
 *
 * NOTE: the first queue must be mess as it is used as a key
 *
 */
static const char     *subdirs[] = { "mess", "local", "remote", "info", "intd", "todo", "bounce", NULL };

void
flush()
{
	substdio_flush(&ssout);
}

void
errflush()
{
	substdio_flush(&sserr);
}

no_return void
die_nomem()
{
	substdio_putsflush(&sserr, "fatal: out of memory\n");
	_exit(111);
}

no_return void
die_chdir(char *dir)
{
	subprintf(&sserr, "chdir: %s: %s\n", dir, error_str(errno));
	errflush();
	_exit(111);
}

int
check_send(char *queuedir)
{
	static stralloc lockfile = { 0 };
	int             fd;

	if (!stralloc_copys(&lockfile, queuedir)) {
		substdio_putsflush(&sserr, "alert: out of memory\n");
		_exit(111);
	}
	if (!stralloc_cats(&lockfile, "/lock/sendmutex")) {
		substdio_putsflush(&sserr, "alert: out of memory\n");
		_exit(111);
	}
	if (!stralloc_0(&lockfile)) {
		substdio_putsflush(&sserr, "alert: out of memory\n");
		_exit(111);
	}
	if ((fd = open_write(lockfile.s)) == -1) {
		subprintf(&sserr, "alert: cannot start: unable to open %s: %s\n", lockfile.s, error_str(errno));
		errflush();
		_exit(111);
	} else
	if (lock_exnb(fd) == -1) {
		close(fd); /*- send already running */
		subprintf(&sserr, "alert: cannot start: qmail-send with queue %s is already running\n", queuedir);
		errflush();
		_exit(111);
	}
	if (access(yank_dir, F_OK)) {
		if (errno != error_noent) {
			subprintf(&sserr, "alert: cannot access: %s: %s\n", yank_dir, error_str(errno));
			errflush();
			_exit(111);
		}
		if (mkdir(yank_dir, 0700)) {
			subprintf(&sserr, "alert: cannot do mkdir: %s: %s\n", yank_dir, error_str(errno));
			errflush();
			_exit(111);
		}
	}
	return(fd);
}

int
main(int argc, char **argv)
{
	int             fd = -1, ch, matches, tmp = 0;
	int             idx, count, qcount, qstart;
	char           *ptr, *pattern = NULL, *qbase = 0, *queuedir = 0;
	const char     *format = "%a %b %d %H:%M:%S %Z %Y", *datearg = NULL;
	struct tm       stime;

	if (argc < 2)
		usage();
	getEnvConfigInt(&conf_split, "CONFSPLIT", auto_split);
	if (conf_split > auto_split)
		conf_split = auto_split;
	while ((ch = getopt(argc, argv, "deirvh?f:n:p:q:s:y:X:x:")) != opteof) {
		switch (ch)
		{
		case 'e':
			regex_flags |= REG_EXTENDED;
		case 'i':
			regex_flags |= REG_ICASE;
			break;
		case 'r':
			remove_files = 1;
			break;
		case 'd':
			delete_files = 1;
			break;
		case 'v':
			verbosity++;
			break;
		case 'n':
			read_bytes = strtoul(optarg, &ptr, 10);
			if ((read_bytes == ULONG_MAX && errno == ERANGE) || (!read_bytes && ptr == optarg)) {
				subprintf(&sserr, "%s: %s\n", optarg, error_str(errno));
				errflush();
				_exit(111);
			}
			if (*ptr) {
				subprintf(&sserr, "invalid char [%s] in %s\n", ptr, optarg);
				errflush();
				_exit(111);
			}
			break;
		case 'p':
			pattern = optarg;
			break;
		case 'q':
			qbase = optarg;
			break;
		case 's':
			tmp = strtoul(optarg, &ptr, 10);
			if ((tmp == ULONG_MAX && errno == ERANGE) || (!tmp && ptr == optarg)) {
				subprintf(&sserr, "%s: %s\n", optarg, error_str(errno));
				errflush();
				_exit(111);
			}
			if (*ptr) {
				subprintf(&sserr, "invalid char [%s] in %s\n", ptr, optarg);
				errflush();
				_exit(111);
			}
			conf_split = tmp;
			break;
		case 'X': /*- Added: 12/23/2003 Clint Martin ( c.martin*earthlink.net ) */
			if (expire_files == 2) {
				subprintf(&sserr, "you cannot specify both -x and -X\n");
				usage();
			}
			datearg = optarg;
			expire_files = 1;
			break;
		case 'f':
			format = optarg;
			break;
		case 'x':
			if (expire_files == 1) {
				subprintf(&sserr, "you cannot specify both -x and -X\n");
				usage();
			}
			datearg = optarg;
			expire_files = 2;
			break;
			/*- End Section Added: 12/23/2003 Clint Martin ( c.martin*earthlink.net ) ***/
		case 'y':
			yank_dir = optarg;
			break;
		case 'h':
		case '?':
		default:
			usage();
			break;
		}
	}
	if (expire_files == 1) {
		expire_date = -1;	/*- make sure we only use the offset */
		/*- lets see if they specified a parameter for seconds */
		tmp = strtoul(datearg, &ptr, 10);
		if ((tmp == ULONG_MAX && errno == ERANGE) || (!tmp && ptr == datearg)) {
			subprintf(&sserr, "%s: %s\n", datearg, error_str(errno));
			errflush();
			_exit(111);
		}
		if (*ptr) {
			subprintf(&sserr, "invalid char [%s] in %s\n", ptr, datearg);
			errflush();
			_exit(111);
		}
		expire_offset = (tmp == 0 ? expire_offset : tmp);
		subprintf(&ssout, "Offsetting timestamps by %ld seconds\n", expire_offset);
		flush();
	}
	if (expire_files == 2) {
		expire_files = 1;
		expire_offset = 0;	/*- make sure we only use the time stamp passed */
		/*- lets test our parsing function to see what it can do */
		ptr = (char *) strptime(datearg, format, &stime);
		expire_date = mktime(&stime);
		if (expire_date >= 0) {
			subprintf(&ssout, "time in seconds: %ld [%s]\n", expire_date, datearg);
			flush();
		} else {
			subprintf(&ssout, "Error parsing the date specified at: [%s] col %ld [%s]\n", ptr, ptr - datearg, datearg);
			flush();
			_exit(111);
		}
	}
	if (chdir(auto_qmail))
		die_chdir(auto_qmail);
	if (!qbase && !(qbase = env_get("QUEUE_BASE"))) {
		switch (control_readfile(&QueueBase, "queue_base", 0))
		{
		case -1:
			substdio_putsflush(&sserr, "fatal: unable to read controls\n");
			_exit(111);
			break;
		case 0:
			if (!stralloc_copys(&QueueBase, auto_qmail) ||
					!stralloc_catb(&QueueBase, "/queue", 6) ||
					!stralloc_0(&QueueBase))
				die_nomem();
			qbase = QueueBase.s;
			break;
		case 1:
			qbase = QueueBase.s;
			break;
		}
	}
	if (access(qbase, F_OK)) {
		subprintf(&sserr, "%s: %s\n", qbase, error_str(errno));
		errflush();
		_exit(111);
	}
	getEnvConfigInt(&qcount, "QUEUE_COUNT", QUEUE_COUNT);
	getEnvConfigInt(&qstart, "QUEUE_START", QUEUE_START);
	for (idx = qstart, count=1; count <= qcount; count++, idx++) {
		if (!stralloc_copys(&Queuedir, qbase) ||
				!stralloc_cats(&Queuedir, "/queue") ||
				!stralloc_catb(&Queuedir, strnum, fmt_ulong(strnum, (unsigned long) idx)) ||
				!stralloc_0(&Queuedir))
			die_nomem();
		if (access(Queuedir.s, F_OK)) {
			if (errno != error_noent) {
				subprintf(&sserr, "alert: cannot access: %s: %s\n", Queuedir.s, error_str(errno));
				errflush();
				_exit(111);
			}
			break;
		}
		queuedir = Queuedir.s;
		subprintf(&ssout, "processing queue %s\n", queuedir);
		flush();
		if (remove_files)
			fd = check_send(queuedir);
		if (chdir(queuedir))
			die_chdir(queuedir);
		matches = find_files(queuedir, (char **) subdirs, (pattern ? pattern : default_pattern));
		if (matches >= 0) {
			subprintf(&ssout, "%d file(s) match\n", matches);
			flush();
		}
		if (remove_files)
			close(fd);
	}
	_exit(0);
}


no_return void
usage(void)
{
	subprintf(&sserr, "%s [options]\n", (char *) __progname);
	subprintf(&sserr,
			"  -e            use extended POSIX regular expressions\n"
			"  -h, -?        this help message\n"
			"  -i            search case insensitively [default: case sensitive]\n"
			"  -n <bytes>    limit our search to the first <bytes> bytes of each file\n"
			"  -p <pattern>  specify the pattern to search for\n");
	subprintf(&sserr,
			"  -q <qbase>    specify the base qmail queue dir [default: %s/queue]\n", auto_qmail);
	subprintf(&sserr,
			"  -d            actually remove files not yank them, no -p will delete all the messages!\n"
			"  -r            actually remove files, without this we'll only print them\n");
	subprintf(&sserr,
			"  -s <split>    specify your conf-split value if non-standard [default: %d]\n", auto_split);
	subprintf(&sserr,
			"  -v            increase verbosity (can be used more than once)\n"
			"  -y <trash>    directory to put files removed from the queue [default: <queuedir>/trash]\n"
	/*- Begin CLM 12/23/2003 */
			"  -X <secs>     modify timestamp on matching files, to make qmail-send expire mail\n"
			"                <secs> is the number of seconds we want to move the file into the past.\n");
	subprintf(&sserr,
			"                specifying a value of 0 causes this to default to [%ld]\n", expire_offset);
	subprintf(&sserr,
			"  -x <timespec> modify timestamp on matching files, to make qmail-send expire mail\n"
			"                <timespec> is a date/time string in the format \"%%a %%b %%d %%H:%%M:%%S %%Z %%Y\"\n"
			"                You can change the format using -f option\n");
	errflush();
	/*- End CLM 12/23/2003 */
	_exit(111);
}

char           *
read_file(const char *filename)
{
	off_t           bytes;
	int             fd;
	char           *buff = NULL;
	struct stat     fd_stat;

	if (filename == NULL)
		return NULL;
	if ((fd = open(filename, (O_RDONLY | O_NOFOLLOW), 0)) == -1) {
		subprintf(&sserr, "open: %s: %s\n", filename, error_str(errno));
		errflush();
		return NULL;
	}
	if (fstat(fd, &fd_stat)) {
		subprintf(&sserr, "fstat: %s: %s\n", filename, error_str(errno));
		errflush();
		close(fd);
		return NULL;
	}
	bytes = fd_stat.st_size;
	if (!(buff = malloc(bytes + 1))) {
		subprintf(&sserr, "malloc: %s\n", error_str(errno));
		errflush();
		close(fd);
		return NULL;
	}
	buff[bytes] = '\0';
	if (read(fd, buff, bytes) != bytes) {
		free(buff);
		close(fd);
		subprintf(&sserr, "%s: read too short\n", (char *) __progname);
		errflush();
		return NULL;
	}
	close(fd);
	return buff;
}

int
search_file(const char *filename, const char *pattern)
{
	regex_t         match_me;
	int             err_code, match = 0;
	char           *file_inards = NULL, err_string[128];

	if (pattern == NULL)
		return (-1);
	if (filename == NULL)
		return (-1);
	if ((file_inards = read_file(filename))) {
		err_code = regcomp(&match_me, pattern, regex_flags | REG_NOSUB);
		if (err_code == 0) {
			if (regexec(&match_me, file_inards, 0, NULL, 0) == 0)
				match = 1;
		} else {
			/*- regex error */
			regerror(err_code, &match_me, err_string, 128);
			subprintf(&sserr, "regcomp(): %s\n", err_string);
			errflush();
		}
		regfree(&match_me);
		free(file_inards);
	}
	if (match == 1)
		return (0);
	return (-1);
}


int
find_files(char *queuedir, char *dir_list[], const char *pattern)
{
	FTS            *fts;
	FTSENT         *ftsp;
	char           *argv[2];
	int             i = 0, tmp_fd = -1, newfd;

	argv[0] = dir_list[0];
	argv[1] = NULL;
	if ((fts = fts_open((char **) argv, FTS_PHYSICAL, NULL)) == NULL) {
		subprintf(&sserr, "fts_open: %s\n", error_str(errno));
		errflush();
		return -1;
	}
	errno = 0;
	while ((ftsp = fts_read(fts)) != NULL) {
		switch (ftsp->fts_info)
		{
		case FTS_F:
			subprintf(&ssout, "%s/%s: ", queuedir, ftsp->fts_path);
			if (search_file(ftsp->fts_accpath, pattern) == 0) {
				subprintf(&ssout, "yes\n");
				flush();
				i++;
				if ((tmp_fd = open_read(".")) == -1) {
					subprintf(&sserr, "unable to open current directory: %s\n", error_str(errno));
					errflush();
					_exit(111);
				}
				if (remove_files == 1) {
					if (chdir(queuedir))
						die_chdir(queuedir);
					if ((newfd = open_read(yank_dir)) == -1) {
						subprintf(&sserr, "open: %s: %s\n", yank_dir, error_str(errno));
						errflush();
						_exit(111);
					}
					remove_file(tmp_fd, newfd, ftsp->fts_name);
					if (fchdir(tmp_fd) != 0) {
						subprintf(&sserr, "fchdir: %s\n", error_str(errno));
						errflush();
					}
				} else
				if (delete_files == 1) {
					if (chdir(queuedir))
						die_chdir(queuedir);
					delete_file(tmp_fd, ftsp->fts_name);
					if (fchdir(tmp_fd) != 0) {
						subprintf(&sserr, "fchdir: %s\n", error_str(errno));
						errflush();
					}
				} else
				if (expire_files == 1) {	/* this makes sure the the Remove option takes precedence over the eXpire options. */
					if (chdir(queuedir))
						die_chdir(queuedir);
					expire_file(ftsp->fts_name);
					if (fchdir(tmp_fd) != 0) {
						subprintf(&sserr, "fchdir: %s\n", error_str(errno));
						errflush();
					}
				}
				close(tmp_fd);
				tmp_fd = -1;
			} else {
				subprintf(&ssout, "no\n");
				flush();
			}
			break;
		case FTS_DNR:	/*- couldn't read */
		case FTS_ERR:	/*- error */
		case FTS_NS: 	/*- no stat info */
			subprintf(&sserr, "fts_read: %s: %s\n", ftsp->fts_path, error_str(errno));
			errflush();
			break;
		default:
			break;
		}
	}
	fts_close(fts);
	return i;
}


/*
 * remove_file()
 *
 * Takes a filename and assumes it is the path to a qmail queue file
 * (named after its inode). It attempts to find it in all the queues and
 * move them to the global "yank_dir". It returns the inode number of the
 * file it removed from the queue or -1 on an error.
 */
int
remove_file(int oldfd, int newfd, const char *filename)
{
	int             i, count = 0;
	unsigned long   inode_num;
	char           *ptr, *my_name, *old_name = NULL, *new_name = NULL;
	struct stat     statinfo;

	if (filename == NULL) {
		substdio_putsflush(&sserr, "remove_file: no filename\n");
		return -1;
	}
	if (!(my_name = strrchr(filename, '/')))
		my_name = (char *) filename;
	else
		my_name++;
	inode_num = strtoul(my_name, &ptr, 10);
	if ((inode_num == ULONG_MAX && errno == ERANGE) || (inode_num == 0 && ptr == my_name)) {
		subprintf(&sserr, "%s doesn't look like an inode number\n", my_name);
		errflush();
		return -1;
	}
	if (*ptr) {
		subprintf(&sserr, "Invalid char [%s] in filename %s\n", ptr, my_name);
		errflush();
		return -1;
	}
	for (i = 0; (subdirs[i] != NULL); i++) {
		if (!(new_name = mk_newpath((char *) subdirs[i], inode_num)))
			return -1;
		if (!(old_name = mk_hashpath((char *) subdirs[i], inode_num))) {
			free(new_name);
			return -1;
		}
#ifdef HASRENAMEAT
		if (!renameat(oldfd, old_name, newfd, new_name)) {
#else
		if (!rename(old_name, new_name)) {
#endif
			/*- succeeded */
			subprintf(&ssout, "moved %s to %s\n", old_name, new_name);
			flush();
			count++;
		} else {
			if (errno == ENOENT) {
				if (old_name) {
					if (verbosity >= 2) {
						subprintf(&sserr, "remove_file: %s: %s\n", old_name, error_str(errno));
						errflush();
					}
					free(old_name);
					old_name = NULL;
				}
				if (!(old_name = mk_nohashpath((char *) subdirs[i], inode_num))) {
					free(new_name);
					return -1;
				}
				if (stat(old_name, &statinfo) == -1) {
					if (verbosity >= 2) {
						subprintf(&sserr, "remove_file: stat %s: %s\n", old_name, error_str(errno));
						errflush();
					}
					continue;
				}
				if (!S_ISREG(statinfo.st_mode)) {
					if (verbosity >= 2) {
						subprintf(&sserr, "remove_file: %s not a file\n", old_name);
						errflush();
					}
					continue;
				}
#ifdef HASRENAMEAT
				if (!renameat(oldfd, old_name, newfd, new_name)) {
#else
				if (!rename(old_name, new_name)) {
#endif
					/*- succeeded */
					subprintf(&ssout, "moved %s to %s\n", old_name, new_name);
					flush();
					count++;
				} else
				if (errno != ENOENT) {
					subprintf(&sserr, "rename: %s -> %s: %s\n", old_name, new_name, error_str(errno));
					errflush();
				}
			} else { /*- failed but exists */
				subprintf(&sserr, "rename: %s -> %s: %s\n", old_name, new_name, error_str(errno));
				errflush();
			}
		}
	}
	/*- garbage collection */
	if (old_name)
		free(old_name);
	if (new_name)
		free(new_name);
	return count;
}


/*
 * Clint Martin 12/23/2003
 *
 * expire_file()
 *
 * Takes a filename and assumes it is the path to a qmail queue file
 * (named after its inode). It attempts to find it in the INFO dir, and
 * changes the file time stamp
 *
 * returns the number of successful file changes, or -1 on error.
 */
int
expire_file(const char *filename)
{
	int             count = 0;
	unsigned long   inode_num;
	char           *ptr, *my_name, *old_name;
	struct stat     statinfo;
	struct timeval  ut[2] = { {0} };

	if (filename == NULL) {
		substdio_putsflush(&sserr, "expire_file: no filename\n");
		return -1;
	}
	if (!(my_name = strrchr(filename, '/')))
		my_name = (char *) filename;
	else
		my_name++;
	/*- make sure the INODE NUMBER is really an INODE */
	inode_num = strtoul(my_name, &ptr, 10);
	if ((inode_num == ULONG_MAX && errno == ERANGE) || (inode_num == 0 && ptr == my_name)) {
		subprintf(&sserr, "%s doesn't look like an inode number\n", my_name);
		errflush();
		return -1;
	}
	if (*ptr) {
		subprintf(&sserr, "Invalid char [%s] in filename %s\n", ptr, my_name);
		errflush();
		return -1;
	}
	/*- generate the relative path to the specified file */
	if (!(old_name = mk_hashpath("info", inode_num)))
		return -1;
	if (expire_date > 0L) { /*- use the date specified if it was passed */
		ut[0].tv_sec = expire_date;
		ut[1].tv_sec = expire_date;
	} else /*- otherwise use the relative date offset form */
	if (!stat(old_name, &statinfo)) {
		ut[0].tv_sec = statinfo.st_mtime - expire_offset;
		ut[1].tv_sec = statinfo.st_ctime - expire_offset;
	}
	else {
		/*- do some error detection */
		if (errno == ENOENT && old_name) {
			if (verbosity >= 2) {
				subprintf(&sserr, "expire_file: %s: %s\n", old_name, error_str(errno));
				errflush();
			}
			free(old_name);
			old_name = NULL;
			return -1;
		}
	}
	/*- now, update the time stamp */
	if (utimes(old_name, ut) == 0) {
		subprintf(&sserr, "Set timestamp on %s\n", old_name);
		errflush();
		count++;
	} else
	if (errno == ENOENT) {
		if (old_name) {
			if (verbosity >= 2) {
				subprintf(&sserr, "expire_file: %s: %s\n", old_name, error_str(errno));
				errflush();
			}
			free(old_name);
			old_name = NULL;
		}
	}
	/*- garbage collection */
	if (old_name)
		free(old_name);
	return count;
}

char           *
mk_nohashpath(char *queue, int inode_name)
{
	int             len = 0;
	char           *old_name = NULL;

	if ((queue == NULL) || (inode_name <= 0))
		return NULL;
	len = strlen(queue);
	len += digits(inode_name);
	len += 4;
	old_name = malloc(len);
	if (old_name) {
		len = fmt_str(old_name, queue);
		len += fmt_str(old_name + len, "/");
		len += fmt_ulong(old_name + len, inode_name);
		old_name[len] = 0;
		return(old_name);
	} else {
		subprintf(&sserr, "mk_nohashpath: malloc: %s\n", error_str(errno));
		errflush();
		return NULL;
	}
}

char           *
mk_hashpath(const char *queue, int inode_name)
{
	int             len = 0, hash_num = 0;
	char           *old_name = NULL;

	if ((queue == NULL) || (inode_name <= 0))
		return NULL;
	hash_num = (inode_name % conf_split);
	len = strlen(queue);
	len += digits(hash_num);
	len += digits(inode_name);
	len += 4;
	old_name = malloc(len);
	if (old_name) {
		len = fmt_str(old_name, queue);
		len += fmt_str(old_name + len, "/");
		len += fmt_ulong(old_name + len, hash_num);
		len += fmt_str(old_name + len, "/");
		len += fmt_ulong(old_name + len, inode_name);
		old_name[len] = 0;
		return old_name;
	} else {
		subprintf(&sserr, "mk_nohashpath: malloc: %s\n", error_str(errno));
		errflush();
		return NULL;
	}
}

char           *
mk_newpath(const char *subdir, int inode_name)
{
	int             len = 0;
	char           *new_name = NULL;

	if ((subdir == NULL) || (inode_name <= 0)) {
		substdio_putsflush(&sserr, "mk_newpath: invalid subdir\n");
		errflush();
		return NULL;
	}
	len = strlen(subdir);
	len += strlen(yank_dir);
	len += digits(inode_name);
	len += 3;
	new_name = malloc(len);
	if (new_name) {
		len = fmt_str(new_name, yank_dir);
		len += fmt_str(new_name + len, "/");
		len += fmt_ulong(new_name + len, inode_name);
		len += fmt_str(new_name + len, ".");
		len += fmt_str(new_name + len, subdir);
		new_name[len] = 0;
		return new_name;
	} else {
		subprintf(&sserr, "mk_newpath: malloc: %s\n", error_str(errno));
		errflush();
		return NULL;
	}
}

/*
 * delete_file()
 *
 *     Takes a filename and assumes it is the path to a qmail queue file
 * (named after its inode). It attempts to find it in all the queues and
 * removes the file(s). It returns the inode number of the file it removed
 * from the queue or -1 on an error. This works with version .94 of qmail
 * -remove.
 */
int
delete_file(int fd, const char *filename)
{
	int             i, count = 0;
	unsigned long   inode_num;
	char           *my_name, *old_name = NULL;
	struct stat     statinfo;

	if (filename == NULL) {
		substdio_putsflush(&sserr, "expire_file: no filename\n");
		return -1;
	}
	my_name = strrchr(filename, '/');
	if (my_name == NULL)
		my_name = (char *) filename;
	else
		my_name++;

	inode_num = strtoul(my_name, NULL, 10);
	if ((inode_num == ULONG_MAX) || (inode_num == 0)) {
		subprintf(&sserr, "%s doesn't look like an inode number\n", my_name);
		errflush();
		return -1;
	}

	for (i = 0; (subdirs[i] != NULL); i++) {
		old_name = mk_hashpath((char *) subdirs[i], inode_num);
		if (old_name == NULL) {
			substdio_putsflush(&sserr, "delete_file: unable to create old name\n");
			return -1;
		}
#ifdef HASUNLINKAT
		if (unlinkat(fd, old_name, 0) == -1) {
#else
		if (unlink(old_name) == 0) { /*- succeeded */
#endif
			subprintf(&sserr, "remove %s\n", old_name);
			errflush();
			count++;
		} else {
			if (errno == ENOENT) {
				if (old_name) {
					if (verbosity >= 2) {
						subprintf(&sserr, "delete_file: %s: not a file\n", old_name);
						errflush();
					}
					free(old_name);
					old_name = NULL;
				}
				if (!(old_name = mk_nohashpath((char *) subdirs[i], inode_num)))
					return -1;
				if (stat(old_name, &statinfo) == -1) {
					if (verbosity >= 2) {
						subprintf(&sserr, "delete_file: %s: no stat info\n", old_name);
						errflush();
					}
					continue;
				}
				if (!S_ISREG(statinfo.st_mode)) {
					if (verbosity >= 2) {
						subprintf(&sserr, "delete_file: %s: not a file\n", old_name);
						errflush();
					}
					continue;
				}
#ifdef HASUNLINKAT
				if (unlinkat(fd, old_name, 0) == -1) {
#else
				if (unlink(old_name) == 0) {
#endif
					/*- succeeded */
					subprintf(&sserr, "remove %s\n", old_name);
					errflush();
					count++;
				} else {
					if (errno != ENOENT) {
						subprintf(&sserr, "unlink: %s: %s\n", old_name, error_str(errno));
						errflush();
					}
				}
			} else {
				/*- failed but exists */
				subprintf(&sserr, "unlink: %s: %s\n", old_name, error_str(errno));
				errflush();
			}
		}
	}
	/*- garbage collection */
	if (old_name)
		free(old_name);
	return count;
}

/*
 * digits()
 *
 * Returns the number of digits needed to represent the number "num".
 *
 */
unsigned long
digits(unsigned long num)
{
	unsigned long   i = 0;

	while (num >= 10) {
		num /= 10;
		i++;
	}
	i++;
	return (i);
}

void
getversion_qmail_rm_c()
{
	const char     *x = "$Id: qmail-rm.c,v 1.29 2025-01-22 00:30:35+05:30 Cprogrammer Exp mbhangui $";

	x++;
}

/*
 * $Log: qmail-rm.c,v $
 * Revision 1.29  2025-01-22 00:30:35+05:30  Cprogrammer
 * Fixes for gcc14
 *
 * Revision 1.28  2024-05-13 22:10:17+05:30  Cprogrammer
 * added -f option to specify strptime format
 *
 * Revision 1.27  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.26  2024-02-12 20:57:47+05:30  Cprogrammer
 * use unlinkat() if available instead of unlink()
 * use renameat() if available instead of rename()
 *
 * Revision 1.25  2023-07-13 02:45:04+05:30  Cprogrammer
 * replaced out(), logerr() with subprintf
 *
 * Revision 1.24  2022-03-27 20:21:52+05:30  Cprogrammer
 * define non returning functions as no_return
 *
 * Revision 1.23  2021-05-29 23:50:55+05:30  Cprogrammer
 * fixed qbase path
 *
 * Revision 1.22  2021-05-26 10:46:11+05:30  Cprogrammer
 * handle access() error other than ENOENT
 *
 * Revision 1.21  2021-05-16 00:48:57+05:30  Cprogrammer
 * use configurable conf_split instead of auto_split variable
 *
 * Revision 1.20  2021-05-12 15:50:28+05:30  Cprogrammer
 * set conf_split from CONFSPLIT env variable
 *
 * Revision 1.19  2020-09-16 19:05:27+05:30  Cprogrammer
 * fix compiler warning for FreeBSD
 *
 * Revision 1.18  2020-07-04 22:26:25+05:30  Cprogrammer
 * removed utime() with utimes()
 *
 * Revision 1.17  2020-05-11 11:00:29+05:30  Cprogrammer
 * fixed shadowing of global variables by local variables
 *
 * Revision 1.16  2019-06-07 11:26:36+05:30  Cprogrammer
 * replaced getopt() with subgetopt()
 *
 * Revision 1.15  2016-06-15 11:59:09+05:30  Cprogrammer
 * added -d option
 *
 * Revision 1.14  2014-01-29 14:03:50+05:30  Cprogrammer
 * fixed compilation warnings
 *
 * Revision 1.13  2013-12-04 15:45:47+05:30  Cprogrammer
 * added q option in getopt
 *
 * Revision 1.12  2011-11-06 22:53:08+05:30  Cprogrammer
 * corrected usage of strtoul()
 *
 * Revision 1.11  2010-07-20 20:10:46+05:30  Cprogrammer
 * process multiple queues
 *
 * Revision 1.10  2009-04-17 20:15:25+05:30  Cprogrammer
 * rearranged cases in switch statement
 *
 * Revision 1.9  2004-10-24 22:04:26+05:30  Cprogrammer
 * display invalid char given to strtoul
 *
 * Revision 1.8  2004-10-24 21:18:16+05:30  Cprogrammer
 * create yankdir only if the queue is a valid qmail queue
 *
 * Revision 1.7  2004-10-22 20:29:32+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.6  2004-10-09 09:41:24+05:30  Cprogrammer
 * use utime() instead of utimes()
 *
 * Revision 1.5  2004-10-09 00:28:45+05:30  Cprogrammer
 * removed stdio functions
 *
 * Revision 1.4  2004-08-14 02:25:35+05:30  Cprogrammer
 * option to expire files added
 */
