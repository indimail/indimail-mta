/*
** Copyright 2011 Double Precision, Inc.
** See COPYING for distribution information.
**
*/

#include	"unicode_config.h"
#include	"unicode.h"

#include <stdlib.h>

struct i {
	size_t n_start;
	size_t n_size;
	unicode_char v;
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

unicode_char unicode_html40ent_lookup(const char *n)
{
	const struct i *ptr=
		(const struct i *)bsearch(n, ii,
					  sizeof(ii)/sizeof(ii[0]),
					  sizeof(ii[0]), compar);

	if (ptr)
		return ptr->v;
	return 0;
}
