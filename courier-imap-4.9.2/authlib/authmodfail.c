/*
** Copyright 1998 - 1999 Double Precision, Inc.  See COPYING for
** distribution information.
*/

#include	"auth.h"
#include	"authmod.h"
#include	<stdio.h>
#include	<string.h>
#include	<stdlib.h>
#include	<signal.h>
#if	HAVE_UNISTD_H
#include	<unistd.h>
#endif

static const char rcsid[]="$Id: authmodfail.c,v 1.4 1999/12/20 03:10:53 mrsam Exp $";

void	authmod_fail_completely()
{
char	**argv;
int	argc, n;
const char *p=getenv("AUTHARGC");
char	buf[20];

	if (!p || sscanf(p, "%d", &argc) <= 0 || argc <= 0)
	{
		write(2, "AUTHFAILURE\n", 12);
		authexit(1);
	}

	if ((argv=(char **)malloc((argc+1)*sizeof(char *))) == 0)
	{
		perror("malloc");
		authexit(1);
	}

	for (n=0; n<argc; n++)
	{
		sprintf(buf, "AUTHARGV%d", n);
		if ((argv[n]=getenv(buf)) == 0)
			authexit(1);
	}
	argv[n]=0;

	p=getenv("AUTHUSER");
	if (!p)
		authexit(1);

	execv(p, argv);
	perror(p);
	authexit(1);
}
