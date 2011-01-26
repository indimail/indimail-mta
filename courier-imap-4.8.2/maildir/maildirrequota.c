/*
** Copyright 1998 - 2002 Double Precision, Inc.
** See COPYING for distribution information.
*/

#include	"config.h"
#include	<stdlib.h>
#include	<string.h>
#include	<stdio.h>
#include	<ctype.h>
#include	"maildirrequota.h"

static const char rcsid[]="$Id: maildirrequota.c,v 1.4 2002/07/02 20:01:37 mrsam Exp $";

char *maildir_requota(const char *oldname, unsigned long s)
{
char	buf[40];
char	*p;
const char *q;

	sprintf(buf, ",S=%lu", s);

	if ((p=malloc(strlen(oldname)+strlen(buf)+1)) == 0)	return (0);

	if ((q=strrchr(oldname, '/')) == 0)	q=oldname;
	while (*q)
	{
		if ((*q == ',' && q[1] == 'S' && q[2] == '=') || *q == MDIRSEP[0])
		{
			memcpy(p, oldname, q-oldname);
			strcpy(p + (q-oldname), buf);

			if (*q == ',')	q += 3;

			for ( ; isdigit((int)(unsigned char)*q); q++)
				;
			strcat(p, q);
			return (p);
		}
		++q;
	}
	return (strcat(strcpy(p, oldname), buf));
}
