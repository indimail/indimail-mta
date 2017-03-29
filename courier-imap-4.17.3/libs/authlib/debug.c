/*
** Copyright 2002 Double Precision, Inc.  See COPYING for
** distribution information.
*/

#include <stdio.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "auth.h"
#include "debug.h"

static const char rcsid[]="$Id: debug.c,v 1.5 2004/05/09 02:52:23 mrsam Exp $";

/* for internal use */

static int auth_debug( const char *ofmt, const char *fmt, va_list ap );

/*
** 0 - dont debug
** 1 - debug auth
** 2 - debug auth + write out passwords
*/

int auth_debug_login_level = 0;

/*
** purpose: initialize debugging
** function: read environment variable DEBUG_LOGIN
**           and set up debugging according to it
** args: none
*/

void auth_debug_login_init( void )
{
	const char *p=getenv(DEBUG_LOGIN_ENV);

	auth_debug_login_level = atoi( p ? p:"0" );
}

/*
** purpose: print debug message to logger
** does nothing if debug level is zero
** adds prefix "DEBUG: " and suffix "\n"
** Since we have a return value, we can use the convenient production
**	auth_debug_login_level && auth_debug_printf(...)
** (as a macro, saves function calls when debugging is disabled)
*/

int auth_debug_printf( const char *fmt, ... ) {

	va_list ap;
	int rc;

	if (auth_debug_login_level == 0) return 0;
	va_start( ap, fmt );
	rc = auth_debug( "DEBUG: %s\n", fmt, ap );
	va_end( ap );
	return rc;
}

/** Print error log message **/

int err( const char *fmt, ... ) {

	va_list ap;
	int rc;

	va_start( ap, fmt );
	rc = auth_debug( "ERR: %s\n", fmt, ap );
	va_end( ap );
	return rc;
}

/*
** purpose: print debug messages to logger - handy use
** function: take message with logging level and drop
**           messages with too high level.
**           also include into the message the IP address
** args:
** * level - level to be compared with DEBUG_LOGIN env var.
** * fmt - message format as like in printf().
** * ... - and "arguments" for fmt
*/

void auth_debug_login( int level, const char *fmt, ... ) {

	va_list ap;
	char ofmt[128];

	/* logging severity */

	if( level > auth_debug_login_level )
		return;

	snprintf( ofmt, sizeof ofmt, "LOGIN: DEBUG: ip=[%s], %%s\n", getenv("TCPREMOTEIP") );
	va_start( ap, fmt );
	auth_debug( ofmt, fmt, ap );
	va_end( ap );
}

/*
** purpose: print debug messages to logger - general use
** function: read format string and arguments
**           and convert them to suitable form for output.
** args:
** ofmt- printf() format string for output, where %s = the assembled text
** fmt - printf() format string for arguments
** ... - variable arguments
*/

static int auth_debug( const char *ofmt, const char *fmt, va_list ap )
{

	char	buf[DEBUG_MESSAGE_SIZE];
	int	i;
	int	len;

	/* print into buffer to be able to replace control and other unwanted chars. */
	vsnprintf( buf, DEBUG_MESSAGE_SIZE, fmt, ap );
	len = strlen( buf );

	/* replace nonprintable chars by dot */
	for( i=0 ; i<len ; i++ )
		if( !isprint(buf[i]) )
			buf[i] = '.';

	/* emit it */

	return fprintf( stderr, ofmt , buf );
}

/*
 * Print the information retrieved from the database into struct authinfo.
 *
 * The structure members 'clearpasswd' and 'passwd' are not always set at
 * the point where this function is called, so we take separate values
 * for these
 */

int auth_debug_authinfo(const char *pfx, const struct authinfo *auth,
	const char *clearpasswd, const char *passwd)
{
	char uidstr[32] = "<null>";
	
	if (!auth_debug_login_level) return 0;
	
	if (auth->sysuserid)
		snprintf(uidstr, sizeof uidstr, "%ld", (long)*auth->sysuserid);
	fprintf(stderr, "%ssysusername=%s, sysuserid=%s, "
		"sysgroupid=%ld, homedir=%s, address=%s, fullname=%s, "
		"maildir=%s, quota=%s, options=%s\n", pfx,
		auth->sysusername ? auth->sysusername : "<null>",
		uidstr, (long)auth->sysgroupid,
		auth->homedir ? auth->homedir : "<null>",
		auth->address ? auth->address : "<null>",
		auth->fullname ? auth->fullname : "<null>",
		auth->maildir ? auth->maildir : "<null>",
		auth->quota ? auth->quota : "<null>",
		auth->options ? auth->options : "<null>");
	if (auth_debug_login_level >= 2)
		fprintf(stderr, "%sclearpasswd=%s, passwd=%s\n", pfx,
			clearpasswd ? clearpasswd : "<null>",
			passwd ? passwd : "<null>");
	return 0;
}
