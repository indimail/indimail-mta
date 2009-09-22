/*
** Copyright 1998 - 1999 Double Precision, Inc.  See COPYING for
** distribution information.
*/


static const char rcsid[]="$Id: preauthcram.c,v 1.5 2000/03/01 23:01:36 mrsam Exp $";

#include	"auth.h"

extern int auth_userdb_pre_common(const char *userid, const char *service,
	int needpass,
	int (*callback)(struct authinfo *, void *),
                        void *arg);

int auth_cram_pre(const char *userid, const char *service,
        int (*callback)(struct authinfo *, void *),
                        void *arg)
{
	return (auth_userdb_pre_common(userid, service, 0, callback, arg));
}
