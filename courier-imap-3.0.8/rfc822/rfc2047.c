/*
** Copyright 1998 - 2004 Double Precision, Inc.  See COPYING for
** distribution information.
*/


#include	<stdio.h>
#include	<ctype.h>
#include	<string.h>
#include	<stdlib.h>

#include	"rfc822.h"
#include	"rfc2047.h"

static const char rcsid[]="$Id: rfc2047.c,v 1.20 2006/01/22 03:33:24 mrsam Exp $";

#define	RFC2047_ENCODE_FOLDLENGTH	76

static const char xdigit[]="0123456789ABCDEF";
static const char base64tab[]=
"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";


static char *rfc2047_search_quote(const char **ptr)
{
const char *p= *ptr;
char	*s;

	while (**ptr && **ptr != '?')
		++ *ptr;
	if ((s=malloc( *ptr - p + 1)) == 0)
		return (0);
	memcpy(s, p, *ptr-p);
	s[*ptr - p]=0;
	return (s);
}

static int nyb(int c)
{
const char	*p;

	c=toupper( (int)(unsigned char)c );
	p=strchr(xdigit, c);
	return (p ? p-xdigit:0);
}

static unsigned char decode64tab[256];
static int decode64tab_init=0;

static size_t decodebase64(char *ptr, size_t cnt)
{
size_t  i, j;
char    a,b,c;
size_t  k;

	if (!decode64tab_init)
	{
		for (i=0; i<256; i++)   decode64tab[i]=0;
		for (i=0; i<64; i++)
			decode64tab[(int)(base64tab[i])]=i;
		decode64tab[ (int)'=' ] = 99;
	}

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
		ptr[k++]=a;
		if ( ptr[j+2] != '=')
			ptr[k++]=b;
		if ( ptr[j+3] != '=')
			ptr[k++]=c;
	}
	return (k);
}

/*
**	This is the main rfc2047 decoding function.  It receives rfc2047-encoded
**	text, and a callback function.  The callback function is repeatedly
**	called, each time receiving a piece of decoded text.  The decoded
**	info includes a text fragment - string, string length arg - followed
**	by the character set, followed by a context pointer that is received
**	from the caller.  If the callback function returns non-zero, rfc2047
**	decoding terminates, returning the result code.  Otherwise,
**	rfc2047_decode returns 0 after a successfull decoding (-1 if malloc
**	failed).
*/

int rfc2047_decode(const char *text, int (*func)(const char *, int,
						 const char *,
						 const char *, void *),
		void *arg)
{
int	rc;
int	had_last_word=0;
const char *p;
char	*chset, *lang;
char	*encoding;
char	*enctext;
char	*enctext_s=NULL, *chset_s=NULL, *lang_s=NULL;

#define	FREE_SAVED	{ \
				if (enctext_s)	free(enctext_s); \
				enctext_s=NULL; \
				if (chset_s)	free(chset_s); \
				chset_s=NULL; \
			}

	while (text && *text)
	{
		if (text[0] != '=' || text[1] != '?')
		{
			p=text;
			while (*text)
			{
				if (text[0] == '=' && text[1] == '?')
					break;
				if (!isspace((int)(unsigned char)*text))
					had_last_word=0;
				++text;
			}
			if (text > p && !had_last_word)
			{
				if (enctext_s)
				/* Flush buffer. */
				{
					rc=(*func)(enctext_s,
						   strlen(enctext_s), chset_s,
						   lang_s, arg);
					FREE_SAVED;
					if (rc)	return rc;
				} 
				rc=(*func)(p, text-p, 0, 0, arg);
				if (rc) return (rc);
			}
			continue;
		}

		text += 2;
		if ((chset=rfc2047_search_quote( &text )) == 0)
		{
			FREE_SAVED;
			return (-1);
		}
		if (*text)	++text;
		if ((encoding=rfc2047_search_quote( &text )) == 0)
		{
			free(chset);
			FREE_SAVED;
			return (-1);
		}
		if (*text)	++text;
		if ((enctext=rfc2047_search_quote( &text )) == 0)
		{
			free(encoding);
			free(chset);
			FREE_SAVED;
			return (-1);
		}
		if (*text == '?' && text[1] == '=')
			text += 2;
		if (strcmp(encoding, "Q") == 0 || strcmp(encoding, "q") == 0)
		{
			char *q, *r;

			for (q=r=enctext; *q; )
			{
				int c;

				if (*q == '=' && q[1] && q[2])
				{
					*r++ = (char)(
						nyb(q[1])*16+nyb(q[2]));
					q += 3;
					continue;
				}

				c=*q++;
				if (c == '_')
					c=' ';
				*r++ = c ;
			}
			*r=0;
		}
		else if (strcmp(encoding, "B") == 0 || strcmp(encoding, "b")==0)
		{
			enctext[decodebase64(enctext, strlen(enctext))]=0;
		}

		lang=strrchr(chset, '*'); /* RFC 2231 language */

		if (lang)
			*lang++=0;

		if (enctext_s)
		{
		/*
		 * If charset or language is changed, flush buffer.
		 * Otherwise, add decoded string to buffer.
		 */
			if ((lang_s && lang && strcasecmp(lang_s, lang) != 0) ||
			    (!lang_s && lang) || (lang_s && !lang) ||
			    strcasecmp(chset_s, chset) != 0)
			{
				rc=(*func)(enctext_s, strlen(enctext_s),
					   chset_s, lang_s, arg);
				FREE_SAVED;
				if (rc)
				{
					free(chset);
					free(enctext);
					free(encoding);
					return rc;
				}
				enctext_s = enctext;
				chset_s = chset;
				lang_s = lang;
			}
			else
			{
			char *p;
				if (!(p=malloc(strlen(enctext_s) +
					       strlen(enctext) + 1)))
				{
					FREE_SAVED;
					free(chset);
					free(enctext);
					free(encoding);
					return (-1);
				}
				strcat(strcpy(p, enctext_s), enctext);
				free(chset);
				free(enctext);
				free(enctext_s);
				enctext_s = p;
			}

		}
		else
		{
			enctext_s = enctext;
			chset_s = chset;
			lang_s = lang;
		}
		free(encoding);

		had_last_word=1;	/* Ignore blanks between enc words */
	}

	/* Flush buffer. */
	if (enctext_s)
	{
		rc=(*func)(enctext_s, strlen(enctext_s), chset_s, lang_s, arg);
		FREE_SAVED;
		if (rc)	return (rc);
	}
	return (0);
#undef FREE_SAVED
}

/*
** rfc2047_decode_simple just strips out the rfc2047 decoding, throwing away
** the character set.  This is done by calling rfc2047_decode twice, once
** to count the number of characters in the decoded text, the second time to
** actually do it.
*/

struct simple_info {
	char *string;
	int index;
	const char *mychset;
	} ;

static int count_simple(const char *txt, int len, const char *chset,
			const char *lang, void *arg)
{
struct simple_info *iarg= (struct simple_info *)arg;

	iarg->index += len;

	return (0);
}

static int save_simple(const char *txt, int len, const char *chset,
		       const char *lang,
		       void *arg)
{
struct simple_info *iarg= (struct simple_info *)arg;

	memcpy(iarg->string+iarg->index, txt, len);
	iarg->index += len;
	return (0);
}

char *rfc2047_decode_simple(const char *text)
{
struct	simple_info info;

	info.index=1;
	if (rfc2047_decode(text, &count_simple, &info))
		return (0);

	if ((info.string=malloc(info.index)) == 0)	return (0);
	info.index=0;
	if (rfc2047_decode(text, &save_simple, &info))
	{
		free(info.string);
		return (0);
	}
	info.string[info.index]=0;
	return (info.string);
}

/*
** rfc2047_decode_enhanced is like simply, but prefixes the character set
** name before the text, in brackets.
*/

static int do_enhanced(const char *txt, int len, const char *chset,
		       const char *lang,
		       void *arg,
		       int (*func)(const char *, int, const char *,
				   const char *, void *)
		       )
{
int	rc=0;
struct	simple_info *info=(struct simple_info *)arg;

	if (chset && info->mychset && strcasecmp(chset, info->mychset) == 0)
		chset=0;

	if (chset)
	{
		rc= (*func)(" [", 2, 0, 0, arg);
		if (rc == 0)
			rc= (*func)(chset, strlen(chset), 0, 0, arg);
		if (rc == 0 && lang)
			rc= (*func)("*", 1, 0, 0, arg);
		if (rc == 0 && lang)
			rc= (*func)(lang, strlen(lang), 0, 0, arg);
		if (rc == 0)
			rc= (*func)("] ", 2, 0, 0, arg);
	}

	if (rc == 0)
		rc= (*func)(txt, len, 0, 0, arg);
	return (rc);
}

static int count_enhanced(const char *txt, int len, const char *chset,
			  const char *lang,
			  void *arg)
{
	return (do_enhanced(txt, len, chset, lang, arg, &count_simple));
}

static int save_enhanced(const char *txt, int len, const char *chset,
			 const char *lang,
			 void *arg)
{
	return (do_enhanced(txt, len, chset, lang, arg, &save_simple));
}

char *rfc2047_decode_enhanced(const char *text, const char *mychset)
{
struct	simple_info info;

	info.mychset=mychset;
	info.index=1;
	if (rfc2047_decode(text, &count_enhanced, &info))
		return (0);

	if ((info.string=malloc(info.index)) == 0)	return (0);
	info.index=0;
	if (rfc2047_decode(text, &save_enhanced, &info))
	{
		free(info.string);
		return (0);
	}
	info.string[info.index]=0;
	return (info.string);
}

void rfc2047_print(const struct rfc822a *a,
	const char *charset,
	void (*print_func)(char, void *),
	void (*print_separator)(const char *, void *), void *ptr)
{
	rfc822_print_common(a, &rfc2047_decode_enhanced, charset,
		print_func, print_separator, ptr);
}

static char *a_rfc2047_encode_str(const char *str, const char *charset);

static void rfc2047_encode_header_do(const struct rfc822a *a,
	const char *charset,
	void (*print_func)(char, void *),
	void (*print_separator)(const char *, void *), void *ptr)
{
	rfc822_print_common(a, &a_rfc2047_encode_str, charset,
		print_func, print_separator, ptr);
}

/*
** When MIMEifying names from an RFC822 list of addresses, strip quotes
** before MIMEifying them, and add them afterwards.
*/

static char *a_rfc2047_encode_str(const char *str, const char *charset)
{
	size_t	l;
	char	*p, *r, *s;
	int (*qp_func)(char);
	char save_char;

	for (l=0; str[l]; l++)
		if (str[l] & 0x80)
			break;
	if (str[l] == 0)
		return (strdup(str));

	l=strlen(str);

	if (*str == '"' && str[l-1] == '"')
		qp_func=rfc2047_qp_allow_word;
	else if (*str == '(' && str[l-1] == ')')
		qp_func=rfc2047_qp_allow_comment;
	else
		return (rfc2047_encode_str(str, charset,
					   rfc2047_qp_allow_comment));

	save_char=*str;

	p=malloc(l);
	if (!p)	return (0);
	memcpy(p, str+1, l-2);
	p[l-2]=0;
	for (r=s=p; *r; r++)
	{
		if (*r == '\\' && r[1])
			++r;
		else
			*s++=*r;
	}
	*s=0;

	s=rfc2047_encode_str(p, charset, qp_func);
	free(p);

	if (save_char == '(')
	{
		p=malloc(strlen(s)+3);
		if (p)
		{
			p[0]='(';
			strcpy(p+1, s);
			strcat(p, ")");
		}
		free(s);
		s=p;
	}
	return s;
}




static void count(char c, void *p);
static void counts2(const char *c, void *p);
static void save(char c, void *p);
static void saves2(const char *c, void *p);

char *rfc2047_encode_header(const struct rfc822a *a,
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

static void count(char c, void *p)
{
	++*(size_t *)p;
}

static void counts2(const char *c, void *p)
{
	if (strcmp(c, ", ") == 0)
		c=",\n  ";

	while (*c)	count(*c++, p);
}

static void save(char c, void *p)
{
	**(char **)p=c;
	++*(char **)p;
}

static void saves2(const char *c, void *p)
{
	if (strcmp(c, ", ") == 0)
		c=",\n  ";

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

#if	HAVE_LIBUNICODE

#include	"../unicode/unicode.h"

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
#endif

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
#if	HAVE_LIBUNICODE
const struct unicode_info *ci = unicode_find(charset);
#endif

	if (!str || !*str)
		return 0;

#if	HAVE_LIBUNICODE

	if (ci && ci->flags & UNICODE_SISO)
		return rfc2047_encode_callback_base64(str, charset, qp_allow,
						      func, arg);
#endif
  
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
