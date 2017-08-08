/*
** Copyright 2011 Double Precision, Inc.
** See COPYING for distribution information.
**
*/

#include	"unicode_config.h"
#include	"courier-unicode.h"
#include	<string.h>
#include	<stdlib.h>
#include	<stdio.h>
#include	<errno.h>


int main(int argc, char **argv)
{
	if (argc >= 3)
	{
		printf("%d\n",
		       unicode_grapheme_break(strtol(argv[1], NULL, 0),
					      strtol(argv[2], NULL, 0)));
	}
	return (0);
}
