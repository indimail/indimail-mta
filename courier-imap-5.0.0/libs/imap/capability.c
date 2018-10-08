/*
** Copyright 1998 - 2008 Double Precision, Inc.
** See COPYING for distribution information.
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
#include	"imapwrite.h"


static int capa_keywords=0;

extern const char *externalauth();

void initcapability()
{
	const char *p=getenv("IMAP_KEYWORDS");

	if (p) capa_keywords=atoi(p);
}

int have_starttls()
{
const char *p;

	if ((p=getenv("IMAP_STARTTLS")) == 0)	return (0);
	if (*p != 'y' && *p != 'Y')		return (0);

	p=getenv("COURIERTLS");
	if (!p || !*p)	return (0);
	if (access(p, X_OK))	return (0);
	return (1);
}


int tlsrequired()
{
const char *p=getenv("IMAP_TLS_REQUIRED");

	if (p && atoi(p))       return (1);
	return (0);
}

int keywords()
{
	return capa_keywords != 0;
}

int fastkeywords()
{
	return capa_keywords == 1;
}

int magictrash()
{
	const char *p;

	p=getenv("IMAP_MOVE_EXPUNGE_TO_TRASH");

	if (p && atoi(p))
		return 1;
	return 0;
}

const char *imap_externalauth()
{
	const char *p;

	if ((p=getenv("IMAP_TLS")) && atoi(p))
		return externalauth();

	return NULL;
}


void imapcapability()
{
	const char *p;

	if ((p=getenv("IMAP_TLS")) && atoi(p) &&
	    (p=getenv("IMAP_CAPABILITY_TLS")) && *p)
		writes(p);
	else if ((p=getenv("IMAP_CAPABILITY")) != 0 && *p)
		writes(p);
	else
		writes("IMAP4rev1");

#if SMAP
	p=getenv("SMAP_CAPABILITY");

	if (p && *p)
	{
		writes(" ");
		writes(p);

		if (keywords())
			writes(" KEYWORDS");
	}
#endif

	if ((p=getenv("IMAP_ACL")) && atoi(p))
		writes(" ACL ACL2=UNION");

	if (getenv("IMAP_ID_FIELDS"))
		writes(" ID");

	if (have_starttls())
	{
		writes(" STARTTLS");
		if (tlsrequired())
			writes(" LOGINDISABLED");
	}
	else
	{
		if (imap_externalauth())
			writes(" AUTH=EXTERNAL");
	}
			

	p=getenv("OUTBOX");

	if (p && *p)
	{
		writes(" XCOURIEROUTBOX=INBOX");
		writes(p);
	}

	if (magictrash())
		writes(" XMAGICTRASH");
}
