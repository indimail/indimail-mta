/*
 * $Log: vpriv.c,v $
 * Revision 2.8  2016-06-09 15:32:32+05:30  Cprogrammer
 * run if indimail gid is present in process supplementary groups
 *
 * Revision 2.7  2016-06-09 14:22:56+05:30  Cprogrammer
 * allow privilege to process running with indimail gid
 *
 * Revision 2.6  2011-11-09 19:46:29+05:30  Cprogrammer
 * removed getversion
 *
 * Revision 2.5  2009-10-19 11:24:54+05:30  Cprogrammer
 * allow only root/indimail user to execute program
 *
 * Revision 2.4  2008-06-13 10:58:23+05:30  Cprogrammer
 * print error when run if CLUSTERED_SITE not defined
 *
 * Revision 2.3  2008-05-28 17:41:52+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 2.2  2003-09-16 12:35:49+05:30  Cprogrammer
 * added option V_PRIV_GRANT to grant access to all programs
 *
 * Revision 2.1  2003-09-14 02:00:21+05:30  Cprogrammer
 * function to administer privileges
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: vpriv.c,v 2.8 2016-06-09 15:32:32+05:30 Cprogrammer Stab mbhangui $";
#endif

#ifdef CLUSTERED_SITE
#include <unistd.h>
#include <sys/types.h>

#define V_PRIV_SELECT 0
#define V_PRIV_INSERT 1
#define V_PRIV_DELETE 2
#define V_PRIV_UPDATE 3
#define V_PRIV_DELUSR 4
#define V_PRIV_GRANT  5

void            usage();
int             get_options(int, char **, char **, char **, char **, char **, int *);

int
main(argc, argv)
	int             argc;
	char           *argv[];
{
	int             action, err, i;
	char           *ptr, *user, *program, *cmdargs, *oldcmdargs;
	uid_t           uid;
	gid_t           gid;

	if (indimailuid == -1 || indimailgid == -1)
		GetIndiId(&indimailuid, &indimailgid);
	uid = getuid();
	gid = getgid();
	if (uid != 0 && uid != indimailuid && gid != indimailgid && check_group(indimailgid) != 1)
	{
		error_stack(stderr, "you must be root or indimail to run this program\n");
		return(1);
	}
	if(get_options(argc, argv, &user, &program, &cmdargs, &oldcmdargs, &action))
		return(1);
	if(action != V_PRIV_SELECT)
	{
		if(!user || !*user)
		{
			usage();
			return(1);
		}
		if(mgmtpassinfo(user, 0) && userNotFound)
			return(1);
	}
	switch (action)
	{
	case V_PRIV_SELECT:
		for(;;)
		{
			if(!(ptr = vpriv_select(&user, &program)))
				break;
			printf("%s -> %-20s %s\n", user, program, ptr);
		}
		break;
	case V_PRIV_INSERT:
		return(vpriv_insert(user, program, cmdargs));
		break;
	case V_PRIV_DELETE: /*- Delete program */
		return(vpriv_delete(user, program));
		break;
	case V_PRIV_UPDATE:
		return(vpriv_update(user, program, cmdargs));
		break;
	case V_PRIV_DELUSR:
		return(vpriv_delete(user, 0));
		break;
	case V_PRIV_GRANT:
		for (err = i = 0; adminCommands[i].name; i++)
		{
			if(vpriv_insert(user, adminCommands[i].name, "*"))
				err = 1;
		}
		return(err);
		break;
	default:
		fprintf(stderr, "error, Action is invalid %d\n", action);
		return(1);
	}
	return(0);
}

void
usage()
{
	fprintf(stderr, "usage: vpriv [options] user CommandLineSwitches\n");
	fprintf(stderr, "options: -V ( print version number )\n");
	fprintf(stderr, "         -v ( verbose )\n");
	fprintf(stderr, "         -s ( show privileges )\n");
	fprintf(stderr, "         -d program (remove privilege to run program)\n");
	fprintf(stderr, "         -i program (add privilege to run program)\n");
	fprintf(stderr, "         -m program (modify privilege)\n");
	fprintf(stderr, "         -D Delete All Privileges for user\n");
	fprintf(stderr, "         -a Set All Privileges for user\n");
}

int
get_options(int argc, char **argv, char **user, char **program,
	char **cmdargs, char **oldcmdargs, int *action)
{
	int             c;
	extern char    *optarg;
	extern int      optind;

	verbose = 0;
	*action = -1;
	*user = *program = *cmdargs = 0;
	while ((c = getopt(argc, argv, "vasDd:i:m:")) != -1)
	{
		switch (c)
		{
		case 'v':
			verbose = 1;
			break;
		case 'a':
			*action = V_PRIV_GRANT;
			break;
		case 's':
			*action = V_PRIV_SELECT;
			break;
		case 'D':
			*action = V_PRIV_DELUSR;
			break;
		case 'd':
			*action = V_PRIV_DELETE;
			*program = optarg;
			break;
		case 'i':
			*action = V_PRIV_INSERT;
			*program = optarg;
			break;
		case 'm':
			*action = V_PRIV_UPDATE;
			*program = optarg;
			break;
		default:
			usage();
			return(1);
		}
	}
	if (*action == -1)
	{
		usage();
		return(1);
	}
	if (optind < argc)
		*user = argv[optind++];
	if (optind < argc)
		*cmdargs = argv[optind++];
	if (*action != V_PRIV_SELECT && !*user)
	{
		usage();
		fprintf(stderr, "must supply user\n");
		return(1);
	}
	if(*action == V_PRIV_GRANT)
		return(0);
	if(!*program && (*action == V_PRIV_INSERT || *action == V_PRIV_UPDATE
		|| *action == V_PRIV_DELETE))
	{
		usage();
		fprintf(stderr, "must supply program\n");
		return(1);
	}
	if((*action == V_PRIV_UPDATE || *action == V_PRIV_INSERT ) && !*cmdargs)
	{
		usage();
		fprintf(stderr, "must supply Command Line Switches\n");
		return(1);
	}
	return(0);
}
#else
int
main()
{
	printf("IndiMail not configured with --enable-user-cluster=y\n");
	return(0);
}
#endif

void
getversion_vpriv_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
