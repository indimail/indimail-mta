/*
** Copyright 2011 Double Precision, Inc.
** See COPYING for distribution information.
**
*/

#include	"unicode_config.h"
#include	"courier-unicode.h"

#include <stdlib.h>

struct i {
	size_t n_start;
	size_t n_size;
	char32_t v;
};

#include "unicode_htmlent.h"


static int compar(const void *key, const void *obj)
{
	size_t j;
	const char *p=n + ((const struct i *)obj)->n_start;
	const char *ckey=(const char *)key;

	for (j=0; j<((const struct i *)obj)->n_size; ++j)
	{
		if (*ckey < *p)
			return -1;

		if (*ckey > *p)
			return 1;

		++p;
		++ckey;
	}

	if (*ckey)
		return 1;

	return 0;
}

char32_t unicode_html40ent_lookup(const char *n)
{
	const struct i *ptr;

	if (*n == '#')
	{
		const char *p=n;
		char32_t uc;
		char *endptr;

		++p;

		if (*p == 'x' || *p == 'X')
		{
			if (*++p)
			{
				uc=strtoull(p, &endptr, 16);

				if (*endptr == 0)
					return uc;
			}
		}

		uc=strtoull(p, &endptr, 10);

		if (*endptr == 0)
			return uc;
	}

	ptr=(const struct i *)bsearch(n, ii,
				      sizeof(ii)/sizeof(ii[0]),
				      sizeof(ii[0]), compar);

	if (ptr)
		return ptr->v;
	return 0;
}
