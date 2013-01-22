/*
 * $Log: vhostid.c,v $
 * Revision 2.2  2011-11-09 19:46:16+05:30  Cprogrammer
 * removed getversion
 *
 * Revision 2.1  2002-05-11 15:24:19+05:30  Cprogrammer
 * Usage text corrected
 *
 * Revision 1.1  2002-03-29 20:47:34+05:30  Cprogrammer
 * Initial revision
 *
 */

#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: vhostid.c,v 2.2 2011-11-09 19:46:16+05:30 Cprogrammer Stab mbhangui $";
#endif

#ifdef CLUSTERED_SITE
#include <unistd.h>
#include <string.h>

char            HostId[MAX_BUFF];
char            IpAddr[MAX_BUFF];

#define HOST_SELECT 0
#define HOST_INSERT 1
#define HOST_DELETE 2
#define HOST_UPDATE 3

int             HostAction;

void            usage();
int             get_options(int argc, char **argv);

int
main(argc, argv)
	int             argc;
	char           *argv[];
{
	char           *tmphost_line;

	if(get_options(argc, argv))
		return(0);
	switch (HostAction)
	{
	case HOST_SELECT:
		printf("%-30s %s\n", "Hostid", "IP Address");
		if(*HostId && (tmphost_line = vauth_getipaddr(HostId)))
			printf("%-30s %s\n", HostId, tmphost_line);
		else
		for(;;)
		{
			if(!(tmphost_line = vhostid_select()))
				break;
			printf("%s\n", tmphost_line);
		}
		break;
	case HOST_INSERT:
		vhostid_insert(HostId, IpAddr);
		break;
	case HOST_DELETE:
		vhostid_delete(HostId);
		break;
	case HOST_UPDATE:
		vhostid_update(HostId, IpAddr);
		break;
	default:
		fprintf(stderr, "error, HostId Action is invalid %d\n", HostAction);
		break;
	}
	return(0);
}

void
usage()
{
	fprintf(stderr, "usage: vhostid [options] [HostId]\n");
	fprintf(stderr, "options: -V        (print version number)\n");
	fprintf(stderr, "         -v        (verbose)\n");
	fprintf(stderr, "         -s        (show HostId Mappings)\n");
	fprintf(stderr, "         -d HostId (delete Mapping for HostId)\n");
	fprintf(stderr, "         -i Ipaddr (Add Mapping for HostId to Ipaddr)\n");
	fprintf(stderr, "         -u Ipaddr (Map HostId to New_Ipaddr)\n");
}

int
get_options(int argc, char **argv)
{
	int             c;
	extern char    *optarg;
	extern int      optind;

	memset(HostId, 0, MAX_BUFF);
	memset(IpAddr, 0, MAX_BUFF);
	HostAction = HOST_SELECT;
	while ((c = getopt(argc, argv, "vsdu:i:")) != -1)
	{
		switch (c)
		{
		case 'v':
			verbose = 1;
			break;
		case 's':
			HostAction = HOST_SELECT;
			scopy(IpAddr, optarg, MAX_BUFF);
			break;
		case 'u':
			HostAction = HOST_UPDATE;
			scopy(IpAddr, optarg, MAX_BUFF);
			break;
		case 'd':
			HostAction = HOST_DELETE;
			scopy(IpAddr, optarg, MAX_BUFF);
			break;
		case 'i':
			HostAction = HOST_INSERT;
			scopy(IpAddr, optarg, MAX_BUFF);
			break;
		default:
			usage();
			return(1);
		}
	}
	if (optind < argc)
		scopy(HostId, argv[optind++], MAX_BUFF);
	if (HostAction != HOST_SELECT && !*HostId)
	{
		usage();
		fprintf(stderr, "must supply Host Id\n");
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
getversion_valias_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
