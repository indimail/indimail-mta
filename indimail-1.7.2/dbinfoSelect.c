/*
 * $Log: dbinfoSelect.c,v $
 * Revision 2.6  2010-03-07 09:28:11+05:30  Cprogrammer
 * check return value of is_distributed_domain for error
 *
 * Revision 2.5  2003-10-28 00:23:00+05:30  Cprogrammer
 * display auto generated local dbinfo entries
 *
 * Revision 2.4  2003-10-24 22:40:27+05:30  Cprogrammer
 * print domain name in row format
 *
 * Revision 2.3  2003-03-04 20:58:00+05:30  Cprogrammer
 * added row format display
 *
 * Revision 2.2  2003-01-22 16:01:44+05:30  Cprogrammer
 * added option to select domain
 *
 * Revision 2.1  2003-01-01 02:32:36+05:30  Cprogrammer
 * function to display dbinfo entries
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: dbinfoSelect.c,v 2.6 2010-03-07 09:28:11+05:30 Cprogrammer Stab mbhangui $";
#endif

#ifdef CLUSTERED_SITE
#include <stdlib.h>
#include <string.h>

int
dbinfoSelect(char *filename, char *domain, char *mdahost, int row_format)
{
	char            mcdFile[MAX_BUFF];
	DBINFO        **rhostsptr;
	MYSQL         **mysqlptr;
	int             count, is_dist;
	int             first_flag = 0;
	char           *ptr;

	if (filename && *filename)
	{
		snprintf(mcdFile, MAX_BUFF, "MCDFILE=%s", filename);
		putenv(mcdFile);
	}
	if (OpenDatabases())
		return(1);
	for (count = 1, mysqlptr = MdaMysql, rhostsptr = RelayHosts;(*rhostsptr);mysqlptr++, rhostsptr++, count++)
	{
		if (mdahost && *mdahost && strncmp(mdahost, (*rhostsptr)->mdahost, DBINFO_BUFF))
			continue;
		if (domain && *domain && strncmp(domain, (*rhostsptr)->domain, DBINFO_BUFF))
			continue;
		if (row_format)
		{
			first_flag++;
			printf("%s %s %d %s %s %d %s %s %s\n", 
				filename, (*rhostsptr)->domain, is_distributed_domain((*rhostsptr)->domain), 
				(*rhostsptr)->server, (*rhostsptr)->mdahost, (*rhostsptr)->port, (*rhostsptr)->database,
				(*rhostsptr)->user, (*rhostsptr)->password);
			continue;
		}
		if (!first_flag++)
			printf("MySQL Client Version: %s\n", mysql_get_client_info());
		printf("connection to  %s\n", mysql_get_host_info(*mysqlptr));
		printf("protocol       %d\n", mysql_get_proto_info(*mysqlptr));
		printf("server version %s\n", mysql_get_server_info(*mysqlptr));
		if ((is_dist = is_distributed_domain((*rhostsptr)->domain)) == -1)
			printf("domain         %-15s - can't figure out dist flag\n", (*rhostsptr)->domain);
		else
			printf("domain         %-15s - %s\n", (*rhostsptr)->domain, 
				is_distributed_domain((*rhostsptr)->domain) == 1 ? "Distributed" : "Non Distributed");
		printf("sqlserver[%03d] %s\n", count, (*rhostsptr)->server);
		if (*((*rhostsptr)->mdahost))
			printf("mda host       %s\n", (*rhostsptr)->mdahost);
		if ((*mysqlptr)->unix_socket)
			printf("Unix   Socket  %s\n", (*mysqlptr)->unix_socket);
		else
			printf("TCP/IP Port    %d\n", (*rhostsptr)->port);
		printf("database       %s\n", (*rhostsptr)->database);
		printf("user           %s\n", (*rhostsptr)->user);
		printf("password       %s\n", (*rhostsptr)->password);
		printf("fd             %d\n", (*rhostsptr)->fd);
		printf("DBINFO Method  %s\n", (*rhostsptr)->isLocal ? "Auto" : "DBINFO");
		if ((*rhostsptr)->fd == -1)
			printf("MySQL Stat     mysql_real_connect: %s\n", mysql_error((*mysqlptr)));
		else
		if ((ptr = (char *) mysql_stat((*mysqlptr))))
			printf("MySQL Stat     %s\n", ptr);
		else
			printf("MySQL Stat     %s\n", mysql_error((*mysqlptr)));
		printf("--------------------------\n");
	}
	close_db();
	return(!first_flag);
}
#endif

void
getversion_dbinfoSelect_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
