/*
** Copyright 2015 Double Precision, Inc.
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
	if (argc >= 2)
	{
		printf("%d\n", (int)unicode_script(strtol(argv[1], NULL, 0)));
	}
	return (0);
}
