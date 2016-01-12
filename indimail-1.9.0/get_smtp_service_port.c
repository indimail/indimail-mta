/*
 * $Log: get_smtp_service_port.c,v $
 * Revision 2.6  2016-01-12 14:26:56+05:30  Cprogrammer
 * use AF_INET for get_local_ip()
 *
 * Revision 2.5  2010-05-28 14:11:12+05:30  Cprogrammer
 * use QMTP as default
 *
 * Revision 2.4  2010-04-24 14:58:11+05:30  Cprogrammer
 * return qmtp or smtp port depending on the value of ROUTES env variable
 *
 * Revision 2.3  2008-05-28 16:35:49+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 2.2  2005-12-29 22:44:58+05:30  Cprogrammer
 * use getEnvConfigStr to set variables from environment variables
 *
 * Revision 2.1  2002-10-27 21:15:44+05:30  Cprogrammer
 * used generic function create_table()
 *
 * Revision 1.8  2002-08-03 04:28:11+05:30  Cprogrammer
 * mysql string fix
 *
 * Revision 1.7  2002-03-29 18:04:31+05:30  Cprogrammer
 * compute value of port before calling mysql_free_result()
 *
 * Revision 1.6  2001-12-21 02:20:56+05:30  Cprogrammer
 * create table when errno is ER_NO_SUCH_TABLE
 *
 * Revision 1.5  2001-12-12 13:43:31+05:30  Cprogrammer
 * get_smtp_service_port to look up smtp_port for specific source hosts
 *
 * Revision 1.4  2001-12-11 11:31:21+05:30  Cprogrammer
 * change in open_central_db()
 *
 * Revision 1.3  2001-12-09 23:55:16+05:30  Cprogrammer
 * get_smtp_service_port returns -1 on error
 * removed vcreate_smtp_table
 *
 * Revision 1.2  2001-12-09 01:53:12+05:30  Cprogrammer
 * added code to get the port from MYSQL smtp_port table
 *
 * Revision 1.1  2001-12-08 12:37:19+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: get_smtp_service_port.c,v 2.6 2016-01-12 14:26:56+05:30 Cprogrammer Exp mbhangui $";
#endif

#ifdef CLUSTERED_SITE
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <mysqld_error.h>

int
get_smtp_service_port(char *SrcHost, char *domain, char *hostid)
{
	char            Domain[MAX_BUFF], SqlBuf[SQL_BUF_SIZE];
	char           *ptr, *src_host;
	register int    i;
	int             default_port;
	MYSQL_RES      *res;
	MYSQL_ROW       row;

	if ((ptr = getenv("ROUTES")) && (*ptr && !memcmp(ptr, "smtp", 4)))
		default_port = PORT_SMTP;
	else
		default_port = PORT_QMTP;
	if (open_central_db(0))
		return (default_port);
	if(!domain || !*domain)
	{
		getEnvConfigStr(&ptr, "DEFAULT_DOMAIN", DEFAULT_DOMAIN);
		scopy(Domain, ptr, MAX_BUFF);
	} else
		scopy(Domain, domain, MAX_BUFF);
	if(!SrcHost || !*SrcHost)
	{
		if(!(src_host = get_local_ip(PF_INET)))
			return(default_port);
	} else
		src_host = SrcHost;
	snprintf(SqlBuf, SQL_BUF_SIZE, 
		"select high_priority port,src_host from smtp_port where host=\"%s\" and domain=\"%s\"", 
		hostid, Domain);
	if (mysql_query(&mysql[0], SqlBuf))
	{
		if(mysql_errno(&mysql[0]) == ER_NO_SUCH_TABLE)
			create_table(ON_MASTER, "smtp_port", SMTP_TABLE_LAYOUT);
		else
			fprintf(stderr, "get_smtp_service_port: %s: %s\n", SqlBuf, mysql_error(&mysql[0]));
		return(default_port);
	}
	if (!(res = mysql_store_result(&mysql[0])))
	{
		fprintf(stderr, "get_smtp_service_port: mysql_store_result: %s\n", mysql_error(&mysql[0]));
		return(default_port);
	}
	for(i = -1;;)
	{
		if (!(row = mysql_fetch_row(res)))
			break;
		if(!strncmp(src_host, row[1], MAX_BUFF))
		{
			i = atoi(row[0]);
			mysql_free_result(res);
			return(i ? i : default_port);
		}
		if(*row[1] == '*')
			i = atoi(row[0]);
	}
	mysql_free_result(res);
	if(i == -1)
		return(default_port);
	return(i ? i : default_port);
}
#endif

void
getversion_get_smtp_service_port_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
	return;
}
