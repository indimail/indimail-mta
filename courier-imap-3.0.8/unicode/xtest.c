
/*
** Copyright 2000 Double Precision, Inc.
** See COPYING for distribution information.
**
** $Id: xtest.c,v 1.2 2002/11/18 00:54:22 mrsam Exp $
*/

#include "unicode.h"

/*
** This map is for testing purposes.  Funny stuff.
*/

extern const struct unicode_info unicode_ISO8859_1;

static unicode_char *c2u(const struct unicode_info *u, const char *cp, int *ip)
{
	unicode_char *uc= (*unicode_ISO8859_1.c2u)(&unicode_ISO8859_1, cp, ip);
	int i;

	for (i=0; uc && uc[i]; i++)
		if (uc[i] >= 0x40 && uc[i] <= 0x7d)
			uc[i] ^= 1;
	return (uc);
}

static char *u2c(const struct unicode_info *u, const unicode_char *cp, int *ip)
{
	char *uc= (*unicode_ISO8859_1.u2c)(&unicode_ISO8859_1,cp, ip);

	int i;

	for (i=0; uc && uc[i]; i++)
		if (uc[i] >= 0x40 && uc[i] <= 0x7d)
			uc[i] ^= 1;
	return (uc);
}

static char *toupper_func(const struct unicode_info *u,
			  const char *cp, int *ip)
{
	return ( (*unicode_ISO8859_1.toupper_func)(&unicode_ISO8859_1,cp, ip));
}

static char *tolower_func(const struct unicode_info *u,
			  const char *cp, int *ip)
{
	return ( (*unicode_ISO8859_1.tolower_func)(&unicode_ISO8859_1,cp, ip));
}

static char *totitle_func(const struct unicode_info *u,
			  const char *cp, int *ip)
{
	return ( (*unicode_ISO8859_1.totitle_func)(&unicode_ISO8859_1,cp, ip));
}

const struct unicode_info unicode_XTEST = {
	"X-TEST",
	0,
	c2u,
	u2c,
	toupper_func,
	tolower_func,
	totitle_func};
