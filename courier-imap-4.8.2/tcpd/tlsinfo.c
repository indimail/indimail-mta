/*
** Copyright 2002 Double Precision, Inc.
** See COPYING for distribution information.
*/
#include	"config.h"
#include	"libcouriertls.h"
#include	<stdio.h>
#include	<string.h>
#include	<stdlib.h>
#if	HAVE_UNISTD_H
#include	<unistd.h>
#endif

static const char rcsid[]="$Id: tlsinfo.c,v 1.3 2002/07/11 16:37:42 mrsam Exp $";

static const char *getenv_wrapper(const char *varname, void *dummy)
{
	return getenv(varname);
}

static void report_stderr(const char *errmsg, void *dummy)
{
	fprintf(stderr, "%s\n", errmsg);
}

static const struct tls_info default_info
	= { NULL, NULL, report_stderr, getenv_wrapper, NULL };

const struct tls_info *tls_get_default_info()
{
	return &default_info;
}

