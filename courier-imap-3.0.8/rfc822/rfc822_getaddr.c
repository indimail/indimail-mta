/*
** Copyright 1998 - 2008 Double Precision, Inc.
** See COPYING for distribution information.
*/

/*
** $Id: rfc822_getaddr.c,v 1.8 2008/06/14 14:12:50 mrsam Exp $
*/
#include	"rfc822.h"
#include	<stdlib.h>

static void cntlen(char c, void *p)
{
	if (c != '\n')
		++ *(size_t *)p;
}

static void saveaddr(char c, void *p)
{
	if (c != '\n')
	{
	char **cp=(char **)p;

		*(*cp)++=c;
	}
}

char *rfc822_getaddr(const struct rfc822a *rfc, int n)
{
size_t	addrbuflen=0;
char	*addrbuf, *ptr;

	rfc822_praddr(rfc, n, &cntlen, &addrbuflen);
	if (!(addrbuf=malloc(addrbuflen+1)))
		return (0);

	ptr=addrbuf;
	rfc822_praddr(rfc, n, &saveaddr, &ptr);
	addrbuf[addrbuflen]=0;
	return (addrbuf);
}

/* Get rid of surrounding quotes */

static void dropquotes(char *addrbuf)
{
	char	*p, *q;

	p=q=addrbuf;

	if (*p == '"')
		++p;

	while (*p)
	{
		if (*p == '"' && p[1] == 0)
			break;

		*q++ = *p++;
	}

	*q=0;
}

char *rfc822_getname(const struct rfc822a *rfc, int n)
{
size_t	addrbuflen=0;
char	*addrbuf, *ptr;

	rfc822_prname(rfc, n, &cntlen, &addrbuflen);
	if (!(addrbuf=malloc(addrbuflen+1)))
		return (0);

	ptr=addrbuf;
	rfc822_prname(rfc, n, &saveaddr, &ptr);
	addrbuf[addrbuflen]=0;

	dropquotes(addrbuf);
	return (addrbuf);
}

char *rfc822_getname_orlist(const struct rfc822a *rfc, int n)
{
size_t	addrbuflen=0;
char	*addrbuf, *ptr;

	rfc822_prname_orlist(rfc, n, &cntlen, &addrbuflen);
	if (!(addrbuf=malloc(addrbuflen+1)))
		return (0);

	ptr=addrbuf;
	rfc822_prname_orlist(rfc, n, &saveaddr, &ptr);
	addrbuf[addrbuflen]=0;

	dropquotes(addrbuf);
	return (addrbuf);
}

char *rfc822_gettok(const struct rfc822token *t)
{
size_t	addrbuflen=0;
char	*addrbuf, *ptr;

	rfc822tok_print(t, &cntlen, &addrbuflen);

	if (!(addrbuf=malloc(addrbuflen+1)))
		return (0);

	ptr=addrbuf;
	rfc822tok_print(t, &saveaddr, &ptr);
	addrbuf[addrbuflen]=0;
	return (addrbuf);
}
