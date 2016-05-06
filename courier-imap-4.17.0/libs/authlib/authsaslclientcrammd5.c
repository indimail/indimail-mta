/* $Id: authsaslclientcrammd5.c,v 1.1 2000/07/23 21:00:47 mrsam Exp $ */

/*
** Copyright 2000 Double Precision, Inc.  See COPYING for
** distribution information.
*/

#include	"config.h"
#include	"authsasl.h"

#if	HAVE_HMACLIB
#include	"libhmac/hmac.h"
#endif

#include	"authsaslclient.h"
#include	<stdlib.h>
#include	<stdio.h>
#include	<ctype.h>
#include	<string.h>
#include	<errno.h>

#if	HAVE_HMACLIB

extern int authsaslclient_cram(const struct authsaslclientinfo *info,
				const char *p,
				const struct hmac_hashinfo *);

int authsaslclient_crammd5(const struct authsaslclientinfo *info)
{
const char *p=(*info->start_conv_func)("CRAM-MD5", NULL, info->conv_func_arg);

	if (!p) return (AUTHSASL_CANCELLED);
	return ( authsaslclient_cram(info, p, &hmac_md5));
}

#else
int authsaslclient_crammd5(const struct authsaslclientinfo *info)
{
	return (AUTHSASL_NOMETHODS);
}

#endif

