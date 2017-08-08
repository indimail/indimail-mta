/*
** Copyright 2000-2003 Double Precision, Inc.
** See COPYING for distribution information.
**
*/

#include "unicode_config.h"
#include "courier-unicode.h"

extern const unsigned unicode_case_hash;
extern const char32_t unicode_case_tab[][4];
extern const unsigned unicode_case_offset[];

static unsigned find_case(char32_t c)
{
	unsigned idx= c % unicode_case_hash;

	unsigned i=unicode_case_offset[idx];

	char32_t uc;

	--i;

	do
	{
		uc=unicode_case_tab[++i][0];
		if (uc == c)
			return (i);
	} while ( (uc % unicode_case_hash) == idx);

	return (0);
}

char32_t unicode_uc(char32_t c)
{
	unsigned i=find_case(c);

	return (unicode_case_tab[i][0] != c ? c:unicode_case_tab[i][1]);
}

char32_t unicode_lc(char32_t c)
{
	unsigned i=find_case(c);

	return (unicode_case_tab[i][0] != c ? c:unicode_case_tab[i][2]);
}

char32_t unicode_tc(char32_t c)
{
	unsigned i;
	char32_t oc=c;

	c=unicode_lc(c);
	i=find_case(c);

	return (unicode_case_tab[i][0] != c ? oc:unicode_case_tab[i][3]);
}
