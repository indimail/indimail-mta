/*
** Copyright 1998 - 2001 Double Precision, Inc.
** See COPYING for distribution information.
*/

#if	HAVE_CONFIG_H
#include	"config.h"
#endif
#include	"imaptoken.h"
#include	"imapwrite.h"
#include	"rfc822/rfc822.h"
#include	"rfc2045/rfc2045.h"
#include	<stdio.h>
#include	<ctype.h>
#include	<stdlib.h>
#include	<string.h>


extern void msgenvelope(void (*)(const char *, size_t),
		FILE *, struct rfc2045 *);

extern void msgappends(void (*)(const char *, size_t), const char *, size_t);

static void do_param_list(void (*writefunc)(const char *, size_t),
	struct rfc2045attr *a)
{
int	flag;
char	*p;

	flag=0;
	p="(";
	for (; a; a=a->next)
	{
		(*writefunc)(p, strlen(p));
		(*writefunc)("\"", 1);
		if (a->name)
			msgappends(writefunc, a->name, strlen(a->name));
		(*writefunc)("\" \"", 3);
		if (a->value)
		{
#if	IMAP_CLIENT_BUGS

		/* NETSCAPE */

		char *u, *v, *w;

			u=strdup(a->value);
			if (!u)	write_error_exit(0);
			strcpy(u, a->value);
			for (v=w=u; *v; v++)
				if (*v != '\\')	*w++ = *v;
			*w=0;
			msgappends(writefunc, u, strlen(u));
			free(u);

#else
			msgappends(writefunc, a->value, strlen(a->value));
#endif
		}
		(*writefunc)("\"", 1);
		flag=1;
		p=" ";
	}
	if (flag)
		(*writefunc)(")", 1);
	else
		(*writefunc)("NIL", 3);
}

static void contentstr( void (*writefunc)(const char *, size_t), const char *s)
{
	if (!s || !*s)
	{
		(*writefunc)("NIL", 3);
		return;
	}

	(*writefunc)("\"", 1);
	msgappends(writefunc, s, strlen(s));
	(*writefunc)("\"", 1);
}


static void do_disposition(
	void (*writefunc)(const char *, size_t), const char *disposition_s,
	struct rfc2045attr *disposition_a)
{
	if ( (disposition_s == 0 || *disposition_s == 0) &&
		disposition_a == 0)
	{
		(*writefunc)("NIL", 3);
		return;
	}
	(*writefunc)("(", 1);

	if (disposition_s && *disposition_s)
	{
		(*writefunc)("\"", 1);
		msgappends(writefunc, disposition_s,
			strlen(disposition_s));
		(*writefunc)("\"", 1);
	}
	else
		(*writefunc)("\"\"", 2);

	(*writefunc)(" ", 1);
	do_param_list(writefunc, disposition_a);
	(*writefunc)(")", 1);
}

void msgbodystructure( void (*writefunc)(const char *, size_t), int dox,
	FILE *fp, struct rfc2045 *mimep)
{
const char *content_type_s;
const char *content_transfer_encoding_s;
const char *charset_s;
off_t start_pos, end_pos, start_body;
off_t nlines, nbodylines;
const char *disposition_s;

char	*p, *q;

	rfc2045_mimeinfo(mimep, &content_type_s, &content_transfer_encoding_s,
		&charset_s);
	rfc2045_mimepos(mimep, &start_pos, &end_pos, &start_body,
		&nlines, &nbodylines);

	disposition_s=mimep->content_disposition;

	(*writefunc)("(", 1);

	if (mimep->firstpart && mimep->firstpart->isdummy &&
		mimep->firstpart->next)
		/* MULTIPART */
	{
	struct rfc2045	*childp;

		for (childp=mimep->firstpart; (childp=childp->next) != 0; )
			msgbodystructure(writefunc, dox, fp, childp);

		(*writefunc)(" \"", 2);
		p=strchr(content_type_s, '/');
		if (p)
			msgappends(writefunc, p+1, strlen(p+1));
		(*writefunc)("\"", 1);

		if (dox)
		{
			(*writefunc)(" ", 1);
			do_param_list(writefunc, mimep->content_type_attr);

			(*writefunc)(" ", 1);
			do_disposition(writefunc, disposition_s,
				mimep->content_disposition_attr);

			(*writefunc)(" ", 1);
			contentstr(writefunc, rfc2045_content_language(mimep));
		}
	}
	else
	{
	char	*mybuf;
	char	buf[40];
	const	char *cp;

		mybuf=my_strdup(content_type_s);
		q=strtok(mybuf, " /");
		(*writefunc)("\"", 1);
		if (q)
			msgappends(writefunc, q, strlen(q));
		(*writefunc)("\" \"", 3);
		if (q)	q=strtok(0, " /");
		if (q)
			msgappends(writefunc, q, strlen(q));
		free(mybuf);
		(*writefunc)("\" ", 2);

		do_param_list(writefunc, mimep->content_type_attr);

		(*writefunc)(" ", 1);
		cp=rfc2045_content_id(mimep);
		if (!cp || !*cp)
			contentstr(writefunc, cp);
		else
		{
			(*writefunc)("\"<", 2);
			msgappends(writefunc, cp, strlen(cp));
			(*writefunc)(">\"", 2);
		}
		(*writefunc)(" ", 1);
		contentstr(writefunc, rfc2045_content_description(mimep));

		(*writefunc)(" \"", 2);
		msgappends(writefunc, content_transfer_encoding_s,
			strlen(content_transfer_encoding_s));
		(*writefunc)("\" ", 2);

		sprintf(buf, "%lu", (unsigned long)
			(end_pos-start_body+nbodylines));
			/* nbodylines added for CRs */
		(*writefunc)(buf, strlen(buf));

		if (
		(content_type_s[0] == 't' || content_type_s[0] == 'T') &&
		(content_type_s[1] == 'e' || content_type_s[1] == 'E') &&
		(content_type_s[2] == 'x' || content_type_s[2] == 'X') &&
		(content_type_s[3] == 't' || content_type_s[3] == 'T') &&
			(content_type_s[4] == '/' ||
			 content_type_s[4] == 0))
		{
			(*writefunc)(" ", 1);
			sprintf(buf, "%lu", (unsigned long)nbodylines);
			(*writefunc)(buf, strlen(buf));
		}

		if (mimep->firstpart && !mimep->firstpart->isdummy)
			/* message/rfc822 */
		{
			(*writefunc)(" ", 1);
			msgenvelope(writefunc, fp, mimep->firstpart);
			(*writefunc)(" ", 1);
			msgbodystructure(writefunc, dox, fp, mimep->firstpart);
			(*writefunc)(" ", 1);
			sprintf(buf, "%lu", (unsigned long)nbodylines);
			(*writefunc)(buf, strlen(buf));
		}

		if (dox)
		{
			(*writefunc)(" ", 1);
			contentstr(writefunc, rfc2045_content_md5(mimep));

			(*writefunc)(" ", 1);
			do_disposition(writefunc, disposition_s,
				mimep->content_disposition_attr);

			(*writefunc)(" NIL", 4);
				/* TODO Content-Language: */
		}
	}
	(*writefunc)(")", 1);
}
