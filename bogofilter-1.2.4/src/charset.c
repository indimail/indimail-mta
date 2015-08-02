/* $Id: charset.c 6917 2010-07-05 15:59:15Z m-a $ */

/*****************************************************************************

NAME:
   charset.c -- provide charset support for bogofilter's lexer.

Note:

   Character translation is done to make life easier for the lexer.
   Text is changed only after the message has been saved for
   passthrough.  The end user (mail reader) never sees any changes -
   only the lexer.

AUTHOR:
   David Relson <relson@osagesoftware.com>

******************************************************************************/

#include "common.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "charset.h"
#include "convert_charset.h"
#ifndef	DISABLE_UNICODE
#include "convert_unicode.h"
#endif
#include "xmalloc.h"
#include "xstrdup.h"

#define	SP	' '

#ifndef	DEFAULT_CHARSET
#define	DEFAULT_CHARSET	"us-ascii"
#endif

const char *charset_default = DEFAULT_CHARSET;
const char *charset_unicode = "UTF-8";

byte charset_table[256];
byte casefold_table[256];

#define	DEBUG
#undef	DEBUG

#ifndef	DEBUG
#define	PRINT_CHARSET_TABLE
#else
#define	PRINT_CHARSET_TABLE	print_charset_table()
#undef	DEBUG_GENERAL
#define	DEBUG_GENERAL(level)	(verbose >= level)
#endif

#ifdef	DEBUG
static void print_charset_table(void)
{
    int c,r,i;
    char ch;
    if (!DEBUG_GENERAL(1))
	return;
    printf( "\n" );
    for (r=0; r<4; r+=1) {
	for (c=0; c<64; c+=1) {
	    i=r*64+c;
	    ch=charset_table[i];
	    if (ch != 0x08 && ch != 0x09 && ch != '\n' && ch != '\r')
		printf(" %02X.%2c.%02X", i, ch, ch);
	    else
		printf(" %02X.%02X.%02X", i, ch, ch);
	    if ((c & 15) == 15)
		printf( "\n" );
	}
    }
    printf( "\n" );
}
#endif

void init_charset_table(const char *charset_name)
{
#ifdef	DISABLE_UNICODE
    init_charset_table_orig(charset_name);
#else
#ifdef	ENABLE_UNICODE
    init_charset_table_iconv(charset_name, charset_unicode);
#else
    if (encoding != E_UNICODE)
	init_charset_table_orig(charset_name);
    else
	init_charset_table_iconv(charset_name, charset_unicode);
#endif
#endif
    return;
}

/* like set_charset() but charset is in form blabla="CharsetName" */
void got_charset(const char *charset)
{
    set_charset(strchr(charset, '=') + 1);
}

/* like got_charset() but charset is pure charset name */
void set_charset(const char *charset)
{
    bool q = (charset[0] == '"');
    char *s, *d;
    char *t = xstrdup(charset + q);

    for (s = d = t; *s != '\0'; s++)
    {
	char c = tolower((unsigned char)*s);	/* map upper case to lower */
	if (c == '_')		/* map underscore to dash */
	    c = '-';
	if (c == '-' &&		/* map "iso-" to "iso"     */
	    d - t == 3 &&	/* ensure 3 chars avail    */
	    memcmp(t, "iso", 3) == 0)
	    continue;
	if (q && c == '"')
	    break;
	*d++ = c;
    }
    *d = '\0';
    if (DEBUG_CONFIG(0))
       fprintf(dbgout, "got_charset( '%s' )\n", t);
    init_charset_table( t );
    xfree(t);
}
