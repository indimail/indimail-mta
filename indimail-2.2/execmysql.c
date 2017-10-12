/*
 * $Log: execmysql.c,v $
 * Revision 2.8  2011-11-09 19:44:09+05:30  Cprogrammer
 * removed getversion
 *
 * Revision 2.7  2009-09-23 14:59:40+05:30  Cprogrammer
 * change for new runcmmd()
 *
 * Revision 2.6  2008-05-28 16:35:19+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 2.5  2005-12-21 09:46:01+05:30  Cprogrammer
 * make gcc 4 happy
 *
 * Revision 2.4  2004-07-12 22:45:39+05:30  Cprogrammer
 * replaced system() with runcmmd()
 *
 * Revision 2.3  2003-12-08 20:23:01+05:30  Cprogrammer
 * use MYSQLBINDIR path to invoke mysql client
 *
 * Revision 2.2  2003-12-07 23:09:33+05:30  Cprogrammer
 * removed full path to mysql client to allow non-standard mysql installations
 *
 * Revision 2.1  2002-12-16 02:25:49+05:30  Cprogrammer
 * program to connect to mdahost mysql database
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: execmysql.c,v 2.8 2011-11-09 19:44:09+05:30 Cprogrammer Stab mbhangui $";
#endif

#include <string.h>
#include <unistd.h>
#include <stdlib.h>

static int      get_options(int, char **, char **, char **);

int
main(int argc, char **argv)
{
	DBINFO        **rhostsptr;
	char           *mdahost, *domain = 0;
	int             total;
	char            cmdbuf[SQL_BUF_SIZE];

	if (get_options(argc, argv, &mdahost, &domain))
		return (1);
	if(!RelayHosts && !(RelayHosts = LoadDbInfo_TXT(&total)))
	{
		perror("LoadDbInfo_TXT");
		return(1);
	}
	for (rhostsptr = RelayHosts;*rhostsptr;rhostsptr++)
	{
		if(!strncmp((*rhostsptr)->domain, domain, DBINFO_BUFF) && !strncmp((*rhostsptr)->mdahost, mdahost, DBINFO_BUFF))
		{
			snprintf(cmdbuf, sizeof(cmdbuf), "%s/mysql -u %s -p%s -P %d -h %s %s",
				MYSQLBINDIR, (*rhostsptr)->user, (*rhostsptr)->password,
				(*rhostsptr)->port, (*rhostsptr)->mdahost, (*rhostsptr)->database);
			runcmmd(cmdbuf, 0);
			return(0);
		}
	}
	fprintf(stderr, "execmysql: could not locate MySql server for domain %s server %s\n", domain, mdahost);
	return(1);
}

static int
get_options(int argc, char **argv, char **mdahost, char **domain)
{
	int             c;

	while ((c = getopt(argc, argv, "d:")) != -1)
	{
		switch (c)
		{
		case 'v':
			verbose = 1;
			break;
		case 'd':
			*domain = optarg;
			break;
		default:
			fprintf(stderr, "USAGE: execmysql -d domain host\n");
			return(1);
		}
	}
	if (optind < argc)
		*mdahost = argv[optind++];
	else
	{
		fprintf(stderr, "USAGE: execmysql -d domain host\n");
		return (1);
	}
	return (0);
}

void
getversion_execmysql_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
