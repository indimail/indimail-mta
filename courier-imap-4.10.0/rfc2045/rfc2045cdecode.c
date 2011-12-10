/*
** Copyright 1998 - 1999 Double Precision, Inc.  See COPYING for
** distribution information.
*/

#include	"rfc2045.h"
#include	<ctype.h>
#include	<string.h>


extern void rfc2045_add_buf( char **, size_t *, size_t *,
		const char *, size_t);
extern void rfc2045_add_workbuf(struct rfc2045 *, const char *, size_t);
extern void rfc2045_add_workbufch(struct rfc2045 *, int);

static int decode_raw(struct rfc2045 *p, const char *s, size_t l)
{
	if (s && l)	return ((*p->udecode_func)(s,l,p->misc_decode_ptr));
	return (0);
}

static const char xdigit[]="0123456789ABCDEF";

static int tou(char c)
{
	if (c >= 'a' && c <= 'f')
		return c + ('A'-'a');
	return c;
}

static int do_decode_qp(struct rfc2045 *p)
{
char	*a, *b, *c, *end;
int	d;

	end=p->workbuf + p->workbuflen;
	for (a=b=p->workbuf; a < end; )
	{
		if (*a != '=')
		{
			*b++ = *a++;
			continue;
		}
		++a;
		if (!*a || a >= end || isspace((int)(unsigned char)*a))
			break;

		if ((c=strchr(xdigit, tou(*a))) == 0) continue;
		d= (c-xdigit)*16;
		++a;
		if (!*a || a >= end)
			break;
		if ((c=strchr(xdigit, tou(*a))) == 0) continue;
		d += c-xdigit;
		++a;
		*b++=d;
	}
	p->workbuflen= b-p->workbuf;
	d=(*p->udecode_func)(p->workbuf, p->workbuflen, p->misc_decode_ptr);
	p->workbuflen=0;
	return (d);
}

static const unsigned char decode64tab[256]={
	100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,
	100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,
	100,100,100,100,100,100,100,100,100,100,100,62,100,100,100,63,
	52,53,54,55,56,57,58,59,60,61,100,100,100,99,100,100,
	100,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,
	15,16,17,18,19,20,21,22,23,24,25,100,100,100,100,100,
	100,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,
	41,42,43,44,45,46,47,48,49,50,51,100,100,100,100,100,
	100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,
	100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,
	100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,
	100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,
	100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,
	100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,
	100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,
	100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100
};

/* When we have enough base64-encoded data in the buffer, decode it. */

static int do_decode_base64(struct rfc2045 *p)
{
size_t	i, j;
char	a,b,c;
size_t	k;
int	rc;

	/* Remove everything except base64encoded data */

	for (i=j=0; i<p->workbuflen; i++)
		if (decode64tab[(int)(unsigned char)p->workbuf[i]] < 100)
			p->workbuf[j++]=p->workbuf[i];


	p->workbuflen=j;

	/* Decode the data, in 4-byte pieces */

	i=j / 4;
	i=i*4;
	k=0;
	for (j=0; j<i; j += 4)
	{
	int	w=decode64tab[(int)(unsigned char)p->workbuf[j]];
	int	x=decode64tab[(int)(unsigned char)p->workbuf[j+1]];
	int	y=decode64tab[(int)(unsigned char)p->workbuf[j+2]];
	int	z=decode64tab[(int)(unsigned char)p->workbuf[j+3]];

		a= (w << 2) | (x >> 4);
		b= (x << 4) | (y >> 2);
		c= (y << 6) | z;
		p->workbuf[k++]=a;
		if ( p->workbuf[j+2] != '=')
			p->workbuf[k++]=b;
		if ( p->workbuf[j+3] != '=')
			p->workbuf[k++]=c;
	}
	rc=(*p->udecode_func)(p->workbuf, k, p->misc_decode_ptr);

	/* Anything left?  Move it to the start of the buffer */

	k=0;
	while (j < p->workbuflen)
		p->workbuf[k++]=p->workbuf[j++];
	p->workbuflen=k;
	return (rc);
}

static int decode_qp(struct rfc2045 *p, const char *s, size_t l)
{
size_t	start,i;
int	rc;

	if (!s)
		return (do_decode_qp(p));

	for (start=0; start<l; )
	{
		for (i=start; i<l; i++)
		{
			if (s[i] != '\n') continue;
			rfc2045_add_workbuf(p, s+start, i-start);
			rfc2045_add_workbufch(p, '\n');
			if ((rc=do_decode_qp(p)) != 0)	return (rc);
			start= ++i;
			break;
		}
		rfc2045_add_workbuf(p, s+start, i-start);
		if (p->workbuflen > 1024)
		{
		char	buf[10];
		int	i;

			for (i=p->workbuflen - 5; i<p->workbuflen; i++)
				if (p->workbuf[i] == '=')	break;
			if (i < p->workbuflen)
			{
			int j=p->workbuflen-i;

				memcpy(buf, p->workbuf+i, j);
				buf[j]=0;
				p->workbuflen=i;
			}
			else	buf[0]=0;
			if ((rc=do_decode_qp(p)) != 0)	return (rc);
			rfc2045_add_workbuf(p, buf, strlen(buf));
		}
		start=i;
	}
	return (0);
}

static int decode_base64(struct rfc2045 *p, const char *s, size_t l)
{
	if (!s)
		return (do_decode_base64(p));

	rfc2045_add_workbuf(p, s, l);
	if (p->workbuflen > 256)
		return (do_decode_base64(p));
	return (0);
}

void rfc2045_cdecode_start(struct rfc2045 *p,
	int (*u)(const char *, size_t, void *), void *miscptr)
{
	p->misc_decode_ptr=miscptr;
	p->udecode_func=u;
	p->decode_func= &decode_raw;
	p->workbuflen=0;
	if (p->content_transfer_encoding)
	{
		if (strcmp(p->content_transfer_encoding,
				"quoted-printable") == 0)
			p->decode_func= &decode_qp;
		else if (strcmp(p->content_transfer_encoding, "base64") == 0)
			p->decode_func= &decode_base64;
	}
}

int rfc2045_cdecode_end(struct rfc2045 *p)
{
	return ((*p->decode_func)(p, NULL, 0));
}

int rfc2045_cdecode(struct rfc2045 *p, const char *s, size_t l)
{
	if (s && l)	return ((*p->decode_func)(p, s, l));
	return (0);
}
