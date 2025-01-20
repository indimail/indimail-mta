/*
 * $Log: control.c,v $
 * Revision 1.4  2024-05-09 22:55:54+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.3  2020-08-03 17:21:35+05:30  Cprogrammer
 * replaced buffer with substdio
 *
 * Revision 1.2  2017-03-30 22:42:20+05:30  Cprogrammer
 * made control_readline() exactly duplicate of qmail-1.03 control_readline()
 *
 * Revision 1.1  2003-12-27 17:15:20+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <substdio.h>
#include <stralloc.h>
#include <scan.h>
#include <error.h>
#include <open.h>
#include <getln.h>
#include <env.h>
#include <unistd.h>
#include "control.h"

typedef const char c_char;
static char     inbuf[64];
static stralloc line = { 0 };

int
control_readint(int *i, const char *fn)
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

int
control_readline(stralloc *sa, const char *fn)
{
	substdio        ss;
	int             fd, match;
	static c_char  *controldir;
	static stralloc controlfile = {0};

	if (*fn != '/' && *fn != '.') {
		if (!controldir) {
			if (!(controldir = env_get("CONTROLDIR")))
				controldir = "/etc/indimail/control";
		}
		if (!stralloc_copys(&controlfile, controldir))
			return(-1);
		if (controlfile.s[controlfile.len - 1] != '/' && !stralloc_cats(&controlfile, "/"))
			return(-1);
		if (!stralloc_cats(&controlfile, fn))
			return(-1);
	} else
	if (!stralloc_copys(&controlfile, fn))
		return(-1);
	if (!stralloc_0(&controlfile))
		return(-1);
	if((fd = open_read(controlfile.s)) == -1) {
		if (errno == error_noent)
			return 0;
		return -1;
	}
	substdio_fdbuf(&ss, (ssize_t (*)(int,  char *, size_t)) read, fd, inbuf, sizeof(inbuf));
	if (getln(&ss, sa, &match, '\n') == -1) {
		close(fd);
		return -1;
	}
	striptrailingwhitespace(sa);
	close(fd);
	return 1;
}

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
