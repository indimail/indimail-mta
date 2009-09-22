#ifndef	rfc2047_h
#define	rfc2047_h

/*
** Copyright 1998 - 2002 Double Precision, Inc.  See COPYING for
** distribution information.
*/

#ifdef  __cplusplus
extern "C" {
#endif


static const char rfc2047_h_rcsid[]="$Id: rfc2047.h,v 1.8 2004/05/23 14:28:24 mrsam Exp $";

extern int rfc2047_decode(const char *text,
			  int (*func)(const char *, int,
				      const char *,
				      const char *,
				      void *),
			  void *arg);

extern char *rfc2047_decode_simple(const char *text);

extern char *rfc2047_decode_enhanced(const char *text, const char *mychset);

/*
** If libunicode.a is available, like rfc2047_decode_enhanced, but attempt to
** convert to my preferred charset.
*/

struct unicode_info;

extern char *rfc2047_decode_unicode(const char *text,
	const struct unicode_info *mychset,
	int options);

#define	RFC2047_DECODE_DISCARD	1
	/* options: Discard unknown charsets from decoded string. */
#define RFC2047_DECODE_ABORT	2
	/* options: Abort if we encounter an unknown charset, errno=EINVAL */
#define RFC2047_DECODE_NOTAG	4
	/* options: Do not tag unknown charset strings */
#define	RFC2047_DECODE_REPLACE	8
	/* options: Replace unknown characters */

/*
** rfc2047_print is like rfc822_print, except that it converts RFC 2047
** MIME encoding to 8 bit text.
*/

struct rfc822a;

void rfc2047_print(const struct rfc822a *a,
	const char *charset,
	void (*print_func)(char, void *),
	void (*print_separator)(const char *, void *), void *);

void rfc2047_print_unicode(const struct rfc822a *a,
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
** rfc2047_encode_header allocates a buffer, and MIME-encodes an RFC822 header
**
*/
char *rfc2047_encode_header(const struct rfc822a *a,
			    const char *charset);

#ifdef  __cplusplus
}
#endif

#endif
