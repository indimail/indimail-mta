/*
** Copyright 2000-2003 Double Precision, Inc.  See COPYING for
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
#include	"authpgsql.h"
#include	"authstaticlist.h"
#include	"debug.h"

static const char rcsid[]="$Id: authpgsql.c,v 1.7 2004/05/09 02:52:23 mrsam Exp $";

extern void auth_pgsql_enumerate( void(*cb_func)(const char *name,
						 uid_t uid,
						 gid_t gid,
						 const char *homedir,
						 const char *maildir,
						 void *void_arg),
				  void *void_arg);

static char *auth_pgsql_login(const char *service, char *authdata,
	int issession,
	void (*callback_func)(struct authinfo *, void *), void *callback_arg)
{
char *user, *pass;
struct authpgsqluserinfo *authinfo;

	if ((user=strtok(authdata, "\n")) == 0 ||
		(pass=strtok(0, "\n")) == 0)
	{
		errno=EPERM;
		return (0);
	}

	authinfo=auth_pgsql_getuserinfo(user, service);

	if (!callback_func)
		auth_pgsql_cleanup();

	if (!authinfo)		/* Fatal error - such as MySQL being down */
	{
		errno=EACCES;
		return (0);
	}

	if (authinfo->cryptpw)
	{
		if (authcheckpassword(pass,authinfo->cryptpw))
		{
			errno=EPERM;
			return (0);	/* User/Password not found. */
		}
	}
	else if (authinfo->clearpw)
	{
		if (strcmp(pass, authinfo->clearpw))
		{
			if (auth_debug_login_level >= 2)
				dprintf("supplied password '%s' does not match clearpasswd '%s'",
					pass, authinfo->clearpw);
			else
				dprintf("supplied password does not match clearpasswd");
			errno=EPERM;
			return (0);
		}
	}
	else
	{
		dprintf("no password available to compare");
		errno=EPERM;
		return (0);		/* Username not found */
	}

	if (callback_func == 0)
	{
	static char *maildir=0;
	static char *quota=0;
	static char *options=0;

		authsuccess(authinfo->home, 0, &authinfo->uid,
			    &authinfo->gid, authinfo->username,
			    authinfo->fullname ? authinfo->fullname:"");

		if (authinfo->maildir && authinfo->maildir[0])
		{
			if (maildir)	free(maildir);
			maildir=malloc(sizeof("MAILDIR=")+
					strlen(authinfo->maildir));
			if (!maildir)
			{
				perror("malloc");
				exit(1);
			}
			strcat(strcpy(maildir, "MAILDIR="), authinfo->maildir);
			putenv(maildir);
		}
		else
		{
			putenv("MAILDIR=");
		}

		if (authinfo->quota && authinfo->quota[0])
                {
                        if (quota)    free(quota);
                        quota=malloc(sizeof("MAILDIRQUOTA=")+
                                        strlen(authinfo->quota));
                        if (!quota)
                        {
                                perror("malloc");
                                exit(1);
                        }
                        strcat(strcpy(quota, "MAILDIRQUOTA="), authinfo->quota);
                        putenv(quota);
                }
                else
                {
                        putenv("MAILDIRQUOTA=");
                }

		if (authinfo->options && authinfo->options[0])
                {
                        if (options)    free(options);
                        options=malloc(sizeof("OPTIONS=")+
                                        strlen(authinfo->options));
                        if (!options)
                        {
                                perror("malloc");
                                exit(1);
                        }
                        strcat(strcpy(options, "OPTIONS="), authinfo->options);
                        putenv(options);
                }
                else
                {
                        putenv("OPTIONS=");
                }
	}
	else
	{
	struct	authinfo	aa;

		memset(&aa, 0, sizeof(aa));

		/*aa.sysusername=user;*/
		aa.sysuserid= &authinfo->uid;
		aa.sysgroupid= authinfo->gid;
		aa.homedir=authinfo->home;
		aa.maildir=authinfo->maildir && authinfo->maildir[0] ?
			authinfo->maildir:0;
		aa.address=authinfo->username;
		aa.quota=authinfo->quota && authinfo->quota[0] ?
			authinfo->quota:0;
		aa.fullname=authinfo->fullname;
		aa.options=authinfo->options;
		auth_debug_authinfo("DEBUG: authpgsql: ", &aa,
			authinfo->clearpw, authinfo->cryptpw);
		(*callback_func)(&aa, callback_arg);
	}

	return (strdup(authinfo->username));
}

static int auth_pgsql_changepw(const char *service, const char *user,
				 const char *pass,
				 const char *newpass)
{
struct authpgsqluserinfo *authinfo;

	authinfo=auth_pgsql_getuserinfo(user, service);

	if (!authinfo)
	{
		errno=ENOENT;
		return (-1);
	}

	if (authinfo->cryptpw)
	{
		if (authcheckpassword(pass,authinfo->cryptpw))
		{
			errno=EPERM;
			return (-1);	/* User/Password not found. */
		}
	}
	else if (authinfo->clearpw)
	{
		if (strcmp(pass, authinfo->clearpw))
		{
			errno=EPERM;
			return (-1);
		}
	}
	else
	{
		errno=EPERM;
		return (-1);
	}

	if (auth_pgsql_setpass(user, newpass))
	{
		errno=EPERM;
		return (-1);
	}
	return (0);
}

#if HAVE_HMACLIB

#include	"../libhmac/hmac.h"
#include	"cramlib.h"

struct cram_callback_info {
	struct hmac_hashinfo *h;
	char *user;
	char *challenge;
	char *response;
	char *userret;
	int issession;
	void (*callback_func)(struct authinfo *, void *);
	void *callback_arg;
	};

static int callback_cram(struct authinfo *a, void *vp)
{
struct cram_callback_info *cci=(struct cram_callback_info *)vp;
unsigned char *hashbuf;
unsigned char *p;
unsigned i;
static const char hex[]="0123456789abcdef";
int	rc;

	if (!a->clearpasswd)
		return (-1);

	/*
		hmac->hh_L*2 will be the size of the binary hash.

		hmac->hh_L*4+1 will therefore be size of the binary hash,
		as a hexadecimal string.
	*/

	if ((hashbuf=malloc(cci->h->hh_L*6+1)) == 0)
		return (1);

	hmac_hashkey(cci->h, a->clearpasswd, strlen(a->clearpasswd),
		hashbuf, hashbuf+cci->h->hh_L);

	p=hashbuf+cci->h->hh_L*2;

	for (i=0; i<cci->h->hh_L*2; i++)
	{
	char	c;

		c = hex[ (hashbuf[i] >> 4) & 0x0F];
		*p++=c;

		c = hex[ hashbuf[i] & 0x0F];
		*p++=c;

		*p=0;
	}

	rc=auth_verify_cram(cci->h, cci->challenge, cci->response,
		(const char *)hashbuf+cci->h->hh_L*2);
	free(hashbuf);

	if (rc)	return (rc);

	if ((cci->userret=strdup(a->address)) == 0)
	{
		perror("malloc");
		return (1);
	}

	if (cci->callback_func)
		(*cci->callback_func)(a, cci->callback_arg);
	else
	{
		authsuccess(a->homedir, a->sysusername, a->sysuserid,
			&a->sysgroupid,
			a->address,
			a->quota);

		if (a->maildir && a->maildir[0])
		{
		static char *maildir=0;

			if (maildir)	free(maildir);
			maildir=malloc(sizeof("MAILDIR=")+strlen(a->maildir));
			if (!maildir)
			{
				perror("malloc");
				exit(1);
			}
			strcat(strcpy(maildir, "MAILDIR="), a->maildir);
			putenv(maildir);
		}
		else
		{
			putenv("MAILDIR=");
		}
	}

	return (0);
}

static char *auth_pgsql_cram(const char *service,
	const char *authtype, char *authdata, int issession,
	void (*callback_func)(struct authinfo *, void *), void *callback_arg)
{
struct	cram_callback_info	cci;
int	rc;

	if (auth_get_cram(authtype, authdata,
		&cci.h, &cci.user, &cci.challenge, &cci.response))
		return (0);

	cci.issession=issession;
	cci.callback_func=callback_func;
	cci.callback_arg=callback_arg;

	rc=auth_pgsql_pre(cci.user, service, &callback_cram, &cci);

	if (callback_func == 0)
		auth_pgsql_cleanup();

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
	return (cci.userret);
}
#endif

char *auth_pgsql(const char *service, const char *authtype, char *authdata,
		int issession,
	void (*callback_func)(struct authinfo *, void *), void *callback_arg)
{
	if (strcmp(authtype, AUTHTYPE_LOGIN) == 0)
		return (auth_pgsql_login(service, authdata, issession,
			callback_func, callback_arg));

#if HAVE_HMACLIB
	return (auth_pgsql_cram(service, authtype, authdata, issession,
			callback_func, callback_arg));
#else
	errno=EPERM;
	return (0);
#endif
}

extern int auth_pgsql_pre(const char *user, const char *service,
			  int (*callback)(struct authinfo *, void *),
			  void *arg);

struct authstaticinfo authpgsql_info={
	"authpgsql",
	auth_pgsql,
	auth_pgsql_pre,
	auth_pgsql_cleanup,
	auth_pgsql_changepw,
	NULL,
	auth_pgsql_enumerate};
