/*
** Copyright 1998 - 2004 Double Precision, Inc.  See COPYING for
** distribution information.
*/

#include	<stdio.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>
#include	<errno.h>
#if	HAVE_UNISTD_H
#include	<unistd.h>
#endif
#include	"auth.h"
#include	"authmod.h"
#include	"authstaticlist.h"
#include	"authsasl.h"
#include	"debug.h"

static const char rcsid[]="$Id: authtest.c,v 1.11 2004/04/20 01:34:44 mrsam Exp $";

static void usage()
{
int i;

	fprintf(stderr, "Usage: authtest [-s service] [ -m module ] userid [ password [ newpassword ] ]\n"
		"Modules available:");
	for (i=0; authstaticmodulelist[i]; i++)
		fprintf(stderr, " %s", authstaticmodulelist[i]->auth_name);
	fprintf(stderr, "\n");	        
	exit(1);
}

static int callback_pre(struct authinfo *a, void *dummy)
{
        authsuccess(a->homedir, a->sysuserid ? 0:a->sysusername,
		a->sysuserid, &a->sysgroupid,
                a->address, a->fullname);

	if (a->maildir)
	{
	char	*p=malloc(sizeof("MAILDIR=")+strlen(a->maildir));

		strcat(strcpy(p, "MAILDIR="), a->maildir);
		putenv(p);
	}
	else	putenv("MAILDIR=");

	if (a->options)
	{
		char	*p=malloc(sizeof("OPTIONS=")+strlen(a->options));

		strcat(strcpy(p, "OPTIONS="), a->options);
		putenv(p);
	}
	else	putenv("OPTIONS=");

	return (0);
}

int main(int argc, char **argv)
{
int	argn;
const char *service="login";
const char *module="";
int	i;
int	found=0;
char	*p;

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
		case 's':
			if (!*argp && argn+1 < argc)
				argp=argv[++argn];
			service=argp;
			break;
		case 'm':
			if (!*argp && argn+1 < argc)
				argp=argv[++argn];
			module=argp;
			break;
		default:
			usage();
		}
	}
	if (argc - argn <= 0)
		usage();

	auth_debug_login_level = 2;

	for (i=0; authstaticmodulelist[i]; i++)
	{
	char	*authdata;
	char	buf[1024];

		if (*module && strcmp(module,
				      authstaticmodulelist[i]->auth_name))
			continue;

		found = 1;
		fprintf(stderr, "Trying %s...\n",
			authstaticmodulelist[i]->auth_name);
		if (argc - argn > 2)
		{
			if (authstaticmodulelist[i]->auth_changepwd == 0)
				continue;

			if ((*authstaticmodulelist[i]->auth_changepwd)
			    (service, argv[argn], argv[argn+1],
			     argv[argn+2]) == 0)
			{
				printf("Password CHANGED!\n");
				exit (0);
			}
			break;
		}

		if (argc - argn > 1)
		{
			authdata=malloc(strlen(argv[argn])+strlen(argv[argn+1])+3);
			if (!authdata)
			{
				perror("malloc");
				exit(1);
			}
			sprintf(authdata, "%s\n%s\n",
				argv[argn], argv[argn+1]);

			p= (*authstaticmodulelist[i]->auth_func)(service,
				AUTHTYPE_LOGIN, authdata, 0, 0, 0);
			free(authdata);
			if (p == 0)
			{
				if (errno != EPERM)
				{
					fprintf(stderr,
						"Temporary authentication failure from "
						"module %s\n",
						authstaticmodulelist[i]->auth_name);
					break;
				}
				continue;
			}
		}
		else
		{
		int	rc;

			if ((rc=(*authstaticmodulelist[i]->auth_prefunc)
			     (argv[argn], service ? service:"",
				&callback_pre, 0)) != 0)
			{
				(*authstaticmodulelist[i]->auth_cleanupfunc)();
				if (rc < 0)
					continue;
				fprintf(stderr,
					"Temporary authentication failure from "
					"module %s\n",
					authstaticmodulelist[i]->auth_name);
				break;
			}
			(*authstaticmodulelist[i]->auth_cleanupfunc)();
		}

		printf("Authenticated: module %s\n",
		       authstaticmodulelist[i]->auth_name);
		if ( getcwd(buf, sizeof(buf)))
			printf("Home directory: %s\n", buf);
		else
			printf("Unable to determine home directory!!\n");

		printf("UID/GID: %lu/%lu\n",
			(unsigned long)getuid(), (unsigned long)getgid());

		p=getenv("MAILDIR");
		if (p && *p)
			printf("Maildir: %s\n", p);

		p=getenv("MAILDIRQUOTA");
		if (p && *p)
			printf("Maildir quota: %s\n", p);

		p=getenv("AUTHADDR");
		printf("AUTHADDR=%s\n", p && *p ? p:"<none>");
		p=getenv("AUTHFULLNAME");
		printf("AUTHFULLNAME=%s\n", p && *p ? p:"<none>");
		p=getenv("OPTIONS");
		printf("OPTIONS=%s\n", p && *p ? p:"<none>");
		exit(0);
	}
	if (!found)
		fprintf(stderr, "No matching module found!\n");
	fprintf(stderr, "Authentication FAILED!\n");
	exit(1);
	return (0);
}
