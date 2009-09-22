/*
** Copyright 2001-2003 Double Precision, Inc.
** See COPYING for distribution information.
**
** $Id: windows874u.c,v 1.2 2003/03/07 00:47:31 mrsam Exp $
*/

#include	"unicode_config.h"
#include	"unicode.h"
#include	<string.h>
#include	<stdlib.h>

char *unicode_windows874_u2c(const unicode_char *uc, int *errflag,
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

		if ((ucc & 0x7F) == ucc)
			c=(unsigned char)ucc;
		else if (tab[ ucc & 0x7F ] == ucc)
			c=(int)(ucc & 0x7F) | 0x80;
		else if (ucc > 0x0E00 && ucc < 0x0E60 &&
			 tab[ (ucc + 0x20) & 0x7F] == ucc)
			c=(int)( (ucc + 0x20) & 0x7F) | 0x80;
		else
		{
			for (c=0; c<128; c++)
				if (tab[c] == uc[l])
					break;
			if (c >= 128)
			{
				if (errflag)
				{
					*errflag=l;
					free(p);
					return (0);
				}
				c=uc[l];
			}
			c |= 0x80;
		}
		if (c == 0)
			c=255;
		p[l]=(char)c;
	}
	p[l]=0;
	return (p);
}
