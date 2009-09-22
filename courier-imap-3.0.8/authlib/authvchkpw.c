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
#include	"vpopmail_config.h"
#include	<vpopmail.h>
#include	<vauth.h>

static const char rcsid[]="$Id: authvchkpw.c,v 1.22 2004/04/18 15:54:39 mrsam Exp $";

extern int auth_vchkpw_pre(const char *userid, const char *service,
        int (*callback)(struct authinfo *, void *),
                        void *arg);

extern FILE *authvchkpw_file(const char *, const char *);

struct callback_info {
	const char *pass;
	char *userret;
	int issession;
	void (*callback_func)(struct authinfo *, void *);
	void *callback_arg;
	};

static int callback_vchkpw(struct authinfo *a, void *p)
{
struct callback_info *i=(struct callback_info *)p;

	/* exit with perm failure if the supplied password is empty,
	 * or if the supplied password doesnt match the retrieved password */ 
	if (a->passwd == 0)
	{
		dprintf("no password supplied");
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
		/* switch to the uid/gid. chdir to the homedir.
		 * set the AUTHADDR and AUTHFULLNAME env vars
		 */
		authsuccess(a->homedir, 0, a->sysuserid, &a->sysgroupid,
			a->address, a->fullname);
	}
	else
	{
		a->address=a->sysusername; 
		(*i->callback_func)(a, i->callback_arg);

	}

        return (0);
}

char *auth_vchkpw(const char *service, const char *authtype, char *authdata,
	int issession,
	void (*callback_func)(struct authinfo *, void *), void *callback_arg)
{
char *user, *pass;
struct	callback_info	ci;
int	rc;
	/* Make sure that we have been supplied with the correct
	 * AUTHDATA format which is : userid<NEWLINE>password<NEWLINE>
	 */
	if (strcmp(authtype, AUTHTYPE_LOGIN) ||
		(user=strtok(authdata, "\n")) == 0 ||
		(pass=strtok(0, "\n")) == 0)
	{
		/* login syntax was invalid */
		errno=EPERM;
		return (0);
	}

	ci.pass=pass;
	ci.issession=issession;
	ci.callback_func=callback_func;
	ci.callback_arg=callback_arg;

	/* auth_vchkpw_pre() does this :
	 *   - lookup the passwd entry for this user from the auth backend
	 *   - check to see if this user is permitted to use this service type
	 * If successful it will populate the ci struct with the 
	 * user's passwd entry. Return value of function will be 0.
	 * If unsuccessful (eg user doesnt exist, or is not permitted to 
	 * use this auth method), it will return :
	 *  <0 on a permanent failure (eg user doesnt exist)
	 *  >0 on a temp failure
         */
	rc=auth_vchkpw_pre(user, service, &callback_vchkpw, &ci);

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

	/* user has been successfully auth'ed at this point */

#ifdef HAVE_OPEN_SMTP_RELAY
	if ( (strcmp("pop3", service)==0) || (strcmp("imap", service)==0) ) {
		/* Michael Bowe 13th August 2003
		 *
		 * There is a problem here because open_smtp_relay needs 
		 * to get the user's ip from getenv("TCPREMOTEIP").
		 * If we run --with-authvchkpw --without-authdaemon,
		 * then this var is available.
		 * But if we run --with-authvchkpw --with-authdaemon,
		 * then TCPREMOTEIP is null
		 * 
		 * If TCPREMOTEIP isnt available, then open_smtp_relay()
		 * will just return() back immediately.
		 */
		open_smtp_relay();
	}
#endif

	return (ci.userret);
}

static void authvchkpwclose()
{
}

static int auth_vchkpw_changepass(const char *service,
				  const char *username,
				  const char *pass,
				  const char *npass)
{
struct vqpasswd *vpw;
char	User[256];
char	Domain[256];

        /* Take the supplied userid, and split it out into the user and domain
         * parts. (If a domain was not supplied, then set the domain to be
         * the default domain)
         */
        if ( parse_email(username, User, Domain, 256) != 0) {
                /* Failed to successfully extract user and domain.
                 * So now exit with a permanent failure code
                 */
                return(-1);
        }

	/* check to see if domain exists.
	 * If you pass an alias domain to vget_assign, it will change it
	 * to be the real domain on return from the function
	 */
        if ( vget_assign(Domain,NULL,0,NULL,NULL) ==NULL ) {
		/* domain doesnt exist */
		return (-1);
	}

        if ( (vpw=vauth_getpw(User, Domain)) == NULL) {
		/* That user doesnt exist in the auth backend */
		errno=ENOENT;
		return (-1);
	}

	/* Exit if any of the following :
	 *   - user's password field in the passwd entry is empty
	 *   - supplied current password doesnt match stored password
	 */
	if (vpw->pw_passwd == 0 || authcheckpassword(pass, vpw->pw_passwd)) {
		errno=EPERM;
		return (-1);
	}

	/* save the new password into the auth backend */
	if ( vpasswd(User, Domain, npass, 0) != 0 ) {
		/* password set failed */
		return (-1);
	};

	return (0);
}

struct authstaticinfo authvchkpw_info={
	"authvchkpw",
	auth_vchkpw,
	auth_vchkpw_pre,
	authvchkpwclose,
	auth_vchkpw_changepass,
	NULL};

