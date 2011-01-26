#ifndef	rfc2646_h
#define	rfc2646_h
/*
** Copyright 2000-2011 Double Precision, Inc.  See COPYING for
** distribution information.
*/

/*
** $Id: rfc2646.h,v 1.9 2011/01/15 16:14:37 mrsam Exp $
*/

#include	"../rfc2045/rfc2045_config.h" /* VPATH */
#include	"../unicode/unicode.h"
#include	<stdlib.h>
#include	<string.h>

#ifdef  __cplusplus
extern "C" {
#endif

/*
** These functions are used to handle format=flowed text/plain MIME content.
**
** The first set of functions is used to parse this content into a usable
** form.
*/

struct rfc2646parser {

	int quote_depth;
	int parse_mode;
	char line[4096];
	int linelen;

	int (*handler)(struct rfc2646parser *, int, void *);
	void *voidarg;
} ;

/*
** Initially, rfc2646_alloc is used to allocate and initialize the
** rfc2646parser structure.
*/

struct rfc2646parser *rfc2646_alloc( int (*)(struct rfc2646parser *, int,
					     void *), /* Handler func */
				     void *); /* Transparent ptr for func */
/*
** Then, repeatedly invoke rfc2646_parse() to parse the MIME content.
** rfc2646_parse() will repeatedly call the handler function for each
** flowed text line.  The handler function should return zero.  A non-zero
** return immediately aborts parsing.  The return code from rfc2646_parse()
** is the return code from the handler function.
*/

int rfc2646_parse(struct rfc2646parser *, const char *, size_t);

/*
** rfc2646_parse_cb() is a convenient wrapper that can be used as the callback
** arg to rfc2045_cdecode_start().
*/

int rfc2646_parse_cb(const char *, size_t, void *);

/*
** Call rfc2646_free() when there's no more MIME content.  It deallocates
** the rfc2646parser structure.  If there was a partial line, the handler
** function might still be called here.  The return code from rfc2646_free()
** is the return code from the handler function (or 0).
*/

int rfc2646_free(struct rfc2646parser *);

/*-------------------------------------------------------------------------*/

/*
** The following formats an initial reply to flowed content.  It is called
** automatically by rfc2045_makereply, but can be directly used, if so
** desired.
*/

struct rfc2646reply {
	int current_quote_depth;
	int prev_was_flowed;
	int first_line;
	char replybuffer[74];
	int replylen;
	int (*handler)(const char *, size_t, void *);
	void *voidarg;
} ;

/* Use rfc2646reply_alloc() to allocate this structure. */

struct rfc2646reply *
rfc2646reply_alloc( int (*)(const char *, size_t, void *), /* Callback func */
		    void *);	/* Arg to callback func */

/* Next, a separate rfc2646parser structure needs to be created.  The
** callback for rfc2646parser must be rfc2646_reply_handler(), and the void
** ptr must be the ptr to the rfc2646reply structure:
*/

int rfc2646reply_handler(struct rfc2646parser *, int, void *);

/* This way, the parser functions parse the text, and this handler forms the
** reply.  Here's a convenient macro.
*/

#define RFC2646REPLY_PARSEALLOC(p) \
	(rfc2646_alloc(&rfc2646reply_handler, (p)))

/* Now, repeatedly call rfc2646_parse, as usual, to create the reply.
** Finally, call rfc2646_free(), then rfc2646reply_free()
*/

int rfc2646reply_free(struct rfc2646reply *);


/*
** The following set of functions create format=flowed content.  The content
** is created from a "hybrid" draft.  Lines in the hybrid draft that begin
** with '>' are assumed to be already in the flowed text format, and are
** copied to output verbatim.
** The remaining stuff is assumed to be unwrapped text, with newlines
** at the end of paragraphs only.
*/


struct rfc2646create {
	char *buffer;
	size_t bufsize;
	size_t buflen;

	const char *charset;

	int linesize;
	int has_sent_paragraph;
	int last_sent_quotelevel;
	int sent_firsttime;

	int (*handler)(const char *, size_t, void *);
	void *voidarg;
} ;

struct rfc2646create *rfc2646create_alloc( int (*)(const char *, size_t,
						    void *),
					   const char *,
					   void *);

int rfc2646create_parse(struct rfc2646create *,
			const char *, size_t);

int rfc2646create_free(struct rfc2646create *);

/*
** The rfc2646fwd functions are used to create such a hybrid draft.
** They copy quoted text verbatim, unchanged.  Unquoted text is
** combined into one line per paragraph.
**
** This is used when forwarding inline a flowed text message.
*/

struct rfc2646fwd {
	int prev_was_0depth;
	int (*handler)(const char *, size_t, void *);
	void *voidarg;
} ;

struct rfc2646fwd *rfc2646fwd_alloc( int (*)(const char *, size_t, void *),
					   void *);
int rfc2646fwd_handler(struct rfc2646parser *, int, void *);
int rfc2646fwd_free(struct rfc2646fwd *);

#define RFC2646FWD_PARSEALLOC(p) \
	(rfc2646_alloc(&rfc2646fwd_handler, (p)))

/*
** The following functions rewrap flowed text into a different line width.
** The callback function receives a ptr to the following structure.  It will
** find the 0-terminated string in wrap_buf, with a quote depth of
** quote_depth.  The callback function should ignore the rest of this stuff.
*/

struct rfc2646rewrap {
	int has_prev;
	int quote_depth;

	size_t wrap_width;
	char *wrap_buf;
	size_t wrap_buflen;
	int (*handler)(struct rfc2646rewrap *, void *);
	void *voidarg;
} ;

struct rfc2646rewrap *rfc2646rewrap_alloc( size_t,
					   int (*)(struct rfc2646rewrap *,
						   void *),
					   void *);
int rfc2646rewrap_handler(struct rfc2646parser *, int, void *);
int rfc2646rewrap_free(struct rfc2646rewrap *);

#define RFC2646REWRAP_PARSEALLOC(p) \
	(rfc2646_alloc(&rfc2646rewrap_handler, (p)))

#ifdef  __cplusplus
}
#endif

#endif
