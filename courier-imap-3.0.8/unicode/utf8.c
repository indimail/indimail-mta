
/*
** Copyright 2000-2002 Double Precision, Inc.
** See COPYING for distribution information.
**
** $Id: utf8.c,v 1.4 2002/11/18 00:54:22 mrsam Exp $
*/

#include "unicode.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unicode_char *unicode_utf8_tou(const char *cp, int *ip)
{
size_t l;
size_t n=1;
unicode_char *p, uc;

	for (l=0; cp[l]; ++n)
	{
		if ((cp[l] & 0x80) == 0)
		{
			++l;
			continue;
		}

		if ((cp[l] & 0xE0) == 0xC0)
		{
			if ((cp[l+1] & 0xC0) == 0x80)
			{
				l += 2;
				continue;
			}
		}

		if ((cp[l] & 0xF0) == 0xE0)
		{
			if ((cp[l+1] & 0xC0) == 0x80 &&
				(cp[l+2] & 0xC0) == 0x80)
			{
				l += 3;
				continue;
			}
		}

		if ((cp[l] & 0xF8) == 0xF0)
		{
			if ((cp[l+1] & 0xC0) == 0x80 &&
				(cp[l+2] & 0xC0) == 0x80 &&
				(cp[l+3] & 0xC0) == 0x80)
			{
				l += 4;
				continue;
			}
		}

		if ((cp[l] & 0xFC) == 0xF8)
		{
			if ((cp[l+1] & 0xC0) == 0x80 &&
				(cp[l+2] & 0xC0) == 0x80 &&
				(cp[l+3] & 0xC0) == 0x80 &&
				(cp[l+4] & 0xC0) == 0x80)
			{
				l += 5;
				continue;
			}
		}

		if ((cp[l] & 0xFE) == 0xFC)
		{
			if ((cp[l+1] & 0xC0) == 0x80 &&
				(cp[l+2] & 0xC0) == 0x80 &&
				(cp[l+3] & 0xC0) == 0x80 &&
				(cp[l+4] & 0xC0) == 0x80 &&
				(cp[l+5] & 0xC0) == 0x80)
			{
				l += 6;
				continue;
			}
		}

		if (ip)
		{
			*ip= l;
			return (0);
		}
		++l;
	}
	if (ip)
		*ip = -1;
	if ((p=malloc(n*sizeof(unicode_char))) == 0)
		return (0);
	n=0;

	for (l=0; cp[l]; p[n++]=uc)
	{
		if ((cp[l] & 0x80) == 0)
		{
			uc=cp[l];
			++l;
			continue;
		}

		if ((cp[l] & 0xE0) == 0xC0)
		{
			if ((cp[l+1] & 0xC0) == 0x80)
			{
				uc=cp[l] & 0x1F;
				uc <<= 6; uc |= cp[l+1] & 0x3F;
				l += 2;
				continue;
			}
		}

		if ((cp[l] & 0xF0) == 0xE0)
		{
			if ((cp[l+1] & 0xC0) == 0x80 &&
				(cp[l+2] & 0xC0) == 0x80)
			{
				uc=cp[l] & 0x0F;
				uc <<= 6; uc |= cp[l+1] & 0x3F;
				uc <<= 6; uc |= cp[l+2] & 0x3F;
				l += 3;
				continue;
			}
		}

		if ((cp[l] & 0xF8) == 0xF0)
		{
			if ((cp[l+1] & 0xC0) == 0x80 &&
				(cp[l+2] & 0xC0) == 0x80 &&
				(cp[l+3] & 0xC0) == 0x80)
			{
				uc=cp[l] & 0x07;
				uc <<= 6; uc |= cp[l+1] & 0x3F;
				uc <<= 6; uc |= cp[l+2] & 0x3F;
				uc <<= 6; uc |= cp[l+3] & 0x3F;
				l += 4;
				continue;
			}
		}

		if ((cp[l] & 0xFC) == 0xF8)
		{
			if ((cp[l+1] & 0xC0) == 0x80 &&
				(cp[l+2] & 0xC0) == 0x80 &&
				(cp[l+3] & 0xC0) == 0x80 &&
				(cp[l+4] & 0xC0) == 0x80)
			{
				uc=cp[l] & 0x03;
				uc <<= 6; uc |= cp[l+1] & 0x3F;
				uc <<= 6; uc |= cp[l+2] & 0x3F;
				uc <<= 6; uc |= cp[l+3] & 0x3F;
				uc <<= 6; uc |= cp[l+4] & 0x3F;
				l += 5;
				continue;
			}
		}

		if ((cp[l] & 0xFE) == 0xFC)
		{
			if ((cp[l+1] & 0xC0) == 0x80 &&
				(cp[l+2] & 0xC0) == 0x80 &&
				(cp[l+3] & 0xC0) == 0x80 &&
				(cp[l+4] & 0xC0) == 0x80 &&
				(cp[l+5] & 0xC0) == 0x80)
			{
				uc=cp[l] & 0x01;
				uc <<= 6; uc |= cp[l+1] & 0x3F;
				uc <<= 6; uc |= cp[l+2] & 0x3F;
				uc <<= 6; uc |= cp[l+3] & 0x3F;
				uc <<= 6; uc |= cp[l+4] & 0x3F;
				uc <<= 6; uc |= cp[l+5] & 0x3F;
				l += 6;
				continue;
			}
		}
		uc=cp[l];
		++l;
	}
	p[n]=0;
	return (p);
}

char *unicode_utf8_fromu(const unicode_char *cp, int *ip)
{
	char *p=0;
	int pass;
	size_t l=0;

	for (pass=0; pass<2; pass++)
	{
		if (pass)
		{
			p=malloc(l+1);
			if (!p)
			{
				if (ip)	*ip= -1;
				return (0);
			}
		}

		l=unicode_utf8_fromu_pass(cp, p);
		if (pass)
			p[l]=0;
	}
	return (p);
}


size_t unicode_utf8_fromu_pass(const unicode_char *cp, char *p)
{
	size_t l=0;
	unicode_char uc;

	l=0;

	while (cp && *cp)
	{
		uc= *cp++;

		if ((unicode_char)uc ==
		    (unicode_char)(uc & 0x007F))
		{
			if (p)
			{
				p[l]= (char)uc;
			}
			++l;
			continue;
		}

		if ((unicode_char)uc ==
		    (unicode_char)(uc & 0x07FF))
		{
			if (p)
			{
				p[l+1]=(char)(uc & 0x3F) | 0x80;
				uc >>= 6;
				p[l]= (char)(uc & 0x1F) | 0xC0;
			}
			l += 2;
			continue;
		}

		if ((unicode_char)uc ==
		    (unicode_char)(uc & 0x00FFFF))
		{
			if (p)
			{
				p[l+2]=(char)(uc & 0x3F) | 0x80;
				uc >>= 6;
				p[l+1]=(char)(uc & 0x3F) | 0x80;
				uc >>= 6;
				p[l]= (char)(uc & 0x0F) | 0xE0;
			}
			l += 3;
			continue;
		}

		if ((unicode_char)uc ==
		    (unicode_char)(uc & 0x001FFFFF))
		{
			if (p)
			{
				p[l+3]=(char)(uc & 0x3F) | 0x80;
				uc >>= 6;
				p[l+2]=(char)(uc & 0x3F) | 0x80;
				uc >>= 6;
				p[l+1]=(char)(uc & 0x3F) | 0x80;
				uc >>= 6;
				p[l]= (char)(uc & 0x07) | 0xF0;
			}
			l += 4;
			continue;
		}

		if ((unicode_char)uc ==
		    (unicode_char)(uc & 0x03FFFFFF))
		{
			if (p)
			{
				p[l+4]=(char)(uc & 0x3F) | 0x80;
				uc >>= 6;
				p[l+3]=(char)(uc & 0x3F) | 0x80;
				uc >>= 6;
				p[l+2]=(char)(uc & 0x3F) | 0x80;
				uc >>= 6;
				p[l+1]=(char)(uc & 0x3F) | 0x80;
				uc >>= 6;
				p[l]= (char)(uc & 0x03) | 0xF8;
			}
			l += 5;
			continue;
		}

		if (p)
		{
			p[l+5]=(char)(uc & 0x3F) | 0x80;
			uc >>= 6;
			p[l+4]=(char)(uc & 0x3F) | 0x80;
			uc >>= 6;
			p[l+3]=(char)(uc & 0x3F) | 0x80;
			uc >>= 6;
			p[l+2]=(char)(uc & 0x3F) | 0x80;
			uc >>= 6;
			p[l+1]=(char)(uc & 0x3F) | 0x80;
			uc >>= 6;
			p[l]= (char)(uc & 0x01) | 0xFC;
		}
		l += 6;
	}
	return l;
}
