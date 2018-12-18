/*
*/
#ifndef	rfc822_h
#define	rfc822_h

/*
** Copyright 1998 - 2009 Double Precision, Inc.
** See COPYING for distribution information.
*/

#if	HAVE_CONFIG_H
#include	"rfc822/config.h"
#endif

#include	<time.h>

#ifdef  __cplusplus
extern "C" {
#endif

#define RFC822_SPECIALS "()<>[]:;@\\,.\""

/*
** The text string we want to parse is first tokenized into an array of
** struct rfc822token records.  'ptr' points into the original text
** string, and 'len' has how many characters from 'ptr' belongs to this
** token.
*/

struct rfc822token {
	struct rfc822token *next;	/* Unused by librfc822, for use by
					** clients */
	int token;
/*
  Values for token:

  '(' - comment
  '"' - quoted string
  '<', '>', '@', ',', ';', ':', '.', '[', ']', '%', '!', '=', '?', '/' - RFC atoms.
  0   - atom
*/

#define	rfc822_is_atom(p)	( (p) == 0 || (p) == '"' || (p) == '(' )

	const char *ptr;	/* Pointer to value for the token. */
	int len;		/* Length of token value */
} ;

/*
** After the struct rfc822token array is built, it is used to create
** the rfc822addr array, which is the array of addresses (plus
** syntactical fluff) extracted from those text strings.  Each rfc822addr
** record has several possible interpretation:
**
** tokens is NULL - syntactical fluff, look in name/nname for tokens
**                  representing the syntactical fluff ( which is semicolons
**                  and  list name:
**
** tokens is not NULL - actual address.  The tokens representing the actual
**                  address is in tokens/ntokens.  If there are comments in
**                  the address that are possible "real name" for the address
**                  they are saved in name/nname (name may be null if there
**                  is none).
**                  If nname is 1, and name points to a comment token,
**                  the address was specified in old-style format.  Otherwise
**                  the address was specified in new-style route-addr format.
**
** The tokens and name pointers are set to point to the original rfc822token
** array.
*/

struct rfc822addr {
	struct rfc822token *tokens;
	struct rfc822token *name;
} ;

/***************************************************************************
**
** rfc822 tokens
**
***************************************************************************/

struct rfc822t {
	struct rfc822token *tokens;
	int	ntokens;
} ;

struct rfc822t *rfc822t_alloc_new(const char *p,
	void (*err_func)(const char *, int, void *), void *);
	/* Parse addresses */

void rfc822t_free(struct rfc822t *);		/* Free rfc822 structure */

void rfc822tok_print(const struct rfc822token *, void (*)(char, void *), void *);
						/* Print the tokens */

/***************************************************************************
**
** rfc822 addresses
**
***************************************************************************/

struct rfc822a {
	struct rfc822addr *addrs;
	int	naddrs;
} ;

struct rfc822a *rfc822a_alloc(struct rfc822t *);
void rfc822a_free(struct rfc822a *);		/* Free rfc822 structure */

void rfc822_deladdr(struct rfc822a *, int);

/* rfc822_print "unparses" the rfc822 structure.  Each rfc822addr is "printed"
   (via the attached function).  NOTE: instead of separating addresses by
   commas, the print_separator function is called.
*/

int rfc822_print(const struct rfc822a *a,
	void (*print_func)(char, void *),
	void (*print_separator)(const char *, void *), void *);

/* rfc822_print_common is an internal function */

int rfc822_print_common(const struct rfc822a *a,
			 char *(*decode_func)(const char *, const char *, int),
			 const char *chset,
			 void (*print_func)(char, void *),
			 void (*print_separator)(const char *, void *), void *);

/* Extra functions */

char *rfc822_gettok(const struct rfc822token *);
char *rfc822_getaddr(const struct rfc822a *, int);
char *rfc822_getaddrs(const struct rfc822a *);
char *rfc822_getaddrs_wrap(const struct rfc822a *, int);

void rfc822_mkdate_buf(time_t, char *);
const char *rfc822_mkdate(time_t);

int rfc822_parsedate_chk(const char *, time_t *);

#define CORESUBJ_RE 1
#define CORESUBJ_FWD 2

char *rfc822_coresubj(const char *, int *);
char *rfc822_coresubj_nouc(const char *, int *);
char *rfc822_coresubj_keepblobs(const char *s);

/*
** Display a header. Takes a raw header value, and formats it for display
** in the given character set.
**
** hdrname -- header name. Determines whether the header contains addresses,
**            or unstructured data.
**
** hdrvalue -- the actual value to format.
**
** display_func -- output function.
**
** err_func -- if this function returns a negative value, to indicate an error,
** this may be called just prior to the error return to indicate where the
** formatting error is, in the original header.
**
** ptr -- passthrough last argument to display_func or err_func.
**
** repeatedly invokes display_func to pass the formatted contents.
**
** Returns 0 upon success, -1 upon a failure.
*/

int rfc822_display_hdrvalue(const char *hdrname,
			    const char *hdrvalue,
			    const char *charset,
			    void (*display_func)(const char *, size_t,
						 void *),
			    void (*err_func)(const char *, int, void *),
			    void *ptr);

/*
** Like rfc822_display_hdrvalue, except that the converted header is saved in
** a malloc-ed buffer. The pointer to the malloc-ed buffer is returned, the
** caller is responsible for free-ing it. An error condition is indicated
** by a NULL return value.
*/

char *rfc822_display_hdrvalue_tobuf(const char *hdrname,
				    const char *hdrvalue,
				    const char *charset,
				    void (*err_func)(const char *, int,
						     void *),
				    void *ptr);

/*
** Display a recipient's name in a specific character set.
**
** The index-th recipient in the address structure is formatted for the given
** character set. If the index-th entry in the address structure is not
** a recipient address (it represents an obsolete list name indicator),
** this function reproduces it literally.
**
** If the index-th entry in the address structure is a recipient address without
** a name, the address itself is formatted for the given character set.
**
** If 'charset' is NULL, the name is formatted as is, without converting
** it to any character set.
**
** A callback function gets repeatedly invoked to produce the name.
**
** Returns a negative value upon a formatting error.
*/

int rfc822_display_name(const struct rfc822a *rfcp, int index,
			const char *chset,
			void (*print_func)(const char *, size_t, void *),
			void *ptr);

/*
** Display a recipient's name in a specific character set.
**
** Uses rfc822_display_name to place the generated name into a malloc-ed
** buffer. The caller must free it when it is no longer needed.
**
** Returns NULL upon an error.
*/

char *rfc822_display_name_tobuf(const struct rfc822a *rfcp, int index,
				const char *chset);

/*
** Display names of all addresses. Each name is followed by a newline
** character.
**
*/
int rfc822_display_namelist(const struct rfc822a *rfcp,
			    const char *chset,
			    void (*print_func)(const char *, size_t, void *),
			    void *ptr);

/*
** Display a recipient's address in a specific character set.
**
** The index-th recipient in the address structure is formatted for the given
** character set. If the index-th entry in the address structure is not
** a recipient address (it represents an obsolete list name indicator),
** this function produces an empty string.
**
** If 'charset' is NULL, the address is formatted as is, without converting
** it to any character set.
**
** A callback function gets repeatedly invoked to produce the address.
**
** Returns a negative value upon a formatting error.
*/

int rfc822_display_addr(const struct rfc822a *rfcp, int index,
			const char *chset,
			void (*print_func)(const char *, size_t, void *),
			void *ptr);

/*
** Like rfc822_display_addr, but the resulting displayable string is
** saved in a buffer. Returns a malloc-ed buffer, the caller is responsible
** for free()ing it. A NULL return indicates an error.
*/

char *rfc822_display_addr_tobuf(const struct rfc822a *rfcp, int index,
				const char *chset);

/*
** Like rfc822_display_addr, but the user@domain gets supplied in a string.
*/
int rfc822_display_addr_str(const char *tok,
			    const char *chset,
			    void (*print_func)(const char *, size_t, void *),
			    void *ptr);

/*
** Like rfc822_display_addr_str, but the resulting displayable string is
** saved in a buffer. Returns a malloc-ed buffer, the caller is responsible
** for free()ing it. A NULL return indicates an error.
*/
char *rfc822_display_addr_str_tobuf(const char *tok,
				    const char *chset);

/*
** address is a hostname, which is IDN-encoded. 'address' may contain an
** optional 'user@', which is preserved. Returns a malloc-ed buffer, the
** caller is responsible for freeing it.
*/
char *rfc822_encode_domain(const char *address,
			   const char *charset);

#ifdef  __cplusplus
}
#endif

#endif
