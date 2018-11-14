/*
** Copyright 1998 - 2008 Double Precision, Inc.
** See COPYING for distribution information.
*/

#if	HAVE_CONFIG_H
#undef	PACKAGE
#undef	VERSION
#include	"config.h"
#endif
#include	<stdio.h>
#include	<string.h>
#include	<stdlib.h>
#include	<signal.h>
#include	<ctype.h>
#include	<fcntl.h>
#if	HAVE_UNISTD_H
#include	<unistd.h>
#endif

static const char rcsid[]="$Id: pop3dcapa.c,v 1.5 2008/06/29 20:18:36 mrsam Exp $";

extern const char *externalauth();

int have_starttls()
{
	const char *p;

        if ((p=getenv("POP3_STARTTLS")) == 0)   return (0);
        if (*p != 'y' && *p != 'Y')             return (0);

        p=getenv("COURIERTLS");
        if (!p || !*p)  return (0);
        if (access(p, X_OK))    return (0);
        return (1);
}


int tls_required()
{
	const char *p=getenv("POP3_TLS_REQUIRED");

        if (p && atoi(p))       return (1);
        return (0);
}

const char *pop3_externalauth()
{
	const char *external=NULL;
	const char *p;

	if ((p=getenv("POP3_TLS")) != 0 && atoi(p))
		external=externalauth();

	return external;
}

void pop3dcapa()
{
	const char *p, *q;
	const char *external=pop3_externalauth();

	printf("+OK Here's what I can do:\r\n");

	if ((p=getenv("POP3_TLS")) != 0 && atoi(p) &&
	    (p=getenv("POP3AUTH_TLS")) != 0 && *p)
		;
	else
		p=getenv("POP3AUTH");

	if ((p && *p) || external)
	{
		if (!p)
			p="";

		if (!external)
			external="";

		printf("SASL %s%s%s\r\n", p, *p && *external ? " ":"",
	       *external ? "EXTERNAL":"");
	} else
		printf("SASL LOGIN PLAIN CRAM-MD5 CRAM-SHA1 CRAM-SHA256\r\n");
	if (have_starttls())
		printf("STLS\r\n");
	p = getenv("ENABLE_UTF8_COMPLIANCE");
	if (p && *p)
		printf("TOP\r\nUSER\r\nLOGIN-DELAY 10\r\nPIPELINING\r\nUIDL\r\nUTF8 USER\r\n.\r\n");
	else
		printf("TOP\r\nUSER\r\nLOGIN-DELAY 10\r\nPIPELINING\r\nUIDL\r\n.\r\n");
	fflush(stdout);
}
