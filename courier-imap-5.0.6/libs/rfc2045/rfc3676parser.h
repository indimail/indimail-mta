#ifndef	rfc3676_h
#define	rfc3676_h
/*
** Copyright 2011 Double Precision, Inc.  See COPYING for
** distribution information.
*/

/*
*/

#include	"rfc2045/rfc2045_config.h"
#include	"unicode/courier-unicode.h"
#include	<stdlib.h>
#include	<string.h>

#ifdef  __cplusplus
extern "C" {
#endif

#if 0
}
#endif

typedef struct rfc3676_parser_struct *rfc3676_parser_t;

/*
** Structure passed to rfc3676_parser_init().
*/

struct rfc3676_parser_info {

	const char *charset;
	/*
	** MIME charset parameter. String not used after rfc3676_parser_init()
	** returns.
	*/

	/* MIME format flowed flag set */
	int isflowed;

	/* MIME delsp=yes flag is set */
	int isdelsp;

	/*
	** Callback - start of a line.
	**
	** If this callback returns 0, normal parsing continues. If this
	** callback returns a non-0 value, parsing stops and
	** rfc3676_parse() or rfc3676_deinit() returns the non-0 value.
	*/

	int (*line_begin)(size_t quote_level, /* Line's quote level */
			  void *arg);

	/*
	** Callback - contents of the line, converted to unicode.
	** May be invoked multiple times, consecutively.
	**
	** If this callback returns 0, normal parsing continues. If this
	** callback returns a non-0 value, parsing stops and
	** rfc3676_parse() or rfc3676_deinit() returns the non-0 value.
	*/

	int (*line_contents)(const char32_t *txt, /* Contents */
			     size_t txt_size,
			     /* Count of unicode chars in txt */
			     void *arg);
	/*
	** Optional callback. If not NULL, it gets invoked when
	** a line is logically flowed into the next physical line.
	*/

	int (*line_flowed_notify)(void *);

	/*
	** End of the line's contents.
	**
	** If this callback returns 0, normal parsing continues. If this
	** callback returns a non-0 value, parsing stops and
	** rfc3676_parse() or rfc3676_deinit() returns the non-0 value.
	*/

	int (*line_end)(void *arg);

	/* Argument passed through to the above callback methods */

	void *arg;
};

/*
** Begin parsing.
**
** Returns an opaque parsing handle.
*/
rfc3676_parser_t rfc3676parser_init(const struct rfc3676_parser_info *info);

/*
** Parse next part of rfc3676-encoded message.
**
** Returns non-0 value returned by any callback method, or 0 if all
** invoked callback methods returned 0.
*/

int rfc3676parser(rfc3676_parser_t handle,
		  const char *txt,
		  size_t txt_cnt);

/*
** End parsing.
**
** The handle gets destroyed, and the parsing finishes.
**
** NOTE: rfc3676_deinit() WILL LIKELY invoke some leftover callback methods.
**
** Returns non-0 value returned by any callback method, or 0 if all
** invoked callback methods returned 0.
*/

int rfc3676parser_deinit(rfc3676_parser_t handle,

			 /*
			 ** Optional, if not NULL, set to indicate unicode
			 ** error.
			 */
			 int *errptr);

#if 0
{
#endif

#ifdef  __cplusplus
}

namespace mail {

	extern "C" int tpp_trampoline_line_begin(size_t, void *);

	extern "C" int tpp_trampoline_line_contents(const char32_t *,
						    size_t, void *);

	extern "C" int tpp_trampoline_line_flowed_notify(void *);

	extern "C" int tpp_trampoline_line_end(void *);

	/*
	** C++ binding for the parser logic
	*/
	class textplainparser {

		rfc3676_parser_t handle;

	public:
		textplainparser();
		~textplainparser();

		/*
		** Begin parsing. Returns FALSE if the parsing could
		** not be initialized (probably unknown charset).
		*/
		bool begin(const std::string &charset,
			   bool flowed,
			   bool delsp);

		void end(
			 /*
			 ** Set to true if a unicode conversion error occured.
			 */
			 bool &unicode_errflag);

		void end()
		{
			bool dummy;

			return end(dummy);
		}

		/* Feed raw contents to be parsed */
		void operator<<(const std::string &text)
		{
			if (handle)
				rfc3676parser(handle, text.c_str(),
					      text.size());
		}


		virtual void line_begin(size_t);

		virtual void line_contents(const char32_t *,
					   size_t);

		virtual void line_flowed_notify();

		virtual void line_end();
	};
}
#endif

#endif
