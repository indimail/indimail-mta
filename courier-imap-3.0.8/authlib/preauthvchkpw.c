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
#include	"auth.h"
#include	"authmod.h"
#include	"debug.h"
#include	<vpopmail.h>
#include	<vauth.h>
#include	"vpopmail_config.h"

/* make use of pw_flags only if available */
#ifndef VQPASSWD_HAS_PW_FLAGS
#define pw_flags pw_gid
#endif

extern FILE *authvchkpw_file(const char *, const char *);

static const char rcsid[]="$Id: preauthvchkpw.c,v 1.22 2004/05/09 02:52:23 mrsam Exp $";

/* This function is called by the auth_vchkpw() function
 *
 * This function does the following :
 *   - extract the username and domain from the supplied userid
 *   - lookup the passwd entry for that user from the auth backend
 *   - populate *and return) a courier authinfo structure with the values
 *     from the vpopmail passwd entry 
 * 
 * Return -1 on perm failure
 * Return  0 on success
 * Return  1 on temp failure
 *
 */

int auth_vchkpw_pre(
	const char *userid,
	const char *service,
        int (*callback)(struct authinfo *, void *),
	void *arg)
{
struct vqpasswd *vpw;
static uid_t uid;
gid_t	gid;
struct authinfo auth;
static char User[256];
static char Domain[256];

	/* Make sure the auth struct is empty */
        memset(&auth, 0, sizeof(auth));

	/* Take the supplied userid, and split it out into the user and domain
         * parts. (If a domain was not supplied, then set the domain to be
	 * the default domain)
         */
        if ( parse_email(userid, User, Domain, 256) != 0) {
		/* Failed to successfully extract user and domain.
		 * So now exit with a permanent failure code
		 */
		dprintf("vchkpw: unable to split into user and domain");
		return(-1);
        }

	/* Check to see if the domain exists.
         * If so, on return vget_assign will :
         *   Rewrite Domain to be the real domain if it was sent as an alias domain
         *   Retrieve the domain's uid and gid
	 */
        if ( vget_assign(Domain,NULL,0,&uid, &gid) == NULL ) {
		/* Domain does not exist
		 * So now exit with a permanent failure code */
		dprintf("vchkpw: domain does not exist");
		return (-1);
	}

	/* Try and retrieve the user's passwd entry from the auth backend */
        if ( (vpw=vauth_getpw(User, Domain)) == NULL) {
		/* User does not exist
		 * So now exit with a permanent failure code
		 */
		dprintf("vchkpw: user does not exist");
		return (-1);
	}

        /* Look at what type of connection we are trying to auth.
         * And then see if the user is permitted to make this type
         * of connection
         */
        if ( strcmp("webmail", service) == 0 ) {
                if (vpw->pw_flags & NO_WEBMAIL) {
			dprintf("vchkpw: webmail disabled for this account");
                        return(-1);
                }
        } else if ( strcmp("pop3", service) == 0 ) {
                if ( vpw->pw_flags & NO_POP ) {
			dprintf("vchkpw: pop3 disabled for this account");
                        return(-1);
                }
        } else if ( strcmp("imap", service) == 0 ) {
                if ( vpw->pw_flags & NO_IMAP ) {
			dprintf("vchkpw: imap disabled for this account");
                        return(-1);
                }
        }

	/* Check to see if the user has been allocated a dir yet.
	 * Some of the vpopmail backends (eg mysql) allow users to
	 * be manually inserted into the auth backend but without
	 * allocating a dir. A dir will be created when the user
	 * first trys to auth (or when they 1st receive mail)
	 */
	if (vpw->pw_dir == NULL || strlen(vpw->pw_dir) == 0 ) {
		/* user does not have a dir allocated yet */
		if ( make_user_dir(User, Domain, uid, gid) == NULL) {
			/* Could not allocate a user dir at this time
			 * so exit with a temp error code 
			 */
			dprintf("vchkpw: make_user_dir failed");
			return(1);
		}
		/* We have allocated the user a dir now.
		 * Go and grab the updated passwd entry
		 */
		if ( (vpw=vauth_getpw(User, Domain)) == NULL ) {
			/* Could not get the passwd entry
			 * So exit with a permanent failure code
			 */
			dprintf("vchkpw: could not get the password entry");
			return(-1);
		}
	}

#ifdef HAVE_VSET_LASTAUTH
        /* if we are keeping track of their last auth time,
         * then store this value now. Note that this isnt 
	 * consistent with the authentication via vchkpw 
	 * because it only stores the lastauth attempt
	 * after the password has been verified. Here we are
	 * logging it after the user has been found to exist,
	 * but before the password has been verified. We could
	 * do the logging inside authvchkpw.c, but that would
	 * be a lot harder because we would have to go and
	 * parse_email() again there before calling vset_lastauth()
         */
        vset_lastauth(User, Domain, service);
#endif

	/* save the user's passwd fields into the appropriate 
	 * courier structure 
	 */
	auth.sysusername	= userid;
	auth.sysuserid		= &uid;
	auth.sysgroupid		= gid;
	auth.homedir		= vpw->pw_dir;
	auth.address		= userid;
	auth.fullname		= vpw->pw_gecos;
	auth.passwd		= vpw->pw_passwd;
	auth_debug_authinfo("DEBUG: authvchkpw: ", &auth, 0, vpw->pw_passwd);

	return ((*callback)(&auth, arg));
}
