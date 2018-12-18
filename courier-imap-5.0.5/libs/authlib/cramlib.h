#ifndef	cramlib_h
#define	cramlib_h

/*
** Copyright 1998 - 1999 Double Precision, Inc.  See COPYING for
** distribution information.
*/

/*
** auth_get_cram parses out an authentication request.  It checks whether
** we have the requisite hash function installed, and, if so, base64decodes
** the challenge and the response.
*/

struct hmac_hashinfo;

int auth_get_cram(const char *,			/* authtype */
		char *authdata,			/* authdata */

		/* Returns: */

		struct hmac_hashinfo **,	/* Pointer to the hash func */
		char **,			/* Authenticating user */
		char **,			/* What the challenge was */
		char **);			/* What the response was */

/*
** auth_verify_cram attempts to verify the secret cookie.
*/

int auth_verify_cram(struct hmac_hashinfo *,	/* The hash function */
	const char *,				/* The challenge */
	const char *,				/* The response */
	const char *);				/* Hashed secret, in hex */

#endif
