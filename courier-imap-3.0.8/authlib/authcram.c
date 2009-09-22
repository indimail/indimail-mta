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
#include	"authsasl.h"
#include	"authmod.h"
#include	"authstaticlist.h"
#include	"debug.h"
#include	"userdb/userdb.h"
#include	"../libhmac/hmac.h"
#include	"cramlib.h"

static const char rcsid[]="$Id: authcram.c,v 1.17 2004/04/18 15:54:38 mrsam Exp $";

extern void auth_userdb_cleanup();

char *auth_cram(const char *service, const char *authtype, char *authdata,
	int issession,
	void (*callback_func)(struct authinfo *, void *), void *callback_arg)
{
char	*u;
char	*udbs;
char	*passwords;
char	*services;
struct	userdbs *udb;
char *challenge, *response;
char	*user;
struct hmac_hashinfo	*hmacptr;

	if (auth_get_cram(authtype, authdata, &hmacptr,
		&user, &challenge, &response))
		return (0);

	userdb_set_debug(auth_debug_login_level);
	userdb_init(USERDB ".dat");
        if ( (u=userdb(user)) == 0)
	{
		userdb_close();
		errno=EPERM;
		return (0);
	}

	if ( (udbs=userdbshadow(USERDB "shadow.dat", user)) == 0)
	{
		free(u);
		userdb_close();
		errno=EPERM;
		return (0);
	}

	if ((services=malloc(strlen(service)+strlen(hmacptr->hh_name)
		+sizeof("-hmac-pw"))) == 0)
	{
		free(udbs);
		free(u);
		userdb_close();
		errno=ENOSPC;
		return (0);
	}

	strcat(strcat(strcat(strcpy(services, service), "-hmac-"),
		hmacptr->hh_name), "pw");

	passwords=userdb_gets(udbs, services);
	if (passwords == 0)
	{
		strcat(strcat(strcpy(services, "hmac-"),
			hmacptr->hh_name), "pw");
		passwords=userdb_gets(udbs, services);
	}
	if (passwords == 0)
		dprintf("authcram: no %s-%s or %s value found",
			service, services, services);
	free(services);

	if (passwords == 0)
	{
		errno=EPERM;
		free(udbs);
		free(u);
		userdb_close();
		return (0);
	}

	if (auth_verify_cram(hmacptr, challenge, response, passwords))
	{
		free(passwords);
		errno=EPERM;  /* other modules can do cram too */
		free(udbs);
		free(u);
		userdb_close();
		return (0);
	}

	free(passwords);
	free(udbs);
        if ((udb=userdb_creates(u)) == 0)
        {
		free(u);
		userdb_close();
		errno=EACCES;
                return (0);
        }

	if (callback_func == 0)
		authsuccess(udb->udb_dir, 0, &udb->udb_uid, &udb->udb_gid,
			user, udb->udb_gecos);
	else
	{
	struct	authinfo	aa;

		memset(&aa, 0, sizeof(aa));

		/*aa.sysusername=user;*/
		aa.sysuserid= &udb->udb_uid;
		aa.sysgroupid= udb->udb_gid;
		aa.homedir=udb->udb_dir;
		aa.address=user;
		aa.maildir=udb->udb_mailbox;
		aa.options=udb->udb_options;
		(*callback_func)(&aa, callback_arg);
	}

        free(u);
	userdb_close();

        if (callback_func == 0)
		putenv("MAILDIR=");

        if (callback_func == 0 && udb->udb_mailbox && *udb->udb_mailbox)
        {
        char    *p=malloc(sizeof("MAILDIR=")+strlen(udb->udb_mailbox));
	static	char *prevp=0;

                if (!p)
		{
			userdb_frees(udb);
			perror("malloc");
			authexit(1);
		}
                strcat(strcpy(p, "MAILDIR="), udb->udb_mailbox);
                putenv(p);

		if (prevp)	free(prevp);
		prevp=p;
        }
	userdb_frees(udb);
	if  ((u=strdup(user)) == 0)
	{
		perror("malloc");
		authexit(1);
	}
	return (u);
}

extern int auth_cram_pre(const char *userid, const char *service,
        int (*callback)(struct authinfo *, void *),
		  void *arg);

struct authstaticinfo authcram_info={
	"authcram",
	auth_cram,
	auth_cram_pre,
	auth_userdb_cleanup,
	NULL,
	NULL};
