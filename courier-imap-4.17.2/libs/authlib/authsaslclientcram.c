/* $Id: authsaslclientcram.c,v 1.2 2000/07/24 02:09:55 mrsam Exp $ */

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

int authsaslclient_cram(const struct authsaslclientinfo *info,
				const char *challenge,
				const struct hmac_hashinfo *hashinfo)
{
char	*base64buf=malloc(strlen(challenge)+1);
char	*keybuf, *p;
const	char *userid=info->userid ? info->userid:"";
const	char *password=info->password ? info->password:"";
int	i;

	if (!base64buf)
	{
		perror("malloc");
		return (AUTHSASL_ERROR);
	}
	strcpy(base64buf, challenge);

	if ( (i=authsasl_frombase64(base64buf))<0 ||
		(keybuf=malloc(hashinfo->hh_L*3)) == 0)
	{
		free(base64buf);
		perror("malloc");
		return (AUTHSASL_ERROR);
	}

	hmac_hashkey( hashinfo, password, strlen(password),
                        keybuf, keybuf+hashinfo->hh_L );

	hmac_hashtext( hashinfo, base64buf, i,
                keybuf, keybuf+hashinfo->hh_L,
                keybuf+hashinfo->hh_L*2);

	free(base64buf);
	base64buf=malloc(strlen(userid)+2+hashinfo->hh_L*2);
	if (!base64buf)
	{
		perror("malloc");
		free(keybuf);
		return (AUTHSASL_ERROR);
	}
	strcat(strcpy(base64buf, userid), " ");
	p=base64buf+strlen(base64buf);
	for (i=0; i<hashinfo->hh_L; i++)
	{
	static const char xdigit[]="0123456789abcdef";
	int	c=keybuf[hashinfo->hh_L*2+i];

		*p++ = xdigit[ (c >> 4) & 0x0F ];
		*p++ = xdigit[c & 0x0F];
        }
	*p=0;
	free(keybuf);
	keybuf=authsasl_tobase64(base64buf, -1);
	free(base64buf);

	if (!keybuf)
	{
		perror("malloc");
		free(keybuf);
		return (AUTHSASL_ERROR);
	}
	i= (*info->final_conv_func)(keybuf, info->conv_func_arg);
	free(keybuf);
	return (i);
}

#else

struct hmac_hashinfo;

int authsaslclient_cram(const struct authsaslclientinfo *info,
				const char *challenge,
				const struct hmac_hashinfo *hashinfo)
{
	return (AUTHSASL_NOMETHODS);
}

#endif

