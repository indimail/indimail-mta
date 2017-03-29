/*
** Copyright 2001 Double Precision, Inc.  See COPYING for
** distribution information.
*/

#if	HAVE_CONFIG_H
#include	"config.h"
#endif
#include	<string.h>
#if	HAVE_UNISTD_H
#include	<unistd.h>
#endif
#include	"sha1/sha1.h"
#include	"auth.h"

static const char rcsid[]="$Id: checkpasswordsha1.c,v 1.1 2001/04/19 01:22:17 mrsam Exp $";

int authcheckpasswordsha1(const char *password, const char *encrypted_password)
{
	if (strncasecmp(encrypted_password, "{SHA}", 5) == 0)
	{
		return (strcmp(encrypted_password+5, sha1_hash(password)));
	}
	return (-1);
}
