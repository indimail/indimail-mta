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
#include	<sys/types.h>
#include	<sys/stat.h>
#include	"auth.h"
#include	"authmod.h"

static const char rcsid[]="$Id: authvchkpwlib.c,v 1.7 2000/01/27 02:03:41 mrsam Exp $";

char *authvchkpw_isvirtual(char *c)
{
char *p;

	if ((p=strchr(c, '@')) != 0)    return (p);
#if 0
	if ((p=strchr(c, '%')) != 0)    return (p);
#endif
	if ((p=strchr(c, ':')) != 0)    return (p);
	return (0);
}
