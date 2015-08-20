/*
** Copyright 1998 - 1999 Double Precision, Inc.  See COPYING for
** distribution information.
*/

#if	HAVE_CONFIG_H
#include	"config.h"
#endif
#include	<string.h>
#if	HAVE_UNISTD_H
#include	<unistd.h>
#endif
#if	HAVE_CRYPT_H
#include	<crypt.h>
#endif
#include	"auth.h"
#include	"debug.h"

static const char rcsid[]="$Id: checkpassword.c,v 1.10 2004/04/18 15:54:39 mrsam Exp $";

#if HAVE_CRYPT
#if NEED_CRYPT_PROTOTYPE
extern char *crypt(const char *, const char *);
#endif
#endif

#if	HAVE_MD5LIB
extern int authcheckpasswordmd5(const char *, const char *);
#endif

#if	HAVE_SHA1LIB
extern int authcheckpasswordsha1(const char *, const char *);
#endif

static int do_authcheckpassword(const char *password, const char *encrypted_password)
{
#if	HAVE_MD5LIB
	if (strncmp(encrypted_password, "$1$", 3) == 0
		|| strncasecmp(encrypted_password, "{MD5}", 5) == 0
		)
		return (authcheckpasswordmd5(password, encrypted_password));
#endif

#if	HAVE_SHA1LIB
	if (strncasecmp(encrypted_password, "{SHA}", 5) == 0
		)
		return (authcheckpasswordsha1(password, encrypted_password));
#endif

	return (
#if	HAVE_CRYPT
		strcmp(encrypted_password,
			crypt(password, encrypted_password))
#else
		strcmp(encrypted_password, password)
#endif
				);
}

int authcheckpassword(const char *password, const char *encrypted_password)
{
int rc;

	rc=do_authcheckpassword(password, encrypted_password);
	if (rc == 0)
		dprintf("password matches successfully");
	else if (auth_debug_login_level >= 2)
		dprintf("supplied password '%s' does not match encrypted password '%s'",
			password, encrypted_password);
	else
		dprintf("supplied password does not match encrypted password");
	return rc;
}
