/*
** Copyright 1998 - 2009 Double Precision, Inc.
** See COPYING for distribution information.
*/

/*
*/
#include	"rfc822.h"
#include	<stdlib.h>

static void cntlen(char c, void *p)
{
	c=c;
	++ *(size_t *)p;
}

static void cntlensep(const char *p, void *ptr)
{
	while (*p)	cntlen(*p++, ptr);
}

static void saveaddr(char c, void *ptr)
{
	*(*(char **)ptr)++=c;
}

static void saveaddrsep(const char *p, void *ptr)
{
	while (*p)	saveaddr(*p++, ptr);
}

char *rfc822_getaddrs(const struct rfc822a *rfc)
{
	size_t	addrbuflen=0;
	char	*addrbuf, *ptr;

	if (rfc822_print(rfc, &cntlen, &cntlensep, &addrbuflen) < 0)
		return NULL;

	if (!(addrbuf=malloc(addrbuflen+1)))
		return (0);

	ptr=addrbuf;
	if (rfc822_print(rfc, &saveaddr, &saveaddrsep, &ptr) < 0)
	{
		free(addrbuf);
		return NULL;
	}

	addrbuf[addrbuflen]=0;
	return (addrbuf);
}

static void saveaddrsep_wrap(const char *p, void *ptr)
{
int	c;

	while ((c=*p++) != 0)
	{
		if (c == ' ')	c='\n';
		saveaddr(c, ptr);
	}
}

char *rfc822_getaddrs_wrap(const struct rfc822a *rfc, int w)
{
	size_t	addrbuflen=0;
	char	*addrbuf, *ptr, *start, *lastnl;

	if (rfc822_print(rfc, &cntlen, &cntlensep, &addrbuflen) < 0)
		return NULL;

	if (!(addrbuf=malloc(addrbuflen+1)))
		return (0);

	ptr=addrbuf;

	if (rfc822_print(rfc, &saveaddr, &saveaddrsep_wrap, &ptr) < 0)
	{
		free(addrbuf);
		return NULL;
	}

	addrbuf[addrbuflen]=0;

	for (lastnl=0, start=ptr=addrbuf; *ptr; )
	{
		while (*ptr && *ptr != '\n')	ptr++;
		if (ptr-start < w)
		{
			if (lastnl)	*lastnl=' ';
			lastnl=ptr;
			if (*ptr)	++ptr;
		}
		else
		{
			if (lastnl)
				start=lastnl+1;
			else
			{
				start=ptr+1;
				if (*ptr)	++ptr;
			}
			lastnl=0;
		}
	}
	return (addrbuf);
}
