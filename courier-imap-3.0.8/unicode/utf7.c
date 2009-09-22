/*
** Copyright 2003 Double Precision, Inc.
** See COPYING for distribution information.
**
** $Id: utf7.c,v 1.3 2004/05/23 14:28:25 mrsam Exp $
*/

#include "unicode.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

static const char base64tab[]=
"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

struct dec_base64_struct {

	unsigned char base64buf[4];
	int cnt;

	unicode_char uc1;
	int ucnt;

	int flushing;
	int flushed;
};

/* Poor man's base64 decoder */

static int dec_b64_uchar(struct dec_base64_struct *dc,
			 unsigned char uuc,
			 unicode_char **buffer,
			 size_t *buflen)
{
	if (dc->flushed)
		return 0; /* Flushing trailing junk */

	dc->uc1 <<= 8;
	dc->uc1 |= uuc;

	++dc->ucnt;

	if (dc->ucnt == 2)
	{
		if (dc->uc1 < 0xD800 || dc->uc1 > 0xDFFF)
			/* Not surrogate pair */
		{
			if ( *buffer )
				(*buffer)[*buflen]=dc->uc1;
			++*buflen;
			dc->uc1=0;
			dc->ucnt=0;
			if (dc->flushing)
				dc->flushed=1;
			return (0);
		}

		if (dc->uc1 > 0xDBFF)
			return -1; /* Bad surrogate pair */
		return 0;
	}

	if (dc->ucnt == 4)
	{
		unicode_char uc2= (dc->uc1 >> 16) & 0xFFFF;

		dc->uc1 &= 0xFFFF;

		if (dc->uc1 < 0xDC00 || dc->uc1 > 0xDFFF)
			return -1;


		if (*buffer)
		{
			(*buffer)[*buflen] = ((uc2 & 0x3FF) << 10) |
				(dc->uc1 & 0x3FF);
		}
		++*buflen;

		dc->uc1=0;
		dc->ucnt=0;
		if (dc->flushing)
			dc->flushed=1;
		return (0);
	}
	return 0;
}

static int dec_b64_char(struct dec_base64_struct *dc,
			char c,
			unicode_char **buffer,
			size_t *buflen)
{
	dc->base64buf[dc->cnt]=c;
	if (++dc->cnt >= 4) /* Four characters to base64 decode */
	{
		char	a,b,c;

		int	w=dc->base64buf[0];
		int	x=dc->base64buf[1];
		int	y=dc->base64buf[2];
		int	z=dc->base64buf[3];

		a= (w << 2) | (x >> 4);
		b= (x << 4) | (y >> 2);
		c= (y << 6) | z;
		dc->cnt=0;

		if (dec_b64_uchar(dc, a, buffer, buflen))
			return -1;

		if (dec_b64_uchar(dc, b, buffer, buflen))
			return -1;

		if (dec_b64_uchar(dc, c, buffer, buflen))
			return -1;
	}
	return 0;
}

static unicode_char *tou(const struct unicode_info *foo, const char *p,
			 int *err)
{
	int pass;
	size_t i;
	unicode_char *buffer=NULL;
	size_t buflen=0;

	/* Two passes.  Count the output, alloc buffer, do it */

	for (pass=0; pass<2; pass++)
	{
		if (pass)
		{
			if ((buffer=malloc((buflen+1)*sizeof(unicode_char)))
			    == NULL)
				return NULL;
		}
		buflen=0;

		for (i=0; p[i]; i++)
		{
			char *q;
			struct dec_base64_struct dc;

			if (p[i] != '+')
			{
				if (buffer)
				{
					buffer[buflen]=(unsigned char)p[i];
				}
				++buflen;
				continue;
			}

			if (p[++i] == 0)
				break;

			if (p[i] == '-')
			{
				if (buffer)
					buffer[buflen]='+';
				++buflen;
				continue;
			}

			dc.cnt=0;
			dc.ucnt=0;
			dc.uc1=0;
			dc.flushing=0;
			dc.flushed=0;

			while ( p[i] && (q=strchr(base64tab, p[i])) != NULL)
			{
				if (dec_b64_char(&dc, (q-base64tab),
						 &buffer,
						 &buflen))
				{
					if (err)
					{
						*err=i;
						errno=EINVAL;
						return NULL;
					}

					/* Recover from decoding error */

					dc.cnt=0;
					dc.ucnt=0;
					dc.uc1=0;
				}
				++i;
			}

			dc.flushing=1;

			while (dc.cnt > 0)
			{
				if (dec_b64_char(&dc, 0,
						 &buffer,
						 &buflen))
				{
					if (err)
					{
						*err=i;
						errno=EINVAL;
						return NULL;
					}
					dc.cnt=0;
					dc.ucnt=0;
					dc.uc1=0;
				}
			}

			if (p[i] == 0)
				break;

			if (p[i] != '-')
				--i;
		}


		if (pass)
			buffer[buflen]=0;
	}

	return buffer;
}

/* Poor man's base64 encoder */

struct enc_base64_struct {

	char base64buf[3];
	int cnt;
	int clip;
};

static void encode_base64_char(struct enc_base64_struct *p,
			       char c,
			       char **buffer,
			       size_t *buflen)
{
	p->base64buf[p->cnt]=c;

	if (++p->cnt >= 3) /* Encode three octets in base64 */
	{
		int a, b, c;
		int d, e, f, g;

		a=(unsigned char)p->base64buf[0];
		b=(unsigned char)p->base64buf[1];
		c=(unsigned char)p->base64buf[2];

		d=base64tab[ a >> 2 ];
		e=base64tab[ ((a & 3 ) << 4) | (b >> 4)];
		f=base64tab[ ((b & 15) << 2) | (c >> 6)];
		g=base64tab[ c & 63 ];

		p->cnt=0;

		if (*buffer)
		{
			(*buffer)[*buflen]=d;
			(*buffer)[*buflen+1]=e;
		}
		*buflen += 2;

		if (p->clip < 2) /* Clip trailing junk, don't need it */
		{
			if (*buffer)
			{
				(*buffer)[*buflen]=f;
			}
			++*buflen;
		}

		if (p->clip < 1)
		{
			if (*buffer)
			{
				(*buffer)[*buflen]=g;
			}
			++*buflen;
		}
	}
}

static void encode_base64_u16(struct enc_base64_struct *p,
			      unicode_char uc,
			      char **buffer,
			      size_t *buflen)
{
	encode_base64_char(p, (uc >> 8) & 255, buffer, buflen);
	encode_base64_char(p, uc & 255, buffer, buflen);
}

static void encode_base64_u32(struct enc_base64_struct *p,
			      unicode_char uc,
			      char **buffer,
			      size_t *buflen)
{
	if ((uc >= 0xD800 && uc <= 0xDFFF) /* Really illegal, but punt */
	    || uc > 0xFFFFU)
	{
		encode_base64_u16(p, ((uc >> 10) & 0x3FF) | 0xD800,
				  buffer, buflen);
		encode_base64_u16(p, (uc & 0x3FF) | 0xDC00,
				  buffer, buflen);
	}
	else
	{
		encode_base64_u16(p, uc, buffer, buflen);
	}
}


#define LITERAL(c) ( (c) == '\n' || (c) == '\r' || (c) == '\t' || (c) == ' ' \
	|| ( (c) >= 33 && c <= 125 && c != 92))

static char *fromu(const struct unicode_info *foo, const unicode_char *p,
		   int *err)
{
	char *buffer=0;
	size_t buflen=0;
	int pass;
	size_t i;

	for (pass=0; pass<2; pass++)
	{
		if (pass)
		{
			if ((buffer=malloc(buflen+1)) == NULL)
				return NULL;
		}
		buflen=0;

		for (i=0; p[i]; i++)
		{
			struct enc_base64_struct eb;

			if (p[i] == '+')
			{
				if (pass)
				{
					buffer[buflen]='+';
					buffer[buflen+1]='-';
				}
				buflen += 2;
				continue;
			}

			if (LITERAL(p[i]))
			{
				if (pass)
				{
					buffer[buflen]=(char)p[i];
				}
				++buflen;
				continue;
			}

			if (pass)
				buffer[buflen]='+';
			++buflen;

			eb.cnt=0;
			eb.clip=0;

			do
			{
				if (p[i] >= 0x10FFFF)
				{
					if (err)
					{
						*err=i;
						errno=EINVAL;
						return NULL;
					}
					encode_base64_u32(&eb, 0xFFFD,
							  &buffer, &buflen);
				}
				else
				{
					encode_base64_u32(&eb, p[i],
							  &buffer, &buflen);
				}
				++i;
			} while ( p[i] && !LITERAL(p[i]));

			switch (eb.cnt) {
			case 2:
				eb.clip=2;
				break;
			case 3:
				eb.clip=1;
				break;
			}
					
			while (eb.cnt)
			{
				encode_base64_char(&eb, 0, &buffer, &buflen);
			}

			if (!p[i])
			{
				if (pass)
				{
					buffer[buflen]='-';
				}
				++buflen;
				break;
			}

			if (p[i] == '-')
			{
				if (pass)
				{
					buffer[buflen]='-';
				}
				++buflen;
			}
			--i;
		}

		if (pass)
			buffer[buflen]=0;
	}

	return buffer;
}


/*
** UTF7.toupper/tolower/totitle is implemented by converting UTF8 to
** UCS-4, applying the unicode table lookup, then converting it back to
** UTF7
*/

static char *toupper_func(const struct unicode_info *u,
			  const char *cp, int *ip)
{
	unicode_char *uc=tou(u, cp, ip), *p;
	char *s;

	if (!uc) return (0);

	for (p=uc; *p; p++)
		*p=unicode_uc(*p);

	s=fromu(u, uc, NULL);
	if (!s && ip)
		*ip=0;
	free(uc);
	return (s);
}

static char *tolower_func(const struct unicode_info *u,
			  const char *cp, int *ip)
{
	unicode_char *uc=tou(u, cp, ip), *p;
	char *s;

	if (!uc) return (0);

	for (p=uc; *p; p++)
		*p=unicode_lc(*p);

	s=fromu(u, uc, NULL);
	free(uc);
	if (!s && ip)
		*ip=0;
	return (s);
}

static char *totitle_func(const struct unicode_info *u,
			  const char *cp, int *ip)
{
	unicode_char *uc=tou(u, cp, ip), *p;
	char *s;

	if (!uc) return (0);

	for (p=uc; *p; p++)
		*p=unicode_tc(*p);

	s=fromu(u, uc, NULL);
	if (!s && ip)
		*ip=0;
	free(uc);
	return (s);
}

const struct unicode_info unicode_UTF7 = {
	"UTF-7",
	UNICODE_UTF | UNICODE_MB |
	UNICODE_HEADER_QUOPRI | UNICODE_BODY_QUOPRI,
	tou,
	fromu,
	toupper_func,
	tolower_func,
	totitle_func};

#if 0
extern const struct unicode_info unicode_UTF8;

int main(int argc, char **argv)
{
	char *a, *b, *c;

	a=unicode_xconvert("A+ImIDkQ.", &unicode_UTF7,
			   &unicode_UTF8);
	b=unicode_xconvert("Hi Mom -+Jjo--!", &unicode_UTF7,
			   &unicode_UTF8);
	c=unicode_xconvert("+ZeVnLIqe-", &unicode_UTF7,
			   &unicode_UTF8);

	printf("%s\n", a);
	printf("%s\n", b);
	printf("%s\n", c);

	printf("%s\n", unicode_xconvert(a, &unicode_UTF8, &unicode_UTF7));
	printf("%s\n", unicode_xconvert(b, &unicode_UTF8, &unicode_UTF7));
	printf("%s\n", unicode_xconvert(c, &unicode_UTF8, &unicode_UTF7));

	return 0;
}
#endif
