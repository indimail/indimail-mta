/*
** Copyright 1998 - 2009 Double Precision, Inc.  See COPYING for
** distribution information.
*/

#include	"rfc822.h"
#include	<stdio.h>
#include	<ctype.h>
#include	<string.h>
#include	<stdlib.h>
#include	<errno.h>
#include	"unicode/courier-unicode.h"

#include	"rfc822hdr.h"
#include	"rfc2047.h"

#if LIBIDN
#include <idna.h>
#include <stringprep.h>
#endif


static ssize_t rfc822_decode_rfc2047_atom(const char *str,
					  size_t cnt,

					  void (*callback)(const char *,
							   const char *,
							   const char *,
							   size_t,
							   void *),
					  void *ptr);

static int rfc2047_decode_unicode(const char *text,
				  const char *chset,
				  void (*callback)(const char *, size_t,
						   void *),
				  void *ptr);

struct decode_unicode_s {
	const char *mychset;

	char *bufptr;
	size_t bufsize;
} ;

static void save_unicode_text(const char *p, size_t l, void *ptr)
{
	struct decode_unicode_s *s=
		(struct decode_unicode_s *)ptr;

	if (s->bufptr)
		memcpy(s->bufptr+s->bufsize, p, l);

	s->bufsize += l;
}

struct rfc822_display_name_s {
	const char *chset;
	void (*print_func)(const char *, size_t, void *);
	void *ptr;
};

static void unknown_charset(const char *chset,
			    const char *tochset,
			    void (*print_func)(const char *, size_t, void *),
			    void *ptr)
{
	static const char unknown[]="[unknown character set: ";

	(*print_func)(unknown, sizeof(unknown)-1, ptr);
	(*print_func)(chset, strlen(chset), ptr);
	(*print_func)(" -> ", 4, ptr);
	(*print_func)(tochset, strlen(tochset), ptr);
	(*print_func)("]", 1, ptr);
}

static void rfc822_display_addr_cb(const char *chset,
				   const char *lang,
				   const char *content,
				   size_t cnt,
				   void *dummy)
{
	struct rfc822_display_name_s *s=
		(struct rfc822_display_name_s *)dummy;
	char *ptr;
	char *buf;

	buf=malloc(cnt+1);

	if (!buf)
		return;

	memcpy(buf, content, cnt);
	buf[cnt]=0;

	ptr=unicode_convert_tobuf(buf, chset, s->chset, NULL);
	free(buf);

	if (ptr)
	{
		(*s->print_func)(ptr, strlen(ptr), s->ptr);
		free(ptr);
	}
	else
	{
		unknown_charset(chset, s->chset, s->print_func, s->ptr);
		return;
	}
}

static
int rfc822_display_name_int(const struct rfc822a *rfcp, int index,
			    const char *chset,
			    void (*print_func)(const char *, size_t, void *),
			    void *ptr)
{
	struct rfc822_display_name_s s;
	const struct rfc822addr *addrs;

	struct rfc822token *i;
	int	prev_isatom=0;
	int	isatom=0;
	ssize_t rc;

	if (index < 0 || index >= rfcp->naddrs)	return 0;

	addrs=rfcp->addrs+index;

	if (!addrs->name)
		return rfc822_display_addr(rfcp, index, chset, print_func, ptr);

	if (chset == NULL)
	{
		s.chset="utf-8";
	}
	else
	{
		s.chset=chset;
	}

	s.print_func=print_func;
	s.ptr=ptr;

	for (i=addrs->name; i; i=i->next, prev_isatom=isatom)
	{
		isatom=rfc822_is_atom(i->token);
		if (isatom && prev_isatom)
			(*print_func)(" ", 1, ptr);

		if (i->token == '"' || i->token == '(')
		{
			size_t l=i->len;
			char *p, *q, *r;

			if (i->token == '(')
			{
				if (l > 2)
					l -= 2;
				else
					l=0;
			}

			p=malloc(l+1);

			if (!p)
				return -1;

			if (l)
			{
				if (i->token == '(')
				{
					memcpy(p, i->ptr+1, l);
				}
				else
				{
					memcpy(p, i->ptr, l);
				}
			}


			p[l]=0;

			for (q=r=p; *q; *r++ = *q++)
				if (*q == '\\' && q[1])
					++q;

			*r=0;

			if (chset == NULL)
			{
				(*print_func)(p, strlen(p), ptr);
			}
			else if (rfc822_display_hdrvalue("subject",
							 p, s.chset,
							 print_func,
							 NULL, ptr) < 0)
			{
				free(p);
				return -1;
			}
			free(p);
			continue;
		}

		if (i->token)
		{
			char c= (char)i->token;

			(*print_func)(&c, 1, ptr);
			continue;
		}

		rc=chset ? rfc822_decode_rfc2047_atom(i->ptr, i->len,
						      rfc822_display_addr_cb,
						      &s):0;

		if (rc < 0)
			return -1;

		if (rc == 0)
		{
			(*print_func)(i->ptr, i->len, ptr);
			continue;
		}

		if (i->next && i->next->token == 0)
		{
			rc=rfc822_decode_rfc2047_atom(i->next->ptr,
						      i->next->len,
						      NULL, NULL);

			if (rc < 0)
				return -1;

			if (rc > 0)
				isatom=0; /* Suppress the separating space */
		}
	}
	return 0;
}

int rfc822_display_name(const struct rfc822a *rfcp, int index,
			const char *chset,
			void (*print_func)(const char *, size_t, void *),
			void *ptr)
{
	const struct rfc822addr *addrs;

	if (index < 0 || index >= rfcp->naddrs)	return 0;

	addrs=rfcp->addrs+index;

	if (!addrs->tokens)
		return 0;

	return rfc822_display_name_int(rfcp, index, chset,
				       print_func, ptr);
}

char *rfc822_display_name_tobuf(const struct rfc822a *rfcp, int index,
				const char *chset)
{
	struct decode_unicode_s s;
	char *p;

	s.bufptr=0;
	s.bufsize=1;

	if (rfc822_display_name(rfcp, index, chset, save_unicode_text, &s) < 0)
		return NULL;
	s.bufptr=p=malloc(s.bufsize);
	if (!p)
		return (0);

	s.bufsize=0;
	if (rfc822_display_name(rfcp, index, chset, save_unicode_text, &s) < 0)
	{
		free(s.bufptr);
		return (0);
	}
	save_unicode_text("", 1, &s);

	return (p);
}

int rfc822_display_namelist(const struct rfc822a *rfcp,
			    const char *chset,
			    void (*print_func)(const char *, size_t, void *),
			    void *ptr)
{
	int n;

	for (n=0; n<rfcp->naddrs; n++)
	{
		if (rfcp->addrs[n].tokens)
		{
			int err=rfc822_display_name(rfcp, n, chset,
						    print_func, ptr);

			if (err < 0)
				return err;

			(*print_func)("\n", 1, ptr);
		}
	}
	return 0;
}

int rfc822_display_addr_str(const char *tok,
			    const char *chset,
			    void (*print_func)(const char *, size_t, void *),
			    void *ptr)
{
	const char *p;

	p=strchr(tok,'@');

	if (!p)
		p=tok;
	else
		++p;

	if (chset != NULL)
	{
		int err=0;
		char *utf8_ptr;

		if (p > tok)
			(*print_func)(tok, p-tok, ptr);

#if LIBIDN
		/*
		** Invalid UTF-8 can make libidn go off the deep end. Add
		** padding as a workaround.
		*/
		{
			size_t s=strlen(p)+16;
			char *cpy=malloc(s);

			if (!cpy)
				return 0;
			memset(cpy, 0, s);
			strcpy(cpy, p);

			err=idna_to_unicode_8z8z(cpy, &utf8_ptr, 0);
			free(cpy);
		}

		if (err != IDNA_SUCCESS)
			utf8_ptr=0;
#else
		utf8_ptr=0;
#endif

		if (utf8_ptr == 0)
			(*print_func)(p, strlen(p), ptr);
		else
		{
			char *q=unicode_convert_tobuf(utf8_ptr,
							"utf-8",
							chset, NULL);
			if (q)
			{
				(*print_func)(q, strlen(q), ptr);
				free(q);
			}
			else
			{
				(*print_func)(p, strlen(p), ptr);
			}
			free(utf8_ptr);
		}
	}
	else
	{
		(*print_func)(tok, strlen(tok), ptr);
	}
	return 0;
}

int rfc822_display_addr(const struct rfc822a *rfcp, int index,
			const char *chset,
			void (*print_func)(const char *, size_t, void *),
			void *ptr)
{
	const struct rfc822addr *addrs;
	char *tok;
	int rc;

	if (index < 0 || index >= rfcp->naddrs)	return 0;

	addrs=rfcp->addrs+index;

	if (!addrs->tokens)
		return 0;

	tok=rfc822_gettok(addrs->tokens);

	if (!tok)
		return 0;

	rc=rfc822_display_addr_str(tok, chset, print_func, ptr);
	free(tok);
	return rc;
}

int rfc2047_print_unicodeaddr(const struct rfc822a *a,
			      const char *charset,
			      void (*print_func)(char, void *),
			      void (*print_separator)(const char *, void *),
			      void *ptr)
{
	const char *sep=NULL;
	int n;

	for (n=0; n<a->naddrs; ++n)
	{
		struct decode_unicode_s nbuf;
		const struct rfc822addr *addrs;
		size_t i=0;
		char *cpbuf;
		int need_braces=0;

		addrs=a->addrs+n;

		nbuf.bufptr=0;
		nbuf.bufsize=1;

		if (rfc822_display_name_int(a, n, charset,
					    save_unicode_text, &nbuf) < 0)
			return -1;

		nbuf.bufptr=malloc(nbuf.bufsize);
		nbuf.bufsize=0;
		if (!nbuf.bufptr)
			return -1;

		if (rfc822_display_name_int(a, n, charset,
					    save_unicode_text, &nbuf) < 0)
		{
			free(nbuf.bufptr);
			return -1;
		}
		nbuf.bufptr[nbuf.bufsize]=0;

		if (addrs->tokens == 0)
		{
			size_t i;

			if (nbuf.bufsize == 1) /* ; */
				sep=0;

			if (sep)
				(*print_separator)(sep, ptr);

			for (i=0; i<nbuf.bufsize; ++i)
				(*print_func)(nbuf.bufptr[i], ptr);
			free(nbuf.bufptr);
			if (nbuf.bufsize > 1)
				(*print_separator)(" ", ptr);
			sep=NULL;
			continue;
		}
		if (sep)
			(*print_separator)(sep, ptr);

		if (!addrs->name)
		{
			nbuf.bufsize=0;
			nbuf.bufptr[0]=0;
		}

		for (i=0; i<nbuf.bufsize; i++)
			if (strchr(RFC822_SPECIALS, nbuf.bufptr[i]))
				break;

		cpbuf=unicode_convert_tobuf(nbuf.bufptr, "utf-8", charset,
					      NULL);

		if (!cpbuf)
		{
			const char *errmsg="\"(unknown character set)\"";

			while (*errmsg)
				(*print_func)(*errmsg++, ptr);
			need_braces=1;
		}
		else
		{
			if (i < nbuf.bufsize)
			{
				(*print_func)('"', ptr);

				for (i=0; cpbuf[i]; ++i)
				{
					if (cpbuf[i] == '\\' ||
					    cpbuf[i] == '"')
						(*print_func)('\\', ptr);
					(*print_func)(cpbuf[i], ptr);
				}
				(*print_func)('"', ptr);
				need_braces=1;
			}
                        else
                        {
                                for (i=0; cpbuf[i]; ++i)
				{
					need_braces=1;
                                        (*print_func)(cpbuf[i], ptr);
				}
                        }

			free(cpbuf);
		}
		free(nbuf.bufptr);

		if (need_braces)
		{
			(*print_func)(' ', ptr);
			(*print_func)('<', ptr);
		}

		nbuf.bufptr=0;
		nbuf.bufsize=1;

		if (rfc822_display_addr(a, n, charset,
					save_unicode_text, &nbuf) < 0)
			return -1;

		nbuf.bufptr=malloc(nbuf.bufsize);
		nbuf.bufsize=0;
		if (!nbuf.bufptr)
			return -1;

		if (rfc822_display_addr(a, n, charset,
					save_unicode_text, &nbuf) < 0)
		{
			free(nbuf.bufptr);
			return -1;
		}
		for (i=0; i<nbuf.bufsize; i++)
			(*print_func)(nbuf.bufptr[i], ptr);

		free(nbuf.bufptr);

		if (need_braces)
			(*print_func)('>', ptr);
		sep=", ";
	}

	return 0;
}

static int rfc2047_print_unicode_addrstr(const char *addrheader,
					 const char *charset,
					 void (*print_func)(char, void *),
					 void (*print_separator)(const char *, void *),
					 void (*err_func)(const char *, int, void *),
					 void *ptr)
{
	struct rfc822t *t;
	struct rfc822a *a;
	int rc;

	t=rfc822t_alloc_new(addrheader, err_func, ptr);

	if (!t)
		return -1;

	a=rfc822a_alloc(t);

	if (!a)
	{
		rfc822t_free(t);
		return -1;
	}
	rc=rfc2047_print_unicodeaddr(a, charset, print_func, print_separator,
				     ptr);
	rfc822a_free(a);
	rfc822t_free(t);
	return (rc);
}

struct rfc822_display_hdrvalue_s {

	void (*display_func)(const char *, size_t, void *);
	void *ptr;
};

static void rfc822_display_hdrvalue_print_func(char c, void *ptr)
{
	struct rfc822_display_hdrvalue_s *s=
		(struct rfc822_display_hdrvalue_s *)ptr;

	(*s->display_func)(&c, 1, s->ptr);
}

static void rfc822_display_hdrvalue_print_separator(const char *cp, void *ptr)
{
	struct rfc822_display_hdrvalue_s *s=
		(struct rfc822_display_hdrvalue_s *)ptr;

	(*s->display_func)(cp, strlen(cp), s->ptr);
	(*s->display_func)("", 0, s->ptr); /* Signal wrap point */
}

int rfc822_display_hdrvalue(const char *hdrname,
			    const char *hdrvalue,
			    const char *charset,
			    void (*display_func)(const char *, size_t,
						 void *),
			    void (*err_func)(const char *, int, void *),
			    void *ptr)
{
	struct rfc822_display_hdrvalue_s s;

	s.display_func=display_func;
	s.ptr=ptr;

	if (rfc822hdr_is_addr(hdrname))
	{
		return rfc2047_print_unicode_addrstr(hdrvalue,
						     charset,
						     rfc822_display_hdrvalue_print_func,
						     rfc822_display_hdrvalue_print_separator,
						     NULL,
						     &s);
	}

	return rfc2047_decode_unicode(hdrvalue, charset, display_func, ptr);
}

struct rfc822_display_hdrvalue_tobuf_s {
	void (*orig_err_func)(const char *, int, void *);
	void *orig_ptr;

	size_t cnt;
	char *buf;
};

static void rfc822_display_hdrvalue_tobuf_cnt(const char *ptr, size_t cnt,
					      void *s)
{
	((struct rfc822_display_hdrvalue_tobuf_s *)s)->cnt += cnt;
}

static void rfc822_display_hdrvalue_tobuf_save(const char *ptr, size_t cnt,
					       void *s)
{
	if (cnt)
		memcpy(((struct rfc822_display_hdrvalue_tobuf_s *)s)->buf,
		       ptr, cnt);

	((struct rfc822_display_hdrvalue_tobuf_s *)s)->buf += cnt;
}

static void rfc822_display_hdrvalue_tobuf_errfunc(const char *ptr, int index,
						  void *s)
{
	void (*f)(const char *, int, void *)=
		((struct rfc822_display_hdrvalue_tobuf_s *)s)->orig_err_func;

	if (f)
		f(ptr, index,
		  ((struct rfc822_display_hdrvalue_tobuf_s *)s)->orig_ptr);
}

char *rfc822_display_addr_tobuf(const struct rfc822a *rfcp, int index,
				const char *chset)
{
	struct rfc822_display_hdrvalue_tobuf_s nbuf;
	int errcode;
	char *ptr;

	nbuf.buf=0;
	nbuf.cnt=1;

	errcode=rfc822_display_addr(rfcp, index, chset,
				    rfc822_display_hdrvalue_tobuf_cnt, &nbuf);

	if (errcode < 0)
		return NULL;

	ptr=nbuf.buf=malloc(nbuf.cnt);
	nbuf.cnt=0;
	if (!ptr)
		return NULL;

	errcode=rfc822_display_addr(rfcp, index, chset,
				    rfc822_display_hdrvalue_tobuf_save, &nbuf);

	if (errcode < 0)
	{
		free(nbuf.buf);
		return NULL;
	}
	*nbuf.buf=0;
	return ptr;
}

char *rfc822_display_hdrvalue_tobuf(const char *hdrname,
				    const char *hdrvalue,
				    const char *charset,
				    void (*err_func)(const char *, int,
						     void *),
				    void *ptr)
{
	struct rfc822_display_hdrvalue_tobuf_s s;
	int errcode;
	char *bufptr;

	s.orig_err_func=err_func;
	s.orig_ptr=ptr;
	s.cnt=1;

	errcode=rfc822_display_hdrvalue(hdrname, hdrvalue, charset,
					rfc822_display_hdrvalue_tobuf_cnt,
					rfc822_display_hdrvalue_tobuf_errfunc,
					&s);

	if (errcode < 0)
		return NULL;

	bufptr=s.buf=malloc(s.cnt);

	if (!bufptr)
		return NULL;

	errcode=rfc822_display_hdrvalue(hdrname, hdrvalue, charset,
					rfc822_display_hdrvalue_tobuf_save,
					rfc822_display_hdrvalue_tobuf_errfunc,
					&s);
	if (errcode)
	{
		free(bufptr);
		return NULL;
	}
	*s.buf=0;
	return bufptr;
}

char *rfc822_display_addr_str_tobuf(const char *tok, const char *chset)
{
	struct rfc822_display_hdrvalue_tobuf_s s;
	int errcode;
	char *bufptr;

	s.cnt=1;

	errcode=rfc822_display_addr_str(tok, chset,
					rfc822_display_hdrvalue_tobuf_cnt,
					&s);

	if (errcode < 0)
		return NULL;

	bufptr=s.buf=malloc(s.cnt);

	if (!bufptr)
		return NULL;

	errcode=rfc822_display_addr_str(tok, chset,
					rfc822_display_hdrvalue_tobuf_save,
					&s);
	if (errcode < 0)
	{
		free(bufptr);
		return NULL;
	}
	*s.buf=0;
	return bufptr;
}


static const char xdigit[]="0123456789ABCDEFabcdef";

static const unsigned char decode64tab[]={
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 62,  0,  0,  0, 63,
	52, 53, 54, 55, 56, 57, 58, 59, 60, 61,  0,  0,  0, 99,  0,  0,
	 0,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
	15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,  0,  0,  0,  0,  0,
	 0, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
	41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
};

static int nyb(int c)
{
	const char	*p;
	int n;

	p=strchr(xdigit, c);

	if (!p)
		return 0;

	n=p-xdigit;

	if (n > 15)
		n -= 6;

	return n;
}

static size_t decodebase64(const char *ptr, size_t cnt,
			   char *dec_buf)
{
	size_t  i, j;
	char    a,b,c;
	size_t  k;

	i=cnt / 4;
	i=i*4;
	k=0;
	for (j=0; j<i; j += 4)
	{
		int     w=decode64tab[(int)(unsigned char)ptr[j]];
		int     x=decode64tab[(int)(unsigned char)ptr[j+1]];
		int     y=decode64tab[(int)(unsigned char)ptr[j+2]];
		int     z=decode64tab[(int)(unsigned char)ptr[j+3]];

		a= (w << 2) | (x >> 4);
		b= (x << 4) | (y >> 2);
		c= (y << 6) | z;
		dec_buf[k++]=a;
		if ( ptr[j+2] != '=')
			dec_buf[k++]=b;
		if ( ptr[j+3] != '=')
			dec_buf[k++]=c;
	}
	return (k);
}


static ssize_t rfc822_decode_rfc2047_atom(const char *str,
					  size_t cnt,

					  void (*callback)(const char *,
							   const char *,
							   const char *,
							   size_t,
							   void *),
					  void *ptr)
{
	const char *chset_str;
	const char *enc_str;
	const char *content_str;

	char *chset;
	char *lang;

	char *content;

	size_t i;
	size_t j;
	size_t k;

	size_t content_len;

	if (cnt < 2 || str[0] != '=' || str[1] != '?')
		return 0;

	chset_str=str+2;

	for (i=2; i<cnt; i++)
		if (str[i] == '?')
			break;

	if (i >= cnt)
		return 0;

	enc_str= str + ++i;

	for (; i < cnt; i++)
		if (str[i] == '?')
			break;

	if (i >= cnt)
		return 0;

	content_str= str + ++i;

	while (1)
	{
		if (cnt-i < 2)
			return 0;

		if (str[i] == '?' && str[i+1] == '=')
			break;
		++i;
	}

	for (j=0; chset_str[j] != '?'; ++j)
		;

	chset=malloc(j+1);

	if (!chset)
		return -1;

	memcpy(chset, chset_str, j);
	chset[j]=0;

	lang=strchr(chset, '*');  /* RFC 2231 */

	if (lang)
		*lang++ = 0;
	else
		lang="";

	content_len=str + i - content_str;

	content=malloc(content_len+1);

	if (!content)
	{
		free(chset);
		return -1;
	}

	switch (*enc_str) {
	case 'q':
	case 'Q':

		k=0;
		for (j=0; j<content_len; j++)
		{
			char c;

			if (content_str[j] == '=' && i-j >= 3)
			{
				content[k]=(char)(nyb(content_str[j+1])*16 +
						  nyb(content_str[j+2]));
				++k;
				j += 2;
				continue;
			}

			c=content_str[j];
			if (c == '_')
				c=' ';
			content[k]=c;
			++k;
		}
		break;

	case 'b':
	case 'B':
		k=decodebase64(content_str, content_len, content);
		break;
	default:
		free(content);
		free(chset);
		return (0);
	}

	if (callback)
		(*callback)(chset, lang, content, k, ptr);
	free(content);
	free(chset);
	return i + 2;
}

int rfc2047_decoder(const char *text,
		    void (*callback)(const char *chset,
				     const char *lang,
				     const char *content,
				     size_t cnt,
				     void *dummy),
		    void *ptr)
{
	ssize_t rc;

	while (text && *text)
	{
		size_t i;

		for (i=0; text[i]; i++)
		{
			if (text[i] == '=' && text[i+1] == '?')
				break;
		}

		if (i)
			(*callback)("utf-8", "", text, i, ptr);

		text += i;

		if (!*text)
			continue;

		rc=rfc822_decode_rfc2047_atom(text, strlen(text),
					      callback, ptr);

		if (rc < 0)
			return -1;

		if (rc == 0)
		{
			(*callback)("utf-8", "", text, 2, ptr);
			text += 2;
			continue;
		}

		text += rc;

		for (i=0; text[i]; i++)
		{
			if (strchr(" \t\r\n", text[i]) == NULL)
				break;
		}

		if (text[i] != '=' || text[i+1] != '?')
			continue;

		rc=rfc822_decode_rfc2047_atom(text+i, strlen(text+i), NULL,
					      NULL);

		if (rc < 0)
			return -1;
		if (rc > 0)
			text += i;
	}

	return 0;
}

static int rfc2047_decode_unicode(const char *text,
				  const char *chset,
				  void (*callback)(const char *, size_t,
						   void *),
				  void *ptr)
{
	struct rfc822_display_name_s s;

	s.chset=chset;
	s.print_func=callback;
	s.ptr=ptr;

	return rfc2047_decoder(text, rfc822_display_addr_cb, &s);
}
