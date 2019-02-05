/* $Id: authsasllogin.c,v 1.1 1999/12/13 03:34:28 mrsam Exp $ */

/*
** Copyright 1998 - 1999 Double Precision, Inc.  See COPYING for
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

int authsasl_login(const char *method, const char *initresponse,
	char *(*getresp)(const char *),

	char **authtype,
	char **authdata)
{
char	*uid;
char	*pw;
char	*p;
int	n;

	if (initresponse)
	{
		uid=malloc(strlen(initresponse)+1);
		if (!uid)
		{
			perror("malloc");
			return (AUTHSASL_ERROR);
		}
		strcpy(uid, initresponse);
	}
	else
	{
		p=authsasl_tobase64("Username:", -1);
		if (!p)
		{
			perror("malloc");
			return (AUTHSASL_ERROR);
		}
		uid=getresp(p);
		free(p);
		if (!uid)
		{
			perror("malloc");
			return (AUTHSASL_ERROR);
		}

		if (*uid == '*')
		{
			free(uid);
			return (AUTHSASL_ABORTED);
		}
	}

	p=authsasl_tobase64("Password:", -1);
	if (!p)
	{
		free(uid);
		perror("malloc");
		return (AUTHSASL_ERROR);
	}

	pw=getresp(p);
	free(p);
	if (!pw)
	{
		free(uid);
		perror("malloc");
		return (AUTHSASL_ERROR);
	}

	if (*pw == '*')
	{
		free(pw);
		free(uid);
		return (AUTHSASL_ABORTED);
	}

	if ((n=authsasl_frombase64(uid)) < 0 ||
		(uid[n]=0, n=authsasl_frombase64(pw)) < 0)
	{
		free(uid);
		free(pw);
		return (AUTHSASL_ABORTED);
	}
	pw[n]=0;

	if ( (*authtype=malloc(sizeof(AUTHTYPE_LOGIN))) == 0)
	{
		free(uid);
		free(pw);
		perror("malloc");
		return (AUTHSASL_ERROR);
	}

	strcpy( *authtype, AUTHTYPE_LOGIN);

	if ( (*authdata=malloc(strlen(uid)+strlen(pw)+3)) == 0)
	{
		free( *authtype );
		free(uid);
		free(pw);
		perror("malloc");
		return (AUTHSASL_ERROR);
	}

	strcat(strcat(strcat(strcpy(*authdata, uid), "\n"), pw), "\n");
	return (AUTHSASL_OK);
}
