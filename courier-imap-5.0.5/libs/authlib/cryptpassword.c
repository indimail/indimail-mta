/*
** Copyright 2001-2002 Double Precision, Inc.  See COPYING for
** distribution information.
*/

#if	HAVE_CONFIG_H
#include	"config.h"
#endif
#include	<string.h>
#if	HAVE_UNISTD_H
#include	<unistd.h>
#endif
#include	<stdlib.h>
#if	HAVE_CRYPT_H
#include	<crypt.h>
#endif
#include	"auth.h"
#include	<sys/time.h>

static const char rcsid[]="$Id: cryptpassword.c,v 1.4 2002/12/12 04:23:58 mrsam Exp $";

#if HAVE_CRYPT
#if NEED_CRYPT_PROTOTYPE
extern char *crypt(const char *, const char *);
#endif
#endif

#if	HAVE_MD5LIB
#include        "md5/md5.h"
#endif

#if	HAVE_SHA1LIB
#include	"sha1/sha1.h"
#endif

static const char crypt_salt[65]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789./";

static const char *crypt_hash(const char *pw)
{
	struct timeval tv;
	char salt[3];

	gettimeofday(&tv, NULL);

	tv.tv_sec |= tv.tv_usec;
	tv.tv_sec ^= getpid();

	salt[0]=crypt_salt[ tv.tv_sec % 64 ];

	tv.tv_sec /= 64;

	salt[1]=crypt_salt[ tv.tv_sec % 64 ];
	salt[2]=0;

	return (crypt(pw, salt));
}

char *authcryptpasswd(const char *password, const char *encryption_hint)
{
	const char *(*hash_func)(const char *)=0;
	const char *pfix=0;
	const char *p;
	char *pp;

#if	HAVE_MD5LIB

	if (!encryption_hint || strncmp(encryption_hint, "$1$", 3) == 0
	    || strncasecmp(encryption_hint, "{MD5}", 5) == 0)
	{
               hash_func= &md5_hash_courier;
		pfix="{MD5}";
	}
#endif

#if	HAVE_SHA1LIB
	if (!encryption_hint || strncasecmp(encryption_hint, "{SHA}", 5) == 0)
	{
		hash_func= &sha1_hash;
		pfix="{SHA}";
	}
#endif

	if (!hash_func)
	{
		hash_func= &crypt_hash;
		pfix="{CRYPT}";
	}

	p= (*hash_func)(password);
	if (!p || (pp=malloc(strlen(pfix)+strlen(p)+1)) == 0)
		return (0);

	return (strcat(strcpy(pp, pfix), p));
}
