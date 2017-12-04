/*
 * $Log: hostsync.c,v $
 * Revision 2.2  2002-08-11 18:39:08+05:30  Cprogrammer
 * removed Passwd Sync option (as passwd is not needed in hostcntrl)
 *
 * Revision 2.1  2002-08-11 00:27:42+05:30  Cprogrammer
 * added deletion of PID file
 *
 * Revision 1.5  2001-12-22 18:07:33+05:30  Cprogrammer
 * changed error string for open_master failure
 *
 * Revision 1.4  2001-12-21 02:21:51+05:30  Cprogrammer
 * open master
 *
 * Revision 1.3  2001-12-12 19:26:18+05:30  Cprogrammer
 * added is_alread_running to prevent multiple instances to run
 *
 * Revision 1.2  2001-12-08 17:52:50+05:30  Cprogrammer
 * added verbose option
 *
 * Revision 1.1  2001-11-29 23:22:25+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"

#ifndef lint
static char     sccsid[] = "$Id: hostsync.c,v 2.2 2002-08-11 18:39:08+05:30 Cprogrammer Stab mbhangui $";
#endif

#ifdef CLUSTERED_SITE
#include <string.h>
#include <pwd.h>
#include <unistd.h>

char            Domain[MAX_BUFF];

void            usage(char *);
int             get_options(int argc, char **argv);

int
main(int argc, char **argv)
{
	int             first, ret;
	struct passwd  *pw;

	if (get_options(argc, argv))
		return (1);
	if (is_already_running("hostsync"))
	{
		fprintf(stderr, "hostsync is already running\n");
		return (1);
	}
	if ((ret = is_distributed_domain(Domain)) == -1)
	{
		fprintf(stderr, "Unable to verify %s as a distributed domain\n", Domain);
		unlink("/tmp/hostsync.PID");
		return (1);
	} else
	if (!ret)
	{
		fprintf(stderr, "%s is not a distributed domain\n", Domain);
		unlink("/tmp/hostsync.PID");
		return (1);
	}
	if (open_master())
	{
		fprintf(stderr, "hostsync: Failed to open Master Db\n");
		unlink("/tmp/hostsync.PID");
		return (1);
	}
	for (first = 1;;)
	{
		if (!(pw = vauth_getflags(Domain, first)))
			break;
		switch (ret = pw->pw_uid)
		{
		case ADD_FLAG:
			if (verbose)
				printf("Adding      ");
			cntrl_clearaddflag(pw->pw_name, Domain, pw->pw_passwd);
			break;
		case DEL_FLAG:
			if (verbose)
				printf("Deleting    ");
			cntrl_cleardelflag(pw->pw_name, Domain);
			break;
		default:
			if (verbose)
				printf("Ignoring??? ");
			fprintf(stderr, "Invalid case %d\n", ret);
		}
		if (verbose)
			printf("user %s@%s\n", pw->pw_name, Domain);
		first++;
	}
	if (verbose)
		printf("Synced %d entries\n", first - 1);
	vclose();
	unlink("/tmp/hostsync.PID");
	return (0);
}

int
get_options(int argc, char **argv)
{
	int             c;
	int             errflag;

	memset(Domain, 0, MAX_BUFF);
	errflag = 0;
	while (!errflag && (c = getopt(argc, argv, "vd:")) != -1)
	{
		switch (c)
		{
		case 'd':
			scopy(Domain, optarg, MAX_BUFF);
			break;
		case 'v':
			verbose = 1;
			break;
		default:
			errflag = 1;
			break;
		}
	}
	if (!*Domain || errflag == 1)
	{
		usage(argv[0]);
		return (1);
	}
	return (0);
}

void
usage(char *progname)
{
	char           *ptr;

	if ((ptr = strrchr(progname, '/')) != (char *) 0)
		ptr++;
	else
		ptr = progname;
	fprintf(stderr, "usage: %s [options]\n", ptr);
	fprintf(stderr, "options: -d domain\n");
	fprintf(stderr, "         -v (verbose)\n");
}
#else
int
main()
{
	fprintf(stderr, "IndiMail not configured with --enable-user-cluster=y\n");
	return (0);
}
#endif

void
getversion_hostsync_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
