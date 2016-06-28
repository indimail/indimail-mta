/*
 * $Log: vipmap.c,v $
 * Revision 2.4  2011-11-09 19:46:19+05:30  Cprogrammer
 * removed getversion
 *
 * Revision 2.3  2003-03-24 19:29:40+05:30  Cprogrammer
 * changed command line options for MAD compatibility
 *
 * Revision 2.2  2003-01-05 16:06:21+05:30  Cprogrammer
 * set return type for main()
 *
 * Revision 2.1  2002-12-28 09:59:46+05:30  Cprogrammer
 * added informative message if no IP maps are present
 *
 * Revision 1.7  2002-02-24 22:46:57+05:30  Cprogrammer
 * added getversion
 *
 * Revision 1.6  2001-12-27 01:30:48+05:30  Cprogrammer
 * version and verbose switch update
 *
 * Revision 1.5  2001-12-08 17:45:56+05:30  Cprogrammer
 * usage message change
 *
 * Revision 1.4  2001-11-28 23:10:13+05:30  Cprogrammer
 * getversion() function call added
 *
 * Revision 1.3  2001-11-24 12:22:04+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.2  2001-11-20 11:00:53+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.1  2001-10-24 18:15:40+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: vipmap.c,v 2.4 2011-11-09 19:46:19+05:30 Cprogrammer Stab mbhangui $";
#endif

#ifdef IP_ALIAS_DOMAINS
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <memory.h>

void            usage();
int             get_options(int argc, char **argv);

int             Action;
char            Ip[MAX_BUFF];
char            Domain[MAX_BUFF];

#define PRINT_IT  0
#define ADD_IT    1
#define DELETE_IT 2
#define UPDATE_IT 3

int
main(argc, argv)
	int             argc;
	char           *argv[];
{
	int             result, first;
	char            domain[MAX_BUFF];

	if(get_options(argc, argv))
		return(1);
	switch (Action)
	{
	case ADD_IT:
		result = vadd_ip_map(Ip, Domain);
		break;
	case DELETE_IT:
		result = vdel_ip_map(Ip, Domain);
		break;
	case UPDATE_IT:
		result = vupd_ip_map(Ip, Domain);
		break;
	case PRINT_IT:
		for (first = 1;(result = vshow_ip_map(first, Ip, domain, Domain)) == 1;)
		{
			first = 0;
			printf("%s %s\n", Ip, domain);
		}
		if(first && verbose)
			printf("No IP Maps present\n");
		break;
	default:
		usage();
		return(1);
		break;
	}
	return(result);
}

void
usage()
{
	printf("usage: vipmap [options] domain\n");
	printf("options: -d ipaddr (delete mapping)\n");
	printf("         -i ipaddr (add    mapping)\n");
	printf("         -u ipaddr (update mapping)\n");
	printf("         -s print mapping\n");
	printf("         -V print version number\n");
	printf("         -v verbose\n");

}

int
get_options(int argc, char **argv)
{
	int             c;
	int             errflag;

	/*- Action = PRINT_IT; -*/
	*Ip = *Domain = 0;
	errflag = 0;
	while (!errflag && (c = getopt(argc, argv, "vsd:i:u:")) != -1)
	{
		switch (c)
		{
		case 'v':
			verbose = 1;
			break;
		case 's':
			Action = PRINT_IT;
			break;
		case 'i':
			Action = ADD_IT;
			scopy(Ip, optarg, MAX_BUFF);
			break;
		case 'd':
			Action = DELETE_IT;
			scopy(Ip, optarg, MAX_BUFF);
			break;
		case 'u':
			Action = UPDATE_IT;
			scopy(Ip, optarg, MAX_BUFF);
			break;
		default:
			errflag = 1;
			break;
		}
	}
	if (errflag == 1)
	{
		usage();
		return(1);
	}
	if (optind < argc)
		scopy(Domain, argv[optind++], MAX_BUFF);
	if (Action == ADD_IT || Action == DELETE_IT || Action == UPDATE_IT)
	{
		if(!*Ip || !*Domain)
		{
			usage();
			return(1);
		}
	}
	return(0);
}
#else
int
main()
{
	printf("IP aliases are not compiled into IndiMail\n");
	printf("You will need to do the following steps\n");
	printf("make distclean\n");
	printf("./configure --enable-ip-alias-domains=y [your other options]\n");
	printf("make\n");
	printf("make install-strip\n");
	return(1);
}
#endif

void
getversion_vipmap_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
