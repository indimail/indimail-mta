/*
** Copyright 2000-2003 Double Precision, Inc.
** See COPYING for distribution information.
**
** $Id: unicode2.c,v 1.3 2003/03/07 00:47:31 mrsam Exp $
*/

#include	"unicode_config.h"
#include	"unicode.h"
#include	<string.h>
#include	<ctype.h>
#include	<stdlib.h>

char *unicode_toutf8(const unicode_char *u)
{
	return (unicode_utf8_fromu(u, 0));
}

unicode_char *unicode_fromutf8(const char *c)
{
	return (unicode_utf8_tou(c, 0));
}

char *unicode_ctoutf8(const struct unicode_info *ui, const char *c,
		      int *err)
{
	unicode_char *uc= (*ui->c2u)(ui, c, err);
	char *p;

	if (!uc) return (0);

	p=unicode_utf8_fromu(uc, err);
	if (err && *err > 0)
		*err=0;

	free(uc);
	return (p);
}

char *unicode_cfromutf8(const struct unicode_info *ui, const char *c,
			int *err)
{
	unicode_char *uc;
	char *p;

	uc=unicode_utf8_tou(c, err);
	if (!uc) return (0);

	p=(*ui->u2c)(ui, uc, err);
	free(uc);
	if (err && *err > 0)
		*err=0;
	return (p);
}
