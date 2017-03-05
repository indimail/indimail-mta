#ifndef _debug_
#define _debug_

/*
** Copyright 2002 Double Precision, Inc.  See COPYING for
** distribution information.
*/
#if	HAVE_CONFIG_H
#include	"config.h"
#endif

#ifdef	__cplusplus
extern "C" {
#endif

static const char authdebug_h_rcsid[]="$Id: debug.h,v 1.3 2004/05/09 02:52:23 mrsam Exp $";

#define DEBUG_LOGIN_ENV "DEBUG_LOGIN"
#define DEBUG_MESSAGE_SIZE (1<<10)

void auth_debug_login_init( void );
void auth_debug_login( int level, const char *fmt, ... );
int auth_debug_printf( const char *fmt, ... );
int err( const char *fmt, ... );
struct authinfo;
int auth_debug_authinfo(const char *pfx, const struct authinfo *auth,
	const char *clearpasswd, const char *passwd);

extern int auth_debug_login_level;

#define dprintf auth_debug_login_level && auth_debug_printf

#ifdef	__cplusplus
}
#endif

#endif
