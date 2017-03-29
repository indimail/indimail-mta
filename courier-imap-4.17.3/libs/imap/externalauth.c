/*
** Copyright 1998 - 2008 Double Precision, Inc.
** See COPYING for distribution information.
*/

#if	HAVE_CONFIG_H
#include	"config.h"
#endif
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#if	HAVE_UNISTD_H
#include	<unistd.h>
#endif

const char *externalauth()
{
	const char *p=getenv("TLS_EXTERNAL");
	char *q, *r;

	if (!p || !*p)
		return NULL;

	if ((q=malloc(strlen(p)+20)) == NULL)
		return NULL;

	strcat(strcpy(q, "TLS_SUBJECT_"), p);

	for (r=q; *r; r++)
		if (*r >= 'a' && *r <= 'z')
			*r -= 'a' - 'A';

	p=getenv(q);
	free(q);

	if (p && *p)
		return p;
	return 0;
}
