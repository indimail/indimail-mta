/*
** Copyright 1998 - 1999 Double Precision, Inc.  See COPYING for
** distribution information.
*/

#if	HAVE_CONFIG_H
#include	"config.h"
#endif
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#if	HAVE_UNISTD_H
#include	<unistd.h>
#endif
#include	"auth.h"

static const char rcsid[]="$Id: copyargv.c,v 1.3 1999/12/20 03:10:53 mrsam Exp $";

char **authcopyargv(int c, char **oldv, char **prog)
{
char **v;
int	n;

	if ((v=(char **)malloc(sizeof(char *)*(c+1))) == 0)
	{
		perror("malloc");
		authexit(1);
	}
	for (n=0; n<c; n++)
		v[n]=oldv[n];
	v[c]=0;
	if (v[0])
	{
		*prog=v[0];
	}
	else
		*prog=0;

	return (v);
}
