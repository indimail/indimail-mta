/* $Id: authsaslplain.c,v 1.1 2000/11/11 19:52:31 mrsam Exp $ */

/*
** Copyright 2000 Double Precision, Inc.  See COPYING for
** distribution information.
*/

#include	"config.h"
#include	"random128/random128.h"
#include	"authsasl.h"
#include	"authmod.h"
#include	<stdlib.h>
#include	<string.h>
#if	HAVE_UNISTD_H
#include	<unistd.h>
#endif
#include	<ctype.h>
#include	<stdio.h>
#include	<errno.h>

int authsasl_plain(const char *method, const char *initresponse,
	char *(*getresp)(const char *),

	char **authtype,
	char **authdata)
{
char	*uid;
char	*pw;
char	*p;
int	n;
int	i;

	if (initresponse)
	{
		p=malloc(strlen(initresponse)+1);
		if (!p)
		{
			perror("malloc");
			return (AUTHSASL_ERROR);
		}
		strcpy(p, initresponse);
	}
	else
	{
		p=authsasl_tobase64("", -1);
		if (!p)
		{
			perror("malloc");
			return (AUTHSASL_ERROR);
		}
		uid=getresp(p);
		free(p);
		p=uid;
		if (!p)
		{
			perror("malloc");
			return (AUTHSASL_ERROR);
		}

		if (*p == '*')
		{
			free(p);
			return (AUTHSASL_ABORTED);
		}
	}

	if ((n=authsasl_frombase64(p)) < 0)
	{
		free(p);
		return (AUTHSASL_ABORTED);
	}
	p[n]=0;

	uid=pw=0;

	for (i=0; i<n; i++)
	{
		if (p[i] == 0)
		{
			++i;
			for (uid=p+i; i<n; i++)
				if (p[i] == 0)
				{
					pw=p+i+1;
					break;
				}
		}
	}

	if (pw == 0)
	{
		free(p);
		return (AUTHSASL_ABORTED);	/* Bad message */
	}

	if ( (*authtype=malloc(sizeof(AUTHTYPE_LOGIN))) == 0)
	{
		free(p);
		perror("malloc");
		return (AUTHSASL_ERROR);
	}

	strcpy( *authtype, AUTHTYPE_LOGIN);

	if ( (*authdata=malloc(strlen(uid)+strlen(pw)+3)) == 0)
	{
		free( *authtype );
		free(p);
		perror("malloc");
		return (AUTHSASL_ERROR);
	}

	strcat(strcat(strcat(strcpy(*authdata, uid), "\n"), pw), "\n");
	free(p);
	return (AUTHSASL_OK);
}
