/*
** Copyright 2011 Double Precision, Inc.
** See COPYING for distribution information.
**
*/

#include	"unicode_config.h"
#include	"courier-unicode.h"
#include	<stdlib.h>
#include	<string.h>

void unicode_buf_init(struct unicode_buf *p, size_t max)
{
	p->ptr=0;
	p->size=0;
	p->len=0;
	p->max=max;
}

void unicode_buf_deinit(struct unicode_buf *p)
{
	if (p->ptr)
		free(p->ptr);
}

int unicode_buf_append(struct unicode_buf *p,
		       const char32_t *uc, size_t l)
{
	if (l > p->max-p->len)
		l=p->max-p->len;

	if (p->len + l > p->size)
	{
		size_t n=(p->len + l) * 2;
		char32_t *newp;

		if (n < 256)
			n=256;

		if (n > p->max)
			n=p->max;

		newp=p->ptr ? realloc(p->ptr, n * sizeof(char32_t))
			: malloc(n * sizeof(char32_t));

		if (!newp)
			return -1;

		p->ptr=newp;
		p->size=n;
	}

	memcpy(p->ptr + p->len, uc, l * sizeof(char32_t));

	p->len += l;
	return 0;
}

void unicode_buf_append_char(struct unicode_buf *dst,
			     const char *str,
			     size_t cnt)
{
	char32_t unicode_buf[256];

	while (cnt)
	{
		size_t n=sizeof(unicode_buf)/sizeof(unicode_buf[0]), i;

		if (n > cnt)
			n=cnt;

		for (i=0; i<n; ++i)
			unicode_buf[i]=(unsigned char)str[i];

		str += n;
		cnt -= n;
		unicode_buf_append(dst, unicode_buf, i);
	}
}

void unicode_buf_remove(struct unicode_buf *p,
			size_t pos,
			size_t cnt)
{
	if (pos > p->len)
		pos=p->len;

	if (cnt > p->len-pos)
		cnt=p->len-pos;

	if (cnt)
		memmove(p->ptr+pos+cnt, p->ptr+pos, p->len-pos-cnt);
	p->len -= cnt;
}

int unicode_buf_cmp(const struct unicode_buf *a,
		    const struct unicode_buf *b)
{
	size_t i;

	for (i=0; i<a->len && i<b->len; i++)
	{
		if (a->ptr[i] < b->ptr[i])
			return -1;
		if (a->ptr[i] > b->ptr[i])
			return 1;
	}

	return (a->len < b->len ? -1:a->len > b->len ? 1:0);
}

int unicode_buf_cmp_str(const struct unicode_buf *p, const char *c,
			size_t cl)
{
	size_t i;

	for (i=0; i<p->len && i < cl; ++i)
	{
		if (p->ptr[i] < c[i])
			return -1;

		if (p->ptr[i] > c[i])
			return 1;
	}

	return (p->len < cl ? -1: p->len > cl ? 1:0);
}
