/*
** Copyright 2000 Double Precision, Inc.
** See COPYING for distribution information.
*/

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include	<sys/types.h>
#include	<sys/stat.h>
#include	<string.h>
#include	<stdlib.h>
#include	<time.h>
#if	HAVE_UNISTD_H
#include	<unistd.h>
#endif
#include	<stdio.h>
#include	<ctype.h>
#include	<errno.h>

#include	"maildirmisc.h"


char *maildir_name2dir(const char *maildir,	/* DIR location */
		       const char *foldername) /* INBOX.name */
{
	const char *inbox=INBOX;
	int l=strlen(inbox);
	char *p;

	if (!maildir)	maildir=".";

	if (foldername && strncasecmp(foldername, INBOX, l) == 0 &&
	    strchr(foldername, '/') == NULL)
	{
		if (foldername[l] == 0)
			return strdup(maildir); /* INBOX: main maildir inbox */

		if (foldername[l] == '.')
		{
			const char *r;

			for (r=foldername; *r; r++)
			{
				if (*r != '.')	continue;

				if (r[1] == 0 || r[1] == '.')
				{
					errno=EINVAL;
					return (0);
				}
			}

			r=strchr(foldername, '.');

			p=malloc(strlen(maildir)+strlen(r) + 2);

			if (!p)
				return NULL;

			return (strcat(strcat(strcpy(p, maildir), "/"),
				       r));
		}
	}

	errno=EINVAL;
	return NULL;
}

char *maildir_location(const char *homedir,
		       const char *maildir)
{
	char *p;

	if (*maildir == '/')
		return strdup(maildir);

	p=malloc(strlen(homedir)+strlen(maildir)+2);

	if (!p)
		return NULL;
	strcat(strcat(strcpy(p, homedir), "/"), maildir);
	return p;
}

char *maildir_folderdir(const char *maildir, const char *foldername)
{
char	*p;
const char *r;
size_t	l;

	if (!maildir)	maildir=".";
	l=strlen(maildir);

	if (foldername == 0 ||
		strcasecmp(foldername, INBOX) == 0)
	{
		if ((p=malloc(l+1)) == 0)	return (0);
		strcpy(p, maildir);
		return(p);
	}

	/* Rules: no leading/trailing periods, no /s */
	if (*foldername == '.' || strchr(foldername, '/'))
	{
		errno=EINVAL;
		return (0);
	}

	for (r=foldername; *r; r++)
	{
		if (*r != '.')	continue;
		if (r[1] == 0 || r[1] == '.')
		{
			errno=EINVAL;
			return (0);
		}
	}

	if ((p=malloc(l+strlen(foldername)+3)) == 0)	return (0);
	*p=0;
	if (strcmp(maildir, "."))
		strcat(strcpy(p, maildir), "/");
	
	return (strcat(strcat(p, "."), foldername));
}
