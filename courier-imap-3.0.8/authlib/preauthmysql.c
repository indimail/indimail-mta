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

#include	"auth.h"
#include	"authmod.h"
#include	"authmysql.h"


static const char rcsid[]="$Id: preauthmysql.c,v 1.9 2004/01/19 19:34:09 mrsam Exp $";


int	auth_mysql_pre(const char *user, const char *service,
		int (*callback)(struct authinfo *, void *), void *arg)
{
struct authmysqluserinfo *authinfo;
struct	authinfo	aa;

	authinfo=auth_mysql_getuserinfo(user, service);

	if (!authinfo)		/* Fatal error - such as MySQL being down */
		return (1);

	if (!authinfo->home)	/* User not found */
		return (-1);

	memset(&aa, 0, sizeof(aa));

	/*aa.sysusername=user;*/
	aa.sysuserid= &authinfo->uid;
	aa.sysgroupid= authinfo->gid;
	aa.homedir=authinfo->home;
	aa.maildir=authinfo->maildir && authinfo->maildir[0] ?
		authinfo->maildir:0;
	aa.address=authinfo->username;
	aa.passwd=authinfo->cryptpw;
	aa.clearpasswd=authinfo->clearpw;
	aa.fullname=authinfo->fullname;
	aa.quota=authinfo->quota && authinfo->quota[0] ?
		authinfo->quota:0;
	aa.options=authinfo->options;
	return ((*callback)(&aa, arg));
}
