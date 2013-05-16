/*
** Copyright 1998 - 2000 Double Precision, Inc.  See COPYING for
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
#include	"authstaticlist.h"

static const char rcsid[]="$Id: authpwdenumerate.c,v 1.1 2004/01/11 02:47:32 mrsam Exp $";


void auth_pwd_enumerate( void(*cb_func)(const char *name,
					uid_t uid,
					gid_t gid,
					const char *homedir,
					const char *maildir,
					void *void_arg),
			 void *void_arg)
{
	struct passwd *pw;

	setpwent();

	while ( (pw=getpwent()) != NULL)
	{
		if (pw->pw_uid < 100)
			continue;

		(*cb_func)(pw->pw_name, pw->pw_uid,
			   pw->pw_gid,
			   pw->pw_dir,
			   NULL,
			   void_arg);
	}
	endpwent();
	(*cb_func)(NULL, 0, 0, NULL, NULL, void_arg);
}
