/* $Id: debug.main.c 3447 2003-10-29 18:51:23Z relson $ */

/*****************************************************************************

NAME:
   debug.main.c - shared debug functions

AUTHOR:
   David Relson <relson@osagesoftware.com>

******************************************************************************/

#include "common.h"

#include <stdlib.h>
#include <string.h>

typedef struct mask_char_to_symbol_s {
    const char *str;
    u_int32_t bit;
}  mask_char_to_symbol_t;

static mask_char_to_symbol_t const char_to_symbol[] =
{
    { "g",	BIT_GENERAL },
    { "s",	BIT_SPAMICITY },
    { "d",	BIT_DATABASE },
    { "l",	BIT_LEXER },
    { "w",	BIT_WORDLIST }
};

int main(void)
{
    size_t i;
    for (i=0; i<sizeof(char_to_symbol)/sizeof(char_to_symbol[0]); i += 1)
    {
	const mask_char_to_symbol_t *ptr = char_to_symbol + i;
	set_debug_mask( ptr->str );
	if ( (debug_mask & ptr->bit) != ptr->bit )
	{
	    fprintf(stderr, "debug_mask for '%s' is wrong.\n", ptr->str);
	    exit(EX_ERROR);
	}
    }
    printf("All O.K.\n");
    return 0;
}
