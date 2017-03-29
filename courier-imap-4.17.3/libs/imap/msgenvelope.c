/*
** Copyright 1998 - 2009 Double Precision, Inc.
** See COPYING for distribution information.
*/

#if	HAVE_CONFIG_H
#include	"config.h"
#endif
#include	"rfc822/rfc822.h"
#include	"rfc822/rfc2047.h"
#include	"rfc2045/rfc2045.h"
#include	"imapwrite.h"
#include	<stdio.h>
#include	<ctype.h>
#include	<stdlib.h>
#include	<string.h>


#define	MAX_HEADER_SIZE	8192

static char *read_header(FILE *fp, off_t *headerpos, off_t *endhp)
{
size_t	i=0;
int	c;
static char headerbuf[MAX_HEADER_SIZE];

	while (*headerpos < *endhp)
	{
		while ((c=getc(fp)) != '\n' && c != EOF)
		{
			if (c == '\r')
				c=' ';	/* Otherwise Outlook croaks */

			if (i < sizeof(headerbuf)-1)
				headerbuf[i++]=c;
			if ( ++*headerpos >= *endhp)	break;
		}
		if (c == EOF || *headerpos >= *endhp)
		{
			*headerpos=*endhp;
			break;
		}
		if (c == '\n' && ++*headerpos >= *endhp)	break;
		c=getc(fp);
		if (c != EOF)	ungetc(c, fp);
		if (c == EOF)
		{
			*headerpos=*endhp;
			break;
		}
		if (c != ' ' && c != '\t')	break;
	}
	headerbuf[i]=0;
	if (*headerpos == *endhp && i == 0)	return (0);
	return (headerbuf);
}

void msgappends(void (*writefunc)(const char *, size_t),
		const char *s, size_t l)
{
	size_t	i,j;
	char *q=0;

	for (i=0; i<l; i++)
		if (s[i] & 0x80)	/* Illegal 8-bit header content */
		{
			char *p=malloc(l+1);

			if (!p)
				write_error_exit(0);
			if (l)
				memcpy(p, s, l);
			p[l]=0;

			/* Assume UTF-8, if not, well, GIGO */
			q=rfc2047_encode_str(p, "utf-8",
					     rfc2047_qp_allow_any);
			free(p);
			if (!q)
				write_error_exit(0);
			s=q;
			l=strlen(s);
		}

	for (i=j=0; i<l; i++)
	{
		if (s[i] == '"' || s[i] == '\\')
		{
			(*writefunc)(s+j, i-j);
			(*writefunc)("\\", 1);
			j=i;
		}
	}
	(*writefunc)(s+j, i-j);
	if (q)
		free(q);
}

static void doenvs(void (*writefunc)(const char *, size_t), char *s)
{
size_t	i,j;
char	*t=s;

	while ( s && *s && isspace((int)(unsigned char)*s))
		++s;

	for (i=j=0; s && s[i]; i++)
		if ( !isspace((int)(unsigned char)s[i]))
			j=i+1;

	if (j == 0)
		(*writefunc)("NIL", 3);
	else
	{
		(*writefunc)("\"", 1);
		msgappends(writefunc, s, j);
		(*writefunc)("\"", 1);
	}

	if (t)	free(t);
}

static void doenva(void (*writefunc)(const char *, size_t), char *s)
{
struct rfc822t *t;
struct rfc822a *a;
int	i;
char	*q, *r;

	if (!s)
	{
		(*writefunc)("NIL", 3);
		return;
	}

	t=rfc822t_alloc_new(s, 0, 0);
	if (!t)
	{
		perror("malloc");
		exit(0);
	}
	a=rfc822a_alloc(t);
	if (!a)
	{
		perror("malloc");
		exit(1);
	}

	if (a->naddrs == 0)
	{
		rfc822a_free(a);
		rfc822t_free(t);
		free(s);
		(*writefunc)("NIL", 3);
		return;
	}

	(*writefunc)("(", 1);
	for (i=0; i<a->naddrs; i++)
	{
		(*writefunc)("(", 1);

		q=rfc822_display_name_tobuf(a, i, NULL);

		if (!q)
		{
			perror("malloc");
			exit(1);
		}
		if (a->addrs[i].tokens == 0)
		{
			if (strcmp(q, ";") == 0)
			{
				(*writefunc)("NIL NIL NIL NIL)", 16);
				free(q);
				continue;
			}
			r=strrchr(q, ':');
			if (r && r[1] == 0)	*r=0;

			(*writefunc)("NIL NIL \"", 9);
			msgappends(writefunc, q, strlen(q));
			(*writefunc)("\" NIL)", 6);
			free(q);
			continue;
		}

		if (a->addrs[i].name == 0)
			*q=0;
		/* rfc822_display_name_tobuf() defaults to addr, ignore. */

		doenvs(writefunc, q);
		(*writefunc)(" NIL \"", 6);	/* TODO @domain list */
		q=rfc822_gettok(a->addrs[i].tokens);
		if (!q)
		{
			perror("malloc");
			exit(1);
		}
		r=strrchr(q, '@');
		if (r)	*r++=0;
		msgappends(writefunc, q, strlen(q));
		(*writefunc)("\" \"", 3);
		if (r)
			msgappends(writefunc, r, strlen(r));
		(*writefunc)("\")", 2);
		free(q);
	}
	(*writefunc)(")", 1);
	rfc822a_free(a);
	rfc822t_free(t);
	free(s);
}

void msgenvelope(void (*writefunc)(const char *, size_t),
		FILE *fp, struct rfc2045 *mimep)
{
char	*date=0, *subject=0;
char	*from=0, *sender=0, *replyto=0, *to=0, *cc=0, *bcc=0;
char	*inreplyto=0, *msgid=0;

off_t start_pos, end_pos, start_body;
off_t nlines, nbodylines;

char	*p, *q, *r;

	rfc2045_mimepos(mimep, &start_pos, &end_pos, &start_body,
		&nlines, &nbodylines);

	if (fseek(fp, start_pos, SEEK_SET) < 0)
	{
		perror("fseek");
		exit(0);
	}

	while ((p=read_header(fp, &start_pos, &start_body)) != 0)
	{
	char	**hdrp=0;
	size_t	oldl, newl;
	size_t	c;

		if ((q=strchr(p, ':')) != 0)
			*q++=0;
		for (r=p; *r; r++)
			*r=tolower((int)(unsigned char)*r);
		if (strcmp(p, "date") == 0)	hdrp= &date;
		if (strcmp(p, "subject") == 0)	hdrp= &subject;
		if (strcmp(p, "from") == 0)	hdrp= &from;
		if (strcmp(p, "sender") == 0)	hdrp= &sender;
		if (strcmp(p, "reply-to") == 0)	hdrp= &replyto;
		if (strcmp(p, "to") == 0)	hdrp= &to;
		if (strcmp(p, "cc") == 0)	hdrp= &cc;
		if (strcmp(p, "bcc") == 0)	hdrp= &bcc;
		if (strcmp(p, "in-reply-to") == 0) hdrp= &inreplyto;
		if (strcmp(p, "message-id") == 0) hdrp= &msgid;
		if (!hdrp)	continue;
		if (!q)	q="";
		oldl= *hdrp ? strlen(*hdrp):0;
		newl= strlen(q);
		c=oldl+newl+1;
		if (c > 8192)	c=8192;
		r= (char *)(*hdrp ? realloc(*hdrp, c+1):malloc(c+1));
		if (!r)
		{
			perror("malloc");
			exit(1);
		}
		if (oldl && oldl < c)
			r[oldl++]=',';
		newl=c-oldl;
		if (newl)	memcpy(r+oldl, q, newl);
		r[oldl+newl]=0;
		*hdrp= r;
	}

#if 1
	if (!replyto)
		replyto=strdup(from ? from:"");
	if (!sender)
		sender=strdup(from ? from:"");

	if (!replyto || !sender)
	{
		perror("malloc");
		exit(1);
	}
#endif

	(*writefunc)("(", 1);
	doenvs(writefunc, date);
	(*writefunc)(" ", 1);
	doenvs(writefunc, subject);
	(*writefunc)(" ", 1);
	doenva(writefunc, from);
	(*writefunc)(" ", 1);
	doenva(writefunc, sender);
	(*writefunc)(" ", 1);
	doenva(writefunc, replyto);
	(*writefunc)(" ", 1);
	doenva(writefunc, to);
	(*writefunc)(" ", 1);
	doenva(writefunc, cc);
	(*writefunc)(" ", 1);
	doenva(writefunc, bcc);
	(*writefunc)(" ", 1);
	doenvs(writefunc, inreplyto);
	(*writefunc)(" ", 1);
	doenvs(writefunc, msgid);
	(*writefunc)(")", 1);
}
