/*
** Copyright 2003 Double Precision, Inc.  See COPYING for
** distribution information.
*/

#include	"auth.h"
#include	"authmod.h"
#include	"authstaticlist.h"
#include	"authsasl.h"
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<errno.h>
#if	HAVE_UNISTD_H
#include	<unistd.h>
#endif
#include	"numlib/numlib.h"

static const char rcsid[]="$Id: authenumerate.c,v 1.1 2004/01/11 02:47:32 mrsam Exp $";

static void enum_cb(const char *name,
		    uid_t uid,
		    gid_t gid,
		    const char *homedir,
		    const char *maildir,
		    void *void_arg)
{
	char buf1[NUMBUFSIZE];
	char buf2[NUMBUFSIZE];

	if (name == NULL)
	{
		*(int *)void_arg=0;
		return;
	}

	printf("%s\t%s\t\%s\t%s", name,
	       libmail_str_uid_t(uid, buf1),
	       libmail_str_gid_t(gid, buf2),
	       homedir);
	if (maildir && *maildir)
	{
		printf("\t%s", maildir);
	}
	printf("\n");
}

int main(int argc, char **argv)
{
	int	argn;
	const char *module="";
	int	i;
	int exit_code;

	if (getuid())
	{
		printf("I must be run as root!\n");
		exit(0);
	}

	for (argn=1; argn<argc; argn++)
	{
	const char *argp;

		if (argv[argn][0] != '-')	break;
		if (argv[argn][1] == 0)
		{
			++argn;
			break;
		}

		argp=argv[argn]+2;

		switch (argv[argn][1])	{
		case 'm':
			if (!*argp && argn+1 < argc)
				argp=argv[++argn];
			module=argp;
			break;
		default:
			fprintf(stderr, "Usage: %s [-m module]\n",
				argv[0]);
			exit(1);
		}
	}

	exit_code=1;
	for (i=0; authstaticmodulelist[i]; i++)
	{
		if (*module && strcmp(module,
				      authstaticmodulelist[i]->auth_name))
			continue;

		if (authstaticmodulelist[i]->auth_enumerate == NULL)
			continue;

		printf("# %s\n\n", authstaticmodulelist[i]->auth_name);
		exit_code=1;
		(*authstaticmodulelist[i]->auth_enumerate)(enum_cb,
							   &exit_code);

		if (exit_code)
			exit(exit_code);
	}
	exit(0);
	return (0);
}
