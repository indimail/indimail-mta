/*
** Copyright 2002 Double Precision, Inc.
** See COPYING for distribution information.
*/

#include "config.h"
#include "maildirsearch.h"

static const char rcsid[]="$Id: maildirsearch.c,v 1.1 2002/08/11 20:50:59 mrsam Exp $";

int maildir_search_start(struct maildir_searchengine *sei, const char *s)
{
	unsigned i, j, *r;

	sei->string=s;

	if (sei->r)
		free(sei->r);

	sei->i=0;
	if ((sei->r=r=(unsigned *)malloc(sizeof(unsigned)*strlen(s))) == 0)
		return (-1);

	for (i=0; s[i]; i++)
		r[i]=0;

	for (i=0; s[i]; i++)
		for (j=0; s[i+j]; j++)
			if (s[j] != s[i+j])
			{
				if (r[i+j] < j)
					r[i+j]=j;
				break;
			}

	maildir_search_reset(sei);
	return (0);
}

