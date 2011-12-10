/*
 * $Log: hostcntrl.c,v $
 * Revision 2.9  2011-11-09 19:44:15+05:30  Cprogrammer
 * removed getversion
 *
 * Revision 2.8  2008-06-13 08:56:03+05:30  Cprogrammer
 * fixed compilation error if CLUSTERED_SITE was not defined
 *
 * Revision 2.7  2008-06-03 19:15:53+05:30  Cprogrammer
 * parse_email segmentation fault if emailid was null
 *
 * Revision 2.6  2004-07-03 23:57:15+05:30  Cprogrammer
 * use parse_email()
 *
 * Revision 2.5  2004-06-19 00:18:07+05:30  Cprogrammer
 * added option to list all hostcntrl entries
 *
 * Revision 2.4  2004-05-22 23:48:19+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 2.3  2004-05-17 14:09:33+05:30  Cprogrammer
 * added option to display 'Added on' field
 *
 * Revision 2.2  2004-05-17 01:08:16+05:30  Cprogrammer
 * force flag argument added to addusercntrl()
 *
 * Revision 2.1  2004-05-17 00:58:54+05:30  Cprogrammer
 * hostcntrl maintainance utility
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: hostcntrl.c,v 2.9 2011-11-09 19:44:15+05:30 Cprogrammer Stab mbhangui $";
#endif

#ifdef CLUSTERED_SITE
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define V_USER_SELECT 0
#define V_USER_INSERT 1
#define V_USER_DELETE 2
#define V_USER_UPDATE 3
#define V_USER_DELUSR 4
#define V_SELECT_ALL  5

void            usage();
int             get_options(int, char **, char **, char **, int *);

int
main(argc, argv)
	int             argc;
	char           *argv[];
{
	int             action, i;
	char            HostID[MAX_BUFF];
	static char     user[MAX_BUFF], domain[MAX_BUFF];
	char           *hostid, *emailid;
	char          **row;
	time_t          tmval;

	if (get_options(argc, argv, &emailid, &hostid, &action))
		return(1);
	if (action != V_SELECT_ALL && action != -1 && parse_email(emailid, user, domain, MAX_BUFF))
	{
		fprintf(stderr, "%s: Email too long\n", emailid);
		return (1);
	}
	switch (action)
	{
	case V_SELECT_ALL:
		printf("%-20s %-20s %-9s %-15s %s\n", "User", "Domain", 
			"Host ID", "IP Address", "Added on");
		for(;;)
		{
			if (!(row = hostcntrl_select_all()))
				break;
			hostid = ((hostid = vauth_getipaddr(row[2])) ? hostid : "????");
			tmval = atol(row[3]);
			printf("%-20s %-20s %-9s %-15s %s", row[0], row[1], row[2],
				hostid, ctime(&tmval));
		}
		break;
	case V_USER_SELECT:
		if (!hostcntrl_select(user, domain, &tmval, HostID, MAX_BUFF))
		{
			hostid = ((hostid = vauth_getipaddr(HostID)) ? hostid : "????");
			printf("%-25s %-11s %-16s %s\n", "Email", "Host ID", "IP Address", 
				"Added on");
			printf("%-25s %-11s %-16s %s", emailid, HostID, hostid, ctime(&tmval));
			return(0);
		} else
			return (1);
	case V_USER_INSERT:
		switch ((i = addusercntrl(user, domain, hostid, "manual", 1)))
		{
		case 0:
		case 1:
		default:
			return(i);
		case 2:
			fprintf(stderr, "hostcntrl: duplicate entry %s@%s host %s\n",
				user, domain, hostid);
			return(1);

		}
		break;
	case V_USER_DELETE: /*- Delete user */
		return (delusercntrl(user, domain, 1));
	case V_USER_UPDATE:
		return (updusercntrl(user, domain, hostid, 1));
	default:
		fprintf(stderr, "error, Action %d is invalid\n", action);
		return(1);
	}
	return(0);
}

void
usage()
{
	fprintf(stderr, "usage: hostcntrl [options] emailid\n");
	fprintf(stderr, "options: -V        ( print version number )\n");
	fprintf(stderr, "         -v        ( verbose )\n");
	fprintf(stderr, "         -l        ( List all hostcntrl entries )\n");
	fprintf(stderr, "         -s        ( show   hostcntrl entry )\n");
	fprintf(stderr, "         -d        ( remove hostcntrl entry )\n");
	fprintf(stderr, "         -i hostid ( insert hostcntrl entry )\n");
	fprintf(stderr, "         -m hostid ( modify hostcntrl entry )\n");
}

int
get_options(int argc, char **argv, char **email, char **hostid, int *action)
{
	int             c;

	verbose = 0;
	*action = -1;
	*hostid = *email = 0;
	while ((c = getopt(argc, argv, "vlsdi:m:")) != -1)
	{
		switch (c)
		{
		case 'v':
			verbose = 1;
			break;
		case 'l':
			*action = V_SELECT_ALL;
			break;
		case 's':
			*action = V_USER_SELECT;
			break;
		case 'd':
			*action = V_USER_DELETE;
			break;
		case 'i':
			*action = V_USER_INSERT;
			*hostid = optarg;
			break;
		case 'm':
			*action = V_USER_UPDATE;
			*hostid = optarg;
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
	if (*action == V_SELECT_ALL)
		return(0);
	if (optind < argc)
		*email = argv[optind++];
	if (!*email)
	{
		fprintf(stderr, "must supply emailid\n");
		usage();
		return(1);
	}
	if ((*action == V_USER_INSERT || *action == V_USER_UPDATE) && !*hostid)
	{
		fprintf(stderr, "must supply hostid\n");
		usage();
		return(1);
	}
	return(0);
}
#else
int
main()
{
	fprintf(stderr, "IndiMail not configured with --enable-user-cluster=y\n");
	return(0);
}
#endif

void
getversion_hostcntrl_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
