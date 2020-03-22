/* 
 * $Log: qmail-qfilter.c,v $
 * Revision 1.11  2019-02-17 11:40:14+05:30  Cprogrammer
 * fixed indentation
 *
 * Revision 1.10  2016-06-03 09:58:12+05:30  Cprogrammer
 * moved qmail-multi to sbin
 *
 * Revision 1.9  2011-07-13 20:56:18+05:30  Cprogrammer
 * removed useless close()
 *
 * Revision 1.8  2011-02-17 22:11:19+05:30  Cprogrammer
 * lseek envelope fd
 *
 * Revision 1.7  2009-04-24 23:50:47+05:30  Cprogrammer
 * version 2.1 of qmail-qfilter
 *
 * Revision 1.6  2009-04-22 20:04:30+05:30  Cprogrammer
 * added customized error messages
 *
 * Revision 1.5  2009-04-21 20:43:38+05:30  Cprogrammer
 * use bin/qmail-multi instead of absolute path
 *
 * Revision 1.4  2009-04-02 20:35:42+05:30  Cprogrammer
 * call qmail-multi instead of qmail-queue
 *
 * Revision 1.3  2004-10-22 20:28:40+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.2  2004-06-20 17:33:08+05:30  Cprogrammer
 * use setenv() for mysetenv()
 * use mkstemp() instead of tempnam()
 *
 * Revision 1.1  2004-05-23 20:28:44+05:30  Cprogrammer
 * Initial revision
 *
 *
 * Copyright (C) 2001,2004-2005 Bruce Guenter <bruceg@em.ca>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <stdlib.h>
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include "auto_qmail.h"
#include "qmail.h"
#include "substdio.h"
#include "env.h"
#include "scan.h"
#include "alloc.h"
#include "str.h"
#include "fmt.h"
#include "byte.h"

#ifndef TMPDIR
#define TMPDIR "/tmp"
#endif

#ifndef BUFSIZE
#define BUFSIZE 4096
#endif

#define QQ_OOM         51
#define QQ_WRITE_ERROR 53
#define QQ_READ_ERROR  54
#define QQ_INTERNAL    81
#define QQ_BAD_ENV     91
#define QQ_DROP_MSG    99

#define MSGIN  0
#define MSGOUT 1
#define ENVIN  3
#define ENVOUT 4
#define QQFD   5

#define memcpy(x,y,z)      byte_copy((x), (z), (y))

static size_t   env_len = 0;
static size_t   msg_len = 0;
char            strnum[FMT_ULONG];
static char    *binqqargs[2] = { "sbin/qmail-multi", 0 };
struct substdio sserr;
char            errbuf[256];

void
custom_error(char *flag, char *status, char *code)
{
	char           *c;

	if (substdio_put(&sserr, flag, 1) == -1)
		_exit(QQ_WRITE_ERROR);
	if (substdio_put(&sserr, "qmail-qfilter: ", 15) == -1)
		_exit(QQ_WRITE_ERROR);
	if (substdio_puts(&sserr, status) == -1)
		_exit(QQ_WRITE_ERROR);
	if (code) {
		if (substdio_put(&sserr, " (#", 3) == -1)
			_exit(QQ_WRITE_ERROR);
		c = (*flag == 'Z') ? "4" : "5";
		if (substdio_put(&sserr, c, 1) == -1)
			_exit(QQ_WRITE_ERROR);
		if (substdio_put(&sserr, code + 1, 4) == -1)
			_exit(QQ_WRITE_ERROR);
		if (substdio_put(&sserr, ")", 1) == -1)
			_exit(QQ_WRITE_ERROR);
	}
	if (substdio_flush(&sserr) == -1)
		_exit(QQ_WRITE_ERROR);
	return;
}
/*
 * Parse the sender address into user and host portions 
 */
size_t
parse_sender(char *env)
{
	char           *ptr = env;
	int             at, len;

	for (len = 0;*ptr != 'F' && len < env_len;len++,ptr++);
	if (*ptr != 'F') {
		custom_error("Z", "bad envelope. (#4.3.0)", 0);
		_exit(88);
	}
	ptr++;
	len++;
	if (!env_unset("QMAILNAME"))
		_exit(QQ_OOM);
	if (!*ptr) {
		if (!env_put("QMAILUSER=") || !env_put("QMAILHOST="))
			_exit(QQ_OOM);
		return 2;
	}
	at = str_chr(ptr, '@');
	if (ptr[at]) {
		if (!env_put2("QMAILHOST", ptr + at + 1))
			_exit(QQ_OOM);
		ptr[at] = 0;
		if (!env_put2("QMAILUSER", ptr))
			_exit(QQ_OOM);
		ptr[at] = '@';
	} else {
		if (!env_put2("QMAILUSER", ptr))
			_exit(QQ_OOM);
		if (!env_put("QMAILHOST="))
			_exit(QQ_OOM);
	}
	for (;*ptr; len++, ptr++);
	return (len + 1);
}

void
parse_rcpts(char *env, int offset)
{
	size_t          len = env_len - offset;
	char           *ptr = env + offset;
	char           *buf = alloc(len);
	char           *tmp = buf;
	unsigned long   count;

	count = 0;
	while (ptr < env + env_len && *ptr == 'T') {
		size_t          rcptlen = str_len(++ptr);
		memcpy(tmp, ptr, rcptlen);
		tmp[rcptlen] = '\n';
		tmp += rcptlen + 1;
		ptr += rcptlen + 1;
		++count;
	}
	*tmp = 0;
	if (!env_put2("QMAILRCPTS", buf))
		_exit(QQ_OOM);
	strnum[fmt_ulong(strnum, count)] = 0;
	if (!env_put2("NUMRCPTS", strnum))
		_exit(QQ_OOM);
	alloc_free(buf);
}

void
parse_envelope(void)
{
	char           *env;
	size_t          offset;

	if ((env = mmap(0, env_len, PROT_READ|PROT_WRITE, MAP_PRIVATE, ENVIN, 0)) == MAP_FAILED) {
		custom_error("Z", "mmap failed. (#4.3.0)", 0);
		_exit(88);
	}
	offset = parse_sender(env);
	parse_rcpts(env, offset);
	munmap((void *) env, env_len);
}

/*
 * Create a temporary invisible file opened for read/write 
 */
int
mktmpfile()
{
	char            filename[sizeof (TMPDIR) + 19] = TMPDIR "/fixheaders.XXXXXX";
	int             fd = mkstemp(filename);

	if (fd == -1) {
		custom_error("Z", "unable to create temp file. (#4.3.0)", 0);
		_exit(88);
	}
	/*
	 * The following makes the temporary file disappear immediately on
	 * program exit. 
	 */
	if (unlink(filename) == -1) {
		custom_error("Z", "unable to unlink temp file. (#4.3.0)", 0);
		close(fd);
		_exit(88);
	}
	return fd;
}

/*
 * Renumber from one FD to another 
 */
void
move_fd(int currfd, int newfd)
{
	if (currfd == newfd)
		return;
	if (dup2(currfd, newfd) != newfd) {
		custom_error("Z", "move_fd: dup2 failed. (#4.3.0)", 0);
		_exit(88);
	}
	if (close(currfd) != 0) {
		custom_error("Z", "move_fd: close failed. (#4.3.0)", 0);
		_exit(88);
	}
}

/*
 * Copy from one FD to a temporary FD 
 */
void
copy_fd(int fdin, int fdout, size_t * var)
{
	unsigned long   bytes;
	int             tmp = mktmpfile();

	/*
	 * Copy the message into the temporary file 
	 */
	for (bytes = 0;;) {
		char            buf[BUFSIZE];
		ssize_t         rd = read(fdin, buf, BUFSIZE);

		if (rd == -1)
			exit(QQ_READ_ERROR);
		if (rd == 0)
			break;
		if (write(tmp, buf, rd) != rd)
			exit(QQ_WRITE_ERROR);
		bytes += rd;
	}
	close(fdin);
	if (lseek(tmp, 0, SEEK_SET) != 0) {
		custom_error("Z", "unable to lseek. (#4.3.0)", 0);
		_exit(88);
	}
	move_fd(tmp, fdout);
	*var = bytes;
}

struct command {
	char          **argv;
	struct command *next;
};
typedef struct command command;

/*
 * Split up the command line into a linked list of seperate commands 
 */
command        *
parse_args(int argc, char *argv[])
{
	command        *tail = 0;
	command        *head = 0;

	while (argc > 0) {
		command        *cmd;
		int             end = 0;
		while (end < argc && str_diff(argv[end], "--"))
			++end;
		if (end == 0)
			_exit(QQ_INTERNAL);
		argv[end] = 0;
		cmd = (command *) alloc(sizeof (command));
		cmd->argv = argv;
		cmd->next = 0;
		if (tail)
			tail->next = cmd;
		else
			head = cmd;
		tail = cmd;
		++end;
		argv += end;
		argc -= end;
	}
	return head;
}

static void
mktmpfd(int fd)
{
	int             tmp;

	tmp = mktmpfile();
	move_fd(tmp, fd);
}

static void
move_unless_empty(int src, int dst, const void *reopen, size_t *var)
{
	struct stat     st;

	if (fstat(src, &st) != 0)
		_exit(QQ_INTERNAL);
	if (st.st_size > 0) {
		move_fd(src, dst);
		*var = st.st_size;
		if (reopen) {
			mktmpfd(src);
			if (src == ENVOUT)
				parse_envelope();
		}
	} else
	if (!reopen)
		close(src);
	if (lseek(dst, 0, SEEK_SET) != 0) {
		custom_error("Z", "unable to lseek. (#4.3.0)", 0);
		_exit(88);
	}
}

static void
read_qqfd(void)
{
	struct stat     st;
	char           *buf;

	if (fstat(QQFD, &st) != 0)
		_exit(QQ_INTERNAL);
	if (st.st_size > 0) {
		if ((buf = alloc(st.st_size + 1)) == 0)
			_exit(QQ_OOM);
		if (read(QQFD, buf, st.st_size) != st.st_size)
			_exit(QQ_READ_ERROR);
		buf[st.st_size] = 0;
		binqqargs[0] = buf;
	}
	close(QQFD);
}

/*
 * Run each of the filters in sequence 
 */
void
run_filters(const command * first)
{
	const command  *c;

	mktmpfd(MSGOUT);
	mktmpfd(ENVOUT);

	for (c = first; c; c = c->next) {
		pid_t           pid;
		int             status;

		strnum[fmt_ulong(strnum, env_len)] = 0;
		if (!env_put2("ENVSIZE", strnum))
			_exit(QQ_OOM);
		strnum[fmt_ulong(strnum, msg_len)] = 0;
		if (!env_put2("MSGSIZE", strnum))
			_exit(QQ_OOM);
		if (lseek(1, 0, SEEK_SET) != 0) {
			custom_error("Z", "unable to lseek envelope. (#4.3.0)", 0);
			_exit(88);
		}
		switch ((pid = fork()))
		{
		case -1:
			_exit(121);
		case 0:
			execvp(c->argv[0], c->argv);
			_exit(QQ_INTERNAL);
		}
		if (waitpid(pid, &status, WUNTRACED) == -1)
			exit(QQ_INTERNAL);
		if (!WIFEXITED(status))
			exit(QQ_INTERNAL);
		if (WEXITSTATUS(status))
			exit((WEXITSTATUS(status) == QQ_DROP_MSG) ? 0 : WEXITSTATUS(status));
		move_unless_empty(MSGOUT, MSGIN, c->next, &msg_len);
		move_unless_empty(ENVOUT, ENVIN, c->next, &env_len);
		if (lseek(QQFD, 0, SEEK_SET) != 0) {
			custom_error("Z", "unable to lseek qqfd. (#4.3.0)", 0);
			_exit(88);
		}
	}
}

int
main(int argc, char *argv[])
{
	const command  *filters;
	char           *x;
	int             errfd;

	if (!(x = env_get("ERROR_FD")))
		errfd = CUSTOM_ERR_FD;
	else
		scan_int(x, &errfd);
	substdio_fdbuf(&sserr, write, errfd, errbuf, sizeof(errbuf));
	if (!(binqqargs[0] = getenv("QQF_QMAILQUEUE")))
		binqqargs[0] = "sbin/qmail-multi";
	filters = parse_args(argc - 1, argv + 1);
	copy_fd(0, 0, &msg_len);
	copy_fd(1, ENVIN, &env_len);
	parse_envelope();
	mktmpfd(QQFD);
	run_filters(filters);
	read_qqfd();
	move_fd(ENVIN, 1);
	execv(*binqqargs, binqqargs);
	return QQ_INTERNAL;
}

void
getversion_qmail_qfilter_c()
{
	static char    *x = "$Id: qmail-qfilter.c,v 1.11 2019-02-17 11:40:14+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
