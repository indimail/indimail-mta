/*
** Copyright 2000-2001 Double Precision, Inc.  See COPYING for
** distribution information.
*/

#include	"auth.h"
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#if	HAVE_UNISTD_H
#include	<unistd.h>
#endif
#include	<fcntl.h>
#include        <unistd.h>
#include        <stdlib.h>
#include        <errno.h>
#include	<signal.h>

static const char rcsid[]="$Id: authdaemonpwd.c,v 1.2 2004/04/11 23:14:03 mrsam Exp $";

extern int authdaemondopasswd(char *, int);

int main()
{
	char buf[BUFSIZ];
	char *p;

	strcpy(buf,"PASSWD ");

	if (fgets(buf+7, sizeof(buf)-10, stdin) == NULL)
		exit(1);

	if ((p=strchr(buf, '\n')) != 0)
		*p=0;
	strcat(buf, "\n");
	signal(SIGPIPE, SIG_IGN);
	exit(authdaemondopasswd(buf, sizeof(buf)));
	return (0);
}
