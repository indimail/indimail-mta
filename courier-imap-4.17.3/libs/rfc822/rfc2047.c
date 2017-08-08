/*
** Copyright 1998 - 2011 Double Precision, Inc.  See COPYING for
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
	size_t s=strlen(domain)+16;
	char *cpy=malloc(s);

	if (!cpy)
		return NULL;

	/*
	** Invalid UTF-8 can make libidn go off the deep end. Add
	** padding as a workaround.
	*/

	memset(cpy, 0, s);
	strcpy(cpy, domain);

	err=idna_to_ascii_8z(cpy, &p, 0);
	free(cpy);

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
	char *p=unicode_convert_tobuf(address, charset, "utf-8", NULL);
	char *cp, *q;

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
			int (*qp_allow)(char),
			int (*func)(const char *, size_t, void *), void *arg)
{
	unsigned char ibuf[3];
	char obuf[4];
	int	rc;

	if ((rc=(*func)("=?", 2, arg)) ||
	    (rc=(*func)(charset, strlen(charset), arg))||
	    (rc=(*func)("?B?", 3, arg)))
		return rc;

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
	}

	if ((rc=(*func)("?=", 2, arg)))
		return rc;
	return 0;
}

#define ISSPACE(i) ((i)=='\t' || (i)=='\r' || (i)=='\n' || (i)==' ')
#define DOENCODEWORD(c) \
	((c) < 0x20 || (c) > 0x7F || (c) == '"' || \
	 (c) == '_' || (c) == '=' || (c) == '?' || !(*qp_allow)((char)c))

/*
** Encode a character stream using quoted-printable encoding.
*/
static int encodeqp(const char *ptr, size_t len,
		    const char *charset,
		    int (*qp_allow)(char),
		    int (*func)(const char *, size_t, void *), void *arg)
{
	size_t i;
	int rc;
	char buf[3];

	if ((rc=(*func)("=?", 2, arg)) ||
	    (rc=(*func)(charset, strlen(charset), arg))||
	    (rc=(*func)("?Q?", 3, arg)))
		return rc;

	for (i=0; i<len; ++i)
	{
		size_t j;

		for (j=i; j<len; ++j)
		{
			if (ptr[j] == ' ' || DOENCODEWORD(ptr[j]))
				break;
		}

		if (j > i)
		{
			rc=(*func)(ptr+i, j-i, arg);

			if (rc)
				return rc;
			if (j >= len)
				break;
		}
		i=j;

		if (ptr[i] == ' ')
			rc=(*func)("_", 1, arg);
		else
		{
			buf[0]='=';
			buf[1]=xdigit[ ( ptr[i] >> 4) & 0x0F ];
			buf[2]=xdigit[ ptr[i] & 0x0F ];

			rc=(*func)(buf, 3, arg);
		}

		if (rc)
			return rc;
	}

	return (*func)("?=", 2, arg);
}

/*
** Calculate whether the next word should be RFC2047-encoded.
**
** Returns 0 if not, 1 if any character in the next word is flagged by
** DOENCODEWORD().
*/

static int encode_word(const char32_t *uc,
		       size_t ucsize,
		       int (*qp_allow)(char),

		       /*
		       ** Points to the starting offset of word in uc.
		       ** At exit, points to the end of the word in uc.
		       */
		       size_t *word_ptr)
{
	size_t i;
	int encode=0;

	for (i=*word_ptr; i<ucsize; ++i)
	{
		if (ISSPACE(uc[i]))
			break;

		if (DOENCODEWORD(uc[i]))
			encode=1;
	}

	*word_ptr=i;
	return encode;
}

/*
** Calculate whether the next sequence of words should be RFC2047-encoded.
**
** Whatever encode_word() returns for the first word, look at the next word
** and keep going as long as encode_word() keeps returning the same value.
*/

static int encode_words(const char32_t *uc,
			size_t ucsize,
			int (*qp_allow)(char),

			/*
			** Points to the starting offset of words in uc.
			** At exit, points to the end of the words in uc.
			*/

			size_t *word_ptr)
{
	size_t i= *word_ptr, j, k;

	int flag=encode_word(uc, ucsize, qp_allow, &i);

	if (!flag)
	{
		*word_ptr=i;
		return flag;
	}

	j=i;

	while (j < ucsize)
	{
		if (ISSPACE(uc[j]))
		{
			++j;
			continue;
		}

		k=j;

		if (!encode_word(uc, ucsize, qp_allow, &k))
			break;
		i=j=k;
	}

	*word_ptr=i;
	return flag;
}

/*
** Encode a sequence of words.
*/
static int do_encode_words_method(const char32_t *uc,
				  size_t ucsize,
				  const char *charset,
				  int (*qp_allow)(char),
				  size_t offset,
				  int (*encoder)(const char *ptr, size_t len,
						 const char *charset,
						 int (*qp_allow)(char),
						 int (*func)(const char *,
							     size_t, void *),
						 void *arg),
				  int (*func)(const char *, size_t, void *),
				  void *arg)
{
	char    *p;
	size_t  psize;
	int rc;
	int first=1;

	while (ucsize)
	{
		size_t j;
		size_t i;

		if (!first)
		{
			rc=(*func)(" ", 1, arg);

			if (rc)
				return rc;
		}
		first=0;

		j=(RFC2047_ENCODE_FOLDLENGTH-offset)/2;

		if (j >= ucsize)
			j=ucsize;
		else
		{
			/*
			** Do not split rfc2047-encoded works across a
			** grapheme break.
			*/

			for (i=j; i > 0; --i)
				if (unicode_grapheme_break(uc[i-1], uc[i]))
				{
					j=i;
					break;
				}
		}

		if ((rc=unicode_convert_fromu_tobuf(uc, j, charset,
						      &p, &psize,
						      NULL)) != 0)
			return rc;


		if (psize && p[psize-1] == 0)
			--psize;

		rc=(*encoder)(p, psize, charset, qp_allow,
			      func, arg);
		free(p);
		if (rc)
			return rc;
		offset=0;
		ucsize -= j;
		uc += j;
	}
	return 0;
}

static int cnt_conv(const char *dummy, size_t n, void *arg)
{
	*(size_t *)arg += n;
	return 0;
}

/*
** Encode, or not encode, words.
*/

static int do_encode_words(const char32_t *uc,
			   size_t ucsize,
			   const char *charset,
			   int flag,
			   int (*qp_allow)(char),
			   size_t offset,
			   int (*func)(const char *, size_t, void *),
			   void *arg)
{
	char    *p;
	size_t  psize;
	int rc;
	size_t b64len, qlen;

	/*
	** Convert from unicode
	*/

	if ((rc=unicode_convert_fromu_tobuf(uc, ucsize, charset,
					      &p, &psize,
					      NULL)) != 0)
		return rc;

	if (psize && p[psize-1] == 0)
		--psize;

	if (!flag) /* If not converting, then the job is done */
	{
		rc=(*func)(p, psize, arg);
		free(p);
		return rc;
	}
	free(p);

	/*
	** Try first quoted-printable, then base64, then pick whichever
	** one gives the shortest results.
	*/
	qlen=0;
	b64len=0;

	rc=do_encode_words_method(uc, ucsize, charset, qp_allow, offset,
				  &encodeqp, cnt_conv, &qlen);
	if (rc)
		return rc;

	rc=do_encode_words_method(uc, ucsize, charset, qp_allow, offset,
				  &encodebase64, cnt_conv, &b64len);
	if (rc)
		return rc;

	return do_encode_words_method(uc, ucsize, charset, qp_allow, offset,
				      qlen < b64len ? encodeqp:encodebase64,
				      func, arg);
}

/*
** RFC2047-encoding pass.
*/
static int rfc2047_encode_callback(const char32_t *uc,
				   size_t ucsize,
				   const char *charset,
				   int (*qp_allow)(char),
				   int (*func)(const char *, size_t, void *),
				   void *arg)
{
	int	rc;
	size_t	i;
	int	flag;

	size_t	offset=27; /* FIXME: initial offset for line length */

	while (ucsize)
	{
		/* Pass along all the whitespace */

		if (ISSPACE(*uc))
		{
			char c= *uc++;
			--ucsize;

			if ((rc=(*func)(&c, 1, arg)) != 0)
				return rc;
			continue;
		}

		i=0;

		/* Check if the next word needs to be encoded, or not. */

		flag=encode_words(uc, ucsize, qp_allow, &i);

		/*
		** Then proceed to encode, or not encode, the following words.
		*/

		if ((rc=do_encode_words(uc, i, charset, flag,
					qp_allow, offset,
					func, arg)) != 0)
			return rc;

		offset=0;
		uc += i;
		ucsize -= i;
	}

	return 0;
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
	char32_t *uc;
	size_t ucsize;
	int err;

	/* Convert string to unicode */

	if (unicode_convert_tou_tobuf(str, strlen(str), charset,
					&uc, &ucsize, &err))
		return NULL;

	/*
	** Perform two passes: calculate size of the buffer where the
	** encoded string gets saved into, then allocate the buffer and
	** do a second pass to actually do it.
	*/

	if (rfc2047_encode_callback(uc, ucsize,
				    charset,
				    qp_allow,
				    &count_char, &i))
	{
		free(uc);
		return NULL;
	}

	if ((s=malloc(i)) == 0)
	{
		free(uc);
		return NULL;
	}

	p=s;
	(void)rfc2047_encode_callback(uc, ucsize,
				      charset,
				      qp_allow,
				      &save_char, &p);
	*p=0;
	free(uc);
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
