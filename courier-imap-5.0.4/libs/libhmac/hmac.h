#ifndef	hmac_h
#define	hmac_h

#include	<string.h>

/*
** Copyright 1998 - 2005 Double Precision, Inc.
** See COPYING for distribution information.
*/

#ifdef	__cplusplus
extern "C" {
#endif


struct hmac_hashinfo {	/* HMAC hash function descriptor */

	const char *hh_name;	/* Name of this hash function (md5, sha1...) */

	size_t hh_B;	/* Length of compression blocks */
	size_t hh_L;	/* Length of hash outputs */

	size_t hh_S;	/* Length of 'context structure' */

	/* Hash functions */

	void (*hh_init)(void *);	/* Initialize context structure */
	void (*hh_hash)(void *, const void *, unsigned);
					/* Feed the hash function */
	void (*hh_endhash)(void *, unsigned long); /* Calculate final hash */

	void (*hh_getdigest)(void *, unsigned char *); /* Get the hash value */
	void (*hh_setdigest)(void *, const unsigned char *);
						/* Set the hash value */

	/* Some helper functions */

	/* Allocate context on stack, instead of mallocing it.  Calls the
	** provided function pointer, with context as first arg.  The second
	** arg will be passed as provided.
	*/

	void (*hh_allocacontext)(void (*)(void *, void *), void *);

	/* Like allocacontext, but alloc buffer for hash value, hh_L */

	void (*hh_allocaval)(void (*)(unsigned char *, void *), void *);

	} ;

/* Known hash functions */

extern struct hmac_hashinfo hmac_md5, hmac_sha1, hmac_sha256;

/*
** List of installed hash functions, dynamically generated at configuration
** time.
*/

extern struct hmac_hashinfo *hmac_list[];

/*
   To calculate an HMAC, allocate three buffers - outer, inner, and hash.
   Call hmac_hashkey, then hmac_hashtext.

   After hmac_haskey returns, the contents of inner and outer can be
   saved, as they contain a complete intermediate state of the hash
   calculation.
*/

void hmac_hashkey(
	const struct hmac_hashinfo *,
	const char *,	/* Key */
	size_t,	/* Key length */
	unsigned char *,	/* Output - outer buffer, prehashed */
	unsigned char *);	/* Output - inner buffer, prehashed */

void hmac_hashtext (
	const struct hmac_hashinfo *,
	const char *,	/* Text */
	size_t,	/* Text length */
	const unsigned char *,	/* outer buffer, prehashed */
	const unsigned char *,	/* inner buffer, prehashed */
	unsigned char *);	/* Output - the hash */

#ifdef	__cplusplus
}
#endif

#endif
