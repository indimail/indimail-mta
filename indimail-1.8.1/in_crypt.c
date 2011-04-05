/*
 * $Log: in_crypt.c,v $
 * Revision 2.1  2008-09-12 09:56:29+05:30  Cprogrammer
 * use internal crypt funcations if not present
 *
 *
 */
#include "indimail.h"

#define _XOPEN_SOURCE
#include <unistd.h>
#ifdef HAVE_CRYPT_H
#include <crypt.h>
#else
char           *crypt(const char *, const char *);
#endif

#ifndef	lint
static char     sccsid[] = "$Id: in_crypt.c,v 2.1 2008-09-12 09:56:29+05:30 Cprogrammer Stab mbhangui $";
#endif

/*
 * UNIX password
 */
char           *
in_crypt(const char *pw, const char *salt)
{
	int             passwd_hash;

	getEnvConfigInt((long *) &passwd_hash, "PASSWORD_HASH", PASSWORD_HASH);
	switch (passwd_hash)
	{
	case DES_HASH:
		return(crypt(pw, salt));
		break;
	case MD5_HASH:
#ifndef HAVE_MD5_CRYPT
		return(md5_crypt(pw, salt));
#endif
		break;
	case SHA256_HASH:
#ifndef HAVE_SHA256_CRYPT
		return(sha256_crypt(pw, salt));
#endif
		break;
	case SHA512_HASH:
#ifndef HAVE_SHA512_CRYPT
		return(sha512_crypt(pw, salt));
#endif
		break;
	}
	return(crypt(pw, salt));
}

void
getversion_in_crypt_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
