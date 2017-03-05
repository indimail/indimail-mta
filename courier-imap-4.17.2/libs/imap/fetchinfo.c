/*
** Copyright 1998 - 1999 Double Precision, Inc.
** See COPYING for distribution information.
*/

#ifndef	HAVE_CONFIG_H
#include	"config.h"
#endif

#include    <stdio.h>
#include    <stdlib.h>
#include    <string.h>
#include    <ctype.h>
#include    <sys/types.h>

#include	"imaptoken.h"
#include	"imapwrite.h"
#include	"fetchinfo.h"


/* This file contains functions to parse a FETCH attribute list */

static struct fetchinfo *alloc_headerlist(int);
static char *good_section(char *);

struct fetchinfo *fetchinfo_alloc(int oneonly)
{
struct fetchinfo *list, **listtail, *p;
struct imaptoken *tok;

	list=0;
	listtail= &list;

	while ((tok=currenttoken())->tokentype == IT_ATOM)
	{
		if (oneonly && list)	break;
		*listtail=p=(struct fetchinfo *)malloc(sizeof(*list));
		if (!p)	write_error_exit(0);
		p->next=0;
		p->name=my_strdup(tok->tokenbuf);
		p->bodysection=0;
		p->bodysublist=0;
		p->ispartial=0;
		listtail= &p->next;

		if (strcmp(p->name, "ALL") == 0 ||
			strcmp(p->name, "BODYSTRUCTURE") == 0 ||
			strcmp(p->name, "ENVELOPE") == 0 ||
			strcmp(p->name, "FLAGS") == 0 ||
			strcmp(p->name, "FAST") == 0 ||
			strcmp(p->name, "FULL") == 0 ||
			strcmp(p->name, "INTERNALDATE") == 0 ||
			strcmp(p->name, "RFC822") == 0 ||
			strcmp(p->name, "RFC822.HEADER") == 0 ||
			strcmp(p->name, "RFC822.SIZE") == 0 ||
			strcmp(p->name, "RFC822.TEXT") == 0 ||
			strcmp(p->name, "UID") == 0)
		{
			nexttoken();
			continue;
		}
		if (strcmp(p->name, "BODY") && strcmp(p->name, "BODY.PEEK"))
			break;
		if (nexttoken()->tokentype != IT_LBRACKET)	continue;

		/* Parse BODY[ ... ] */

		if ((tok=nexttoken())->tokentype != IT_RBRACKET)
		{
		char	*s;

			if ( (tok->tokentype != IT_ATOM &&
				tok->tokentype != IT_NUMBER) ||
				!(s=good_section(tok->tokenbuf)))
			{
				fetchinfo_free(list);
				return (0);
			}
			p->bodysection=my_strdup(tok->tokenbuf);

			if (strcmp(s, "HEADER.FIELDS") == 0 ||
				strcmp(s, "HEADER.FIELDS.NOT") == 0)
			{
				/* Must be followed by header list */

				if ((tok=nexttoken_nouc())->tokentype
						!= IT_LPAREN)
				{
					p->bodysublist=alloc_headerlist(1);
					if (p->bodysublist == 0)
					{
						fetchinfo_free(list);
						return (0);
					}
				}
				else
				{
					nexttoken_nouc();
					p->bodysublist=alloc_headerlist(0);
					if ( currenttoken()->tokentype
						!= IT_RPAREN)
					{
						fetchinfo_free(list);
						return (0);
					}
				}
			}
			tok=nexttoken();
			
		}
		else p->bodysection=my_strdup("");

		if (tok->tokentype != IT_RBRACKET)
		{
			fetchinfo_free(list);
			return (0);
		}
		tok=nexttoken();
		if (tok->tokentype == IT_ATOM && tok->tokenbuf[0] == '<' &&
			tok->tokenbuf[strlen(tok->tokenbuf)-1] == '>' &&
			(p->ispartial=sscanf(tok->tokenbuf+1, "%lu.%lu",
				&p->partialstart, &p->partialend)) > 0)
			nexttoken();
	}
	return (list);
}

/* Just validate that the syntax of the attribute is correct */

static char *good_section(char *p)
{
int	has_mime=0;

	while (isdigit((int)(unsigned char)*p))
	{
		if (*p == '0')	return (0);
		has_mime=1;
		while (isdigit((int)(unsigned char)*p))	++p;
		if (*p == '\0')
			return (p);

		if (*p != '.')	return (0);
		++p;
	}

	if (strcmp(p, "HEADER") == 0 ||
		strcmp(p, "HEADER.FIELDS") == 0 ||
		strcmp(p, "HEADER.FIELDS.NOT") == 0 ||
		strcmp(p, "TEXT") == 0)
		return (p);

	if (strcmp(p, "MIME") == 0 && has_mime)	return (p);
	return (0);
}

/* Header list looks like atoms to me */

static struct fetchinfo *alloc_headerlist(int oneonly)
{
struct fetchinfo *list, **listtail, *p;
struct imaptoken *tok;

	list=0;
	listtail= &list;

	while ((tok=currenttoken())->tokentype == IT_ATOM ||
	       tok->tokentype == IT_QUOTED_STRING ||
	       tok->tokentype == IT_NUMBER)
	{
		*listtail=p=(struct fetchinfo *)malloc(sizeof(*list));
		if (!p)	write_error_exit(0);
		p->next=0;
		p->name=my_strdup(tok->tokenbuf);
		p->bodysublist=0;
		p->bodysection=0;
		listtail= &p->next;
		if (oneonly)
			break;
		nexttoken_nouc();
	}
	return (list);
}

void fetchinfo_free(struct fetchinfo *p)
{
struct fetchinfo *q;

	while (p)
	{
		if (p->bodysublist)	fetchinfo_free(p->bodysublist);
		q=p->next;
		if (p->name)	free(p->name);
		if (p->bodysection) free(p->bodysection);
		free(p);
		p=q;
	}
}
