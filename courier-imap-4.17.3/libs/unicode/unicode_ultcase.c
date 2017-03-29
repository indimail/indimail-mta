/*
** Copyright 2000-2003 Double Precision, Inc.
** See COPYING for distribution information.
**
*/

#include "unicode_config.h"
#include "unicode.h"

extern const unsigned unicode_case_hash;
extern const unicode_char unicode_case_tab[][4];
extern const unsigned unicode_case_offset[];

static unsigned find_case(unicode_char c)
{
	unsigned idx= c % unicode_case_hash;

	unsigned i=unicode_case_offset[idx];

	unicode_char uc;

	--i;

	do
	{
		uc=unicode_case_tab[++i][0];
		if (uc == c)
			return (i);
	} while ( (uc % unicode_case_hash) == idx);

	return (0);
}

unicode_char unicode_uc(unicode_char c)
{
	unsigned i=find_case(c);

	return (unicode_case_tab[i][0] != c ? c:unicode_case_tab[i][1]);
}

unicode_char unicode_lc(unicode_char c)
{
	unsigned i=find_case(c);

	return (unicode_case_tab[i][0] != c ? c:unicode_case_tab[i][2]);
}

unicode_char unicode_tc(unicode_char c)
{
	unsigned i;
	unicode_char oc=c;

	c=unicode_lc(c);
	i=find_case(c);

	return (unicode_case_tab[i][0] != c ? oc:unicode_case_tab[i][3]);
}
