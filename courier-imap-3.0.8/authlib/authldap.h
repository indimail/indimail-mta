#ifndef	authldap_h
#define	authldap_h

/*
** $Id: authldap.h,v 1.5 2003/05/01 21:53:30 mrsam Exp $
**
** Copyright 1998 - 2003 Double Precision, Inc.  See COPYING for
** distribution information.
*/

/* Based on code by Luc Saillard <luc.saillard@alcove.fr>. */

#if	HAVE_CONFIG_H
#include	"config.h"
#endif


struct authinfo;

int authldapcommon(const char *, const char *,
	const char *, int (*)(struct authinfo *, void *), void *);

void authldapclose();

#endif
