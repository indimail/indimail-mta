#ifndef	authlib_auth_h
#define	authlib_auth_h

/*
** Copyright 1998 - 2004 Double Precision, Inc.  See COPYING for
** distribution information.
*/

#if	HAVE_CONFIG_H
#include	"config.h"
#endif
#include	<sys/types.h>

#ifdef	__cplusplus
extern "C" {
#endif

static const char auth_h_rcsid[]="$Id: auth.h,v 1.10 2004/02/17 00:58:12 mrsam Exp $";

/*
** authcopyargv prepares the arguments to execv for a module that receives
** the next program to run, in a chain, via the command line.
**
** execv is documented to require a terminating null char **.  The fact
** that argv is also null char ** is not always documented, and we can't
** assume that.
**
** authcopyargv receives the new argc/argv arguments, and allocates a
** new argv array.  argv[0] is stripped of any path, and the original
** argv[0] is returned separately.
*/

extern char **authcopyargv(int,	/* argc */
	char **,		/* argv */
	char **);		/* original argv[0] */

/*
** authchain - chain to the next authentication module via exec.
**
** Runs the next authentication module, and passes it a copy of the
** authentication request on file descriptor 3.
**
** authchain sets up a pipe on file descriptor 3, and forks.  The parent
** runs the next authentication module.  The child sends the authentication
** information down the pipe, and terminates.
*/

extern void authchain(int,	/* argc */
	char **,		/* argv */
	const char *);		/* Authentication request */

/*
** Authentication functions must call authsuccess if the authentication
** request succeeds, and provide the following parameters.
*/

void authsuccess(
		const char *,	/* home directory */
		const char *,	/* username */
		const uid_t	*,	/* userid */
		const gid_t	*,	/* group id */

		const char *,	/* AUTHADDR */
		const char *);	/* AUTHFULLNAME */
/*
** EITHER username or userid can be specified (leave the other pointer null).
** authmod_success changes to the home directory, and initializes the
** process's uid and gid.  gid can be null if username is provided, in which
** case gid will be picked up from the password file. gid CANNOT be null
** if username is null.
*/


/* authcheckpassword is the general password validation routine.
** It returns 0 if the password matches the encrypted password.
*/

int authcheckpassword(const char *,	/* password */
	 const char *);			/* encrypted password */

	/*
	** authcryptpasswd is a password hashing function, used to create
	** new password.  password is the cleartext password.
	** encryption_hint is a hint to the type of hashing to be used
	** (NULL means use a default hash function).
	*/

char *authcryptpasswd(const char *password,
		      const char *encryption_hint);

/* Stub function */

extern void authexit(int);


/*
	LOW LEVEL AUTHENTICATION DRIVERS.

Each low level authentication driver provides three functions:

1) Primary authentication function.  This function is used to build a
   standalone authentication module based on the mod.h template (see mod.h).
   This function takes an authentication request.  If its valid, it
   changes its userid/groupid to the one for the authenticated user,
   changes the current directory to the authenticated user's home directory,
   and sets the following environment variables:

             MAILDIR - nondefault mailbox location (optional).

             OPTIONS - optional settings.  This is a string containing
	               comma-delimited "IDENTIFIER=VALUE" tuples.  See
		       "Account OPTIONS" in INSTALL for more information.


2) User lookup function.  This function is prototyped as follows:

     int functionname(const char *userid, const char *service,
		int (*callback)(struct authinfo *, void *),
			void *);

     This function populates the following structure:
*/

struct authinfo {
	const char *sysusername;
	const uid_t *sysuserid;
	gid_t sysgroupid;
	const char *homedir;

	const char *address;
	const char *fullname;
	const char *maildir;
	const char *quota;

	const char *passwd;
	const char *clearpasswd;	/* For authldap */

	const char *options;

	unsigned staticindex;	/* When statically-linked functions are
				** called, this holds the index of the
				** authentication module in authstaticlist */

	} ;

/*
	Either sysusername or sysuserid may be NULL, but not both of them.
	They, and sysgroupid, specify the authenticated user's system
	userid and groupid.  homedir points to the authenticated user's
	home directory.  address, fullname, and maildir, are obvious.
	quota is populated with any maildir quota (see
	maildir/README.maildirquota).

	'options' is an optional string that contains per-user custom settings.
	See "OPTIONS" above.

	After populating this tructure, the lookup function calls the
	callback function that's specified in its second argument.  The
	callback function receives a pointer to the authinfo structure.

	The callback function also receives a context pointer, which is
	the third argument to the lookup function.

	The lookup function should return a negative value if he userid
	does not exist, a positive value if there was a temporary error
	looking up the userid, or whatever is the return code from the
	callback function, if the user exists.


NOTE: the passwd field is used internally by modules which implement
the primary authentication function by sharing code with the lookup function.

3) Cleanup function.  This function should close any resources that were
   opened by the lookup function.  Note that in applications which have a
   daemon process that uses this library it is possible for the lookup
   function to be called multiple times, before the cleanup function is
   called.
*/


/* Utility function: parse OPTIONS string for a particular keyword */

extern char *authgetoptionenv(const char *keyword);
extern char *authgetoption(const char *options, const char *keyword);

#ifdef	__cplusplus
}
#endif

#endif
