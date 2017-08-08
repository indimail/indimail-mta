/*
** Copyright 1998 - 2010 Double Precision, Inc.
** See COPYING for distribution information.
*/

#if	HAVE_CONFIG_H
#include	"config.h"
#endif

#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<errno.h>
#include	"unicode/courier-unicode.h"
#include	"searchinfo.h"
#include	"imapwrite.h"
#include	"imaptoken.h"


struct searchinfo *alloc_search(struct searchinfo **head)
{
struct searchinfo *si=(struct searchinfo *)malloc(sizeof(**head));

	if (si == 0)	write_error_exit(0);
	memset(si, 0, sizeof(*si));
	maildir_search_init(&si->sei);
	si->next= *head;
	*head=si;
	return (si);
}

void free_search(struct searchinfo *si)
{
struct searchinfo *p;

	while (si)
	{
		p=si->next;
		if (si->as)	free(si->as);
		if (si->bs)	free(si->bs);
		if (si->cs)	free(si->cs);

		maildir_search_destroy(&si->sei);

		free(si);
		si=p;
	}
}

static struct searchinfo *alloc_search_andlist(struct searchinfo **);
static struct searchinfo *alloc_search_notkey(struct searchinfo **);
static struct searchinfo *alloc_search_key(struct searchinfo **);

struct searchinfo *alloc_parsesearch(struct searchinfo **head)
{
struct	searchinfo *si;

	*head=0;
	if ((si=alloc_search_andlist(head)) == 0)
	{
		free_search(*head);
		return (0);
	}
	return (si);
}

struct searchinfo *alloc_searchextra(struct searchinfo *top,
	struct searchinfo **head, search_type t)
{
	struct searchinfo *si;

	if (t == search_references1)
	{
		/* Automatically add third and second dummy node */

		top=alloc_searchextra(top, head, search_references4);
		top=alloc_searchextra(top, head, search_references3);
		top=alloc_searchextra(top, head, search_references2);
	}
	si=alloc_search(head);
	si->type=t;
	si->a=top;
	return (si);
}

static struct searchinfo *alloc_search_andlist(struct searchinfo **head)
{
struct searchinfo *si, *a, *b;
struct imaptoken *t;

	si=alloc_search_notkey(head);
	if (!si)	return (0);
	while ((t=currenttoken())->tokentype != IT_RPAREN && t->tokentype !=
		IT_EOL)
	{
		if ((a=alloc_search_notkey(head)) == 0)	return (0);
		b=alloc_search(head);
		b->type=search_and;
		b->a=si;
		b->b=a;
		si=b;
	}
	return (si);
}

static struct searchinfo *alloc_search_notkey(struct searchinfo **head)
{
struct imaptoken *t=currenttoken();

	if (t->tokentype == IT_ATOM && strcmp(t->tokenbuf, "NOT") == 0)
	{
	struct searchinfo *si=alloc_search(head);

		si->type=search_not;
		nexttoken();
		if ((si->a=alloc_search_key(head)) == 0)
			return (0);
		return (si);
	}
	return (alloc_search_key(head));
}

static struct searchinfo *alloc_search_key(struct searchinfo **head)
{
struct imaptoken *t=currenttoken();
struct searchinfo *si;
const char *keyword;

	if (t->tokentype == IT_LPAREN)
	{
		nexttoken();
		if ((si=alloc_search_andlist(head)) == 0 ||
			currenttoken()->tokentype != IT_RPAREN)
			return (0);
		nexttoken();
		return (si);
	}

	if (t->tokentype != IT_ATOM && t->tokentype != IT_NUMBER)
		return (0);

	keyword=t->tokenbuf;

	if (strcmp(keyword, "ALL") == 0)
	{
	struct searchinfo *si;

		(si=alloc_search(head))->type=search_all;
		nexttoken();
		return (si);
	}

	if (strcmp(keyword, "OR") == 0)
	{
	struct searchinfo *si;

		si=alloc_search(head);
		si->type=search_or;
		nexttoken();
		if ((si->a=alloc_search_notkey(head)) == 0 ||
			(si->b=alloc_search_notkey(head)) == 0)	return (0);
		return (si);
	}

	if (strcmp(keyword, "HEADER") == 0)
	{
	struct imaptoken *t;
	struct searchinfo *si;

		si=alloc_search(head);
		si->type=search_header;
		t=nexttoken_okbracket();
		if (t->tokentype != IT_ATOM &&
		    t->tokentype != IT_NUMBER &&
		    t->tokentype != IT_QUOTED_STRING)
			return (0);
		si->cs=strdup(t->tokenbuf);
		if (!si->cs)
			write_error_exit(0);
		t=nexttoken_okbracket();
		if (t->tokentype != IT_ATOM &&
		    t->tokentype != IT_NUMBER &&
		    t->tokentype != IT_QUOTED_STRING)
			return (0);
		si->as=my_strdup(t->tokenbuf);
		nexttoken();
		return (si);
	}

	if (strcmp(keyword, "BCC") == 0 ||
		strcmp(keyword, "CC") == 0 ||
		strcmp(keyword, "FROM") == 0 ||
		strcmp(keyword, "TO") == 0 ||
		strcmp(keyword, "SUBJECT") == 0)
	{
	struct imaptoken *t;
	struct searchinfo *si;

		si=alloc_search(head);
		si->type=search_header;
		si->cs=my_strdup(keyword);
		t=nexttoken_okbracket();
		if (t->tokentype != IT_ATOM &&
		    t->tokentype != IT_NUMBER &&
		    t->tokentype != IT_QUOTED_STRING)
			return (0);
		si->as=my_strdup(t->tokenbuf);
		nexttoken();
		return (si);
	}

	if (strcmp(keyword, "BEFORE") == 0)
	{
	struct imaptoken *t;
	struct searchinfo *si;

		si=alloc_search(head);
		si->type=search_before;
		t=nexttoken();
		if (t->tokentype != IT_ATOM &&
		    t->tokentype != IT_NUMBER &&
		    t->tokentype != IT_QUOTED_STRING)
			return (0);
		si->as=my_strdup(t->tokenbuf);
		nexttoken();
		return (si);
	}

	if (strcmp(keyword, "BODY") == 0)
	{
	struct imaptoken *t;
	struct searchinfo *si;

		si=alloc_search(head);
		si->type=search_body;
		t=nexttoken_okbracket();
		if (t->tokentype != IT_ATOM &&
		    t->tokentype != IT_NUMBER &&
		    t->tokentype != IT_QUOTED_STRING)
			return (0);
		si->as=my_strdup(t->tokenbuf);
		nexttoken();
		return (si);
	}
	if (strcmp(keyword, "LARGER") == 0)
	{
	struct imaptoken *t;
	struct searchinfo *si;

		si=alloc_search(head);
		si->type=search_larger;
		t=nexttoken();
		if (t->tokentype != IT_NUMBER)
			return (0);
		si->as=my_strdup(t->tokenbuf);
		nexttoken();
		return (si);
	}

	if (strcmp(keyword, "ON") == 0)
	{
	struct imaptoken *t;
	struct searchinfo *si;

		si=alloc_search(head);
		si->type=search_on;
		t=nexttoken();
		if (t->tokentype != IT_ATOM &&
		    t->tokentype != IT_NUMBER &&
		    t->tokentype != IT_QUOTED_STRING)
			return (0);
		si->as=my_strdup(t->tokenbuf);
		nexttoken();
		return (si);
	}

	if (strcmp(keyword, "SENTBEFORE") == 0)
	{
	struct imaptoken *t;
	struct searchinfo *si;

		si=alloc_search(head);
		si->type=search_sentbefore;
		t=nexttoken();
		if (t->tokentype != IT_ATOM &&
		    t->tokentype != IT_NUMBER &&
		    t->tokentype != IT_QUOTED_STRING)
			return (0);
		si->as=my_strdup(t->tokenbuf);
		nexttoken();
		return (si);
	}

	if (strcmp(keyword, "SENTON") == 0)
	{
	struct imaptoken *t;
	struct searchinfo *si;

		si=alloc_search(head);
		si->type=search_senton;
		t=nexttoken();
		if (t->tokentype != IT_ATOM &&
		    t->tokentype != IT_NUMBER &&
		    t->tokentype != IT_QUOTED_STRING)
			return (0);
		si->as=my_strdup(keyword);
		nexttoken();
		return (si);
	}

	if (strcmp(keyword, "SENTSINCE") == 0)
	{
	struct imaptoken *t;
	struct searchinfo *si;

		si=alloc_search(head);
		si->type=search_sentsince;
		t=nexttoken();
		if (t->tokentype != IT_ATOM &&
		    t->tokentype != IT_NUMBER &&
		    t->tokentype != IT_QUOTED_STRING)
			return (0);
		si->as=my_strdup(t->tokenbuf);
		nexttoken();
		return (si);
	}

	if (strcmp(keyword, "SINCE") == 0)
	{
	struct imaptoken *t;
	struct searchinfo *si;

		si=alloc_search(head);
		si->type=search_since;
		t=nexttoken();
		if (t->tokentype != IT_ATOM &&
		    t->tokentype != IT_NUMBER &&
		    t->tokentype != IT_QUOTED_STRING)
			return (0);
		si->as=my_strdup(t->tokenbuf);
		nexttoken();
		return (si);
	}

	if (strcmp(keyword, "SMALLER") == 0)
	{
	struct imaptoken *t;
	struct searchinfo *si;

		si=alloc_search(head);
		si->type=search_smaller;
		t=nexttoken();
		if (t->tokentype != IT_NUMBER)
			return (0);
		si->as=my_strdup(t->tokenbuf);
		nexttoken();
		return (si);
	}

	if (strcmp(keyword, "TEXT") == 0)
	{
	struct imaptoken *t;
	struct searchinfo *si;

		si=alloc_search(head);
		si->type=search_text;
		t=nexttoken_okbracket();
		if (t->tokentype != IT_ATOM &&
		    t->tokentype != IT_NUMBER &&
		    t->tokentype != IT_QUOTED_STRING)
			return (0);
		si->as=my_strdup(t->tokenbuf);
		nexttoken();
		return (si);
	}

	if (strcmp(keyword, "UID") == 0)
	{
	struct searchinfo *si;
	struct imaptoken *t;

		si=alloc_search(head);
		si->type=search_uid;
		t=nexttoken();
		if (!ismsgset(t))
			return (0);
		si->as=my_strdup(t->tokenbuf);
		nexttoken();
		return (si);
	}

	if (strcmp(keyword, "KEYWORD") == 0
		|| strcmp(keyword, "UNKEYWORD") == 0)
	{
	int	isnot= *keyword == 'U';
	struct imaptoken *t;
	struct searchinfo *si;

		si=alloc_search(head);
		si->type=search_msgkeyword;
		t=nexttoken_okbracket();
		if (t->tokentype != IT_ATOM &&
		    t->tokentype != IT_NUMBER &&
		    t->tokentype != IT_QUOTED_STRING)
			return (0);
		si->as=my_strdup(t->tokenbuf);
		nexttoken();

		if (isnot)
		{
		struct searchinfo *si2=alloc_search(head);

			si2->type=search_not;
			si2->a=si;
			si=si2;
		}
		return (si);
	}
	if (strcmp(keyword, "ANSWERED") == 0 ||
		strcmp(keyword, "DELETED") == 0 ||
		strcmp(keyword, "DRAFT") == 0 ||
		strcmp(keyword, "FLAGGED") == 0 ||
		strcmp(keyword, "RECENT") == 0 ||
		strcmp(keyword, "SEEN") == 0)
	{
	struct searchinfo *si;

		si=alloc_search(head);
		si->type=search_msgflag;
		if ((si->as=malloc(strlen(keyword)+2)) == 0)
			write_error_exit(0);
		si->as[0]='\\';
		strcpy(si->as+1, keyword);
		nexttoken();
		return (si);
	}

	if (strcmp(keyword, "UNANSWERED") == 0 ||
		strcmp(keyword, "UNDELETED") == 0 ||
		strcmp(keyword, "UNDRAFT") == 0 ||
		strcmp(keyword, "UNFLAGGED") == 0 ||
		strcmp(keyword, "UNSEEN") == 0)
	{
	struct searchinfo *si;
	struct searchinfo *si2;

		si=alloc_search(head);
		si->type=search_msgflag;
		if ((si->as=malloc(strlen(keyword))) == 0)
			write_error_exit(0);
		si->as[0]='\\';
		strcpy(si->as+1, keyword+2);
		nexttoken();

		si2=alloc_search(head);
		si2->type=search_not;
		si2->a=si;
		return (si2);
	}

	if (strcmp(keyword, "NEW") == 0)
	{
	struct searchinfo *si, *si2;

		si=alloc_search(head);
		si->type=search_and;
		si2=si->a=alloc_search(head);
		si2->type=search_msgflag;
		si2->as=my_strdup("\\RECENT");
		si2=si->b=alloc_search(head);
		si2->type=search_not;
		si2=si2->a=alloc_search(head);
		si2->type=search_msgflag;
		si2->as=my_strdup("\\SEEN");
		nexttoken();
		return (si);
	}

	if (strcmp(keyword, "OLD") == 0)
	{
	struct searchinfo *si, *si2;

		si=alloc_search(head);
		si->type=search_not;
		si2=si->a=alloc_search(head);
		si2->type=search_msgflag;
		si2->as=my_strdup("\\RECENT");
		nexttoken();
		return (si);
	}

	if (ismsgset(t))
	{
		si=alloc_search(head);
		si->type=search_messageset;
		si->as=my_strdup(t->tokenbuf);
		nexttoken();
		return (si);
	}

	return (0);
}

/*
** We are about to search in charset 'textcharset'.  Make sure that all
** search_text nodes in the search string are in that character set.
*/

void search_set_charset_conv(struct searchinfo *si, const char *charset)
{
	for (; si; si=si->next)
	{
		if (si->type != search_text && si->type != search_body
		    && si->type != search_header)
			continue;
		if (si->value > 0)
			continue; /* Already found, no need to do this again */

		if (maildir_search_start_str_chset(&si->sei,
						   si->as ? si->as:"",
						   charset))
		{
			si->value=0;
			continue;
		}
	}
}


#if 0

void debug_search(struct searchinfo *si)
{
	if (!si)	return;

	switch (si->type) {
	case search_messageset:
		writes("MESSAGE SET: ");
		writes(si->as);
		return;
	case search_all:
		writes("ALL");
		return;
	case search_msgflag:
		writes("FLAG \"");
		writeqs(si->as);
		writes("\"");
		return;
	case search_msgkeyword:
		writes("KEYWORD \"");
		writeqs(si->as);
		writes("\"");
		return;
	case search_not:
		writes("NOT (");
		debug_search(si->a);
		writes(")");
		return;
	case search_and:
		writes("AND (");
		debug_search(si->a);
		writes(", ");
		debug_search(si->b);
		writes(")");
		return;
	case search_or:
		writes("OR (");
		debug_search(si->a);
		writes(", ");
		debug_search(si->b);
		writes(")");
		return;
	case search_header:
		writes("HEADER \"");
		writeqs(si->cs);
		writes("\" \"");
		writeqs(si->bs);
		writes("\"");
		return;
	case search_before:
		writes("BEFORE \"");
		writeqs(si->as);
		writes("\"");
		return;
	case search_body:
		writes("BODY \"");
		writeqs(si->as);
		writes("\"");
		return;
	case search_larger:
		writes("LARGER \"");
		writeqs(si->as);
		writes("\"");
		return;
	case search_on:
		writes("ON \"");
		writeqs(si->as);
		writes("\"");
		return;
	case search_sentbefore:
		writes("SENTBEFORE \"");
		writeqs(si->as);
		writes("\"");
		return;
	case search_senton:
		writes("SENTON \"");
		writeqs(si->as);
		writes("\"");
		return;
	case search_sentsince:
		writes("SENTSINCE \"");
		writeqs(si->as);
		writes("\"");
		return;
	case search_since:
		writes("SINCE \"");
		writeqs(si->as);
		writes("\"");
		return;
	case search_smaller:
		writes("SMALLER \"");
		writeqs(si->as);
		writes("\"");
		return;
	case search_text:
		writes("TEXT \"");
		writeqs(si->as);
		writes("\"");
		return;
	case search_uid:
		writes("UID \"");
		writeqs(si->as);
		writes("\"");
		return;
	case search_orderedsubj:
		writes("ORDEREDSUBJ ");
		debug_search(si->a);
		return;
	case search_references1:
		writes("REFERENCES[References/In-Reply-To]=");
		writeqs(si->as ? si->as:"");
		writes("/");
		writeqs(si->bs ? si->bs:"");
		writes(" ");
		debug_search(si->a);
		return;
	case search_references2:
		writes("REFERENCES[Date:]=");
		writeqs(si->as ? si->as:"");
		writes(" ");
		debug_search(si->a);
		return;
	case search_references3:
		writes("REFERENCES[Subject]=");
		writeqs(si->as ? si->as:"");
		writes(" ");
		debug_search(si->a);
		return;
	case search_references4:
		writes("REFERENCES[Message-ID]=");
		writeqs(si->as ? si->as:"");
		writes(" ");
		debug_search(si->a);
		return;
	case search_arrival:
		writes("ARRIVAL");
		return;
	case search_cc:
		writes("CC");
		return;
	case search_date:
		writes("DATE");
		return;
	case search_from:
		writes("FROM");
		return;
	case search_reverse:
		writes("REVERSE");
		return;
	case search_size:
		writes("SIZE");
		return;
	case search_to:
		writes("TO");
		return;
	}
}

#endif
