/* $Id: bool.c 4207 2004-03-03 16:29:12Z relson $ */

/*****************************************************************************

NAME:
   bool.c -- functions to support the "bool" type.

AUTHOR:
   David Relson <relson@osagesoftware.com>

******************************************************************************/

#include "common.h"

#include <ctype.h>
#include <stdlib.h>

#include "bool.h"

bool str_to_bool(const char *str)
{
    char ch;

    while (isspace((unsigned char)*str))
	   str += 1;
    ch = toupper((unsigned char)*str);

    switch (ch)
    {
    case 'Y':		/* Yes */
    case 'T':		/* True */
    case '1':
	return (true);
    case 'N':		/* No */
    case 'F':		/* False */
    case '0':
	return (false);
    default:
	fprintf(stderr, "Invalid boolean value - %s\n", str);
	exit(EX_ERROR);
    }
    return (0);
}
