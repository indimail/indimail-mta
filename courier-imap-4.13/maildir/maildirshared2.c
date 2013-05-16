/*
** Copyright 2000 Double Precision, Inc.
** See COPYING for distribution information.
*/

#include	"config.h"
#include	"maildirmisc.h"
#include	<stdio.h>
#include	<string.h>
#include	<stdlib.h>
#include	<ctype.h>
#include	<errno.h>


FILE *maildir_shared_fopen(const char *maildir, const char *mode)
{
char	*m;
FILE	*fp;

	m=malloc(strlen(maildir)+sizeof("/shared-maildirs"));
	if (!m)
	{
		perror("malloc");
		return (0);
	}
	strcat(strcpy(m, maildir), "/shared-maildirs");

	fp=fopen(m, mode);
	free(m);
	return (fp);
}

void maildir_shared_fparse(char *p, char **name, char **dir)
{
char	*q;

	*name=0;
	*dir=0;

	if ((q=strchr(p, '\n')) != 0)	*q=0;
	if ((q=strchr(p, '#')) != 0)	*q=0;

	for (q=p; *q; q++)
		if (isspace((int)(unsigned char)*q))	break;
	if (!*q)	return;
	*q++=0;
	while (*q && isspace((int)(unsigned char)*q))
		++q;
	if (*q)
	{
		*name=p;
		*dir=q;
	}
}

char *maildir_shareddir(const char *maildir, const char *sharedname)
{
char    *p, *q;
const char *r;

	if (!maildir)   maildir=".";

	if (strchr(sharedname, '.') == 0 || *sharedname == '.' ||
		strchr(sharedname, '/'))
	{
		errno=EINVAL;
		return (0);
	}

	for (r=sharedname; *r; r++)
	{
		if (*r == '.' && (r[1] == '.' || r[1] == '\0'))
		{
			errno=EINVAL;
			return (0);
		}
	}

	p=malloc(strlen(maildir)+sizeof("/" SHAREDSUBDIR "/")+strlen(sharedname));
	if (!p)	return (0);

        *p=0;
        if (strcmp(maildir, "."))
		strcat(strcpy(p, maildir), "/");
	strcat(p, SHAREDSUBDIR "/");
	q=p+strlen(p);
	strcpy(q, sharedname);
	*strchr(q, '.')='/';
	return (p);
}
