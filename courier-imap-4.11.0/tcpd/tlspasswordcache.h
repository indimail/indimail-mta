/*
** Copyright 2003 Double Precision, Inc.
** See COPYING for distribution information.
*/

#ifndef tlspasswordcache_h
#define tlspasswordcache_h

#include	"config.h"

#include	<unistd.h>


#ifdef  __cplusplus
extern "C" {
#endif



	/*
	** This module implements a password cache - an encrypted password
	** store. OpenSSL 0.9.7 is required.
	*/

int tlspassword_init(); /* Returns 0 if OpenSSL 0.9.7 is installed */

int tlspassword_save( const char * const *, /* NULL-terminated URL list */
		      const char * const *, /* NULL-terminated password list */

		      const char *,	/* Master password */

		      int (*)(const char *, size_t, void *),
		      /* Output function receives encrypted data */
		      void *); /* Passthrough arg to output function */

int tlspassword_load( int (*)(char *, size_t, void *), /* Input function */
		      void *, /* Passthrough arg to input function */

		      const char *,	/* Master password */

		      void(*)(const char * const *,
			      const char * const *,
			      void *), /* Callback function - decrypted pwds */
		      void *); /* Passthrough arg to callback function */


#ifdef  __cplusplus
}
#endif

#endif
