/* $Id: authsaslcram.c,v 1.3 2000/06/29 01:40:18 mrsam Exp $ */

/*
** Copyright 1998 - 1999 Double Precision, Inc.  See COPYING for
** distribution information.
*/

#include	"config.h"
#include	"random128/random128.h"
#include	"authsasl.h"
#include	<stdlib.h>
#include	<string.h>
#if	HAVE_UNISTD_H
#include	<unistd.h>
#endif
#include	<ctype.h>
#include	<stdio.h>
#include	<errno.h>

int authsasl_cram(const char *method, const char *initresponse,
	char *(*getresp)(const char *),

	char **authtype,
	char **authdata)
{
const	char *randtoken;
char	hostnamebuf[256];
char	*challenge;
char	*challenge_base64;
char	*response;
char	*chrsp;

	if (initresponse && *initresponse)
	{
		write(2, "authsasl_cram: invalid request.\n", 32);
		return (AUTHSASL_ERROR);
	}

	randtoken=random128();
	hostnamebuf[0]=0;
	if (gethostname(hostnamebuf, sizeof(hostnamebuf)-1))
		strcpy(hostnamebuf, "cram");

	challenge=malloc(strlen(randtoken)+strlen(hostnamebuf)
			+sizeof("<@>"));
	if (!challenge)
	{
		perror("malloc");
		return (AUTHSASL_ERROR);
	}
	strcat(strcat(strcat(strcat(strcpy(challenge, "<"),
		randtoken), "@"), hostnamebuf), ">");

	challenge_base64=authsasl_tobase64(challenge, -1);
	free(challenge);
	if (!challenge_base64)
	{
		perror("malloc");
		return (AUTHSASL_ERROR);
	}

	response=getresp(challenge_base64);
	if (!response)
	{
		free(challenge_base64);
		return (AUTHSASL_ERROR);
	}

	if (*response == '*')
	{
		free(challenge_base64);
		return (AUTHSASL_ABORTED);
	}

	chrsp=malloc(strlen(challenge_base64)+strlen(response)+3);
	if (!chrsp)    
	{
		free(challenge_base64);
		free(response);
		perror("malloc");
		return (AUTHSASL_ERROR);
	}

	strcat(strcat(strcat(strcpy(chrsp, challenge_base64), "\n"),
		response), "\n");
	free(challenge_base64);
	free(response);

	if ( (*authtype=malloc(strlen(method)+1)) == 0)
	{
		free(chrsp);
		perror("malloc");
		return (AUTHSASL_ERROR);
	}
	strcpy( *authtype, method );
	*authdata=chrsp;

	for (chrsp= *authtype; *chrsp; chrsp++)
		*chrsp= tolower( (int)(unsigned char)*chrsp );

	return (AUTHSASL_OK);
}
