/*
** Copyright 1998 - 1999 Double Precision, Inc.  See COPYING for
** distribution information.
*/

#ifndef	authstaticlist_h
#define	authstaticlist_h

#if	HAVE_CONFIG_H
#include	"config.h"
#endif
#include	<sys/types.h>

#ifdef	__cplusplus
extern "C" {
#endif

static const char authstaticlist_h_rcsid[]="$Id: authstaticlist.h,v 1.11 2004/05/09 03:32:01 mrsam Exp $";

struct authinfo;

struct authstaticinfo {
	const char *auth_name;
	char * (*auth_func)(const char *, const char *, char *, int,
			void (*)(struct authinfo *, void *),
                        void *);
	int (*auth_prefunc)(const char *, const char *,
			int (*)(struct authinfo *, void *),
                        void *);
	void (*auth_cleanupfunc)();
	int (*auth_changepwd)(const char *, /* service */
			      const char *, /* userid */
			      const char *, /* oldpassword */
			      const char *); /* new password */

	void (*auth_idle)();
	/* Not null - gets called every 5 mins when we're idle */

	void (*auth_enumerate)( void(*cb_func)(const char *name,
					       uid_t uid,
					       gid_t gid,
					       const char *homedir,
					       const char *maildir,
					       void *void_arg),
				void *void_arg);
	} ;

extern int auth_syspasswd(const char *,
			  const char *,
			  const char *,
			  const char *);

extern struct authstaticinfo *authstaticmodulelist[];

/*
** Call statically-linked authentication modules, according to the order
** specified in configuration file 'configfile'.  If configfile does not
** exist, the installed order is used.
**
** The first time this function is called, the contents of configfile
** are read and saved in a memory buffer, and subsequent invocations will
** use the saved copy of the module list.
*/

int authstaticlist_search(
	const char *userid,		/* userid to authenticate */
	const char *service,		/* service to authenticate */
	const char *configfile,		/* configuration file */
	int (*callback)(struct authinfo *, void *),	/* callback function */
	void *callback_arg);		/* argument to the callback function */

/*
** Call statically-linked authentication modules, according to the order
** specified in the space-separated list of module names 'configdata'.
** If this is a null pointer, the installed order is used.
*/

int authstaticlist_search_string(
	const char *userid,		/* userid to authenticate */
	const char *service,		/* service to authenticate */
	const char *configdata,		/* list of modules */
	int (*callback)(struct authinfo *, void *),	/* callback function */
	void *callback_arg);		/* argument to the callback function */

/*
**	If an authentication module is not found for this userid, a negative
**	value is returned.  If there was a temporary error (database offline,
**	etc) a positive value is returned.  Otherwise, the callback function
**	is invoked, and the exit code from the callback function is returned
**	(which should be zero).
*/

/*
** Now, go through the authentication modules and attempt to log in as
** someone.  This is basically authstaticlist_search, but calling
** auth_func instead.
*/

char *authlogin_search(const char *configfilename,
		       const char *service,
		       const char *authtype,
		       const char *authdata,
		       int issession,
		       void (*callback_func)(struct authinfo *, void *),
		       void *callback_arg,
		       int *driver);

char *authlogin_search_string(const char *configdata,
		       const char *service,
		       const char *authtype,
		       const char *authdata,
		       int issession,
		       void (*callback_func)(struct authinfo *, void *),
		       void *callback_arg,
		       int *driver);

extern int auth_changepass;

#ifdef	__cplusplus
}
#endif

#endif
