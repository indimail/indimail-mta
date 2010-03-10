/*
** Copyright 1998 - 2009 Double Precision, Inc.  See COPYING for
** distribution information.
*/


#include	<stdio.h>
#include	<ctype.h>
#include	<string.h>
#include	<stdlib.h>
#include	<errno.h>

#include	"rfc822.h"
#include	"rfc822hdr.h"
#include	"rfc2047.h"
#include	"../unicode/unicode.h"
#if LIBIDN
#include <idna.h>
#include <stringprep.h>
#endif

static const char rcsid[]="$Id: rfc2047.c,v 1.23 2009/11/18 03:38:50 mrsam Exp $";

#define	RFC2047_ENCODE_FOLDLENGTH	76

static const char xdigit[]="0123456789ABCDEF";
static const char base64tab[]=
"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static char *a_rfc2047_encode_str(const char *str, const char *charset,
				  int isaddress);

static void rfc2047_encode_header_do(const struct rfc822a *a,
				     const char *charset,
				     void (*print_func)(char, void *),
				     void (*print_separator)(const char *,
							     void *), void *ptr)
{
	rfc822_print_common(a, &a_rfc2047_encode_str, charset,
			    print_func, print_separator, ptr);
}

static char *rfc822_encode_domain_int(const char *pfix,
				      size_t pfix_len,
				      const char *domain)
{
	char *q;

#if LIBIDN
	int err;
	char *p;

	err=idna_to_ascii_8z(domain, &p, 0);

	if (err != IDNA_SUCCESS)
	{
		errno=EINVAL;
		return NULL;
	}
#else
	char *p;

	p=strdup(domain);

	if (!p)
		return NULL;
#endif

	q=malloc(strlen(p)+pfix_len+1);

	if (!q)
	{
		free(p);
		return NULL;
	}

	if (pfix_len)
		memcpy(q, pfix, pfix_len);

	strcpy(q + pfix_len, p);
	free(p);
	return q;
}

char *rfc822_encode_domain(const char *address,
			    const char *charset)
{
	const struct unicode_info *ui=unicode_find(charset);
	const char *cp;
	char *p, *q;

	if (!ui)
	{
		errno=EINVAL;
		return NULL;
	}

	p=unicode_convert(address, ui, &unicode_UTF8);

	if (!p)
		return NULL;

	cp=strchr(p, '@');

	if (!cp)
	{
		q=rfc822_encode_domain_int("", 0, p);
		free(p);
		return q;
	}

	++cp;
	q=rfc822_encode_domain_int(p, cp-p, cp);
	free(p);
	return q;
}

static char *a_rfc2047_encode_str(const char *str, const char *charset,
				  int isaddress)
{
	size_t	l;
	char	*p;

	if (isaddress)
		return rfc822_encode_domain(str, charset);

	for (l=0; str[l]; l++)
		if (str[l] & 0x80)
			break;

	if (str[l] == 0)
	{
		size_t n;

		for (l=0; str[l]; l++)
			if (strchr(RFC822_SPECIALS, str[l]))
				break;

		if (str[l] == 0)
			return (strdup(str));

		for (n=3, l=0; str[l]; l++)
		{
			switch (str[l]) {
			case '"':
			case '\\':
				++n;
			break;
			}

			++n;
		}

		p=malloc(n);

		if (!p)
			return NULL;

		p[0]='"';

		for (n=1, l=0; str[l]; l++)
		{
			switch (str[l]) {
			case '"':
			case '\\':
				p[n++]='\\';
			break;
			}

			p[n++]=str[l];
		}
		p[n++]='"';
		p[n]=0;

		return (p);
	}

	return rfc2047_encode_str(str, charset, rfc2047_qp_allow_word);
}

static void count(char c, void *p);
static void counts2(const char *c, void *p);
static void save(char c, void *p);
static void saves2(const char *c, void *p);

char *rfc2047_encode_header_addr(const struct rfc822a *a,
			    const char *charset)
{
size_t	l;
char	*s, *p;

	l=1;
	rfc2047_encode_header_do(a, charset, &count, &counts2, &l);
	if ((s=malloc(l)) == 0)	return (0);
	p=s;
	rfc2047_encode_header_do(a, charset, &save, &saves2, &p);
	*p=0;
	return (s);
}


char *rfc2047_encode_header_tobuf(const char *name, /* Header name */
				  const char *header, /* Header's contents */
				  const char *charset)
{
	if (rfc822hdr_is_addr(name))
	{
		char *s=0;

		struct rfc822t *t;
		struct rfc822a *a;

		if ((t=rfc822t_alloc_new(header, NULL, NULL)) != 0)
		{
			if ((a=rfc822a_alloc(t)) != 0)
			{
				s=rfc2047_encode_header_addr(a, charset);
				rfc822a_free(a);
			}
			rfc822t_free(t);
		}
		return s;
	}

	return rfc2047_encode_str(header, charset, rfc2047_qp_allow_word);
}

static void count(char c, void *p)
{
	++*(size_t *)p;
}

static void counts2(const char *c, void *p)
{
	if (*c == ',')
		count(*c++, p);

	count('\n', p);
	count(' ', p);

	while (*c)	count(*c++, p);
}

static void save(char c, void *p)
{
	**(char **)p=c;
	++*(char **)p;
}

static void saves2(const char *c, void *p)
{
	if (*c == ',')
		save(*c++, p);

	save('\n', p);
	save(' ', p);

	while (*c)	save(*c++, p);
}

static int encodebase64(const char *ptr, size_t len, const char *charset,
			int (*func)(const char *, size_t, void *), void *arg,
			size_t foldlen, size_t offset)
{
	unsigned char ibuf[3];
	char obuf[4];
	int	i, rc;

	while (len)
	{
		if ((rc=(*func)("=?", 2, arg)) ||
		    (rc=(*func)(charset, strlen(charset), arg))||
		    (rc=(*func)("?B?", 3, arg)))
			return rc;
		i = offset + 2 + strlen(charset) + 3;
		offset = 0;

		while (len)
		{
			size_t n=len > 3 ? 3:len;

			ibuf[0]= ptr[0];
			if (n>1)
				ibuf[1]=ptr[1];
			else
				ibuf[1]=0;
			if (n>2)
				ibuf[2]=ptr[2];
			else
				ibuf[2]=0;
			ptr += n;
			len -= n;

			obuf[0] = base64tab[ ibuf[0]        >>2 ];
			obuf[1] = base64tab[(ibuf[0] & 0x03)<<4|ibuf[1]>>4];
			obuf[2] = base64tab[(ibuf[1] & 0x0F)<<2|ibuf[2]>>6];
			obuf[3] = base64tab[ ibuf[2] & 0x3F ];
			if (n < 2)
				obuf[2] = '=';
			if (n < 3)
				obuf[3] = '=';

			if ((rc=(*func)(obuf, 4, arg)))
				return rc;

			i += 4;
			if (foldlen && i + 2 > foldlen - 1 + 4)
				break;
		}

		if ((rc=(*func)("?=", 2, arg)))
			return rc;
		if (len)
			/*
			 * Encoded-words must be sepalated by
			 * linear-white-space.
			 */
			if ((rc=(*func)(" ", 1, arg)))
				return rc;
	}
	return 0;
}

#define ISSPACE(i) ((i)=='\t' || (i)=='\r' || (i)=='\n' || (i)==' ')
#define DOENCODE(i) (((i) & 0x80) || (i)=='"' || (i)=='=' || \
			((unsigned char)(i) < 0x20 && !ISSPACE(i)) || \
			!(*qp_allow)(i))

int rfc2047_encode_callback_base64(const char *str, const char *charset,
				   int (*qp_allow)(char),
				   int (*func)(const char *, size_t, void *),
				   void *arg)
{
int	rc;
int	dummy=-1;
size_t	i;
size_t	offset=27; /* FIXME: initial offset for line length */
const struct unicode_info *uiptr = unicode_find(charset);
unicode_char *ustr, *uptr;

	if (!str || !*str)
		return 0;

	for (i=0; str[i]; i++)
	if (DOENCODE(str[i]))
		break;
	if (str[i] == 0)
		return i? (*func)(str, strlen(str), arg): 0;
 
	/*
	 * Multibyte or stateful charsets must be encoded with care of 
	 * character boundaries.  Charsets with replaceable capability can be 
	 * encoded replacing errorneous characters.  Otherwise, output without
	 * care of character boundaries or errors.
	 */
	if (!uiptr ||
	    !(uiptr->flags & (UNICODE_MB | UNICODE_SISO)) ||
	    (!(uiptr->flags & UNICODE_REPLACEABLE) &&
	     !(ustr = (uiptr->c2u)(uiptr, str, &dummy))) ||
	    !(ustr = (uiptr->c2u)(uiptr, str, NULL)))
		return encodebase64(str, strlen(str), charset, func, arg,
				    RFC2047_ENCODE_FOLDLENGTH, offset);
 
	uptr = ustr;
	while (*uptr)
	{
	unicode_char save_uc;
	char *wstr=NULL;
	size_t i, end, j;

		if ((i = offset + 2 + strlen(charset) + 3) >
		    RFC2047_ENCODE_FOLDLENGTH - 2)
			/* Keep room for at least one character. */
			i = RFC2047_ENCODE_FOLDLENGTH - 2;
		offset = 0;

		/*
		 * Figure out where to break encoded-word.
		 * Take a small chunk of Unicode string and convert it back to
		 * the original charset.  If the result exseeds line length,
		 * try again with a shorter chunk.  
		 */
		end = 0;
		while (uptr[end] && end < (RFC2047_ENCODE_FOLDLENGTH - i) / 2)
			end++; 
			/*
			 * FIXME: Unicode character with `combining'
			 * property etc. should not be treated as
			 * separate character.
			 */
		j = end;
		while (j)
		{
			save_uc = uptr[j];
			uptr[j] = (unicode_char)0;
			wstr = (uiptr->u2c)(uiptr, uptr, &dummy);
			uptr[j] = save_uc;

			if (!wstr)
			/* Possiblly a part of one character extracted to 
			 * multiple Unicode characters (e.g. base unicode 
			 * character of one combined character).  Try on 
			 * shorter chunk.
			 */
			{
				if (j == 0)
					break;

				j--;	/* FIXME */
				continue;
			}

			if (i + ((strlen(wstr) + 3-1) / 3) * 4 + 2 >
			    RFC2047_ENCODE_FOLDLENGTH - 1)
			/*
			 * Encoded string exceeded line length.
			 * Try on shorter chunk.
			 */
			{
			size_t	k=j;

				j--;	/* FIXME */
				if (j == 0)
				/* Only one character exeeds line length.
				 * Anyway, encode it. */
				{
					j = k;
					break;
				}
				free(wstr);
				continue;
			}

			break;
		}

		if (!wstr)
		{
			end = 1;
			rc = encodebase64("?", 1, charset, func, arg, 0, 0);
		}
		else
		{
			end = j;
			rc = encodebase64(wstr, strlen(wstr),
					  charset, func, arg, 0, 0);
			free(wstr);
		}
		if (rc)
		{
			free(ustr);
			return rc;
		}
		uptr += end;

		if (*uptr)
			/*
			 * Encoded-words must be sepalated by 
			 * linear-white-space.
			 */
			if ((rc=(*func)(" ", 1, arg)))
			{
				free(ustr);
				return rc;
			}
	}
	free(ustr);
	return 0;
}

#define DOENCODEWORD(c) \
	(((c) & 0x80) || (c) == '"' || (unsigned char)(c) <= 0x20 || \
	 (c) == '_' || (c) == '=' || (c) == '?' || !(*qp_allow)(c))
 
int rfc2047_encode_callback(const char *str, const char *charset,
			    int (*qp_allow)(char),
			    int (*func)(const char *, size_t, void *),
			    void *arg)
{
int	rc;
int     maxlen;
const struct unicode_info *ci = unicode_find(charset);

	if (!str || !*str)
		return 0;

	if (ci && ci->flags & UNICODE_SISO)
		return rfc2047_encode_callback_base64(str, charset, qp_allow,
						      func, arg);
  
	/* otherwise, output quoted-printable-encoded. */

	while (*str)
	{
		size_t	i, j, n, c;

		for (i=0; str[i]; i++)
			if (!ISSPACE((int)(unsigned char)str[i])
			    && DOENCODE(str[i]))
				break;
		if (str[i] == 0)
			return ( i ? (*func)(str, i, arg):0);

		/* Find start of word */

		while (i)
		{
			--i;
			if (ISSPACE((int)(unsigned char)str[i]))
			{
				++i;
				break;
			}
		}
		if (i)
		{
			rc= (*func)(str, i, arg);
			if (rc)	return (rc);
			str += i;
		}

		/*
		** Figure out when to stop MIME decoding.  Consecutive
		** MIME-encoded words are MIME-encoded together.
		*/

		i=0;

		for (;;)
		{
			for ( ; str[i]; i++)
				if (ISSPACE((int)(unsigned char)str[i]))
					break;
			if (str[i] == 0)
				break;

			for (c=i; str[c] && ISSPACE((int)(unsigned char)str[c]);
				++c)
				;
			
			for (; str[c]; c++)
				if (ISSPACE((int)(unsigned char)str[c]) ||
				    DOENCODE(str[c]))
					break;

			if (str[c] == 0 || ISSPACE((int)(unsigned char)str[c]))
				break;
			i=c;
		}

		/*
		** Figure out whether base64 is a better choice.
		*/

		n=0;

		for (j=0; j<i; j++)
			if (DOENCODEWORD(str[j]))
				++n;

		if (n > i/10)
		{
			encodebase64(str, i, charset, func, arg,
				     70, 0);
			str += i;
			continue;
		}



		/* Output mimeified text, insert spaces at 70+ character
		** boundaries for line wrapping.
		*/

		maxlen=strlen(charset)+10;

		if (maxlen < 65)
			maxlen=74-maxlen;
		else
			maxlen=10;

		c=0;
		while (i)
		{
			if (c == 0)
			{
				if ( (rc=(*func)("=?", 2, arg)) != 0 ||
					(rc=(*func)(charset, strlen(charset),
						arg)) != 0 ||
					(rc=(*func)("?Q?", 3, arg)) != 0)
					return (rc);
				c += strlen(charset)+5;
			}

			if (DOENCODEWORD(*str))
			{
				char	buf[3];

				buf[0]='=';
				buf[1]=xdigit[ ( *str >> 4) & 0x0F ];
				buf[2]=xdigit[ *str & 0x0F ];

				if ( (rc=*str == ' ' ? (*func)("_", 1, arg)
				      : (*func)(buf, 3, arg)) != 0)
					return (rc);
				c += *str == ' ' ? 1:3;
				++str;
				--i;
			}
			else
			{
				for (j=0; j < i && !DOENCODEWORD(str[j]); j++)
					if (j + c >= maxlen)
						break;
				if ( (rc=(*func)(str, j, arg)) != 0)
					return (rc);
				c += j;
				str += j;
				i -= j;
			}

			if (i == 0 || c >= maxlen)
			{
				if ( (rc=(*func)("?= ", i ? 3:2, arg)) != 0)
					return (rc);
				
				c=0;
			}
		}
	}
	return (0);
}

static int count_char(const char *c, size_t l, void *p)
{
size_t *i=(size_t *)p;

	*i += l;
	return (0);
}

static int save_char(const char *c, size_t l, void *p)
{
char **s=(char **)p;

	memcpy(*s, c, l);
	*s += l;
	return (0);
}

char *rfc2047_encode_str(const char *str, const char *charset,
			 int (*qp_allow)(char c))
{
size_t	i=1;
char	*s, *p;

	(void)rfc2047_encode_callback(str, charset,
				      qp_allow,
				      &count_char, &i);
	if ((s=malloc(i)) == 0)	return (0);
	p=s;
	(void)rfc2047_encode_callback(str, charset,
				      qp_allow,
				      &save_char, &p);
	*p=0;
	return (s);
}

int rfc2047_qp_allow_any(char c)
{
	return 1;
}

int rfc2047_qp_allow_comment(char c)
{
	if (c == '(' || c == ')' || c == '"')
		return 0;
	return 1;
}

int rfc2047_qp_allow_word(char c)
{
	return strchr(base64tab, c) != NULL ||
	       strchr("*-=_", c) != NULL;
}
