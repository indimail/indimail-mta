#ifndef	authsasl_h
#define	authsasl_h

/*
** Copyright 1998 - 1999 Double Precision, Inc.  See COPYING for
** distribution information.
*/

#if	HAVE_CONFIG_H
#include	"config.h"
#endif
#include	<sys/types.h>

#ifdef	__cplusplus
extern "C" {
#endif

static const char authsasl_h_rcsid[]="$Id: authsasl.h,v 1.1 1999/12/13 03:34:28 mrsam Exp $";

/*
	These family of functions are used to implement the SASL interface
	on top of authlib.  It is mainly used by the authentication user
	process to build the authentication request data for authmod()
	based upon the SASL challenge/response interaction.
*/

/*
** The authsasl_info is built dynamically by configure, it lists the supported
** SASL methods.  Each method is implemented by a function that's prototyped
** like this:
**
**  int authsasl_function(const char *method, const char *initresponse,
**      char *(*getresp)(const char *),
**
**	char **authtype,
**	char **authdata)
**
** Normally, there's no need to call the appropriate function directly, as
** authsasl() automatically searches this array, and finds one.
**
*/

struct authsasl_info {
	const char *sasl_method;	/* In uppercase */
	int (*sasl_func)(const char *method, const char *initresponse,
			char *(*getresp)(const char *),
			char **,
			char **);
	} ;

extern struct authsasl_info authsasl_list[];

/*
**	authsasl searched for the right method, and calls the appropriate
**	sasl function.  authsasl received the following arguments:
**
**	initresponse -- initial response for the authentication request,
**	if provided.  If provided, the actual response MUST BE PROVIDED
**	in initresponse using base64 encoding!!!
**
**	sasl_func -- the callback function which is used to carry out the
**	SASL conversation.  The function receives a single argument, the
**	base64-encoded challenge.  The callback function must return
**	a malloced pointer to the base64-encoded response, or NULL to abort
**	SASL.
**
**	authsasl returns two values, provided via call by reference:
**	the authtype and authdata argument to authmod.
*/

int authsasl(const char *,		/* Method */
	const char *,			/* Initial response - base64encoded */
	char *(*)(const char *),	/* Callback conversation functions */
	char **,			/* Returned - AUTHTYPE */
	char **);			/* Returned - AUTHDATA */

/* Some convenience functions */

char *authsasl_tobase64(const char *, int);
int authsasl_frombase64(char *);

/* Return values from authsasl */

#define	AUTHSASL_OK	0
#define	AUTHSASL_ERROR	-1	/*
				** System error, usually malloc failure,
				** authsasl reports the error to stderr.
				*/

#define	AUTHSASL_ABORTED -2	/*
				** SASL exchange aborted. authsasl does NOT
				** report any errors.
				*/

#ifdef	__cplusplus
}
#endif

#endif
