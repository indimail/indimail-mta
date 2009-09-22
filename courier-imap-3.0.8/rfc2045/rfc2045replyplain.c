/*
** Copyright 2000 Double Precision, Inc.  See COPYING for
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

static const char rcsid[]="$Id: rfc2045replyplain.c,v 1.2 2003/03/07 00:47:31 mrsam Exp $";

int rfc2045_makereply_do(struct rfc2045_mkreplyinfo *);

static	void decodenoconvert(int, struct rfc2045 *, const char *,
			     int (*)(const char *, size_t, void *),
			     void *);

int rfc2045_makereply(struct rfc2045_mkreplyinfo *ri)
{
	ri->decodesectionfunc= &decodenoconvert;
	return (rfc2045_makereply_do(ri));
}

static	void decodenoconvert(int fd, struct rfc2045 *p, const char *chset,
			     int (*func)(const char *, size_t, void *),
			     void *arg)
{
	rfc2045_decodemimesection(fd, p, func, arg);
}

