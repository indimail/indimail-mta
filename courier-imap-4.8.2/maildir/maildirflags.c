/*
** Copyright 2000-2002 Double Precision, Inc.
** See COPYING for distribution information.
*/

#include	"config.h"
#include	<sys/types.h>
#include	<string.h>

static const char rcsid[]="$Id: maildirflags.c,v 1.4 2011/01/09 05:27:05 mrsam Exp $";

int maildir_hasflag(const char *filename, char flag)
{
	const char *p=strrchr(filename, '/');

	if (p)
		filename=p+1;

	p=strrchr(filename, MDIRSEP[0]);
	if (p && strncmp(p, MDIRSEP "2,", 3) == 0 &&
	    strchr(p+3, flag))
		return (1);
	return (0);
}
