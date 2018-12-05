/*
** Copyright 2011 Double Precision, Inc.
** See COPYING for distribution information.
**
*/

#include	"unicode_config.h"
#include	"courier-unicode.h"

#define BLOCK_SIZE	256

uint8_t unicode_tab_lookup(char32_t ch,
			   const size_t *unicode_indextab,
			   size_t unicode_indextab_sizeof,
			   const uint8_t (*unicode_rangetab)[2],
			   const uint8_t *unicode_classtab,
			   uint8_t uclass)
{
	size_t cl=ch / BLOCK_SIZE;

	if (cl < unicode_indextab_sizeof-1)
	{
		const size_t start_pos=unicode_indextab[cl];
		const uint8_t (*p)[2]=unicode_rangetab + start_pos;
		size_t b=0, e=unicode_indextab[cl+1] - start_pos;
		uint8_t chmodcl= ch & (BLOCK_SIZE-1);

		while (b < e)
		{
			size_t n=b + (e-b)/2;

			if (chmodcl >= p[n][0])
			{
				if (chmodcl <= p[n][1])
				{
					uclass=unicode_classtab[start_pos+n];
					break;
				}
				b=n+1;
			}
			else
			{
				e=n;
			}
		}
	}

	return uclass;
}

uint32_t unicode_tab32_lookup(char32_t ch,
			      const size_t *unicode_indextab,
			      size_t unicode_indextab_sizeof,
			      const uint8_t (*unicode_rangetab)[2],
			      const uint32_t *unicode_classtab,
			      uint32_t uclass)
{
	size_t cl=ch / BLOCK_SIZE;

	if (cl < unicode_indextab_sizeof-1)
	{
		const size_t start_pos=unicode_indextab[cl];
		const uint8_t (*p)[2]=unicode_rangetab + start_pos;
		size_t b=0, e=unicode_indextab[cl+1] - start_pos;
		uint32_t chmodcl= ch & (BLOCK_SIZE-1);

		while (b < e)
		{
			size_t n=b + (e-b)/2;

			if (chmodcl >= p[n][0])
			{
				if (chmodcl <= p[n][1])
				{
					uclass=unicode_classtab[start_pos+n];
					break;
				}
				b=n+1;
			}
			else
			{
				e=n;
			}
		}
	}

	return uclass;
}
