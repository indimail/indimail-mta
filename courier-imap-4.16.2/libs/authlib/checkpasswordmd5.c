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
#include	"md5/md5.h"
#include	"auth.h"

static const char rcsid[]="$Id: checkpasswordmd5.c,v 1.6 2002/12/12 04:23:58 mrsam Exp $";

int authcheckpasswordmd5(const char *password, const char *encrypted_password)
{
	if (strncmp(encrypted_password, "$1$", 3) == 0)
	{
		return (strcmp(encrypted_password,
			md5_crypt(password, encrypted_password)));
	}

	if (strncasecmp(encrypted_password, "{MD5}", 5) == 0)
	{
               return (strcmp(encrypted_password+5, md5_hash_courier(password)));
	}
	return (-1);
}
