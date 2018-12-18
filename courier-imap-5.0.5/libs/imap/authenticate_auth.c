/*
** Copyright 1998 - 2003 Double Precision, Inc.
** See COPYING for distribution information.
*/

#if	HAVE_CONFIG_H
#include	"config.h"
#endif
#include	<string.h>
#include	<stdlib.h>
#if	HAVE_UNISTD_H
#include	<unistd.h>
#endif
#include	"imaptoken.h"
#include	"imapwrite.h"
#include	"authlib/auth.h"
#include	"authlib/authmod.h"
#include	"authlib/authsasl.h"
#include	"random128/random128.h"
#include	"rfc2045/rfc2045.h"

static const char rcsid[]="$Id: authenticate_auth.c,v 1.9 2003/05/27 15:55:09 mrsam Exp $";

extern int main_argc;
extern char **main_argv;

static char *send_auth_reply(const char *q)
{
	struct imaptoken *tok;
	char	*p, *cp;

#if SMAP
	cp=getenv("PROTOCOL");

	if ((cp && !strcmp(cp, "SMAP1")))
		writes("> ");
	else
#endif

	{
		writes("+ ");
	}
	writes(q);
	writes("\r\n");
	writeflush();
	if (!(cp = getenv("SOCKET_TIMEOUT")))
		read_timeout(SOCKET_TIMEOUT);
	else
		read_timeout(atoi(cp));
	tok=nexttoken_nouc();

	switch (tok->tokentype)	{
	case IT_ATOM:
	case IT_NUMBER:
		p=my_strdup(tok->tokenbuf);
		break;
	case IT_EOL:
		p=my_strdup("");
		break;
	default:
		return (0);
	}
	if (!p)
	{
		perror("malloc");
		return (0);
	}

	if (nexttoken()->tokentype != IT_EOL)
	{
		free(p);
		fprintf(stderr, "Invalid SASL response\n");
		return (0);
	}
	read_eol();
	return (p);
}

int authenticate(const char *tag)
{
struct imaptoken *tok=nexttoken();
char	*authmethod;
char	*initreply=0;
char	*tagenv;
char	*authtype, *authdata;
char	authservice[40];
char	*p ;

	switch (tok->tokentype)	{
	case IT_ATOM:
	case IT_QUOTED_STRING:
		break;
	default:
		return (0);
	}

	authmethod=my_strdup(tok->tokenbuf);

	tok=nexttoken_nouc();
	if (tok->tokentype != IT_EOL)
	{
		switch (tok->tokentype)	{
		case IT_ATOM:
		case IT_NUMBER:
			break;
		default:
			return (0);
		}
		initreply=my_strdup(tok->tokenbuf);
		if (strcmp(initreply, "=") == 0)
			*initreply=0;
		tok=nexttoken_nouc();
	}

	if (tok->tokentype != IT_EOL)	return (0);

	read_eol();
	if (authsasl(authmethod, initreply, &send_auth_reply, &authtype, &authdata))
	{
		free(authmethod);
		if (initreply)
			free(initreply);
		return (-1);
	}

	free(authmethod);
	if (initreply)
		free(initreply);

	if (!(tagenv = malloc(sizeof("IMAPLOGINTAG=") + strlen(tag))))
		write_error_exit(0);
	strcat(strcpy(tagenv, "IMAPLOGINTAG="), tag);
	putenv(tagenv);

	strcat(strcpy(authservice, "AUTHSERVICE"), getenv("TCPLOCALPORT"));
	p=getenv(authservice);

	if (!p || !*p)
		p="imap";

	authmod(main_argc-1, main_argv+1, p, authtype, authdata);
	return (-1);
}
