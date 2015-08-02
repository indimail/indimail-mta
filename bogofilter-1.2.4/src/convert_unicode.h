/* $Id: convert_unicode.h 6882 2010-03-14 14:16:28Z m-a $ */

/*****************************************************************************

NAME:
   charset_unicode.h -- constants and declarations for charset_unicode.c

AUTHOR:
   David Relson <relson@osagesoftware.com>

******************************************************************************/

#ifndef	CHARSET_UNICODE_H
#define	CHARSET_UNICODE_H

#include <iconv.h>

extern void init_charset_table_iconv(const char *from_charset, 
				     const char *to_charset);

extern iconv_t bf_iconv_open( const char *to_charset, 
			       const char *from_charset );

#if	defined(CP866) && !defined(ENABLE_UNICODE) && !defined(DISABLE_UNICODE)
extern int  decode_and_htmlUNICODE_to_cp866(byte *buf, int len);
#endif

#endif /* CHARSET_UNICODE_H */
