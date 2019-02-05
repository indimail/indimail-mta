/* $Id: authsasl.c,v 1.2 2000/07/23 21:00:47 mrsam Exp $ */

/*
** Copyright 1998 - 2000 Double Precision, Inc.  See COPYING for
** distribution information.
*/

#include	"config.h"
#include	"authsasl.h"
#include	<stdlib.h>
#include	<ctype.h>
#include	<string.h>
#include	<errno.h>

/* Use the SASL_LIST macro to build authsasl_list */

#define	SASL(a,b,c) int b(const char *, const char *, \
			char *(*)(const char *), \
			char **, \
			char **);
SASL_LIST

#undef	SASL

#define	SASL(a,b,c) {a, b},

struct authsasl_info authsasl_list[] = {

SASL_LIST

	{ 0, 0}};

int authsasl(const char *method,
	const char *initreply,
	char *(*callback_func)(const char *),
	char **authtype_ptr,			/* Returned - AUTHTYPE */
	char **authdata_ptr)
{
int	i;
char	*p, *q;

	if ((p=malloc(strlen(method)+1)) == 0)
		return (0);
	strcpy(p, method);
	for (q=p; *q; q++)
		*q=toupper((int)(unsigned char)*q);

	for (i=0; authsasl_list[i].sasl_method; i++)
	{
		if (strcmp(p, authsasl_list[i].sasl_method) == 0)
		{
			free(p);
			return ( (*authsasl_list[i].sasl_func)(method,
					initreply, callback_func,
					authtype_ptr, authdata_ptr));
		}
	}
	free(p);
	errno=ENOENT;
	return (-1);
}
