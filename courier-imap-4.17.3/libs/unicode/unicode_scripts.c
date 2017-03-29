/*
** Copyright 2015 Double Precision, Inc.
** See COPYING for distribution information.
**
*/

#include	"unicode_config.h"
#include	"unicode.h"
#include "scriptstab.h"

unicode_script_t unicode_script(unicode_char a)
{
	return unicode_tab_lookup(a, unicode_indextab,
				  sizeof(unicode_indextab)
				  /sizeof(unicode_indextab[0]),
				  unicode_rangetab,
				  unicode_classtab,
				  unicode_script_unknown);
}
