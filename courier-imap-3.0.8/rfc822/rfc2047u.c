/*
** Copyright 1998 - 2000 Double Precision, Inc.  See COPYING for
** distribution information.
*/


#include	<stdio.h>
#include	<ctype.h>
#include	<string.h>
#include	<stdlib.h>
#include	<errno.h>

#include	"rfc822.h"
#include	"rfc2047.h"

static const char rcsid[]="$Id: rfc2047u.c,v 1.5 2004/05/23 14:28:24 mrsam Exp $";

#if HAVE_LIBUNICODE

#include "../unicode/unicode.h"

struct decode_unicode_s {
	const struct unicode_info *mychset;
	int options;

	char *bufptr;
	size_t bufsize;
} ;

static void save_unicode_text(const char *p, int l, struct decode_unicode_s *s)
{
	if (s->bufptr)
		memcpy(s->bufptr+s->bufsize, p, l);

	s->bufsize += l;
}

static int save_unicode(const char *txt, int len, const char *chset,
			const char *lang,
			void *arg)
{
	struct decode_unicode_s *p=(struct decode_unicode_s *)arg;
	char *txts=malloc(len+1);
	char *s;
	int i;

	if (!txts)
		return (-1);
	memcpy(txts, txt, len);
	txts[len]=0;

	if (!chset)
		chset=unicode_ISO8859_1.chset;

	s=unicode_convert_fromchset(txts, chset, p->mychset);
	if (!s && p->options & RFC2047_DECODE_REPLACE)
	{
	const struct unicode_info *uiptr=unicode_find(chset);
		if (uiptr)
			s=unicode_xconvert(txts, uiptr, p->mychset);
	}
	free(txts);
	if (s)
	{
		save_unicode_text(s, strlen(s), p);
		free(s);
		return (0);
	}

	if (p->options & RFC2047_DECODE_ABORT)
	{
		errno=EINVAL;
		return (-1);
	}

	if (p->options & RFC2047_DECODE_DISCARD)
		return (0);

	if (!(p->options & RFC2047_DECODE_NOTAG))
	{
		save_unicode_text(" [", 2, p);
		save_unicode_text(chset, strlen(chset), p);
		save_unicode_text("] ", 2, p);
		if (!(p->options & RFC2047_DECODE_REPLACE))
		{
			save_unicode_text(txt, len, p);
			return (0);
		}
	}

	if (p->options & RFC2047_DECODE_REPLACE)
		for (i=0; i < strlen(txt); i++)
			save_unicode_text("?", 1, p);

	return (0);
}

char *rfc2047_decode_unicode(const char *text,
	const struct unicode_info *mychset,
	int options)
{
	struct decode_unicode_s s;
	char *p=0;

	s.mychset=mychset;
	s.options=0;

	s.bufptr=0;
	s.bufsize=1;


	if (rfc2047_decode(text, &save_unicode, &s))
		return (0);

	s.bufptr=p=malloc(s.bufsize);
	if (!s.bufptr)
		return (0);

	s.bufsize=0;
	if (rfc2047_decode(text, &save_unicode, &s))
	{
		free(p);
		return (0);
	}
	save_unicode_text("", 1, (void *)&s);
	return (p);
}


static char *do_rfc2047_decode_enhanced(const char *text, const char *mychset)
{
	const struct unicode_info *u=unicode_find(mychset);

	if (!u) u=&unicode_ISO8859_1;

	return rfc2047_decode_unicode(text, u, 0);
}

void rfc2047_print_unicode(const struct rfc822a *a,
			   const char *charset,
			   void (*print_func)(char, void *),
			   void (*print_separator)(const char *, void *),
			   void *ptr)
{
	rfc822_print_common(a, &do_rfc2047_decode_enhanced, charset,
			    print_func, print_separator, ptr);
}


#endif
