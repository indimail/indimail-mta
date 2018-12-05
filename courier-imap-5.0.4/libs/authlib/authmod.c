/*
** Copyright 1998 - 1999 Double Precision, Inc.  See COPYING for
** distribution information.
*/

#include	"auth.h"
#include	"authmod.h"
#include	"authwait.h"
#include	<stdio.h>
#include	<errno.h>
#include	<string.h>
#include	<stdlib.h>
#include	<signal.h>
#if	HAVE_UNISTD_H
#include	<unistd.h>
#endif

static const char rcsid[]="$Id: authmod.c,v 1.5 2001/11/29 02:57:15 mrsam Exp $";

static char	authrec[BUFSIZ];
static char	buf[BUFSIZ];

void	authmod_init(
		int argc,
		char **argv,
		const char **service,
		const char **authtype,
		char **authdata)
{
FILE	*fpin;
int	waitstat;
const char *a=getenv("AUTHENTICATED");
char	*p;
int	n;

	if (a && *a)	/* Already a good guy */
		authmod_success(argc, argv, a);

	n=0;
	fpin=fdopen(3, "r");
	if (fpin)
	{
	int	c;

		while ((c=getc(fpin)) != EOF)
			if (n < sizeof(buf)-1)
				buf[n++]=c;
		buf[n]=0;
	}

	if (n == 0)
	{
		write(2, "AUTHFAILURE\n", 12);
		authexit(1);
	}

	fclose(fpin);
	close(3);	/* Insurance */
	strcpy(authrec, buf);

	signal(SIGCHLD, SIG_DFL);

	while (wait(&waitstat) >= 0)
		;

	p=buf;
	*service=p;
	while (*p && *p != '\n')
		++p;
	if (*p)	*p++=0;
	*authtype=p;
	while (*p && *p != '\n')
		++p;
	if (*p)	*p++=0;
	*authdata=p;
}

void authmod_success(int argc, char **argv, const char *a)
{
char	**vec, *prog;
char	*b;

	vec=authcopyargv(argc-1, argv+1, &prog);
	if (!prog)	authexit(1);

	b=malloc(sizeof("AUTHENTICATED=")+strlen(a));
	if (!b)
	{
		perror("malloc");
		authexit(1);
	}
	strcat(strcpy(b, "AUTHENTICATED="), a);
	putenv(b);
	execv(prog, vec);
	perror(prog);
	authexit(1);
}

void authmod_fail(int argc, char **argv)
{
	authchain(argc-1, argv+1, authrec);	/* Next module */
}
