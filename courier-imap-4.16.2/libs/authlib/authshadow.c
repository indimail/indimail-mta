/*
** Copyright 1998 - 2001 Double Precision, Inc.  See COPYING for
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
#include	"authmod.h"
#include	"authstaticlist.h"

static const char rcsid[]="$Id: authshadow.c,v 1.13 2004/01/11 02:47:32 mrsam Exp $";


extern int auth_shadow_pre(const char *userid, const char *service,
        int (*callback)(struct authinfo *, void *),
                        void *arg);

extern void auth_pwd_enumerate( void(*cb_func)(const char *name,
					       uid_t uid,
					       gid_t gid,
					       const char *homedir,
					       const char *maildir,
					       void *void_arg),
				void *void_arg);

struct callback_info {
	const char *pass;
	char *userret;
	int issession;
	void (*callback_func)(struct authinfo *, void *);
	void *callback_arg;
	};

static int callback_pwd(struct authinfo *a, void *p)
{
struct callback_info *i=(struct callback_info *)p;

	if (a->passwd == 0 || authcheckpassword(i->pass, a->passwd))
		return (-1);

	if ((i->userret=strdup(a->sysusername)) == 0)
	{
		perror("malloc");
		return (1);
	}

	if (i->callback_func == 0)
		authsuccess(a->homedir, i->userret, 0, &a->sysgroupid,
			a->address, a->fullname);
	else
	{
		a->address=a->sysusername;
		(*i->callback_func)(a, i->callback_arg);
	}

	return (0);
}

char *auth_shadow(const char *service, const char *authtype, char *authdata,
	int issession,
	void (*callback_func)(struct authinfo *, void *), void *callback_arg)
{
const char *user, *pass;
struct callback_info ci;
int	rc;

	if (strcmp(authtype, AUTHTYPE_LOGIN) ||
		(user=strtok(authdata, "\n")) == 0 ||
		(pass=strtok(0, "\n")) == 0)
	{
		errno=EPERM;
		return (0);
	}

	ci.pass=pass;
	ci.issession=issession;
	ci.callback_func=callback_func;
	ci.callback_arg=callback_arg;
	rc=auth_shadow_pre(user, service, &callback_pwd, &ci);

	if (rc < 0)
	{
		errno=EPERM;
		return (0);
	}
	if (rc > 0)
	{
		errno=EACCES;
		return (0);
	}

	if (putenv("MAILDIR="))
	{
		perror("putenv");
		free(ci.userret);
		return (0);
	}
	return (ci.userret);
}


static void auth_shadow_cleanup()
{
#if	HAVE_ENDPWENT

	endpwent();
#endif

#if	HAVE_ENDSPENT

	endspent();
#endif
}

struct authstaticinfo authshadow_info={
	"authshadow",
	auth_shadow,
	auth_shadow_pre,
	auth_shadow_cleanup,
	auth_syspasswd,
	NULL,
	auth_pwd_enumerate,
};
