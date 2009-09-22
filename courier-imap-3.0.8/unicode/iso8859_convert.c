/*
** Copyright 2000-2003 Double Precision, Inc.
** See COPYING for distribution information.
**
** $Id: iso8859_convert.c,v 1.2 2003/03/07 00:47:31 mrsam Exp $
*/

#include	"unicode_config.h"
#include	"unicode.h"
#include	<string.h>
#include	<stdlib.h>


char *unicode_iso8859_convert(const char *str, int *err,
					const char *table)
{
char *p=strdup(str), *q;	/* Lazy bum */

	if (err)	*err= -1;

	if (!p)	return (0);

	for (q=p; *q; q++)
	{
	char c= table[(int)(unsigned char)*q];

		if (!c)
		{
			if (err)
			{
				*err= q-p;
				free(p);
				return (0);
			}
		}
		else
			*q=c;
	}
	return (p);
}
