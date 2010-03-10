/*
** Copyright 2000-2009 Double Precision, Inc.  See COPYING for
** distribution information.
*/

#include "rfc2045_config.h"
#include	"rfc2045.h"
#include	"rfc822/rfc2047.h"
#include	"rfc2045charset.h"
#include	"rfc822/rfc822.h"
#include	<stdio.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

static const char rcsid[]="$Id: rfc2045replyunicode.c,v 1.3 2009/11/08 18:14:47 mrsam Exp $";

int rfc2045_makereply_do(struct rfc2045_mkreplyinfo *);

static	void decodenoconvert(int, struct rfc2045 *, const char *,
			     int (*)(const char *, size_t, void *),
			     void *);

int rfc2045_makereply_unicode(struct rfc2045_mkreplyinfo *ri)
{
	ri->decodesectionfunc= &decodenoconvert;
	return (rfc2045_makereply_do(ri));
}

static	void decodenoconvert(int fd, struct rfc2045 *p, const char *chset,
			     int (*func)(const char *, size_t, void *),
			     void *arg)
{
	rfc2045_decodetextmimesection(fd, p, chset, func, arg);
}
