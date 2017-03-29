/*
** Copyright 1998 - 1999 Double Precision, Inc.  See COPYING for
** distribution information.
*/

#if	HAVE_CONFIG_H
#include	"config.h"
#endif
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<errno.h>
#include	"authsasl.h"
#include	"debug.h"

#if	HAVE_HMACLIB

#include	"../libhmac/hmac.h"
#include	"cramlib.h"

static int nybble(int c)
{
	if (c >= '0' && c <= '9')	return (c-'0');
	if (c >= 'a' && c <= 'f')	return (c-'a'+10);
	if (c >= 'A' && c <= 'F')	return (c-'A'+10);
	return (-1);
}

static int do_auth_verify_cram(struct hmac_hashinfo *hash,
	const char *challenge, const char *response,
	const char *hashsecret)
{
unsigned char *context;
unsigned i;

	if (strlen(hashsecret) != hash->hh_L*4 ||
		strlen(response) != hash->hh_L*2)
		return (-1);

	if ((context=malloc(hash->hh_L*3)) == 0)	return (-1);

	for (i=0; i<hash->hh_L*2; i++)
	{
	int	a=nybble(hashsecret[i*2]), b=nybble(hashsecret[i*2+1]);

		if (a < 0 || b < 0)
		{
			free(context);
			return (-1);
		}
		context[i]= a*16 + b;
	}

	hmac_hashtext(hash, challenge, strlen(challenge),
		context, context+hash->hh_L,
		context+hash->hh_L*2);

	for (i=0; i<hash->hh_L; i++)
	{
	int	a=nybble(response[i*2]), b=nybble(response[i*2+1]);

		if ( (unsigned char)(a*16+b) !=
			context[hash->hh_L*2+i])
		{
			free(context);
			return (-1);
		}
	}
	free(context);
	return (0);
}

int auth_verify_cram(struct hmac_hashinfo *hash,
	const char *challenge, const char *response,
	const char *hashsecret)
{
int rc;

	rc = do_auth_verify_cram(hash, challenge, response, hashsecret);
	dprintf(rc ? "cram validation failed" : "cram validation succeeded");
	return rc;
}

int auth_get_cram(const char *authtype, char *authdata,
	struct hmac_hashinfo **hmacptr,
	char **user, char **challenge, char **response)
{
int	i;
int	challenge_l;
int	response_l;

	if (strncmp(authtype, "cram-", 5) ||
		(*challenge=strtok(authdata, "\n")) == 0 ||
		(*response=strtok(0, "\n")) == 0)
	{
		dprintf("cram: only supports authtype=cram-*");
		errno=EPERM;
		return (-1);
	}

	for (i=0; hmac_list[i]; i++)
		if (strcmp(hmac_list[i]->hh_name, authtype+5) == 0)
			break;

	if (hmac_list[i] == 0
		|| (challenge_l=authsasl_frombase64(*challenge)) < 0
		|| (response_l=authsasl_frombase64(*response)) < 0)
	{
		dprintf("cram: unrecognised authtype or invalid base64 encoding");
		errno=EACCES;
		return (-1);
	}
	*hmacptr=hmac_list[i];

	for (i=0; i<response_l; i++)
		if ((*response)[i] == ' ')
			break;
	if (i >= response_l)
	{
		dprintf("cram: invalid base64 encoding");
		errno=EACCES;
		return (-1);
	}
	(*response)[i++]=0;
	*user = *response;
	(*response) += i;
	response_l -= i;

	/* Since base64decoded data is always lesser in size (at least),
	** we can do the following:
	*/
	(*challenge)[challenge_l]=0;
	(*response)[response_l]=0;

	/* we rely on dprintf doing a "safe" print here */
	dprintf("cram: decoded challenge/response, username '%s'", *user);
	return (0);
}

#endif
