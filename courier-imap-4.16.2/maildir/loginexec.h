#ifndef	loginexec_h
#define	loginexec_h

/*
** Copyright 2003 Double Precision, Inc.
** See COPYING for distribution information.
*/

#if	HAVE_CONFIG_H
#include	"config.h"
#endif

#ifdef  __cplusplus
extern "C" {
#endif

/* Check for a 'loginexec' executable in the current directory, if so
   run it, then delete it if exit code zero */

void maildir_loginexec(void);

#ifdef  __cplusplus
}
#endif

#endif
