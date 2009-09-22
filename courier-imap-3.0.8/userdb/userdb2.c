/*
** Copyright 1998 - 1999 Double Precision, Inc.
** See COPYING for distribution information.
*/

#if	HAVE_CONFIG_H
#include	"config.h"
#endif
#include	"dbobj.h"
#include	"userdb.h"
#include	<string.h>
#include	<stdlib.h>
#include	<stdio.h>
#include	<errno.h>

static const char rcsid[]="$Id: userdb2.c,v 1.4 2004/04/18 15:54:39 mrsam Exp $";

extern int userdb_debug_level;

char	*userdbshadow(const char *sh, const char *u)
{
struct dbobj d;
char	*p,*q;
size_t	l;

	dbobj_init(&d);

	if (dbobj_open(&d, sh, "R"))
	{
		userdb_debug_level && fprintf(stderr,
			"DEBUG: userdbshadow: unable to open %s\n", sh);
		return (0);
	}

	q=dbobj_fetch(&d, u, strlen(u), &l, "");
	dbobj_close(&d);
	if (!q)
	{
		userdb_debug_level && fprintf(stderr,
			"DEBUG: userdbshadow: entry not found\n");
		errno=ENOENT;
		return(0);
	}

	p=malloc(l+1);
	if (!p)	return (0);

	if (l)	memcpy(p, q, l);
	free(q);
	p[l]=0;
	return (p);
}
