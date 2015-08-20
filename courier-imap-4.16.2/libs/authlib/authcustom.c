/*
** Copyright 1998 - 2001 Double Precision, Inc.  See COPYING for
** distribution information.
*/

#if HAVE_CONFIG_H
#include "config.h"
#endif
#include	<stdio.h>
#include	<stdlib.h>
#include	<ctype.h>
#include	<string.h>
#include	<errno.h>

#include	"auth.h"
#include	"authmod.h"
#include	"authcustom.h"
#include	"authstaticlist.h"

static const char rcsid[]="$Id: authcustom.c,v 1.4 2002/08/19 15:30:45 mrsam Exp $";


struct callback_info {
	char *userret;
	int issession;
	void (*callback_func)(struct authinfo *, void *);
	void *callback_arg;
	};

static int callback_custom(struct authinfo *a, void *p)
{
struct callback_info *i=(struct callback_info *)p;
char	*pp;

	/* No need to check passwords, authcustomlib did it already */

	if (i->callback_func == 0)
	{
	static char *env_buf=0;
	char *p;
	const char *d=a->maildir;

		if (!d)	d="";

		p=malloc(sizeof("MAILDIR=")+strlen(d)+1);
		if (!p)
		{
			perror("malloc");
			authexit(1);
		}
		sprintf(p,"MAILDIR=%s", d);
		putenv(p);
		if (env_buf)	free(env_buf);
		env_buf=p;
	}

	if (i->callback_func == 0)
	{
	static char *env_buf=0;
	char *p;
	const char *d=a->quota;

		if (!d)	d="";

		p=malloc(sizeof("MAILDIRQUOTA=")+strlen(d)+1);
		if (!p)
		{
			perror("malloc");
			authexit(1);
		}
		sprintf(p,"MAILDIRQUOTA=%s", d);
		putenv(p);
		if (env_buf)	free(env_buf);
		env_buf=p;
	}

        if ((i->userret=pp=strdup(a->address)) == 0)
        {
                perror("malloc");
                return (1);
        }

	if (i->callback_func)
	{
		a->address=i->userret;
		(*i->callback_func)(a, i->callback_arg);
	}
	else
		authsuccess(a->homedir, 0, a->sysuserid, &a->sysgroupid,
			a->address, a->fullname);

        return (0);
}

static char *auth_custom_login(const char *service, char *authdata, int issession,
	void (*callback_func)(struct authinfo *, void *), void *callback_arg)
{
const char *user, *pass;
struct	callback_info ci;
int	rc;

	if ((user=strtok(authdata, "\n")) == 0 ||
		(pass=strtok(0, "\n")) == 0)
	{
		errno=EPERM;
		return (0);
	}

	ci.issession=issession;
	ci.callback_func=callback_func;
	ci.callback_arg=callback_arg;
	rc=authcustomcommon(user, pass, &callback_custom, &ci);

	if (callback_func == 0)
		authcustomclose();

	if (rc)
	{
		errno=EPERM;
		return (0);
	}
	return (ci.userret);
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
struct	callback_info ci;

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

	ci.issession=cci->issession;
	ci.callback_func=cci->callback_func;
	ci.callback_arg=cci->callback_arg;

	if ((cci->userret=strdup(a->address)) == 0)
	{
		perror("malloc");
		return (-1);
	}

	rc=callback_custom(a, &ci);
	if (rc == 0)
		free(ci.userret);
	return (rc);
}

static char *auth_custom_cram(const char *service,
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

	rc=authcustomcommon(cci.user, 0, &callback_cram, &cci);

	if (callback_func == 0)
		authcustomclose();

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

char *auth_custom(const char *service, const char *authtype, char *authdata,
		int issession,
	void (*callback_func)(struct authinfo *, void *), void *callback_arg)
{
	if (strcmp(authtype, AUTHTYPE_LOGIN) == 0)
		return (auth_custom_login(service, authdata, issession,
			callback_func, callback_arg));

#if HAVE_HMACLIB
	return (auth_custom_cram(service, authtype, authdata, issession,
			callback_func, callback_arg));
#else
	errno=EPERM;
	return (0);
#endif
}


extern int auth_custom_pre(const char *userid, const char *service,
        int (*callback)(struct authinfo *, void *),
		  void *arg);

static int auth_custom_chgpwd(const char *service,
			      const char *uid,
			      const char *oldpwd,
			      const char *newpwd)
{
	/*
	** Insert code to change the account's password here.
	**
	** return 0 if changed.
	**
	** return 1 if failed.
	** Set errno to EPERM if we had a temporary failure (such as invalid
	** old pwd).
	**
	** Set errno to EINVAL if we failed because we did not recognize uid.
	*/

	errno=EINVAL;
	return (-1);
}

static void auth_custom_idle()
{
	/*
	** Insert code to temporarily deallocate resources after remaining
	** idle (as part of authdaemond) for more than 5 minutes.
	*/
}

struct authstaticinfo authcustom_info={
	"authcustom",
	auth_custom,
	auth_custom_pre,
	authcustomclose,
	auth_custom_chgpwd,
	auth_custom_idle};
