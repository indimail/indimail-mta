/*
 * $Id: control.c,v 1.23 2022-10-12 16:44:09+05:30 Cprogrammer Exp mbhangui $
 */
#include <unistd.h>
#include <open.h>
#include <now.h>
#include <str.h>
#include <getln.h>
#include <stralloc.h>
#include <substdio.h>
#include <error.h>
#include <alloc.h>
#include <scan.h>
#include <env.h>
#include <lock.h>
#include <fmt.h>
#include <wait.h>
#include <makeargs.h>
#include "control.h"
#include "auto_control.h"
#include "variables.h"

static char     inbuf[2048];
static stralloc line = { 0 };
static stralloc me = { 0 };
static int      meok = 0;

/*-
 * remove white space from the end
 * of a stralloc variable
 */
void
striptrailingwhitespace(stralloc *sa)
{
	while (sa->len > 0) {
		switch (sa->s[sa->len - 1])
		{
		case '\n':
		case ' ':
		case '\t':
			--sa->len;
			break;
		default:
			return;
		}
	}
}

int
control_init()
{
	int             r;

	if ((r = control_readline(&me, "me")) == 1)
		meok = 1;
	return r; /*- negative for error */
}

/*
 * open control file fn
 * if not found, copy value of me to sa if flagme is specified and
 *   control file me exists
 * else
 * if def is defined, copy value of def to sa
 *   else leave sa untouched.
 */

int
control_rldef(stralloc *sa, char *fn, int flagme, char *def)
{
	int             r;

	if ((r = control_readline(sa, fn)))
		return r;
	if (flagme && meok)
		return stralloc_copy(sa, &me) ? 1 : -1;
	if (def)
		return stralloc_copys(sa, def) ? 1 : -1;
	return r;
}

/*
 * read a line from control file fn
 * into sa without the terminating newline
 * WARNING!!! sa is not null terminalted
 */
int
control_readline(stralloc *sa, char *fn)
{
	substdio        ss;
	int             fd, match;
	static stralloc controlfile = {0};

	if (*fn != '/' && *fn != '.') {
		if (!controldir) {
			if (!(controldir = env_get("CONTROLDIR")))
				controldir = auto_control;
		}
		if (!stralloc_copys(&controlfile, controldir) ||
				(controlfile.s[controlfile.len - 1] != '/' && !stralloc_cats(&controlfile, "/")) ||
				!stralloc_cats(&controlfile, fn))
			return -1;
	} else
	if (!stralloc_copys(&controlfile, fn))
		return -1;
	if (!stralloc_0(&controlfile))
		return -1;
	if ((fd = open_read(controlfile.s)) == -1) {
		if (errno == error_noent)
			return 0;
		return -1;
	}
	substdio_fdbuf(&ss, read, fd, inbuf, sizeof(inbuf));
	if (getln(&ss, sa, &match, '\n') == -1) {
		close(fd);
		return -1;
	}
	striptrailingwhitespace(sa);
	close(fd);
	return 1;
}

/*
 * read an int number from control file fn
 * into sa without the terminating newline
 */

int
control_readint(int *i, char *fn)
{
	int             u;

	switch (control_readline(&line, fn))
	{
	case 0:
		return 0;
	case -1:
		return -1;
	}
	if (!stralloc_0(&line))
		return -1;
	if (!scan_int(line.s, &u))
		return 0;
	*i = u;
	return 1;
}

/*
 * read an unsigned number from control file fn
 * into sa without the terminating newline
 */

int
control_readulong(unsigned long *i, char *fn)
{
	unsigned long   u;

	switch (control_readline(&line, fn))
	{
	case 0:
		return 0;
	case -1:
		return -1;
	}
	if (!stralloc_0(&line))
		return -1;
	if (!scan_ulong(line.s, &u))
		return 0;
	*i = u;
	return 1;
}

/*
 * read entire file in variable sa
 * without any interpretation (e.g. comments)
 * To be used in case a file contains '#' character
 * in the first column (which control_readfile() will
 * skip
 */
int
control_readnativefile(stralloc *sa, char *fn, int mode)
{
	substdio        ss;
	int             fd, match;
	static stralloc controlfile = {0};

	if (!stralloc_copys(sa, ""))
		return -1;
	if (*fn != '/' && *fn != '.') {
		if (!controldir) {
			if (!(controldir = env_get("CONTROLDIR")))
				controldir = auto_control;
		}
		if (!stralloc_copys(&controlfile, controldir) ||
				(controlfile.s[controlfile.len - 1] != '/' && !stralloc_cats(&controlfile, "/")) ||
				!stralloc_cats(&controlfile, fn))
			return -1;
	} else
	if (!stralloc_copys(&controlfile, fn))
		return -1;
	if (!stralloc_0(&controlfile))
		return -1;
	if ((fd = open_read(controlfile.s)) == -1) {
		if (errno == error_noent)
			return 0;
		return -1;
	}
	substdio_fdbuf(&ss, read, fd, inbuf, sizeof(inbuf));
	for (;;) {
		if (getln(&ss, &line, &match, '\n') == -1)
			break;
		if (!match && !line.len) {
			close(fd);
			return 1;
		}
		if (mode) { /* for qmail-dk, qmail-dkim */
			striptrailingwhitespace(&line);
			if (!stralloc_0(&line))
				break;
			if (line.s[0] && !stralloc_cat(sa, &line))
				break;
		} else
		if (!stralloc_cat(sa, &line))
			break;
		if (!match) {
			close(fd);
			return 1;
		}
	}
	close(fd);
	return -1;
}

/*
 * read entire file into variable sa
 * skipping line containing comments
 *
 * returns
 *  0 - file not found
 *  1 - file is opened successfuly
 * -1 - system error
 */
int
control_readfile(stralloc *sa, char *fn, int flagme)
{
	substdio        ss;
	int             fd, match;
	static stralloc controlfile = {0};

	if (!stralloc_copys(sa, ""))
		return -1;
	if (*fn != '/' && *fn != '.') {
		if (!controldir) {
			if (!(controldir = env_get("CONTROLDIR")))
				controldir = auto_control;
		}
		if (!stralloc_copys(&controlfile, controldir) ||
				(controlfile.s[controlfile.len - 1] != '/' && !stralloc_cats(&controlfile, "/")) ||
				!stralloc_cats(&controlfile, fn))
			return -1;
	} else
	if (!stralloc_copys(&controlfile, fn))
		return -1;
	if (!stralloc_0(&controlfile))
		return -1;
	if ((fd = open_read(controlfile.s)) == -1) {
		if (errno == error_noent) {
			if (flagme && meok) {
				if (!stralloc_copy(sa, &me) ||
						!stralloc_0(sa))
					return -1;
				return 1;
			}
			return 0;
		}
		return -1;
	}
	substdio_fdbuf(&ss, read, fd, inbuf, sizeof(inbuf));
	for (;;) {
		if (getln(&ss, &line, &match, '\n') == -1)
			break;
		if (!match && !line.len) {
			close(fd);
			return 1;
		}
		striptrailingwhitespace(&line);
		if (!stralloc_0(&line) ||
				(line.s[0] && line.s[0] != '#' && !stralloc_cat(sa, &line)))
			break;
		if (!match) {
			close(fd);
			return 1;
		}
	}
	close(fd);
	return -1;
}

/*-
 * pick a random line from fn and copy it to sa
 */
int
control_readrandom(stralloc *sa, char *fn)
{
	substdio        ss;
	char           *ptr;
	int             fd, match, len, ilen, count;
	unsigned long   random;
	static stralloc controlfile = {0};

	if (!stralloc_copys(sa, ""))
		return -1;
	if (*fn != '/' && *fn != '.') {
		if (!controldir) {
			if (!(controldir = env_get("CONTROLDIR")))
				controldir = auto_control;
		}
		if (!stralloc_copys(&controlfile, controldir) ||
				(controlfile.s[controlfile.len - 1] != '/' && !stralloc_cats(&controlfile, "/")) ||
					!stralloc_cats(&controlfile, fn))
			return -1;
	} else
	if (!stralloc_copys(&controlfile, fn))
		return -1;
	if (!stralloc_0(&controlfile))
		return -1;
	if ((fd = open_read(controlfile.s)) == -1) {
		if (errno == error_noent)
			return 0;
		return -1;
	}
	substdio_fdbuf(&ss, read, fd, inbuf, sizeof(inbuf));
	for (count = 0;;count++) {
		if (getln(&ss, &line, &match, '\n') == -1)
			goto error;
		if (!match && !line.len)
			break;
		striptrailingwhitespace(&line);
		if (!stralloc_0(&line) ||
				(line.s[0] && line.s[0] != '#' && !stralloc_cat(sa, &line)))
			goto error;
		if (!match)
			break;
	}
	random = (now() % count);
	for (count = len = 0, ptr = sa->s;len < sa->len;count++) {
		len += ((ilen = str_len(ptr)) + 1);
		if (count == random) {
			if (!stralloc_copyb(sa, ptr, ilen)) /*- copy without the \0 */
				break;
			return 1;
		}
		ptr += (ilen + 1);
	}
error:
	close(fd);
	return -1;
}

extern int rename (const char *, const char *);

/*-
 * write the content of sa to control file fn,
 * after transforming null character to
 * newlines
 */
int
control_writefile(stralloc *sa, char *fn)
{
	int             i, wfd;
	static stralloc controlfileold = {0}, controlfilenew = {0};

	if (*fn != '/' && *fn != '.') {
		if (!controldir) {
			if (!(controldir = env_get("CONTROLDIR")))
				controldir = auto_control;
		}
		if (!stralloc_copys(&controlfileold, controldir) ||
				(controlfileold.s[controlfileold.len - 1] != '/' && !stralloc_cats(&controlfileold, "/")) ||
				!stralloc_cats(&controlfileold, fn))
			return -1;
	} else
	if (!stralloc_copys(&controlfileold, fn))
		return -1;
	if (!stralloc_copy(&controlfilenew, &controlfileold) ||
			!stralloc_0(&controlfileold) ||
			!stralloc_catb(&controlfilenew, ".tmp", 4) ||
			!stralloc_0(&controlfilenew))
		return -1;
	i = access(controlfilenew.s, F_OK);
	if ((wfd = (i ? open_excl : open_write) (controlfilenew.s)) == -1)
		return -1;
	else
	if (lock_ex(wfd) == -1) {
		unlink(controlfilenew.s);
		close(wfd);
		return -1;
	}
	for (i = 0; i < sa->len; i++) {
		if (sa->s[i] == '\0' )
			sa->s[i] = '\n';
	}
	if (write(wfd, sa->s, sa->len) == -1) {
		unlink(controlfilenew.s);
		close(wfd);
		return -1;
	}
	if (rename(controlfilenew.s, controlfileold.s))
		return -1;
	close(wfd);
	return 0;
}

/*-
 * write an int value to control file fn
 */
int
control_writeint(int val, char *fn)
{
	int             i, wfd;
	static stralloc controlfileold = {0}, controlfilenew = {0};
	char            strnum[FMT_ULONG];

	if (*fn != '/' && *fn != '.') {
		if (!controldir) {
			if (!(controldir = env_get("CONTROLDIR")))
				controldir = auto_control;
		}
		if (!stralloc_copys(&controlfileold, controldir) ||
				(controlfileold.s[controlfileold.len - 1] != '/' && !stralloc_cats(&controlfileold, "/")) ||
				!stralloc_cats(&controlfileold, fn))
			return -1;
	} else
	if (!stralloc_copys(&controlfileold, fn))
		return -1;
	if (!stralloc_copy(&controlfilenew, &controlfileold) ||
			!stralloc_0(&controlfileold) ||
			!stralloc_catb(&controlfilenew, ".tmp", 4) ||
			!stralloc_0(&controlfilenew))
		return -1;
	i = access(controlfilenew.s, F_OK);
	if ((wfd = (i ? open_excl : open_write) (controlfilenew.s)) == -1)
		return -1;
	else
	if (lock_ex(wfd) == -1) {
		unlink(controlfilenew.s);
		close(wfd);
		return -1;
	}
	strnum[i = fmt_int(strnum, val)] = 0;
	if (!stralloc_copyb(&line, strnum, i) ||
			!stralloc_append(&line, "\n")) {
		unlink(controlfilenew.s);
		close(wfd);
		return -1;
	}
	if (write(wfd, line.s, line.len) == -1) {
		unlink(controlfilenew.s);
		close(wfd);
		return -1;
	}
	if (rename(controlfilenew.s, controlfileold.s))
		return -1;
	close(wfd);
	return 0;
}

/*
 * 1. read lines from control file having '!'
 *    as the first char.
 * 2. execute the command after the '!'
 *    character
 * 3. Store the output in sa
 */
int
control_readcmd(stralloc *sa, char *fn)
{
	substdio        ss, ssin;
	int             fd, match, child, wstat;
	char          **argv;
	int             pi[2];
	static stralloc controlfile = {0};

	if (*fn != '/' && *fn != '.') {
		if (!controldir) {
			if (!(controldir = env_get("CONTROLDIR")))
				controldir = auto_control;
		}
		if (!stralloc_copys(&controlfile, controldir) ||
				(controlfile.s[controlfile.len - 1] != '/' && !stralloc_cats(&controlfile, "/")) ||
				!stralloc_cats(&controlfile, fn))
			return -1;
	} else
	if (!stralloc_copys(&controlfile, fn))
		return -1;
	if (!stralloc_0(&controlfile))
		return -1;
	if ((fd = open_read(controlfile.s)) == -1) {
		if (errno == error_noent)
			return 0;
		return -1;
	}
	substdio_fdbuf(&ss, read, fd, inbuf, sizeof(inbuf));
	if (getln(&ss, sa, &match, '\n') == -1) {
		close(fd);
		return -1;
	}
	striptrailingwhitespace(sa);
	close(fd);
	if (sa->s[0] == '!') {
		if (pipe(pi) == -1)
			return -1;
		switch ((child = fork()))
		{
		case -1:
			return -1;
		case 0:
			if (dup2(pi[1], 1) == -1)
				return -1;
			close(pi[0]); /*- close read end */
			if (!stralloc_0(sa) ||
					!(argv = makeargs(sa->s + 1)))
				return -1;
			execv(*argv, argv);
			_exit (1);
		}
		close(pi[1]); /*- close write end */
		substdio_fdbuf(&ssin, read, pi[0], inbuf, sizeof(inbuf));
		if (getln(&ssin, sa, &match, '\n') == -1) {
			close(fd);
			close(pi[0]);
			return -1;
		}
		striptrailingwhitespace(sa);
		close(pi[0]);
		if (wait_pid(&wstat, child) == -1 ||
				wait_crashed(wstat) ||
				wait_exitcode(wstat))
			return -1;
	}
	return 1;
}

#ifndef lint
void
getversion_control_c()
{
	static char    *x = "$Id: control.c,v 1.23 2022-10-12 16:44:09+05:30 Cprogrammer Exp mbhangui $";

	x++;
	x = sccsidmakeargsh;
	x++;
}
#endif

/*
 * $Log: control.c,v $
 * Revision 1.23  2022-10-12 16:44:09+05:30  Cprogrammer
 * enabled control_readcmd() which translates output of command as a control file
 *
 * Revision 1.22  2022-04-20 23:10:53+05:30  Cprogrammer
 * added control_writefile(), control_readint() functions
 *
 * Revision 1.21  2020-04-01 16:13:09+05:30  Cprogrammer
 * added header for MakeArgs() function
 *
 * Revision 1.20  2016-05-17 19:44:58+05:30  Cprogrammer
 * use auto_control, set by conf-control to set control directory
 *
 * Revision 1.19  2014-04-15 13:06:44+05:30  Cprogrammer
 * control_readrandom - copy to sa without \0
 *
 * Revision 1.18  2011-07-03 16:58:41+05:30  Cprogrammer
 * new function control_readrandom to pick up a random line from control file
 *
 * Revision 1.17  2011-01-14 20:36:48+05:30  Cprogrammer
 * added documentation on control_readfile()
 *
 * Revision 1.16  2010-06-27 09:02:43+05:30  Cprogrammer
 * added control_readcmd() function giving ability of control files to
 * have lines which are output of external commands
 *
 * Revision 1.15  2008-11-15 17:31:25+05:30  Cprogrammer
 * read signed integers instead of unsigned
 *
 * Revision 1.14  2004-10-22 20:24:11+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.13  2004-10-22 15:34:50+05:30  Cprogrammer
 * removed readwrite.h
 *
 * Revision 1.12  2004-10-21 22:44:36+05:30  Cprogrammer
 * override control directory for absolute path names
 *
 * Revision 1.11  2004-10-11 13:51:41+05:30  Cprogrammer
 * added function prototypes
 *
 * Revision 1.10  2004-02-03 13:50:15+05:30  Cprogrammer
 * increased size of inbuf for performance
 *
 * Revision 1.9  2003-12-20 01:46:21+05:30  Cprogrammer
 * use stralloc functions for preparing control file
 *
 * Revision 1.8  2003-10-23 01:18:17+05:30  Cprogrammer
 * fixed compilation warnings
 *
 * Revision 1.7  2003-10-18 18:31:51+05:30  Cprogrammer
 * removed SPAMTHROTTLE
 *
 * Revision 1.6  2003-07-20 16:59:51+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.5  2003-06-08 16:35:29+05:30  Cprogrammer
 * made striptrailingwhitespace visibale outside control.c
 *
 * Revision 1.4  2002-09-30 22:53:00+05:30  Cprogrammer
 * set sccsid
 *
 */
