#ifndef	unicode_h
#define	unicode_h

/*
** Copyright 2000-2001 Double Precision, Inc.
** See COPYING for distribution information.
**
** $Id: unicode.h,v 1.18 2008/07/20 16:24:52 mrsam Exp $
*/

#ifdef	__cplusplus
extern "C" {
#endif

#include	"../unicode/unicode_config.h" /* VPATH build */

#include	<stdlib.h>

#include	<stdio.h>
#if HAVE_WCHAR_H
#include	<wchar.h>
#endif

#if HAVE_STDDEF_H
#include	<stddef.h>
#endif

typedef wchar_t unicode_char;

struct unicode_info {
	const char *chset;		/* Official character set */
	int flags;			/* Flags */

#define UNICODE_UTF	1		/* Direct UTF mapping */
#define UNICODE_MB	2		/* Multibyte characters present */
#define UNICODE_SISO	4	/*
				** Composite mapping, using shift in/out
				** (verbatim text comparison may not work,
				** must convert to UTF, or something).
				** (replaces search_chset).
				*/

#define UNICODE_USASCII 8	/* Character set is a US-ASCII superset */
#define	UNICODE_REPLACEABLE	16	/*
				 * Conversion errors can be replaced by
				 * adequate placeholders (replacement
				 * characters).
				 */
#define	UNICODE_HEADER_QUOPRI	32	/*
				** Quoted-printable (Q) encoding is preferred
				** for MIME message headers.
				*/
#define	UNICODE_HEADER_BASE64	64	/*
				** Base64 (B) encoding is preferred
				** for MIME message headers.
				*/
#define	UNICODE_BODY_QUOPRI	128	/*
				** Quoted-printable (Q) encoding is preferred
				** MIME message body.
				*/
#define	UNICODE_BODY_BASE64	256	/*
				** Base64 (B) encoding is preferred
				** for MIME message body.
				*/

	unicode_char *(*c2u)(const struct unicode_info *, const char *, int *);
		/* Convert character string in this charset to unicode */

	char *(*u2c)(const struct unicode_info *, const unicode_char *, int *);
		/* Convert unicode to character string in this charset */

	/* Convert the string in this character set to upper/lower/titlecase */

	char *(*toupper_func)(const struct unicode_info *,
			      const char *, int *);
	char *(*tolower_func)(const struct unicode_info *,
			      const char *, int *);
	char *(*totitle_func)(const struct unicode_info *,
			      const char *, int *);

	const struct unicode_info *search_chset;
	} ;

extern const struct unicode_info unicode_ISO8859_1;
extern const struct unicode_info unicode_UTF8;
extern const struct unicode_info unicode_IMAP_MODUTF7;

extern char *unicode_iso8859_u2c(const unicode_char *, int *,
	const unicode_char *);

extern char *unicode_windows874_u2c(const unicode_char *, int *,
	const unicode_char *);

/* ISO8859 charsets all share the same functions */

extern unicode_char *unicode_iso8859_c2u(const char *, int *,
					const unicode_char *);

extern char *unicode_iso8859_convert(const char *, int *,
					const char *);

/* IBM864 charset has some funkiness */

unicode_char *unicode_ibm864_c2u(const char *, int *,
				 const unicode_char *);

char *unicode_ibm864_u2c(const unicode_char *, int *,
			 const unicode_char *);


struct unicode_chsetlist {
	const char *chsetname;
	const struct unicode_info *ptr;
	} ;

extern const struct unicode_chsetlist unicode_chsetlist[];
extern const char *unicode_default_chset();
extern const struct unicode_info *unicode_find(const char *);

/*
** UTF8 functions
*/

	/* Convert Unicode to/from UTF-8 */

extern char *unicode_toutf8(const unicode_char *);
extern unicode_char *unicode_fromutf8(const char *);

	/* Unicode upper/lower/title case conversion functions */

extern unicode_char unicode_uc(unicode_char);
extern unicode_char unicode_lc(unicode_char);
extern unicode_char unicode_tc(unicode_char);

	/* Convert charsets to/from UTF-8 */

extern char *unicode_ctoutf8(const struct unicode_info *, const char *,
			     int *);
extern char *unicode_cfromutf8(const struct unicode_info *, const char *,
			       int *);


	/* Return width of unicode character */

extern int unicode_wcwidth(unicode_char c);

	/* Internal functions: */

extern unicode_char *unicode_utf8_tou(const char *, int *);
extern char *unicode_utf8_fromu(const unicode_char *, int *);

size_t unicode_utf8_fromu_pass(const unicode_char *, char *);

#define UNICODE_UTF8_MAXLEN	6

extern char *unicode_convert(const char *txt,
			     const struct unicode_info *from,
			     const struct unicode_info *to);
	/* errno=EINVAL if conversion could not be performed */

extern char *unicode_xconvert(const char *txt,
			      const struct unicode_info *from,
			      const struct unicode_info *to);
	/* Like unicode_convert(), except unconvertable chars are replaced
	** by periods (or something similar), instead of aborting with EINVAL
	*/


extern char *unicode_convert_fromchset(const char *txt,
				    const char *from,
				    const struct unicode_info *to);
	/* Like, unicode_convert, except that we search for a character set
	** from a list of chsets we support.
	** errno=EINVAL if 'to' character set does not exist.
	*/

	/*
	** Convert between unicode and modified-UTF7 encoding used for
	** IMAP folder names.
	*/

unicode_char *unicode_modutf7touc(const char *s, int *err);

	/* err < 0 if out of memory, else ptr to first illegal modutf7-char */
	/* This can be used to test if string is a valid mod-utf7 string */

char *unicode_uctomodutf7(const unicode_char *);

char *unicode_uctomodutf7x(const unicode_char *, const unicode_char *);

#ifdef	__cplusplus
}
#endif

#endif
