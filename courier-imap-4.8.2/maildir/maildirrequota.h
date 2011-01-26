#ifndef	maildirrequota_h
#define	maildirrequota_h

/*
** Copyright 1998 - 1999 Double Precision, Inc.
** See COPYING for distribution information.
*/

#if	HAVE_CONFIG_H
#include	"config.h"
#endif

#ifdef  __cplusplus
extern "C" {
#endif

static const char maildirrequota_h_rcsid[]="$Id: maildirrequota.h,v 1.4 1999/12/06 13:21:05 mrsam Exp $";

/* Change the quota embedded in the filename of a maildir message */

char *maildir_requota(const char *, unsigned long);

#ifdef  __cplusplus
}
#endif

#endif
