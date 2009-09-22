/*
** Copyright 2000-2003 Double Precision, Inc.
** See COPYING for distribution information.
**
** $Id: ibm864convert.c,v 1.2 2003/03/07 00:47:31 mrsam Exp $
*/

#include	"unicode_config.h"
#include	"unicode.h"
#include	<string.h>
#include	<stdlib.h>

/* IBM864 has this pesky 0x25 mapping charsets all share the same functions */

unicode_char *unicode_ibm864_c2u(const char *str, int *err,
				 const unicode_char *table)
{
size_t l=strlen(str);
unicode_char *p=(unicode_char *)malloc((l+1) * sizeof(unicode_char));

	if (err)
		*err= -1;

	if (!p)
		return (0);

	for (l=0; str[l]; l++)
	{
		unicode_char c=table[(unsigned char)str[l]];

		if (!c)
		{
			if (err)
			{
				*err=l;
				free(p);
				return (0);
			}
			c=(int)(unsigned char)str[l];
		}
		p[l]=c;
	}
	p[l]=0;
	return (p);
}


char *unicode_ibm864_u2c(const unicode_char *uc, int *errflag,
	const unicode_char *tab)
{
size_t l;
char *p;

	for (l=0; uc[l]; l++)
		;

	if (errflag)	*errflag= -1;
	p=malloc(l+1);
	if (!p)
		return (0);

	for (l=0; uc[l]; l++)
	{
		int c;
		unicode_char ucc=uc[l];

		/* First, guess */

		if (tab[ ucc & 0xFF ] == uc[l])
			c=(int)(ucc & 0xFF);
		else
		{
			for (c=0; c<256; c++)
				if (tab[c] == uc[l])
					break;
			if (c >= 256)
			{
				if (errflag)
				{
					*errflag=l;
					free(p);
					return (0);
				}
				c=uc[l];
			}
		}
		if (c == 0)
			c=255;
		p[l]=(char)c;
	}
	p[l]=0;
	return (p);
}
