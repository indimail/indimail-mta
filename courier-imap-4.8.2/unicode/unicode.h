#ifndef	unicode_h
#define	unicode_h

/*
** Copyright 2000-2011 Double Precision, Inc.
** See COPYING for distribution information.
**
** $Id: unicode.h,v 1.28 2011/01/22 22:07:05 mrsam Exp $
*/

#ifdef	__cplusplus

#include <string>
#include <vector>

extern "C" {
#endif

#if 0
}
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
#include	<stdint.h>

#include	<sys/types.h>

typedef uint32_t unicode_char;

/*
** The system default character set, from the locale.
*/

extern const char *unicode_default_chset();

/* Unicode upper/lower/title case conversion functions */

extern unicode_char unicode_uc(unicode_char);
extern unicode_char unicode_lc(unicode_char);
extern unicode_char unicode_tc(unicode_char);

/* Return width of unicode character */

extern int unicode_wcwidth(unicode_char c);


/*
** A wrapper for iconv(3). This wrapper provides a different API for iconv(3).
** A handle gets created by libmail_u_convert_init().
** libmail_u_convert_init() receives a pointer to the output function
** which receives converted character text.
**
** The output function receives a pointer to the converted character text, and
** the number of characters in the converted text.
**
** The character text to convert gets passed, repeatedly, to
** libmail_u_convert(). Each call to libmail_u_convert() results in
** the output function being invoked, zero or more times, with the converted
** text. Finally, libmail_u_convert_deinit() stops the conversion and
** deallocates the conversion handle.
**
** Internal buffering takes place. libmail_u_convert_deinit() may result
** in the output function being called one or more times, to receive the final
** part of the converted character stream.
**
** The output function should return 0. A non-0 value causes
** libmail_u_convert() and/or libmail_u_convert_deinit() returning
** non-0.
*/

struct libmail_u_convert_hdr;

typedef struct libmail_u_convert_hdr *libmail_u_convert_handle_t;

/*
** libmail_u_convert_init() returns a non-NULL handle for the requested
** conversion, or NULL if the requested conversion is not available.
*/

libmail_u_convert_handle_t
libmail_u_convert_init(/* Convert from this chset */
		       const char *src_chset,

		       /* Convert to this chset */
		       const char *dst_chset,

		       /* The output function */

		       int (*output_func)(const char *, size_t, void *),

		       /* Passthrough arg */
		       void *convert_arg);

/*
** Repeatedly pass the character text to convert to libmail_u_convert().
**
** Returns non-0 if the output function returned non-0, or 0 if all invocations
** of the output function returned 0.
*/

int libmail_u_convert(/* The conversion handle */
		      libmail_u_convert_handle_t handle,

		      /* Text to convert */
		      const char *text,

		      /* Number of bytes to convert */
		      size_t cnt);

/*
** Finish character set conversion. The handle gets deallocated.
**
** May still result in one or more invocations of the output function.
** Returns non-zero if any previous invocation of the output function returned
** non-zero (this includes any invocations of the output function resulting
** from this call, or prior libmail_u_convert() calls), or 0 if all
** invocations of the output function returned 0.
**
** If the errptr is not NULL, *errptr is set to non-zero if there were any
** conversion errors -- if there was any text that could not be converted to
** the destination character text.
*/

int libmail_u_convert_deinit(libmail_u_convert_handle_t handle,
			     int *errptr);


/*
** Specialization: save converted character text in a buffer.
**
** Implementation: call libmail_u_convert_tocbuf_init() instead of
** libmail_u_convert_init(), then call libmail_u_convert() and
** libmail_u_convert_deinit(), as usual.
**
** If libmail_u_convert_deinit() returns 0, *cbufptr_ret gets initialized to a
** malloc()ed buffer, and the number of converted characters, the size of the
** malloc()ed buffer, are placed into *csize_ret arguments, that were passed
** to libmail_u_convert_tou_init().
**
** Note: if the converted string is an empty string, *cbufsize_ret is set to 0,
** but *cbufptr_ptr still gets initialized (to a dummy malloced buffer).
**
** The optional nullterminate places a trailing \0 character after the
** converted string (this is included in *cbufsize_ret).
*/

libmail_u_convert_handle_t
libmail_u_convert_tocbuf_init(/* Convert from this chset */
			      const char *src_chset,

			      /* Convert to this chset */
			      const char *dst_chset,

			      /* malloced buffer */
			      char **cbufptr_ret,

			      /* size of the malloced buffer */
			      size_t *cbufsize_ret,

			      /* null terminate the resulting string */
			      int nullterminate
			      );


/*
** Specialization: convert some character text to a unicode_char array.
**
** This is like libmail_u_convert_tocbuf_init(), but converts to a unicode_char
** array.
**
** The returned *ucsize_ret is initialized with the number of unicode_chars,
** rather than the byte count.
**
** In all other ways, this function behaves identically to
** libmail_u_convert_tocbuf_init().
*/

libmail_u_convert_handle_t
libmail_u_convert_tou_init(/* Convert from this chset */
			   const char *src_chset,

			   /* malloc()ed buffer pointer, on exit. */
			   unicode_char **ucptr_ret,

			   /* size of the malloc()ed buffer, upon exit */
			   size_t *ucsize_ret,

			   /* If true, terminate with U+0x0000, for convenience */
			   int nullterminate
			   );

/*
** Specialization: convert a unicode_char array to some character text.
**
** This is the opposite of libmail_u_convert_tou_init(). Call this to
** initialize the conversion handle, then use libmail_u_convert_uc()
** instead of libmail_u_convert.
*/

libmail_u_convert_handle_t
libmail_u_convert_fromu_init(/* Convert to this chset */
			     const char *dst_chset,

			     /* malloc()ed buffer pointer, on exit. */
			     char **cbufptr_ret,

			     /* size of the malloc()ed buffer, upon exit */
			     size_t *cbufsize_ret,

			     /* If true, terminate with U+0x0000, for convenience */
			     int nullterminate
			     );

int libmail_u_convert_uc(/* The conversion handle */
			 libmail_u_convert_handle_t handle,

			 /* Text to convert */
			 const unicode_char *text,

			 /* Number of bytes to convert */
			 size_t cnt);

/*
** Initialize conversion to UTF-8.
**
** This is a wrapper for libmail_u_convert_tocbuf_init() that specifies the
** destination charset as UTF-8.
*/

libmail_u_convert_handle_t
libmail_u_convert_tocbuf_toutf8_init(const char *src_chset,
				     char **cbufptr_ret,
				     size_t *cbufsize_ret,
				     int nullterminate);

/*
** Initialize conversion from UTF-8.
**
** This is a wrapper for libmail_u_convert_tocbuf_init() that specifies the
** source charset as UTF-8.
*/

libmail_u_convert_handle_t
libmail_u_convert_tocbuf_fromutf8_init(const char *dst_chset,
				       char **cbufptr_ret,
				       size_t *cbufsize_ret,
				       int nullterminate);

/*
** Convert a character string to UTF-8.
**
** Returns a malloc-ed buffer holding the UTF-8 string, or NULL if an
** error occured.
*/
char *libmail_u_convert_toutf8(/* Text to convert to UTF-8 */
			       const char *text,

			       /* Character set to convert to UTF-8 */
			       const char *charset,

			       /*
			       ** If non-NULL, and a non-NULL pointer is
			       ** returned, *error is set to non-zero if
			       ** a character conversion error has occured.
			       */
			       int *error);

/*
** Convert UTF-8 text to another character set.
**
** Returns a malloc-ed buffer holding the string converted to the specified
** character set, or NULL if an error occured.
*/

char *libmail_u_convert_fromutf8(/* A UTF-8 string */
				 const char *text,

				 /*
				 ** Convert the UTF-8 string to this character
				 ** set.
				 */

				 const char *charset,

				 /*
				 ** If non-NULL, and a non-NULL pointer is
				 ** returned, *error is set to non-zero if
				 ** a character conversion error has occured.
				 */
				 int *error);

/*
** Convert one charset to another charset, placing the result in a malloc-ed
** buffer.
**
** Returns a malloc-ed buffer holding the string converted to the specified
** character set, or NULL if an error occured.
*/

char *libmail_u_convert_tobuf(/* A string to convert */
			      const char *text,

			      /*
			      ** String's charset.
			      */

			      const char *charset,

			      /*
			      ** Destination charset
			      */
			      const char *dstcharset,

			      /*
			      ** If non-NULL, and a non-NULL pointer is
			      ** returned, *error is set to non-zero if
			      ** a character conversion error has occured.
			      */
			      int *error);

/*
** Convenience function: call libmail_u_convert_tou_init(), feed the
** character string through libmail_u_convert(), then call
** libmail_u_convert_deinit().
**
** If this function returns 0, *uc and *ucsize is set to a malloced buffer+size
** holding the unicode char array.
*/

int libmail_u_convert_tou_tobuf(/* Character text to convert */
				const char *text,

				/* Number of characters */
				size_t text_l,

				/* text's charset */
				const char *charset,

				/*
				** If this function returns 0, this gets
				** initialized
				*/
				unicode_char **uc,

				/*
				** Size of the allocated buffer
				*/
				size_t *ucsize,

				/*
				** If not null and this function returns 0,
				** this is set to non-0 if there
				** was a conversion error (but the output
				** buffer gets still allocated and
				** initialized)
				*/
				int *err);

/*
** Convenience function: call libmail_u_convert_fromu_init(), feed the
** unicode_array through libmail_u_convert_uc(), then call
** libmail_u_convert_deinit().
**
** If this function returns 0, *uc and *ucsize is set to a malloced buffer+size
** holding the converted character string
*/

int libmail_u_convert_fromu_tobuf(/* Unicode array to convert to a char str */
				  const unicode_char *utext,

				  /*
				  ** Size of the unicode array.
				  ** If this is (size_t)-1, utext is a
				  ** 0-terminated array.
				  */
				  size_t utext_l,

				  /*
				  ** Convert the unicode array to this charset.
				  */
				  const char *charset,

				  /*
				  ** If libmail_u_convert_fromu_tobuf()
				  ** returns 0, this is initialized to a
				  ** malloced buffer with a 0-terminated
				  ** string is kept.
				  */
				  char **c,

				  /*
				  ** Size of the initialized array, including
				  ** the 0-terminator.
				  */
				  size_t *csize,

				  /*
				  ** If libmail_u_convert_fromu_tobuf()
				  ** returns 0 and this is not NULL,
				  ** *err is set to non-0 if there was a
				  ** conversion error to the requested
				  ** character set.
				  */
				  int *err);

/*
** Convenience function: convert a string in a given character set
** to/from uppercase, lowercase, or something else.
**
** This is done by calling libmail_u_convert_tou_tobuf() first,
** applying the title_func and char_func, then using
** libmail_u_convert_fromu_tobuf().
**
** A NULL return indicates that the requested conversion cannot be performed.
*/

char *libmail_u_convert_tocase( /* String to convert */
			       const char *str,

			       /* String's character set */

			       const char *charset,

			       /*
			       ** Conversion of the first character in
			       ** str: unicode_uc, unicode_lc, or unicode_tc:
			       */

			       unicode_char (*first_char_func)(unicode_char),

			       /*
			       ** Conversion of the second and the remaining
			       ** character in str. If NULL, same as
			       ** first_char_func.
			       */
			       unicode_char (*char_func)(unicode_char));



/* Either UCS-4BE or UCS-4LE, matching the native unicode_char endianness */

extern const char libmail_u_ucs4_native[];

/* Either UCS-2BE or UCS-2LE, matching the native unicode_char endianness */

extern const char libmail_u_ucs2_native[];

/*
** Modified-UTF7 encoding used for IMAP folder names. Pass it for a charset
** parameter.
**
** This can be followed by a " " and up to 15 characters to be escaped in
** addition to unicode chars.
*/

#define unicode_x_imap_modutf7 "x-imap-modutf7"

#if 0
{
#endif

#ifdef	__cplusplus
}

namespace mail {

	class iconvert {

		libmail_u_convert_handle_t handle;

	public:
		iconvert();
		~iconvert();

		bool begin(const std::string &src_chset,
			   const std::string &dst_chset);

		bool operator()(const char *, size_t);
		virtual int converted(const char *, size_t);

		bool end()
		{
			return end(NULL);
		}

		bool end(int &errptr)
		{
			return end(&errptr);
		}

		/* Convert between two different charsets */

		static std::string convert(const std::string &text,
					   const std::string &charset,
					   const std::string &dstcharset,
					   bool &errflag);

		/* Convert between two different charsets */

		static std::string convert(const std::string &text,
					   const std::string &charset,
					   const std::string &dstcharset)
		{
			bool dummy;

			return convert(text, charset, dstcharset, dummy);
		}

		/* Convert from unicode to a charset */

		static std::string convert(const std::vector<unicode_char> &uc,
					   const std::string &dstcharset,
					   bool &errflag);

		/* Convert from unicode to a charset */

		static std::string convert(const std::vector<unicode_char> &uc,
					   const std::string &dstcharset)
		{
			bool dummy;

			return convert(uc, dstcharset, dummy);
		}

		/* Convert charset to unicode */

		static bool convert(const std::string &text,
				    const std::string &charset,
				    std::vector<unicode_char> &uc);


		/* Convert to upper/lower/title case */

		static std::string
			convert_tocase(/* Text string */
				       const std::string &text,

				       /* Its charset */
				       const std::string &charset,

				       /* First character: unicode_uc, unicode_lc, or unicode_tc */
				       unicode_char (*first_char_func)(unicode_char),

				       /* If not NULL, second and subsequent chars */
				       unicode_char (*char_func)(unicode_char)
				       =NULL)
		{
			bool dummy;

			return convert_tocase(text, charset, dummy,
					      first_char_func,
					      char_func);
		}

		/* Convert to upper/lower/title case */

		static std::string
			convert_tocase(/* Text string */
				       const std::string &text,

				       /* Its charset */
				       const std::string &charset,

				       /* Set if there's a conversion error */
				       bool &err,

				       /* First character: unicode_uc, unicode_lc, or unicode_tc */
				       unicode_char (*first_char_func)(unicode_char),

				       /* If not NULL, second and subsequent chars */
				       unicode_char (*char_func)(unicode_char)
				       =NULL);
	private:
		bool end(int *);
	};
}
#endif

#endif
