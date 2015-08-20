/*
** Copyright 1998 - 2011 Double Precision, Inc.  See COPYING for
** distribution information.
*/

/*
*/
#ifndef	rfc2045_h
#define	rfc2045_h

#include	"rfc2045/rfc2045_config.h" /* VPATH build */
#include	"numlib/numlib.h"
#include	<sys/types.h>
#include	<string.h>
#include	<stdio.h>

#ifdef  __cplusplus
extern "C" {
#endif

#if 0
}
#endif

#define	RFC2045_ISMIME1(p)	((p) && atoi(p) == 1)
#define	RFC2045_ISMIME1DEF(p)	(!(p) || atoi(p) == 1)

struct rfc2045 {
	struct rfc2045 *parent;
	unsigned pindex;
	struct rfc2045 *next;

	off_t	startpos,	/* At which offset in msg this section starts */
		endpos,		/* Where it ends */
		startbody,	/* Where the body of the msg starts */
		endbody;	/* endpos - trailing CRLF terminator */
	off_t	nlines;		/* Number of lines in message */
	off_t	nbodylines;	/* Number of lines only in the body */
	char *mime_version;
	char *content_type;
	struct rfc2045attr *content_type_attr;	/* Content-Type: attributes */

	char *content_disposition;
	char *boundary;
	struct rfc2045attr *content_disposition_attr;
	char *content_transfer_encoding;
	int content_8bit;		/*
					** Set if content_transfer_encoding is
					** 8bit
					*/
	char *content_id;
	char *content_description;
	char *content_language;
	char *content_md5;
	char *content_base;
	char *content_location;
	struct  rfc2045ac *rfc2045acptr;
	int	has8bitchars;	/* For rewriting */
	int	haslongline;	/* For rewriting */
	unsigned rfcviolation;	/* Boo-boos */

#define	RFC2045_ERR8BITHEADER	1	/* 8 bit characters in headers */
#define	RFC2045_ERR8BITCONTENT	2	/* 8 bit contents, but no 8bit
					content-transfer-encoding */
#define	RFC2045_ERR2COMPLEX	4	/* Too many nested contents */
#define RFC2045_ERRBADBOUNDARY	8	/* Overlapping MIME boundaries */

	unsigned numparts;	/* # of parts allocated */

	char	*rw_transfer_encoding;	/* For rewriting */

#define	RFC2045_RW_7BIT	1
#define	RFC2045_RW_8BIT	2

	/* Subsections */

	struct rfc2045 *firstpart, *lastpart;

	/* Working area */

	char *workbuf;
	size_t workbufsize;
	size_t workbuflen;
	int	workinheader;
	int	workclosed;
	int	isdummy;
	int	informdata;	/* In a middle of a long form-data part */
	char *header;
	size_t headersize;
	size_t headerlen;

	int	(*decode_func)(struct rfc2045 *, const char *, size_t);
	void	*misc_decode_ptr;
	int	(*udecode_func)(const char *, size_t, void *);
} ;

struct rfc2045attr {
	struct rfc2045attr *next;
	char *name;
	char *value;
	} ;

struct rfc2045 *rfc2045_alloc();
void rfc2045_parse(struct rfc2045 *, const char *, size_t);
void rfc2045_parse_partial(struct rfc2045 *);
void rfc2045_free(struct rfc2045 *);

void rfc2045_mimeinfo(const struct rfc2045 *,
	const char **,
	const char **,
	const char **);

const char *rfc2045_boundary(const struct rfc2045 *);
int rfc2045_isflowed(const struct rfc2045 *);
int rfc2045_isdelsp(const struct rfc2045 *);
char *rfc2045_related_start(const struct rfc2045 *);
const char *rfc2045_content_id(const struct rfc2045 *);
const char *rfc2045_content_description(const struct rfc2045 *);
const char *rfc2045_content_language(const struct rfc2045 *);
const char *rfc2045_content_md5(const struct rfc2045 *);

void rfc2045_mimepos(const struct rfc2045 *, off_t *, off_t *, off_t *,
	off_t *, off_t *);
unsigned rfc2045_mimepartcount(const struct rfc2045 *);

void rfc2045_xdump(struct rfc2045 *);

struct rfc2045id {
	struct rfc2045id *next;
	int idnum;
} ;

void rfc2045_decode(struct rfc2045 *,
		    void (*)(struct rfc2045 *, struct rfc2045id *, void *),
		    void *);

struct rfc2045 *rfc2045_find(struct rfc2045 *, const char *);


/*
** Source of an rfc2045-formatted content (internal)
*/

struct rfc2045src {
	void (*deinit_func)(void *);

	int (*seek_func)(off_t pos, void *);
	ssize_t (*read_func)(char *buf, size_t cnt, void *);

	void *arg;
};
/* Read from a filedesc, returns a malloced buffer */

struct rfc2045src *rfc2045src_init_fd(int fd);

/* Destroy a rfc2045src */

void rfc2045src_deinit(struct rfc2045src *);

/************************/

void rfc2045_cdecode_start(struct rfc2045 *,
	int (*)(const char *, size_t, void *), void *);
int rfc2045_cdecode(struct rfc2045 *, const char *, size_t);
int rfc2045_cdecode_end(struct rfc2045 *);

const char *rfc2045_getdefaultcharset();
void rfc2045_setdefaultcharset(const char *);
struct rfc2045 *rfc2045_fromfd(int);
#define	rfc2045_fromfp(f)	(rfc2045_fromfd(fileno((f))))
struct rfc2045 *rfc2045header_fromfd(int);
#define        rfc2045header_fromfp(f)        (rfc2045header_fromfd(fileno((f))))

extern void rfc2045_error(const char *);


struct  rfc2045ac {
	void (*start_section)(struct rfc2045 *);
	void (*section_contents)(const char *, size_t);
	void (*end_section)();
	} ;

struct rfc2045 *rfc2045_alloc_ac();
int rfc2045_ac_check(struct rfc2045 *, int);
int rfc2045_rewrite(struct rfc2045 *p, struct rfc2045src *src, int fdout_arg,
		    const char *appname);
int rfc2045_rewrite_func(struct rfc2045 *p, struct rfc2045src *src,
			 int (*funcarg)(const char *, int, void *),
			 void *funcargarg,
			 const char *appname);

/* Internal functions */

int rfc2045_try_boundary(struct rfc2045 *, struct rfc2045src *, const char *);
char *rfc2045_mk_boundary(struct rfc2045 *, struct rfc2045src *);
const char *rfc2045_getattr(const struct rfc2045attr *, const char *);
int rfc2045_attrset(struct rfc2045attr **, const char *, const char *);

/* MIME content base/location */

char *rfc2045_content_base(struct rfc2045 *p);
	/* This joins Content-Base: and Content-Location:, as best as I
	** can figure it out.
	*/

char *rfc2045_append_url(const char *, const char *);
	/* Do this with two arbitrary URLs */

/* MISC mime functions */

struct rfc2045 *rfc2045_searchcontenttype(struct rfc2045 *, const char *);
	/* Assume that the "real" message text is the first MIME section here
	** with the given content type.
	*/

int rfc2045_decodemimesection(struct rfc2045src *, /* Message to decode */
			      struct rfc2045 *,	/* MIME section to decode */
			      int (*)(const char *, size_t, void *),
			      /*
			      ** Callback function that receives decoded
			      ** content.
			      */
			      void *	/* 3rd arg to the callback function */
			      );
/*
** Decode a given MIME section.
*/

int rfc2045_decodetextmimesection(struct rfc2045src *, /* Message to decode */
				  struct rfc2045 *, /* MIME section */
				  const char *,	/* Convert to this character set */
				  int *, /* Set to non-0 if MIME section contained chars that could not be converted to the requested charset */
				  int (*)(const char *, size_t, void *),
				  /*
				  ** Callback function that receives decoded
				  ** content.
				  */
				  void * /* 3rd arg to the callback function */
			      );
	/*
	** Like decodemimesction(), except that the text is automatically
	** convert to the specified character set (this function falls back
	** to decodemimesection() if libunicode.a is not available, or if
	** either the specified character set, or the MIME character set
	** is not supported by libunicode.a
	*/


	/*
	** READ HEADERS FROM A MIME SECTION.
	**
	** Call rfc2045header_start() to allocate a structure for the given
	** MIME section.
	**
	** Call rfc2045header_get() to repeatedly get the next header.
	** Function returns < 0 for a failure (out of memory, or something
	** like that).  Function returns 0 for a success.  Example:
	**
	** rfc2045header_get(ptr, &header, &value, 0);
	**
	** If success: check if header is NULL - end of headers, else
	** "header" and "value" will contain the RFC 822 header.
	**
	** Last argument is flags:
	*/

#define RFC2045H_NOLC 1		/* Do not convert header to lowercase */
#define RFC2045H_KEEPNL 2	/* Preserve newlines in the value string
				** of multiline headers.
				*/

struct rfc2045headerinfo *
	rfc2045header_start(struct rfc2045src *,/* Readonly source */
			    struct rfc2045 *	/* MIME section to read */
			    );

int rfc2045header_get(struct rfc2045headerinfo *,
		      char **,	/* Header return */
		      char **,	/* Value return */
		      int);	/* Flags */

void rfc2045header_end(struct rfc2045headerinfo *);


/*
** Generic MIME header parsing code.
**
** header - something like "text/plain; charset=us-ascii; format=flowed".
**
** header_type_cb - callback function, receives the "text/plain" parameter.
**
** header_param_cb - callback function, repeatedly invoked to process the
** additional parameters.  In this example, receives "charset" and "us-ascii".
** Note -t he first parameter will always be in lowercase.
**
** void_arg - passthrough parameter to the callback functions.
*/

int rfc2045_parse_mime_header(const char *header,
			      void (*header_type_cb)(const char *, void *),
			      void (*header_param_cb)(const char *,
						      const char *,
						      void *),
			      void *void_arg);

/*
** The rfc2045_makereply function is used to generate an initial
** reply to a MIME message.  rfc2045_makereply takes the following
** structure:
*/

struct rfc2045_mkreplyinfo {

	struct rfc2045src *src; /* Original message source */

	struct rfc2045 *rfc2045partp;
	/*
	** rfc2045 structure for the message to reply.  This may actually
	** represent a single message/rfc822 section within a larger MIME
	** message digest, in which case we format a reply to this message.
	*/

	void *voidarg;	/* Transparent argument passed to the callback
			** functions.
			*/

	/*
	** The following callback functions are called to generate the reply
	** message.  They must be initialized.
	*/

	void (*write_func)(const char *, size_t, void *);
	/* Called to write out the content of the message */

	void (*writesig_func)(void *);
	/* Called to write out the sender's signature */

	int (*myaddr_func)(const char *, void *);
	/* myaddr_func receives a pointer to an RFC 822 address, and it
	** should return non-zero if the address is the sender's address
	*/

	const char *replymode;
	/*
	** replymode must be initialized to one of the following.  It sets
	** the actual template for the generated response.
	**
	** "forward" - forward original message.
	** "forwardatt" - forward original message as an RFC822 attachment
	** "reply" - a standard reply to the original message's sender
	** "replydsn" - a DSN reply to the original message's sender
	** "feedback" - generate a feedback report (RFC 5965)
	** "replyfeedback" - "feedback" to the sender's address.
	** "replyall" - a "reply to all" response.
	** "replylist" - "reply to mailing list" response.  This is a reply
	** that's addressed to the mailing list the original message was sent
	** to.
	*/

	int replytoenvelope;
	/*
	** If non-zero, the "reply" or "replydsn" message gets addressed to the
	** "Return-Path" or "Errors-To" address, if available.
	*/

	int donotquote;

	/*
	** If donotquote is set, the contents of the original message are not
	** quoted by any of the "reply" modes, and replysalut (below) does not
	** get emitted.
	*/

	int fullmsg;
	/*
	** For replydsn, feedback, replyfeedback, attach the entire message
	** instead of just its headers.
	*/

	const char *replysalut;
	/*
	** This should be set to the salutation to be used for the reply.
	** The following %-formats may appear in this string:
	**
	** %% - an explicit % character
	**
	** %n - a newline character
	**
	** %C - the X-Newsgroup: header from the original message
	**
	** %N - the Newsgroups: header from the original message
	**
	** %i - the Message-ID: header from the original message
	**
	** %f - the original message's sender's address
	**
	** %F - the original message's sender's name
	**
	** %S - the Subject: header from the original message
	**
	** %d - the original message's date, in the local timezone
	**
	** %{...}d - use strftime() to format the original message's date.
	**           A plain %d is equivalent to %{%a, %d %b %Y %H:%M:%S %z}d.
	**
	** Example:  "%F writes:"
	*/

	const char *forwarddescr;
	/*
	** For forwardatt, this is the Content-Description: header,
	** (typically "Forwarded message").
	*/

	/*
	** If not NULL, overrides the Subject: header
	*/

	const char *subject;

	/*
	** When reply mode is 'replydsn', dsnfrom must be set to a valid
	** email address that's specified as the address that's generating
	** the DSN.
	*/
	const char *dsnfrom;

	/*
	** When reply mode is 'replyfeedback', feedbacktype must be set to
	** one of the registered feedback types:
	** "abuse", "fraud", "other", "virus".
	*/
	const char *feedbacktype;

	/*
	** Feedback report headers.
	**
	** NOTE: rfc2045_makereply() automatically inserts the
	** Feedback-Type: (from feedbacktype), User-Agent:, Version:, and
	** Arrival-Date: headers.
	**
	** This is an array of alternating header name and header value
	** strings. The header name string does not contain a colon,
	** rfc2045_makereply supplies one. And, basically, generates
	** "name: value" from this list.
	**
	** For convenience-sake, the capitalization of the headers get
	** adjusted to match the convention in RFC 5965.
	**
	** The list, which must contain an even number of strings, is terminated
	** by a NULL pointer.
	*/
	const char * const *feedbackheaders;

	/*
	** Set the reply/fwd MIME headers. If this is a NULL pointer,
	** write_func() receives ``Content-Type: text/plain; format=flowed;
	** delsp=yes; charset="charset" '' with the charset specified below,
	** and "Content-Transfer-Encoding: 8bit".
	**
	** If this is not a NULL pointer, the effect of
	** this function should be invocation of write_func() to perform the
	** analogous purpose.
	**
	** The output of content_set_charset() should be consistent with the
	** contents of the charset field.
	*/

	void (*content_set_charset)(void *);

	/*
	** Set the reply/fwd content.
	**
	** This function gets called at the point where the additional contents
	** of the reply/fwd should go.
	**
	** If this is not a NULL pointer, the effect of this function should
	** be invocation of write_func() with the additional contents of the
	** reply/fwd. The added content should be consistent with the
	** charset field.
	**
	** Note -- this content is likely to end up in a multipart MIME
	** message, as such it should not contain any lines that look like
	** MIME boundaries.
	*/

	void (*content_specify)(void *);

	const char *mailinglists;
	/*
	** This should be set to a whitespace-delimited list of mailing list
	** RFC 822 addresses that the respondent is subscribed to.  It is used
	** to figure out which mailing list the original message was sent to
	** (all addresses in the original message are compared against this
	** list).  In the event that we can't find a mailing list address on
	** the original message, "replylist" will fall back to "replyall".
	*/

	const char *charset;
	/* The respondent's local charset */

	const char *forwardsep;
	/* This is used instead of replysalut for forwards. */
} ;

int rfc2045_makereply(struct rfc2045_mkreplyinfo *);

/********** Search message content **********/

/*
** Callback passed rfc2045_decodemsgtoutf8()
*/

struct rfc2045_decodemsgtoutf8_cb {

	int flags; /* Optional flags, see below */

	/* Define a non-null function pointer. It gets the name of a header,
	** and the raw, unformatted, header contents.
	** If returns non-0, the header gets converted and sent to output.
	** If null, all headers are sent
	*/

	int (*headerfilter_func)(const char *name, const char *raw, void *arg);

	/* The output function */
	int (*output_func)(const char *data, size_t cnt, void *arg);

	/* If not null, gets invoked after decoding a single header */
	int (*headerdone_func)(const char *headername, void *arg);

	void *arg; /* Passthrough arg to _funcs */
};

#define RFC2045_DECODEMSG_NOBODY 0x01
/* Do not decode MIME content, headers only */

#define RFC2045_DECODEMSG_NOHEADERS 0x02
/*
** Do not decode MIME headers, only body. This is the same as using a
** headerfilter_func that always returns 0
*/

#define RFC2045_DECODEMSG_NOHEADERNAME 0x04
/*
** Do not prepend name: to converted header content.
*/

/*
** Convert a message into a utf8 bytestream. The output produced by this
** function is a catentation of decoded header and text content data, converted
** to utf8.
**
** This is fed into an output function. The output function takes a single
** octet, and returns 0 if the octet was processed, or a negative value if
** the output was aborted.
*/

int rfc2045_decodemsgtoutf8(struct rfc2045src *src, /* The message */
			    struct rfc2045 *p, /* The parsed message */

			    /* The callback */
			    struct rfc2045_decodemsgtoutf8_cb *callback);


/********** Decode RFC 2231 attributes ***********/

/*
** rfc2231_decodeType() decodes an RFC 2231-encoded Content-Type: header
** attribute, and rfc2231_decodeDisposition() decodes the attribute in the
** Content-Disposition: header.
**
** chsetPtr, langPtr, and textPtr should point to a char ptr.  These
** functions automatically allocate the memory, the caller's responsible for
** freeing it.  A NULL argument may be provided if the corresponding
** information is not wanted.
*/

int rfc2231_decodeType(struct rfc2045 *rfc, const char *name,
		       char **chsetPtr,
		       char **langPtr,
		       char **textPtr);

int rfc2231_decodeDisposition(struct rfc2045 *rfc, const char *name,
			      char **chsetPtr,
			      char **langPtr,
			      char **textPtr);

/*
** The following two functions convert the decoded string to the local
** charset via unicodelib.  textPtr cannot be null, this time, because this
** is the only return value.   A NULL myChset is an alias for the default
** charset.
*/

int rfc2231_udecodeType(struct rfc2045 *rfc, const char *name,
			const char *myChset,
			char **textPtr);

int rfc2231_udecodeDisposition(struct rfc2045 *rfc, const char *name,
			       const char *myChset,
			       char **textPtr);

/*
** Build an RFC 2231-encoded name*=value.
**
** name, value, charset, language: see RFC 2231.
**
** (*cb_func) gets invoked 1 or more time, receives a "name=value" pair
** each time.
**
** cb_func must return 0; a non-0 return terminates rfc2231_attrCreate, which
** passes through the return code.
**
*/
int rfc2231_attrCreate(const char *name, const char *value,
		       const char *charset,
		       const char *language,
		       int (*cb_func)(const char *param,
				      const char *value,
				      void *void_arg),
		       void *cb_arg);

/** NON-PUBLIC DATA **/

struct rfc2231param {
	struct rfc2231param *next;

	int paramnum;
	int encoded;

	const char *value;
};

void rfc2231_paramDestroy(struct rfc2231param *paramList);
int rfc2231_buildAttrList(struct rfc2231param **paramList,
			  const char *name,

			  const char *attrName,
			  const char *attrValue);

void rfc2231_paramDecode(struct rfc2231param *paramList,
			 char *charsetPtr,
			 char *langPtr,
			 char *textPtr,
			 int *charsetLen,
			 int *langLen,
			 int *textLen);

#if 0
{
#endif

#ifdef  __cplusplus
}
#endif

#endif
