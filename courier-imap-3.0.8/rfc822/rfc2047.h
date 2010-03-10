#ifndef	rfc2047_h
#define	rfc2047_h

#include	<stdlib.h>
/*
** Copyright 1998 - 2009 Double Precision, Inc.  See COPYING for
** distribution information.
*/

#ifdef  __cplusplus
extern "C" {
#endif


static const char rfc2047_h_rcsid[]="$Id: rfc2047.h,v 1.12 2009/11/14 21:15:43 mrsam Exp $";

struct unicode_info;

/*
** Raw RFC 2047 parser.
**
** rfc2047_decoder() repeatedly invokes the callback function, passing it
** the decoded RFC 2047 string that's given as an argument.
*/

int rfc2047_decoder(const char *text,
		    void (*callback)(const char *chset,
				     const char *lang,
				     const char *content,
				     size_t cnt,
				     void *dummy),
		    void *ptr);

/*
** rfc2047_print_unicodeaddr is like rfc822_print, except that it converts
** RFC 2047 MIME encoding to 8 bit text.
*/

struct rfc822a;

int rfc2047_print_unicodeaddr(const struct rfc822a *a,
			      const char *charset,
			      void (*print_func)(char, void *),
			      void (*print_separator)(const char *, void *),
			      void *ptr);


/*
** And now, let's encode something with RFC 2047.  Encode the following
** string in the indicated character set, into a malloced buffer.  Returns 0
** if malloc failed.
*/

char *rfc2047_encode_str(const char *str, const char *charset,
			 int (*qp_allow)(char c) /* See below */);

/*
** If you can live with the encoded text being generated on the fly, use
** rfc2047_encode_callback, which calls a callback function, instead of
** dynamically allocating memory.
*/

int rfc2047_encode_callback(const char *str, /* String to encode */
			    const char *charset, /* Native charset */
			    int (*qp_allow)(char c),
			    /* Return true if c can appear in QP-encoded
			    ** word */
			    int (*cb_func)(const char *, size_t, void *),
			    /* Callback function. */
			    void *arg
			    /* Passthrough arg to callback_function */
			    );

/* Potential arguments for qp_allow */

int rfc2047_qp_allow_any(char); /* Any character */
int rfc2047_qp_allow_comment(char); /* Any character except () */
int rfc2047_qp_allow_word(char); /* See RFC2047, bottom of page 7 */



/*
** rfc2047_encode_header allocates a buffer, and MIME-encodes a header.
**
** The name of the header, passed as the first parameter, should be
** "From", "To", "Subject", etc... It is not included in the encoded contents.
*/
char *rfc2047_encode_header_tobuf(const char *name, /* Header name */
				  const char *header, /* Header's contents */
				  const char *charset);

/*
** rfc2047_encode_header_addr allocates a buffer, and MIME-encodes an
** RFC822 address header.
**
*/
char *rfc2047_encode_header_addr(const struct rfc822a *a,
				 const char *charset);

#ifdef  __cplusplus
}
#endif

#endif
