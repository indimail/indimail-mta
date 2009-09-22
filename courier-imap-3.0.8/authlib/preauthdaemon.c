/*
** Copyright 2000 Double Precision, Inc.  See COPYING for
** distribution information.
*/

#include	"auth.h"
#include	"authmod.h"
#include	"authstaticlist.h"
#include	"authsasl.h"
#include	<stdlib.h>
#include	<stdio.h>
#include	<string.h>

static const char rcsid[]="$Id: preauthdaemon.c,v 1.1 2000/04/26 23:28:21 mrsam Exp $";

extern int authdaemondo(const char *authreq,
	int (*func)(struct authinfo *, void *), void *arg);

int auth_daemon_pre(const char *uid, const char *service,
        int (*callback)(struct authinfo *, void *),
                        void *arg)
{
char    *buf=malloc(strlen(service)+strlen(uid)+20);
int     rc;

	if (!buf)
	{
		perror("malloc");
		return (1);
	}
	strcat(strcat(strcat(strcat(strcpy(buf, "PRE . "), service), " "),
		uid), "\n");

	rc=authdaemondo(buf, callback, arg);
	free(buf);
	return (rc);
}
