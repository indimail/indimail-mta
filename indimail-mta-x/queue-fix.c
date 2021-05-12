/*
 * $Log: queue-fix.c,v $
 * Revision 1.17  2021-05-12 15:51:58+05:30  Cprogrammer
 * set conf_split from CONFSPLIT env variable
 *
 * Revision 1.16  2020-11-30 10:19:51+05:30  Cprogrammer
 * replaced stdio with substdio and added option to specify queue subdirectory split
 *
 * Revision 1.15  2009-12-09 23:57:41+05:30  Cprogrammer
 * additional closeflag argument to uidinit()
 *
 * Revision 1.14  2005-07-04 18:05:19+05:30  Cprogrammer
 * size of tcpto changed to TCPTO_BUFSIZ
 *
 * Revision 1.13  2004-10-22 20:29:50+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.12  2004-10-11 14:00:13+05:30  Cprogrammer
 * use scan_int() instead of scan_ulong()
 *
 * Revision 1.11  2003-12-07 13:06:36+05:30  Cprogrammer
 * return non-zero for errors/warnings
 *
 * Revision 1.10  2003-10-23 01:27:16+05:30  Cprogrammer
 * fixed compilation warnings
 *
 * Revision 1.9  2003-10-12 01:14:03+05:30  Cprogrammer
 * added yanked dir
 *
 * Revision 1.8  2003-08-22 23:16:11+05:30  Cprogrammer
 * removed compiler warnings for multi-line strings
 *
 * Revision 1.7  2003-07-29 19:55:28+05:30  Cprogrammer
 * added RCS log
 *
 */

/*-
 * adapted from queue-fix 1.4
 * by Eric Huss
 * e-huss@netmeridian.com
 * 
 * reconstructs qmail's queue
 */
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <substdio.h>
#include <subfd.h>
#include <stralloc.h>
#include <fmt.h>
#include <error.h>
#include <getln.h>
#include <str.h>
#include <open.h>
#include <fifo.h>
#include <scan.h>
#include <strerr.h>
#include <fmt.h>
#include <sgetopt.h>
#include <strmsg.h>
#include "tcpto.h"
#include "direntry.h"
#include "getEnvConfig.h"
#include "auto_split.h"
#include "auto_uids.h"

#define FATAL "queue-fix: fatal: "
#define WARN  "queue-fix: warn: "

extern int rename (const char *, const char *);

stralloc        queue_dir = { 0 };	/*- the root queue dir with trailing slash */
stralloc        check_dir = { 0 };	/*- the current directory being checked */
stralloc        temp_filename = { 0 };	/*- temporary used for checking individuals */
stralloc        temp_dirname = { 0 };	/*- temporary used for checking individuals */
stralloc        old_name = { 0 };	/*- used in rename */
stralloc        new_name = { 0 };	/*- used in rename */
stralloc        mess_dir = { 0 };	/*- used for renaming in mess dir */
stralloc        query = { 0 };	/*- used in interactive query function */

static char     name_num[FMT_ULONG];
static int      flag_interactive = 0;
static int      flag_doit = 1;
static int      flag_dircreate = 0;
static int      flag_filecreate = 0;
static int      flag_permfix = 0;
static int      flag_namefix = 0;
static int      flag_unlink = 0;
static int      flag_verbose = 0;
static uid_t    qmailq_uid;
static uid_t    qmails_uid;
static uid_t    qmailr_uid;
static gid_t    qmail_gid;
static int      queueError = 0;
static int      split;

void
out(char *str)
{
	if (substdio_puts(subfdout, str) == -1)
		strerr_die2sys(111, FATAL, "write: ");
}

void
flush()
{
	if (substdio_flush(subfdout) == -1)
		strerr_die2sys(111, FATAL, "write: ");
	return;
}

void
usage()
{
	char            strnum[FMT_ULONG];

	strnum[fmt_int(strnum, auto_split)] = 0;
	strerr_warn4(WARN,
			"usage: queue-fix [-i | -N | -v] [-s split] queue_dir\n"
			"                 -s - queue split number (default ",
			strnum,
			")\n"
			"                 -i - Interactive Mode\n"
			"                 -N - Test Mode", 0);
	_exit(100);
}

void
die_check()
{
	strmsg_out1("Failed while checking directory structure.\n"
		"Make sure the given queue exists and you have permission to access it.\n\n"
		"Exiting...\n\n");
	_exit(111);
}

void
die_recon()
{
	strmsg_out1("Failed to reconstruct queue.\n"
			"Make sure the queue exists and you have permission to modify it.\n\n"
		   "Exiting...\n\n");
	_exit(111);
}

void
die_rerun()
{
	strmsg_out1(".tmp files exist in the queue.\n"
			"queue-fix may have abnormally terminated in a previous run.\n"
		   "The queue must be manually cleaned of the .tmp files.\n\n"
		   "Exiting...\n\n");
	_exit(111);
}

/*
 * returns 1==yes, 0==no
 */
int
confirm()
{
	int             match;

	if (getln(subfdinsmall, &query, &match, '\n'))
		return 0;
	if (!match)
		return 0;
	if (query.s[0] == 'y' || query.s[0] == 'Y' || query.s[0] == '\n')
		return 1;
	return 0;
}

/*
 * gid may be -1 on files for "unknown
 */
int
check_item(char *name, char *owner, char *group, uid_t uid, gid_t gid, int perm, char type, int size)
{
	struct stat     st;
	int             fd;
	char            strnum1[FMT_ULONG], strnum2[FMT_ULONG];

	/*- check for existence and proper credentials */
	switch (type) {
	case 'd':	/*- directory */
		if (stat(name, &st)) {
			queueError++;
			if (errno != error_noent)
				return -1;
			if (!flag_dircreate && flag_interactive) {
				strmsg_out1("It looks like some directories don't exist, should I create them? (Y/n) - ");
				if (!confirm())
					return -1;
				flag_dircreate = 1;
			}
			/*- create it */
			strmsg_out3("Creating directory [", name, "]\n");
			if (flag_doit && mkdir(name, perm))
				strerr_die4sys(111, FATAL, "mkdir ", name, ": ");
			strnum1[fmt_8long(strnum1, perm)] = 0;
			strmsg_out5("Changing permissions of [", name, "] to [", strnum1, "]\n");
			if (flag_doit && chmod(name, perm))
				strerr_die6sys(111, FATAL, "chmod ", strnum1, " ", name, ": ");
			strnum1[fmt_int(strnum1, uid)] = 0;
			strnum2[fmt_int(strnum2, gid)] = 0;
			strmsg_out11("Changing ownership of [", name, "] to uid ", strnum1, " (", owner, ") gid ", strnum2, " (", group, ")\n");
			if (flag_doit && chown(name, uid, gid))
				strerr_die8sys(111, FATAL, "chown ", strnum1, ":", strnum2, " ", name, ": ");
			return 0;
		}
		/*- check the values */
		if (st.st_uid != uid || st.st_gid != gid) {
			queueError++;
			if (!flag_permfix && flag_interactive) {
				strmsg_out1("It looks like some permissions are wrong, should I fix them? (Y/N) - ");
				if (!confirm())
					return -1;
				flag_permfix = 1;
			}
			/*- fix it */
			strnum1[fmt_int(strnum1, uid)] = 0;
			strnum2[fmt_int(strnum2, gid)] = 0;
			strmsg_out11("Changing ownership of [", name, "] to uid ", strnum1, " (", owner, ") gid ", strnum2, " (", group, ")\n");
			if (flag_doit && chown(name, uid, gid))
				strerr_die8sys(111, FATAL, "chown ", strnum1, ":", strnum2, " ", name, ": ");
		}
		if ((st.st_mode & 07777) != perm) {
			queueError++;
			if (!flag_permfix && flag_interactive) {
				strmsg_out1("It looks like some permissions are wrong, should I fix them? (Y/N) - ");
				if (!confirm())
					return -1;
				flag_permfix = 1;
			}
			/*- fix it */
			strnum1[fmt_8long(strnum1, perm)] = 0;
			strmsg_out5("Changing permissions of [", name, "] to [", strnum1, "]\n");
			if (flag_doit && chmod(name, perm))
				strerr_die6sys(111, FATAL, "chmod ", strnum1, " ", name, ": ");
		}
		return 0;
	case 'f':	/*- regular file */
		if (stat(name, &st))
			return -1;
		/*- check the values */
		if (st.st_uid != uid || (st.st_gid != gid && gid != -1)) {
			queueError++;
			if (!flag_permfix && flag_interactive) {
				strmsg_out1("It looks like some permissions are wrong, should I fix them? (Y/N) - ");
				if (!confirm())
					return -1;
				flag_permfix = 1;
			}
			/*- fix it */
			strnum1[fmt_int(strnum1, uid)] = 0;
			strnum2[fmt_int(strnum2, gid)] = 0;
			strmsg_out11("Changing ownership of [", name, "] to uid ", strnum1, " (", owner, ") gid ", strnum2, " (", group, ")\n");
			if (flag_doit && chown(name, uid, gid))
				strerr_die8sys(111, FATAL, "chown ", strnum1, ":", strnum2, " ", name, ": ");
		}
		if ((st.st_mode & 07777) != perm) {
			queueError++;
			if (!flag_permfix && flag_interactive) {
				strmsg_out1("It looks like some permissions are wrong, should I fix them? (Y/N) - ");
				if (!confirm())
					return -1;
				flag_permfix = 1;
			}
			/*- fix it */
			strnum1[fmt_8long(strnum1, perm)] = 0;
			strmsg_out5("Changing permissions of [", name, "] to [", strnum1, "]\n");
			if (flag_doit && chmod(name, perm))
				strerr_die6sys(111, FATAL, "chmod ", strnum1, " ", name, ": ");
		}
		return 0;
	case 'z':	/*- regular file with a size */
		if (stat(name, &st)) {
			queueError++;
			if (errno != error_noent)
				return -1;
			if (!flag_filecreate && flag_interactive) {
				strmsg_out1("It looks like some files don't exist, should I create them? (Y/N) - ");
				if (!confirm())
					return -1;
				flag_filecreate = 1;
			}
			/*- create it */
			strnum1[fmt_int(strnum1, size)] = 0;
			strmsg_out5("Creating [", name, "] with size [", strnum1, "]\n");
			if (flag_doit) {
				if ((fd = open_trunc(name)) == -1)
					strerr_die4sys(111, FATAL, "open_trunc: ", name, ": ");
				while (size--) {
					if (write(fd, "", 1) != 1)
						strerr_die4sys(111, FATAL, "write: ", name, ": ");
				}
				close(fd);
			}
			strnum1[fmt_8long(strnum1, perm)] = 0;
			strmsg_out5("Changing permissions of [", name, "] to [", strnum1, "]\n");
			if (flag_doit && chmod(name, perm))
				strerr_die6sys(111, FATAL, "chmod ", strnum1, " ", name, ": ");
			strnum1[fmt_int(strnum1, uid)] = 0;
			strnum2[fmt_int(strnum2, gid)] = 0;
			strmsg_out11("Changing ownership of [", name, "] to uid ", strnum1, " (", owner, ") gid ", strnum2, " (", group, ")\n");
			if (flag_doit && chown(name, uid, gid))
				strerr_die8sys(111, FATAL, "chown ", strnum1, ":", strnum2, " ", name, ": ");
			return 0;
		}
		/*- check the values */
		if (st.st_uid != uid || (st.st_gid != gid && gid != -1)) {
			queueError++;
			if (!flag_permfix && flag_interactive) {
				strmsg_out1("It looks like some permissions are wrong, should I fix them? (Y/N) - ");
				if (!confirm())
					return -1;
				flag_permfix = 1;
			}
			/*- fix it */
			strnum1[fmt_int(strnum1, uid)] = 0;
			strnum2[fmt_int(strnum2, gid)] = 0;
			strmsg_out11("Changing ownership of [", name, "] to uid ", strnum1, " (", owner, ") gid ", strnum2, " (", group, ")\n");
			if (flag_doit && chown(name, uid, gid))
				strerr_die8sys(111, FATAL, "chown ", strnum1, ":", strnum2, " ", name, ": ");
		}
		if ((st.st_mode & 07777) != perm) {
			queueError++;
			if (!flag_permfix && flag_interactive) {
				strmsg_out1("It looks like some permissions are wrong, should I fix them? (Y/N) - ");
				if (!confirm())
					return -1;
				flag_permfix = 1;
			}
			/*- fix it */
			strnum1[fmt_8long(strnum1, perm)] = 0;
			strmsg_out5("Changing permissions of [", name, "] to [", strnum1, "]\n");
			if (flag_doit && chmod(name, perm))
				strerr_die6sys(111, FATAL, "chmod ", strnum1, " ", name, ": ");
		}
		if (st.st_size != size) {
			queueError++;
			strerr_warn3(WARN, name, " is not the right size. I will not fix it, please investigate.", 0);
		}
		return 0;
	case 'p':	/*- a named pipe */
		if (stat(name, &st)) {
			queueError++;
			if (errno != error_noent)
				return -1;
			if (!flag_filecreate && flag_interactive) {
				strmsg_out1("It looks like some files don't exist, should I create them? (Y/N) - ");
				if (!confirm())
					return -1;
				flag_filecreate = 1;
			}
			/*- create it */
			strmsg_out3("Creating fifo [", name, "]\n");
			if (flag_doit && fifo_make(name, perm))
				strerr_die4sys(111, FATAL, "fifo_make: ", name, ": ");
			strnum1[fmt_8long(strnum1, perm)] = 0;
			strmsg_out5("Changing permissions of [", name, "] to [", strnum1, "]\n");
			if (flag_doit && chmod(name, perm))
				strerr_die6sys(111, FATAL, "chmod ", strnum1, " ", name, ": ");
			strnum1[fmt_int(strnum1, uid)] = 0;
			strnum2[fmt_int(strnum2, gid)] = 0;
			strmsg_out11("Changing ownership of [", name, "] to uid ", strnum1, " (", owner, ") gid ", strnum2, " (", group, ")\n");
			if (flag_doit && chown(name, uid, gid))
				strerr_die8sys(111, FATAL, "chown ", strnum1, ":", strnum2, " ", name, ": ");
			return 0;
		}
		/*- check the values */
		if (st.st_uid != uid || (st.st_gid != gid && gid != -1)) {
			queueError++;
			if (!flag_permfix && flag_interactive) {
				strmsg_out1("It looks like some permissions are wrong, should I fix them? (Y/N) - ");
				if (!confirm())
					return -1;
				flag_permfix = 1;
			}
			/*- fix it */
			strnum1[fmt_int(strnum1, uid)] = 0;
			strnum2[fmt_int(strnum2, gid)] = 0;
			strmsg_out11("Changing ownership of [", name, "] to uid ", strnum1, " (", owner, ") gid ", strnum2, " (", group, ")\n");
			if (flag_doit && chown(name, uid, gid))
				strerr_die8sys(111, FATAL, "chown ", strnum1, ":", strnum2, " ", name, ": ");
		}
		if ((st.st_mode & 07777) != perm) {
			queueError++;
			if (!flag_permfix && flag_interactive) {
				strmsg_out1("It looks like some permissions are wrong, should I fix them? (Y/N) - ");
				if (!confirm())
					return -1;
				flag_permfix = 1;
			}
			/*- fix it */
			strnum1[fmt_8long(strnum1, perm)] = 0;
			strmsg_out5("Changing permissions of [", name, "] to [", strnum1, "]\n");
			if (flag_doit && chmod(name, perm))
				strerr_die6sys(111, FATAL, "chmod ", strnum1, " ", name, ": ");
		}
		return 0;
	}	/*- end switch */
	return 0;
}

int
check_files(char *directory, char *owner, char *group, uid_t uid, gid_t gid, int perm)
{
	DIR            *dir;
	direntry       *d;

	if (!(dir = opendir(directory)))
		return -1;
	while ((d = readdir(dir))) {
		if (d->d_name[0] == '.')
			continue;
		if (!stralloc_copys(&temp_filename, directory))
			strerr_die2sys(111, FATAL, "out of memory");
		if (!stralloc_append(&temp_filename, "/"))
			strerr_die2sys(111, FATAL, "out of memory");
		if (!stralloc_cats(&temp_filename, d->d_name))
			strerr_die2sys(111, FATAL, "out of memory");
		if (!stralloc_0(&temp_filename))
			strerr_die2sys(111, FATAL, "out of memory");
		if (check_item(temp_filename.s, owner, group, uid, gid, perm, 'f', 0)) {
			closedir(dir);
			return -1;
		}
	}
	closedir(dir);
	return 0;
}

void
warn_files(char *directory)
{
	DIR            *dir;
	direntry       *d;
	int             found = 0;

	if (!(dir = opendir(directory)))
		return;
	while ((d = readdir(dir))) {
		if (d->d_name[0] == '.')
			continue;
		found = 1;
		break;
	}
	closedir(dir);
	if (found) {
		out("Found files in ");
		out(directory);
		out("that shouldn't be there.\n");
		out("I will not remove them. You should consider checking it out.\n\n");
		flush();
	}
}

int
check_splits(char *directory, char *owner, char *group, char *fgroup,
		uid_t dir_uid, gid_t dir_gid, int dir_perm, gid_t file_gid, int file_perm)
{
	DIR            *dir;
	direntry       *d;
	int             i;

	for (i = 0; i < split; i++) {
		name_num[fmt_ulong(name_num, i)] = 0;
		if (!stralloc_copys(&temp_dirname, directory))
			strerr_die2sys(111, FATAL, "out of memory");
		if (!stralloc_append(&temp_dirname, "/"))
			strerr_die2sys(111, FATAL, "out of memory");
		if (!stralloc_cats(&temp_dirname, name_num))
			strerr_die2sys(111, FATAL, "out of memory");
		if (!stralloc_0(&temp_dirname))
			strerr_die2sys(111, FATAL, "out of memory");
		/*- check the split dir */
		if (check_item(temp_dirname.s, owner, group, dir_uid, dir_gid, dir_perm, 'd', 0))
			return -1;
		/*- check its contents */
		if (!(dir = opendir(temp_dirname.s)))
			return -1;
		while ((d = readdir(dir))) {
			if (d->d_name[0] == '.')
				continue;
			if (!stralloc_copy(&temp_filename, &temp_dirname))
				strerr_die2sys(111, FATAL, "out of memory");
			temp_filename.len--; /*- remove NUL */
			if (!stralloc_append(&temp_filename, "/"))
				strerr_die2sys(111, FATAL, "out of memory");
			if (!stralloc_cats(&temp_filename, d->d_name))
				strerr_die2sys(111, FATAL, "out of memory");
			if (!stralloc_0(&temp_filename))
				strerr_die2sys(111, FATAL, "out of memory");
			if (check_item(temp_filename.s, owner, fgroup, dir_uid, file_gid, file_perm, 'f', 0)) {
				closedir(dir);
				return -1;
			}
		}
		closedir(dir);
	} /*- end for */
	return 0;
}

int
rename_mess(char *dir, char *part, char *new_part, char *old_filename, char *new_filename)
{
	struct stat     st;

	if (flag_interactive && !flag_namefix) {
		strmsg_out1("It looks like some files need to be renamed, should I rename them? (Y/N) - ");
		if (!confirm())
			return -1;
		flag_namefix = 1;
	}
	/*- prepare the old filename */
	if (!stralloc_copy(&old_name, &queue_dir))
		strerr_die2sys(111, FATAL, "out of memory");
	if (!stralloc_cats(&old_name, dir))
		strerr_die2sys(111, FATAL, "out of memory");
	if (!stralloc_cats(&old_name, part))
		strerr_die2sys(111, FATAL, "out of memory");
	if (!stralloc_append(&old_name, "/"))
		strerr_die2sys(111, FATAL, "out of memory");
	if (!stralloc_cats(&old_name, old_filename))
		strerr_die2sys(111, FATAL, "out of memory");
	if (!stralloc_0(&old_name))
		strerr_die2sys(111, FATAL, "out of memory");
	/*- prepare the new filename */
	if (!stralloc_copy(&new_name, &queue_dir))
		strerr_die2sys(111, FATAL, "out of memory");
	if (!stralloc_cats(&new_name, dir))
		strerr_die2sys(111, FATAL, "out of memory");
	if (!stralloc_cats(&new_name, new_part))
		strerr_die2sys(111, FATAL, "out of memory");
	if (!stralloc_append(&new_name, "/"))
		strerr_die2sys(111, FATAL, "out of memory");
	if (!stralloc_cats(&new_name, new_filename))
		strerr_die2sys(111, FATAL, "out of memory");
	if (!stralloc_0(&new_name))
		strerr_die2sys(111, FATAL, "out of memory");
	/*- check if destination exists */
	if (stat(new_name.s, &st) == 0) {
		/*- it exists */
		new_name.len--;	/*- remove NUL */
		/*- append an extension to prevent name clash */
		if (!stralloc_cats(&new_name, ".tmp"))
			strerr_die2sys(111, FATAL, "out of memory");
		if (!stralloc_0(&new_name))
			strerr_die2sys(111, FATAL, "out of memory");
		/*- do a double check for collision */
		if (stat(new_name.s, &st) == 0)
			die_rerun();
	}
	strmsg_out5("Renaming [", old_name.s, "] to [", new_name.s, "]\n");
	if (flag_doit && rename(old_name.s, new_name.s) && errno != error_noent)
		return -1;
	return 0;
}

int
fix_part(char *part, int part_num)
{
	DIR            *dir;
	direntry       *d;
	struct stat     st;
	char            inode[FMT_ULONG];
	char            new_part[FMT_ULONG];
	int             old_inode;
	int             correct_part_num;

	if (!stralloc_copy(&mess_dir, &queue_dir))
		strerr_die2sys(111, FATAL, "out of memory");
	if (!stralloc_cats(&mess_dir, "mess/"))
		strerr_die2sys(111, FATAL, "out of memory");
	if (!stralloc_cats(&mess_dir, part))
		strerr_die2sys(111, FATAL, "out of memory");
	if (!stralloc_0(&mess_dir))
		strerr_die2sys(111, FATAL, "out of memory");
	if (!(dir = opendir(mess_dir.s)))
		return -1;
	while ((d = readdir(dir))) {
		if (d->d_name[0] == '.')
			continue;
		/*- check from mess */
		if (!stralloc_copy(&temp_filename, &mess_dir))
			strerr_die2sys(111, FATAL, "out of memory");
		temp_filename.len--; /*- remove NUL */
		if (!stralloc_append(&temp_filename, "/"))
			strerr_die2sys(111, FATAL, "out of memory");
		if (!stralloc_cats(&temp_filename, d->d_name))
			strerr_die2sys(111, FATAL, "out of memory");
		if (!stralloc_0(&temp_filename))
			strerr_die2sys(111, FATAL, "out of memory");
		if (stat(temp_filename.s, &st)) {
			closedir(dir);
			return -1;
		}
		/*
		 * check that filename==inode number
		 * check that inode%auto_split==part_num
		 */
		scan_int(d->d_name, &old_inode);
		correct_part_num = st.st_ino % split;
		if (st.st_ino != old_inode || part_num != correct_part_num) {
			/*- rename */
			inode[fmt_ulong(inode, st.st_ino)] = 0;
			new_part[fmt_ulong(new_part, correct_part_num)] = 0;
			if (rename_mess("mess/", part, new_part, d->d_name, inode)) {
				closedir(dir);
				return -1;
			}
			if (rename_mess("info/", part, new_part, d->d_name, inode)) {
				closedir(dir);
				return -1;
			}
			if (rename_mess("local/", part, new_part, d->d_name, inode)) {
				closedir(dir);
				return -1;
			}
			if (rename_mess("remote/", part, new_part, d->d_name, inode)) {
				closedir(dir);
				return -1;
			}
			if (rename_mess("intd/", part, new_part, d->d_name, inode)) {
				closedir(dir);
				return -1;
			}
			if (rename_mess("todo/", part, new_part, d->d_name, inode)) {
				closedir(dir);
				return -1;
			}
			if (rename_mess("bounce", "", "", d->d_name, inode)) {
				closedir(dir);
				return -1;
			}
		}
	}
	closedir(dir);
	return 0;
}

int
clean_tmp(char *directory, char *part)
{
	DIR            *dir;
	direntry       *d;
	int             length;

	if (!stralloc_copy(&mess_dir, &queue_dir))
		strerr_die2sys(111, FATAL, "out of memory");
	if (!stralloc_cats(&mess_dir, directory))
		strerr_die2sys(111, FATAL, "out of memory");
	if (!stralloc_cats(&mess_dir, part))
		strerr_die2sys(111, FATAL, "out of memory");
	if (!stralloc_0(&mess_dir))
		strerr_die2sys(111, FATAL, "out of memory");
	if (!(dir = opendir(mess_dir.s)))
		return -1;
	while ((d = readdir(dir))) {
		if (d->d_name[0] == '.')
			continue;
		/*- check for tmp extension */
		length = str_len(d->d_name);
		if (length > 4) {
			if (str_equal(d->d_name + length - 4, ".tmp")) {
				/*- remove the extension */
				if (!stralloc_copys(&temp_filename, d->d_name))
					strerr_die2sys(111, FATAL, "out of memory");
				temp_filename.len -= 4;
				if (!stralloc_0(&temp_filename))
					strerr_die2sys(111, FATAL, "out of memory");

				if (rename_mess(directory, part, part, d->d_name, temp_filename.s)) {
					closedir(dir);
					return -1;
				}
			}
		}
	}
	closedir(dir);
	return 0;
}

int
fix_names()
{
	int             i;

	if (!stralloc_copy(&check_dir, &queue_dir))
		strerr_die2sys(111, FATAL, "out of memory");
	if (!stralloc_cats(&check_dir, "mess"))
		strerr_die2sys(111, FATAL, "out of memory");
	if (!stralloc_0(&check_dir))
		strerr_die2sys(111, FATAL, "out of memory");
	/*- make the filenames match their inode */
	for (i = 0; i < split; i++) {
		name_num[fmt_ulong(name_num, i)] = 0;
		if (fix_part(name_num, i))
			return -1;
	}
	/*- clean up any tmp files */
	for (i = 0; i < split; i++) {
		name_num[fmt_ulong(name_num, i)] = 0;
		if (clean_tmp("mess/", name_num))
			return -1;
		if (clean_tmp("info/", name_num))
			return -1;
		if (clean_tmp("local/", name_num))
			return -1;
		if (clean_tmp("remote/", name_num))
			return -1;
		if (clean_tmp("intd/", name_num))
			return -1;
		if (clean_tmp("todo/", name_num))
			return -1;
	}
	if (clean_tmp("bounce", ""))
		return -1;
	return 0;
}

int
check_dirs()
{
	/*- check root existence */
	if (!stralloc_copy(&check_dir, &queue_dir))
		strerr_die2sys(111, FATAL, "out of memory");
	if (!stralloc_0(&check_dir))
		strerr_die2sys(111, FATAL, "out of memory");
	if (check_item(check_dir.s, "qmailq", "qmail", qmailq_uid, qmail_gid, 0750, 'd', 0))
		return -1;
	/*- check the big 4 */
	if (!stralloc_copy(&check_dir, &queue_dir))
		strerr_die2sys(111, FATAL, "out of memory");
	if (!stralloc_cats(&check_dir, "info"))
		strerr_die2sys(111, FATAL, "out of memory");
	if (!stralloc_0(&check_dir))
		strerr_die2sys(111, FATAL, "out of memory");
	if (check_item(check_dir.s, "qmails", "qmail", qmails_uid, qmail_gid, 0700, 'd', 0))
		return -1;
	if (check_splits(check_dir.s, "qmails", "qmail", "qmail", qmails_uid, qmail_gid, 0700, qmail_gid, 0600))
		return -1;
	if (!stralloc_copy(&check_dir, &queue_dir))
		strerr_die2sys(111, FATAL, "out of memory");
	if (!stralloc_cats(&check_dir, "mess"))
		strerr_die2sys(111, FATAL, "out of memory");
	if (!stralloc_0(&check_dir))
		strerr_die2sys(111, FATAL, "out of memory");
	if (check_item(check_dir.s, "qmailq", "qmail", qmailq_uid, qmail_gid, 0750, 'd', 0))
		return -1;
	if (check_splits(check_dir.s, "qmailq", "qmail", 0, qmailq_uid, qmail_gid, 0750, -1, 0644))
		return -1;
	if (!stralloc_copy(&check_dir, &queue_dir))
		strerr_die2sys(111, FATAL, "out of memory");
	if (!stralloc_cats(&check_dir, "remote"))
		strerr_die2sys(111, FATAL, "out of memory");
	if (!stralloc_0(&check_dir))
		strerr_die2sys(111, FATAL, "out of memory");
	if (check_item(check_dir.s, "qmails", "qmail", qmails_uid, qmail_gid, 0700, 'd', 0))
		return -1;
	if (check_splits(check_dir.s, "qmails", "qmail", "qmail", qmails_uid, qmail_gid, 0700, qmail_gid, 0600))
		return -1;
	if (!stralloc_copy(&check_dir, &queue_dir))
		strerr_die2sys(111, FATAL, "out of memory");
	if (!stralloc_cats(&check_dir, "local"))
		strerr_die2sys(111, FATAL, "out of memory");
	if (!stralloc_0(&check_dir))
		strerr_die2sys(111, FATAL, "out of memory");
	if (check_item(check_dir.s, "qmails", "qmail", qmails_uid, qmail_gid, 0700, 'd', 0))
		return -1;
	if (check_splits(check_dir.s, "qmails", "qmail", "qmail", qmails_uid, qmail_gid, 0700, qmail_gid, 0600))
		return -1;
	if (!stralloc_copy(&check_dir, &queue_dir))
		strerr_die2sys(111, FATAL, "out of memory");
	if (!stralloc_cats(&check_dir, "todo"))
		strerr_die2sys(111, FATAL, "out of memory");
	if (!stralloc_0(&check_dir))
		strerr_die2sys(111, FATAL, "out of memory");
	if (check_item(check_dir.s, "qmailq", "qmail", qmailq_uid, qmail_gid, 0750, 'd', 0))
		return -1;
	if (check_splits(check_dir.s, "qmailq", "qmail", "qmail", qmailq_uid, qmail_gid, 0750, qmail_gid, 0644))
		return -1;
	if (!stralloc_copy(&check_dir, &queue_dir))
		strerr_die2sys(111, FATAL, "out of memory");
	if (!stralloc_cats(&check_dir, "intd"))
		strerr_die2sys(111, FATAL, "out of memory");
	if (!stralloc_0(&check_dir))
		strerr_die2sys(111, FATAL, "out of memory");
	if (check_item(check_dir.s, "qmailq", "qmail", qmailq_uid, qmail_gid, 0700, 'd', 0))
		return -1;
	if (check_splits(check_dir.s, "qmailq", "qmail", "qmail", qmailq_uid, qmail_gid, 0700, qmail_gid, 0644))
		return -1;
	/*- check the others */
	if (!stralloc_copy(&check_dir, &queue_dir))
		strerr_die2sys(111, FATAL, "out of memory");
	if (!stralloc_cats(&check_dir, "bounce"))
		strerr_die2sys(111, FATAL, "out of memory");
	if (!stralloc_0(&check_dir))
		strerr_die2sys(111, FATAL, "out of memory");
	if (check_item(check_dir.s, "qmails", "qmail", qmails_uid, qmail_gid, 0700, 'd', 0))
		return -1;
	if (check_files(check_dir.s, "qmails", "qmail", qmails_uid, qmail_gid, 0600))
		return -1;
	/*- Yank Dir */
	if (!stralloc_copy(&check_dir, &queue_dir))
		strerr_die2sys(111, FATAL, "out of memory");
	if (!stralloc_cats(&check_dir, "yanked"))
		strerr_die2sys(111, FATAL, "out of memory");
	if (!stralloc_0(&check_dir))
		strerr_die2sys(111, FATAL, "out of memory");
	if (check_item(check_dir.s, "qmailq", "qmail", qmailq_uid, qmail_gid, 0700, 'd', 0))
		return -1;

	if (!stralloc_copy(&check_dir, &queue_dir))
		strerr_die2sys(111, FATAL, "out of memory");
	if (!stralloc_cats(&check_dir, "pid"))
		strerr_die2sys(111, FATAL, "out of memory");
	if (!stralloc_0(&check_dir))
		strerr_die2sys(111, FATAL, "out of memory");
	if (check_item(check_dir.s, "qmailq", "qmail", qmailq_uid, qmail_gid, 0700, 'd', 0))
		return -1;
	warn_files(check_dir.s);
	/*- lock has special files that must exist */
	if (!stralloc_copy(&check_dir, &queue_dir))
		strerr_die2sys(111, FATAL, "out of memory");
	if (!stralloc_cats(&check_dir, "lock"))
		strerr_die2sys(111, FATAL, "out of memory");
	if (!stralloc_0(&check_dir))
		strerr_die2sys(111, FATAL, "out of memory");
	if (check_item(check_dir.s, "qmailq", "qmail", qmailq_uid, qmail_gid, 0750, 'd', 0))
		return -1;
	if (!stralloc_copy(&check_dir, &queue_dir))
		strerr_die2sys(111, FATAL, "out of memory");
	if (!stralloc_cats(&check_dir, "lock/sendmutex"))
		strerr_die2sys(111, FATAL, "out of memory");
	if (!stralloc_0(&check_dir))
		strerr_die2sys(111, FATAL, "out of memory");
	if (check_item(check_dir.s, "qmails", "qmail", qmails_uid, qmail_gid, 0600, 'z', 0))
		return -1;
	if (!stralloc_copy(&check_dir, &queue_dir))
		strerr_die2sys(111, FATAL, "out of memory");
	if (!stralloc_cats(&check_dir, "lock/tcpto"))
		strerr_die2sys(111, FATAL, "out of memory");
	if (!stralloc_0(&check_dir))
		strerr_die2sys(111, FATAL, "out of memory");
	if (check_item(check_dir.s, "qmailr", "qmail", qmailr_uid, qmail_gid, 0644, 'z', TCPTO_BUFSIZ))
		return -1;
	if (!stralloc_copy(&check_dir, &queue_dir))
		strerr_die2sys(111, FATAL, "out of memory");
	if (!stralloc_cats(&check_dir, "lock/trigger"))
		strerr_die2sys(111, FATAL, "out of memory");
	if (!stralloc_0(&check_dir))
		strerr_die2sys(111, FATAL, "out of memory");
	if (check_item(check_dir.s, "qmails", "qmail", qmails_uid, qmail_gid, 0622, 'p', 0))
		return -1;
	return 0;
}

int
check_strays(char *directory)
{
	DIR            *dir;
	direntry       *d;
	struct stat     st;
	int             inode;
	int             part;
	char            new_part[FMT_ULONG];

	dir = opendir(directory);
	if (!dir)
		return -1;
	while ((d = readdir(dir))) {
		if (d->d_name[0] == '.')
			continue;
		scan_int(d->d_name, &inode);
		part = inode % split;
		new_part[fmt_ulong(new_part, part)] = 0;
		/*- check for corresponding entry in mess dir */
		if (!stralloc_copy(&mess_dir, &queue_dir))
			strerr_die2sys(111, FATAL, "out of memory");
		if (!stralloc_cats(&mess_dir, "mess/"))
			strerr_die2sys(111, FATAL, "out of memory");
		if (!stralloc_cats(&mess_dir, new_part))
			strerr_die2sys(111, FATAL, "out of memory");
		if (!stralloc_append(&mess_dir, "/"))
			strerr_die2sys(111, FATAL, "out of memory");
		if (!stralloc_cats(&mess_dir, d->d_name))
			strerr_die2sys(111, FATAL, "out of memory");
		if (!stralloc_0(&mess_dir))
			strerr_die2sys(111, FATAL, "out of memory");
		if (stat(mess_dir.s, &st)) {
			/*- failed to find in mess */
			if (flag_interactive && !flag_unlink) {
				strmsg_out3("There are some stray files in ", directory,  "\nShould I remove them? (Y/N) - ");
				if (!confirm()) {
					closedir(dir);
					return -1;
				}
				flag_unlink = 1;
			}
			if (!stralloc_copys(&temp_filename, directory))
				strerr_die2sys(111, FATAL, "out of memory");
			if (!stralloc_append(&temp_filename, "/"))
				strerr_die2sys(111, FATAL, "out of memory");
			if (!stralloc_cats(&temp_filename, d->d_name))
				strerr_die2sys(111, FATAL, "out of memory");
			if (!stralloc_0(&temp_filename))
				strerr_die2sys(111, FATAL, "out of memory");
			strmsg_out3("Unlinking [", temp_filename.s, "]\n");
			if (flag_doit && unlink(temp_filename.s) == -1) {
				closedir(dir);
				return -1;
			}
		}
	}
	closedir(dir);
	return 0;
}

int
check_stray_parts()
{
	int             i;

	for (i = 0; i < split; i++) {
		name_num[fmt_ulong(name_num, i)] = 0;
		if (!stralloc_copy(&temp_dirname, &check_dir))
			strerr_die2sys(111, FATAL, "out of memory");
		if (!stralloc_append(&temp_dirname, "/"))
			strerr_die2sys(111, FATAL, "out of memory");
		if (!stralloc_cats(&temp_dirname, name_num))
			strerr_die2sys(111, FATAL, "out of memory");
		if (!stralloc_0(&temp_dirname))
			strerr_die2sys(111, FATAL, "out of memory");
		/*- check this dir for strays */
		if (check_strays(temp_dirname.s))
			return -1;
	}
	return 0;
}

int
find_strays()
{
	if (!stralloc_copy(&check_dir, &queue_dir))
		strerr_die2sys(111, FATAL, "out of memory");
	if (!stralloc_cats(&check_dir, "info"))
		strerr_die2sys(111, FATAL, "out of memory");
	if (check_stray_parts())
		return -1;
	if (!stralloc_copy(&check_dir, &queue_dir))
		strerr_die2sys(111, FATAL, "out of memory");
	if (!stralloc_cats(&check_dir, "local"))
		strerr_die2sys(111, FATAL, "out of memory");
	if (check_stray_parts())
		return -1;
	if (!stralloc_copy(&check_dir, &queue_dir))
		strerr_die2sys(111, FATAL, "out of memory");
	if (!stralloc_cats(&check_dir, "remote"))
		strerr_die2sys(111, FATAL, "out of memory");
	if (check_stray_parts())
		return -1;
	if (!stralloc_copy(&check_dir, &queue_dir))
		strerr_die2sys(111, FATAL, "out of memory");
	if (!stralloc_cats(&check_dir, "todo"))
		strerr_die2sys(111, FATAL, "out of memory");
	if (check_stray_parts())
		return -1;
	if (!stralloc_copy(&check_dir, &queue_dir))
		strerr_die2sys(111, FATAL, "out of memory");
	if (!stralloc_cats(&check_dir, "intd"))
		strerr_die2sys(111, FATAL, "out of memory");
	if (check_stray_parts())
		return -1;
	if (!stralloc_copy(&check_dir, &queue_dir))
		strerr_die2sys(111, FATAL, "out of memory");
	if (!stralloc_cats(&check_dir, "bounce"))
		strerr_die2sys(111, FATAL, "out of memory");
	if (!stralloc_0(&check_dir))
		strerr_die2sys(111, FATAL, "out of memory");
	if (check_strays(check_dir.s))
		return -1;
	return 0;
}

int
main(int argc, char **argv)
{
	int             opt;

	getEnvConfigInt(&split, "CONFSPLIT", auto_split);
	while ((opt = getopt(argc, argv, "iNvs:")) != opteof) {
		switch (opt)
		{
		case 'i':
			flag_interactive = 1;
			break;
		case 'N':
			flag_doit = 0;
			break;
		case 'v':
			flag_verbose = 1;
			break;
		case 's':
			scan_int(optarg, &split);
			break;
		default:
			usage();
		}
	}
	if (optind + 1 != argc)
		usage();
	if (!stralloc_copys(&queue_dir, argv[optind]))
		strerr_die2sys(111, FATAL, "out of memory");
	if (queue_dir.s[queue_dir.len - 1] != '/') {
		if (!stralloc_append(&queue_dir, "/"))
			strerr_die2sys(111, FATAL, "out of memory");
	}
	if (uidinit(1) == -1) {
		strerr_warn2(WARN, "Unable to get uids/gids: ", &strerr_sys);
		_exit (111);
	}
	/*- prepare the uid and gid */
	qmailq_uid = auto_uidq;
	qmails_uid = auto_uids;
	qmailr_uid = auto_uidr;
	qmail_gid = auto_gidq;
	/*- check that all the proper directories exist with proper credentials */
	if (check_dirs())
		die_check();
	/*- rename inode filenames */
	if (fix_names())
		die_check();
	/*- check for stray files */
	if (find_strays())
		die_check();
	out("queue-fix finished...\n");
	flush();
	return (queueError);
}

void
getversion_queue_fix_c()
{
	static char    *x = "$Id: queue-fix.c,v 1.17 2021-05-12 15:51:58+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
