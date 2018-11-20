/*
** Copyright 2003 Double Precision, Inc.
** See COPYING for distribution information.
*/


#if	HAVE_CONFIG_H
#include	"config.h"
#endif
#include	<stdio.h>
#include	<string.h>
#include	<stdlib.h>
#if	HAVE_FCNTL_H
#include	<fcntl.h>
#endif
#if	HAVE_UNISTD_H
#include	<unistd.h>
#endif
#include	<sys/types.h>
#if	HAVE_SYS_STAT_H
#include	<sys/stat.h>
#endif
#include	<ctype.h>
#include	<errno.h>

#include	"maildirmisc.h"
#include	"maildiraclt.h"

static const char resetcmd[]="-reset";
static const char listcmd[]="-list";
static const char setcmd[]="-set";
static const char deletecmd[]="-delete";
static const char computecmd[]="-compute";

static void usage()
{
	printf("Usage: maildiracl [ options ] maildir [INBOX[.folder] [identifier rights]]\n");
	exit(1);
}

static int acl_list(const char *identifier,
		    const maildir_aclt *acl,
		    void *cb_arg)
{
	printf("%s\t%s\n", identifier, maildir_aclt_ascstr(acl));
	return (0);
}

struct computeinfo {
	int argc;
	char **argv;
};

static int isme(const char *identifier,
		void *void_arg)
{
	struct computeinfo *ci=(struct computeinfo *)void_arg;
	int i;

	for (i=4; i<ci->argc; i++)
		if (strcmp(ci->argv[i], identifier) == 0)
			return 1;
	return 0;
}

int main(int argc, char *argv[])
{
	const char *cmd;
	const char *maildir;
	const char *folder;

	if (argc < 3)
		usage();

	cmd=argv[1];

	if (strcmp(cmd, resetcmd) &&
	    strcmp(cmd, listcmd) &&
	    strcmp(cmd, setcmd) &&
	    strcmp(cmd, deletecmd) &&
	    strcmp(cmd, computecmd))
		usage();

	maildir=argv[2];

	if (strcmp(cmd, resetcmd) == 0)
	{
		if (maildir_acl_reset(maildir))
		{
			perror(maildir);
			exit(1);
		}
		exit(0);
	}

	if (argc < 4)
		usage();

	folder=argv[3];

	if (strcmp(folder, INBOX) &&
	    strncmp(folder, INBOX ".", sizeof(INBOX ".")-1))
	{
		errno=EINVAL;
		perror(folder);
		exit(1);
	}
	folder += sizeof(INBOX)-1;

	if (!*folder)
		folder=".";

	if (strcmp(cmd, listcmd) == 0)
	{
		maildir_aclt_list l;

		if (maildir_acl_read(&l, maildir, folder) ||
		    maildir_aclt_list_enum(&l, acl_list, NULL))
		{
			perror(maildir);
			exit(1);
		}

		maildir_aclt_list_destroy(&l);
		exit(0);
	}

	if (strcmp(cmd, setcmd) == 0)
	{
		maildir_aclt_list l;
		maildir_aclt a;

		const char *identifier;
		const char *rights;
		const char *err_failedrights;

		if (argc < 6)
			usage();

		identifier=argv[4];
		rights=argv[5];

		if (maildir_acl_read(&l, maildir, folder))
		{
			perror(maildir);
			exit(1);
		}

		if (*rights == '+')
		{
			if (maildir_aclt_init(&a, NULL,
					      maildir_aclt_list_find(&l,
								     identifier
								     )) ||
			    maildir_aclt_add(&a, rights+1, NULL))
			{
				perror(argv[0]);
				exit(1);
			}
		} else if (*rights == '-')
		{
			if (maildir_aclt_init(&a, NULL,
					      maildir_aclt_list_find(&l,
								     identifier
								     )) ||
			    maildir_aclt_del(&a, rights+1, NULL))
			{
				perror(argv[0]);
				exit(1);
			}
		}
		else if (maildir_aclt_init(&a, rights, NULL))
		{
			perror(argv[0]);
			exit (1);
		}

		if (maildir_aclt_list_add(&l, identifier, NULL, &a))
		{
			perror(argv[0]);
			exit(1);
		}

		if (maildir_acl_write(&l, maildir, folder, "owner",
				      &err_failedrights))
		{
			if (err_failedrights)
			{
				fprintf(stderr,
					"Trying to set invalid access"
					" rights for %s\n",
					err_failedrights);
			}
			else perror(maildir);
			exit(1);
		}
	}

	if (strcmp(cmd, deletecmd) == 0)
	{
		maildir_aclt_list l;
		const char *identifier;
		const char *err_failedrights;

		if (argc < 5)
			usage();

		identifier=argv[4];

		if (maildir_acl_read(&l, maildir, folder))
		{
			perror(maildir);
			exit(1);
		}

		if (maildir_aclt_list_del(&l, identifier))
		{
			perror(maildir);
			exit(1);
		}

		if (maildir_acl_write(&l, maildir, folder, "owner",
				      &err_failedrights))
		{
			if (err_failedrights)
			{
				fprintf(stderr,
					"Trying to set invalid access"
					" rights for %s\n",
					err_failedrights);
			}
			else perror(maildir);
			exit(1);
		}
	}

	if (strcmp(cmd, computecmd) == 0)
	{
		maildir_aclt_list l;
		maildir_aclt a;

		struct computeinfo ci;

		ci.argc=argc;
		ci.argv=argv;

		if (argc < 5)
			usage();

		if (maildir_acl_read(&l, maildir, folder))
		{
			perror(maildir);
			exit(1);
		}

		if (maildir_acl_compute(&a, &l, isme, &ci))
		{
			perror(maildir);
			exit(1);
		}

		printf("%s\n", maildir_aclt_ascstr(&a));
	}

	return (0);
}
