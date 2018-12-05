#ifndef	authmod_h
#define	authmod_h

/*
** Copyright 1998 - 1999 Double Precision, Inc.  See COPYING for
** distribution information.
*/

/* Common functions used by standalone authentication modules */

#if	HAVE_CONFIG_H
#include	"config.h"
#endif

#ifdef	__cplusplus
extern "C" {
#endif

static const char authmod_h_rcsid[]="$Id: authmod.h,v 1.5 2000/04/30 01:04:09 mrsam Exp $";

/*
** Authentication modules must call authmod_init the first thing in main.
*/

void	authmod_init(
		int,		/* argc */
		char **,	/* argv */

		const char **,	/* Returns service to authenticate */
		const char **,	/* Returns authentication type */
		char **);	/* Returns authentication data */

/*
** NOTE: authmod_init does NOT return if a previous authentication module
** already succesfully authenticated the request.  authmod_init will run the
** next module automatically, hence we'll eventually wind up with the
** authentication client in the authenticated state.
**
** An authentication module must call authmod_success if it accepted the
** authentication request.
*/

void authmod_success(int,	/* argc */
		char **,	/* argv */
		const char *);	/* authenticated_username */

/*
** Standalone modules should call authmod_fail if the authentication failed.
*/

void authmod_fail(int,		/* argc */
		char **);	/* argv */

/*
** Standalone modules should call authmod_fail_completely, and if the module
** does not want any additional authentication modules to try to authenticate
** this request.  authmod_fail_completely reruns the authentication user
** process (see below).
*/

void authmod_fail_completely();

/*
** authentication clients should call authclient() the first thing in main,
** to check if the authentication succeeded.  If not, authclient terminates
** the process and reruns the authmoduser process
*/

const char *authmodclient();

/*
** authmoduser is called by authentication users as the very first thing
** in main().  It checks the environment variables and returns 0 if
** auth user was reinvoked upon authentication failure.  It returns non-0
** if this is the initial invocation of the auth user process.
**
** authmoduser:
**
**    * checks to make sure the environment variable AUTHUSER is set, which
**      should contain the full pathname to this process (can't rely on
**      argv[0] all the time).  authmoduser terminates if AUTHUSER is not set.
**
**    * checks if the environment variable AUTHARGC is set to a non-zero
**      value.  If it is, it means AUTHUSER was rerun due to an authentication
**      failure, so authmoduser will return 0, after sleeping for the amount
**      of time specified by the fourth argument.
**
**    * otherwise the environment variables AUTHARGC, AUTHARGV0, AUTHARGV1 ...
**      are set to mirror the contents of the argc/argv variables, so that
**      upon authentication failure $AUTHUSER can be rerun, with the same
**      exact parameters.
**
** The third argument to authmoduser specifies the timeout for a successful
** login.  The expiration time is also saved in the environment, and
** authmoduser will call alarm() to cause this process to die if the authmod()
** function is not called before the timer goes off.  The authmod function
** will cancel the alarm signal before running the first authentication
** module, in order to avoid arrivals of unexpected signals.
**
*/

int authmoduser(int,		/* argc - as passed to main */
		char **,	/* argv - as passed to main */

		unsigned,	/* authentication timeout, in seconds */
		unsigned);	/* bad authentication sleep time, in seconds */


/*
** authmod is called by authentication user to attempt to authenticate
** access.  This function never returns as it execs the first authentication
** module.  The authentication module to run is taken from the argv[0]
** parameter (see below) and argc must be > 0.  This means that argc/argv
** received by main must be advanced to skip past any options on the command
** line.
*/

#define	AUTHTYPE_LOGIN	"login"		/* authdata is userid\npassword\n */
#define	AUTHTYPE_CRAMMD5 "cram-md5"	/* authdata is challenge\nresponse\n */

void authmod(int,	/* argc */
	char **,	/* argv */

	const char *,	/* service */
	const char *,	/* authentication type */
	const char *);	/* authentication data */

void authmod_login(int,
	char **,
	const char *,	/* service */
	const char *,	/* userid */
	const char *);	/* password */

/* Magic for authdaemon */

#ifdef	__cplusplus
}
#endif
#endif
