/*
** Copyright 1998 - 1999 Double Precision, Inc.  See COPYING for
** distribution information.
*/

#if	HAVE_CONFIG_H
#include	"config.h"
#endif
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<errno.h>
#include	<pwd.h>
#if	HAVE_UNISTD_H
#include	<unistd.h>
#endif
#if	HAVE_SHADOW_H
#include	<shadow.h>
#endif


#include	"auth.h"
#include	"debug.h"

static const char rcsid[]="$Id: preauthshadow.c,v 1.7 2004/05/09 02:52:23 mrsam Exp $";

int auth_shadow_pre(const char *userid, const char *service,
	int (*callback)(struct authinfo *, void *),
			void *arg)
{
struct authinfo auth;
struct passwd *pw;
struct spwd *spw;

	memset(&auth, 0, sizeof(auth));

	if ((pw=getpwnam(userid)) == 0)
	{
		if (errno == ENOMEM)	return (1);
		return (-1);
	}

	auth.sysusername=userid;
	auth.sysgroupid=pw->pw_gid;
	auth.homedir=pw->pw_dir;
	auth.address=userid;
	auth.fullname=pw->pw_gecos;
	auth.passwd=pw->pw_passwd;

	if ((spw=getspnam(userid)) != 0)
		auth.passwd=spw->sp_pwdp;

	auth_debug_authinfo("DEBUG: authshadow: ", &auth, 0, pw->pw_passwd);
	return ((*callback)(&auth, arg));
}
