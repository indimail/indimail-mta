/*
** Copyright 1998 - 1999 Double Precision, Inc.  See COPYING for
** distribution information.
*/

#include	"auth.h"
#include	"authmod.h"
#include	<stdio.h>
#include	<string.h>
#include	<stdlib.h>
#include	<ctype.h>
#include	<signal.h>
#if	HAVE_UNISTD_H
#include	<unistd.h>
#endif
#include	<time.h>
#include	"authwait.h"

static const char rcsid[]="$Id: authmoduser2.c,v 1.7 2000/05/02 04:31:49 mrsam Exp $";

int authmoduser(int argc, char **argv, unsigned timeout, unsigned errsleep)
{
const char *p;
int	rc;
char	namebuf[60];
int	n;
time_t	t, expinterval;
char	*q, *r;
int	waitstat;

	signal(SIGCHLD, SIG_DFL);

	while (wait(&waitstat) >= 0)
		;
	p=getenv("AUTHUSER");
	if (!p && argc && argv[0][0] == '/')
		/* Set AUTHUSER from argv[0] */
	{
		q=malloc(sizeof("AUTHUSER=")+strlen(argv[0]));
		if (!q)
		{
			perror("malloc");
			authexit(1);
		}
		strcat(strcpy(q, "AUTHUSER="), argv[0]);
		putenv(q);
	}
	else if (!p || *p != '/')
	{
		write(2, argv[0], strlen(argv[0]));
		write(2, ": AUTHUSER is not initialized to a complete path\n",
			49);
		authexit(1);
	}

	putenv("AUTHENTICATED=");
	if (argc < 2)
	{
		write(2, "AUTHFAILURE\n", 12);
		authexit(1);
	}

	p=getenv("AUTHARGC");
	rc= p && *p && *p != '0' ? 0:1;

	sprintf(namebuf, "AUTHARGC=%d", argc);
	r=strdup(namebuf);
	if (!r)
	{
		perror("strdup");
		authexit(1);
	}

	putenv(r);
	for (n=0; n<argc; n++)
	{
	char	*p;

		sprintf(namebuf, "AUTHARGV%d=", n);
		p=malloc(strlen(namebuf)+1+strlen(argv[n]));
		if (!p)
		{
			perror("malloc");
			authexit(1);
		}
		strcat(strcpy(p, namebuf), argv[n]);
		putenv(p);
	}

	if (rc == 0 && errsleep)	sleep(errsleep);

	time(&t);
	p=getenv("AUTHEXPIRE");
	if (p && isdigit((int)(unsigned char)*p))
	{
		expinterval=0;
		do
		{
			expinterval=expinterval * 10 + (*p++ - '0');
		} while ( isdigit((int)(unsigned char)*p) );
	}
	else
	{
	time_t	n;

		expinterval=t + timeout;

		q=namebuf+sizeof(namebuf)-1;
		*q=0;

		n=expinterval;
		do
		{
			*--q = '0' + ( n % 10 );
		} while ( (n = n/10) != 0);

		q -= sizeof("AUTHEXPIRE=")-1;
		memcpy(q, "AUTHEXPIRE=", sizeof("AUTHEXPIRE=")-1);
		q=strdup(q);
		if (!q)
		{
			perror("strdup");
			authexit(1);
		}
		putenv(q);
	}

	if (timeout)
	{
		if (expinterval <= t)	authexit(1);
		alarm(expinterval - t);
	}
	return (rc);
}
