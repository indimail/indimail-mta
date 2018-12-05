/*
** Copyright 1998 - 1999 Double Precision, Inc.  See COPYING for
** distribution information.
*/

#include	<stdio.h>
#include	<stdlib.h>
#include	<errno.h>
#include	"auth.h"
#include	"authmod.h"
#include	"debug.h"

extern char *MODULE(const char *, const char *, char *, int,
	void (*)(struct authinfo *, void *), void *);

static const char mod_h_rcsid[]="$Id: mod.h,v 1.5 2004/04/18 15:54:39 mrsam Exp $";

#ifndef MODNAME
/* The ANSI-C way to convert a token into a string */
#define DEFSTR(x) #x
#define DEFSTR2(x) DEFSTR(x)
#define MODNAME DEFSTR2(MODULE)
#endif

int main(int argc, char **argv)
{
const char *service, *type;
char *authdata;
char	*user;

	auth_debug_login_init();
	authmod_init(argc, argv, &service, &type, &authdata);
	dprintf(MODNAME ": starting client module");
	user=MODULE(service, type, authdata, 1, 0, 0);
	if (!user || !*user)
	{
		if (user || errno != EPERM)
		{
			dprintf(MODNAME ": TEMPFAIL - no more modules will be tried");
			authmod_fail_completely();
		}

		dprintf(MODNAME ": REJECT");
		authmod_fail(argc, argv);
	}
	dprintf(MODNAME ": ACCEPT, username %s", user);
	authmod_success(argc, argv, user);
	return (0);
}

