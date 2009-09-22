/*
** $Id: rfc822.h,v 1.24 2006/04/11 02:24:59 mrsam Exp $
*/
#ifndef	rfc822_h
#define	rfc822_h

/*
** Copyright 1998 - 2006 Double Precision, Inc.
** See COPYING for distribution information.
*/

#if	HAVE_CONFIG_H
#include	"../rfc822/config.h"	/* VPATH build */
#endif

#include	<time.h>

#ifdef  __cplusplus
extern "C" {
#endif

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

struct rfc822t *rfc822t_alloc(const char *p,
	void (*err_func)(const char *, int));	/* Parse addresses */

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

void rfc822_print(const struct rfc822a *a,
	void (*print_func)(char, void *),
	void (*print_separator)(const char *, void *), void *);

/* rfc822_print_common is an internal function */

void rfc822_print_common(const struct rfc822a *a,
	char *(*decode_func)(const char *, const char *),
	const char *chset,
	void (*print_func)(char, void *),
	void (*print_separator)(const char *, void *), void *);

/* Another unparser, except that only the raw addresses are extracted,
   and each address is followed by a newline character */

void rfc822_addrlist(const struct rfc822a *, void (*print_func)(char, void *),
		void *);

/* Now, just the comments.  If comments not given, the address. */
void rfc822_namelist(const struct rfc822a *, void (*print_func)(char, void *),
		void *);

/* Unparse an individual name/addr from a list of addresses.  If the given
   index points to some syntactical fluff, this is a noop */

void rfc822_prname(const struct rfc822a *, int, void (*)(char, void *), void *);
void rfc822_praddr(const struct rfc822a *, int, void (*)(char, void *), void *);

/* Like rfc822_prname, except that we'll also print the legacy format
** of a list designation.
*/

void rfc822_prname_orlist(const struct rfc822a *, int,
			  void (*)(char, void *), void *);

/* Extra functions */

char *rfc822_gettok(const struct rfc822token *);
char *rfc822_getaddr(const struct rfc822a *, int);
char *rfc822_getname(const struct rfc822a *, int);
char *rfc822_getname_orlist(const struct rfc822a *, int);
char *rfc822_getaddrs(const struct rfc822a *);
char *rfc822_getaddrs_wrap(const struct rfc822a *, int);

void rfc822_mkdate_buf(time_t, char *);
const char *rfc822_mkdate(time_t);
time_t rfc822_parsedt(const char *);

#define CORESUBJ_RE 1
#define CORESUBJ_FWD 2

char *rfc822_coresubj(const char *, int *);
char *rfc822_coresubj_nouc(const char *, int *);
char *rfc822_coresubj_keepblobs(const char *s);

#ifdef  __cplusplus
}
#endif

#endif
