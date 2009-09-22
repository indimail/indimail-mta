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
#include	"debug.h"

static const char rcsid[]="$Id: authuserdb.c,v 1.19 2004/04/18 15:54:39 mrsam Exp $";

extern void auth_userdb_enumerate( void(*cb_func)(const char *name,
						  uid_t uid,
						  gid_t gid,
						  const char *homedir,
						  const char *maildir,
						  void *void_arg),
				   void *void_arg);

extern int auth_userdb_pre_common(const char *, const char *, int,
        int (*callback)(struct authinfo *, void *),
                        void *arg);

extern void auth_userdb_cleanup();

struct callback_info {
        const char *pass;
        char *userret;
	int issession;
	void (*callback_func)(struct authinfo *, void *);
	void *callback_arg;
        };

static int callback_userdb(struct authinfo *a, void *p)
{
struct callback_info *i=(struct callback_info *)p;

	if (a->passwd == 0)
	{
		dprintf("no password available to compare");
		return (-1);
	}

	if (authcheckpassword(i->pass, a->passwd))
		return (-1);

        if ((i->userret=strdup(a->sysusername)) == 0)
        {
                perror("malloc");
                return (1);
        }

	if (i->callback_func == 0)
	{
	static	char *prevp=0;
	const char *cp=a->maildir;
        char    *p;

		if (!cp)	cp="";
		p=malloc(sizeof("MAILDIR=")+strlen(cp));
                if (!p)
		{
			perror("malloc");
			free(i->userret);
			return (1);
		}
                strcat(strcpy(p, "MAILDIR="), cp);
                putenv(p);
		if (prevp)	free(prevp);
		prevp=p;
	}

	if (i->callback_func == 0)
	{
		static	char *prevp=0;
		const char *cp=a->options;
		char    *p;

		if (!cp)	cp="";
		p=malloc(sizeof("OPTIONS=")+strlen(cp));
                if (!p)
		{
			perror("malloc");
			free(i->userret);
			return (1);
		}
                strcat(strcpy(p, "OPTIONS="), cp);
                putenv(p);
		if (prevp)	free(prevp);
		prevp=p;
	}

	if (i->callback_func == 0)
	{
	static	char *prevp=0;
	const char *cp=a->quota;
        char    *p;

		if (!cp)	cp="";
		p=malloc(sizeof("MAILDIRQUOTA=")+strlen(cp));
                if (!p)
		{
			perror("malloc");
			free(i->userret);
			return (1);
		}
                strcat(strcpy(p, "MAILDIRQUOTA="), cp);
                putenv(p);
		if (prevp)	free(prevp);
		prevp=p;

		authsuccess(a->homedir, 0, a->sysuserid,
			&a->sysgroupid, a->address, a->fullname);
        }
	else
	{
		a->address=a->sysusername;
		(*i->callback_func)(a, i->callback_arg);
	}
        return (0);
}


char *auth_userdb(const char *service, const char *authtype, char *authdata,
	int issession,
	void (*callback_func)(struct authinfo *, void *), void *callback_arg)
{
const char *user, *pass;
int	rc;
struct	callback_info	ci;

	if (strcmp(authtype, AUTHTYPE_LOGIN) ||
		(user=strtok(authdata, "\n")) == 0 ||
		(pass=strtok(0, "\n")) == 0)
	{
		dprintf("authuserdb only handles authtype=" AUTHTYPE_LOGIN);
		errno=EPERM;
		return (0);
	}

	ci.pass=pass;
	ci.callback_func=callback_func;
	ci.callback_arg=callback_arg;
	ci.issession=issession;
	rc=auth_userdb_pre_common(user, service, 1, &callback_userdb, &ci);
	auth_userdb_cleanup();
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
        return (ci.userret);
}

extern int auth_userdb_pre(const char *userid, const char *service,
        int (*callback)(struct authinfo *, void *),
		    void *arg);

extern int auth_userdb_passwd(const char *service,
			      const char *userid,
			      const char *opwd_buf,
			      const char *npwd_buf);

struct authstaticinfo authuserdb_info={
	"authuserdb",
	auth_userdb,
	auth_userdb_pre,
	auth_userdb_cleanup,
	auth_userdb_passwd,
	NULL,
	auth_userdb_enumerate};


