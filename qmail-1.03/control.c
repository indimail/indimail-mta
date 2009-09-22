/*
 * $Log: control.c,v $
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
#include "open.h"
#include "getln.h"
#include "stralloc.h"
#include "substdio.h"
#include "error.h"
#include "control.h"
#include "alloc.h"
#include "scan.h"
#include "env.h"
#include "variables.h"
#include <unistd.h>

static char     inbuf[2048];
static stralloc line = { 0 };
static stralloc me = { 0 };
static int      meok = 0;

void
striptrailingwhitespace(sa)
	stralloc       *sa;
{
	while (sa->len > 0)
	{
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

	if((r = control_readline(&me, "me")) == 1)
		meok = 1;
	return r; /*- negative for error */
}

int
control_rldef(sa, fn, flagme, def)
	stralloc       *sa;
	char           *fn;
	int             flagme;
	char           *def;
{
	int             r;

	if((r = control_readline(sa, fn)))
		return r;
	if (flagme && meok)
		return stralloc_copy(sa, &me) ? 1 : -1;
	if (def)
		return stralloc_copys(sa, def) ? 1 : -1;
	return r;
}

int
control_readline(sa, fn)
	stralloc       *sa;
	char           *fn;
{
	substdio        ss;
	int             fd, match;
	static stralloc controlfile = {0};

	if (*fn != '/' && *fn != '.')
	{
		if(!controldir)
		{
			if(!(controldir = env_get("CONTROLDIR")))
				controldir = "control";
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
	if((fd = open_read(controlfile.s)) == -1)
	{
		if (errno == error_noent)
			return 0;
		return -1;
	}
	substdio_fdbuf(&ss, read, fd, inbuf, sizeof(inbuf));
	if (getln(&ss, sa, &match, '\n') == -1)
	{
		close(fd);
		return -1;
	}
	striptrailingwhitespace(sa);
	close(fd);
	return 1;
}

int
control_readint(i, fn)
	int            *i;
	char           *fn;
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

int
control_readulong(i, fn)
	unsigned long  *i;
	char           *fn;
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
 * in the first column (which control_readnativefile() will
 * skip
 */
int
control_readnativefile(sa, fn, mode)
	stralloc       *sa;
	char           *fn;
	int             mode;
{
	substdio        ss;
	int             fd, match;
	static stralloc controlfile = {0};

	if (!stralloc_copys(sa, ""))
		return -1;
	if (*fn != '/' && *fn != '.')
	{
		if(!controldir)
		{
			if(!(controldir = env_get("CONTROLDIR")))
				controldir = "control";
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
	if ((fd = open_read(controlfile.s)) == -1)
	{
		if (errno == error_noent)
			return(0);
		return -1;
	}
	substdio_fdbuf(&ss, read, fd, inbuf, sizeof(inbuf));
	for (;;)
	{
		if (getln(&ss, &line, &match, '\n') == -1)
			break;
		if (!match && !line.len)
		{
			close(fd);
			return 1;
		}
		if (mode) /* for qmail-dk */
		{
			striptrailingwhitespace(&line);
			if (!stralloc_0(&line))
				break;
			if (line.s[0] && !stralloc_cat(sa, &line))
				break;
		} else
		if (!stralloc_cat(sa, &line))
			break;
		if (!match)
		{
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
 */
int
control_readfile(sa, fn, flagme)
	stralloc       *sa;
	char           *fn;
	int             flagme;
{
	substdio        ss;
	int             fd, match;
	static stralloc controlfile = {0};

	if (!stralloc_copys(sa, ""))
		return -1;
	if (*fn != '/' && *fn != '.')
	{
		if(!controldir)
		{
			if(!(controldir = env_get("CONTROLDIR")))
				controldir = "control";
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
	if((fd = open_read(controlfile.s)) == -1)
	{
		if (errno == error_noent)
		{
			if (flagme && meok)
			{
				if (!stralloc_copy(sa, &me))
					return -1;
				if (!stralloc_0(sa))
					return -1;
				return 1;
			}
			return 0;
		}
		return -1;
	}
	substdio_fdbuf(&ss, read, fd, inbuf, sizeof(inbuf));
	for (;;)
	{
		if (getln(&ss, &line, &match, '\n') == -1)
			break;
		if (!match && !line.len)
		{
			close(fd);
			return 1;
		}
		striptrailingwhitespace(&line);
		if (!stralloc_0(&line))
			break;
		if (line.s[0] && line.s[0] != '#' && !stralloc_cat(sa, &line))
			break;
		if (!match)
		{
			close(fd);
			return 1;
		}
	}
	close(fd);
	return -1;
}

void
getversion_control_c()
{
	static char    *x = "$Id: control.c,v 1.15 2008-11-15 17:31:25+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
