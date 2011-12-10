/*
 * $Log: vshow_ip_map.c,v $
 * Revision 2.4  2008-05-28 15:37:15+05:30  Cprogrammer
 * removed cdb code
 *
 * Revision 2.3  2004-05-22 22:34:08+05:30  Cprogrammer
 * renamed ip_addr to ipaddr
 *
 * Revision 2.2  2003-03-24 19:31:44+05:30  Cprogrammer
 * added domain filter option
 *
 * Revision 2.1  2002-12-16 20:26:24+05:30  Cprogrammer
 * added missing table creation on error ER_NO_SUCH_TABLE
 *
 * Revision 1.4  2001-11-24 12:22:32+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.3  2001-11-20 11:02:18+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.2  2001-11-14 19:29:49+05:30  Cprogrammer
 * distributed arch change
 *
 * Revision 1.1  2001-10-24 18:15:46+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: vshow_ip_map.c,v 2.4 2008-05-28 15:37:15+05:30 Cprogrammer Stab mbhangui $";
#endif

#ifdef IP_ALIAS_DOMAINS
#include <mysqld_error.h>

int
vshow_ip_map(int first, char *ip, char *domain, char *domain_filter)
{
	static int      more = 0;
	static MYSQL_RES *res;
	MYSQL_ROW       row;
	char            SqlBuf[SQL_BUF_SIZE];

	if (!ip)
		return (-1);
	if (!domain)
		return (-1);
	if (vauth_open((char *) 0) != 0)
		return (-1);
	if (first == 1)
	{
		if (vauth_open((char *) 0) != 0)
			return (-1);
		if(domain_filter && *domain_filter)
			snprintf(SqlBuf, SQL_BUF_SIZE, "select high_priority ipaddr, domain from ip_alias_map where domain=\"%s\"", 
				domain_filter);
		else
			snprintf(SqlBuf, SQL_BUF_SIZE, "select high_priority ipaddr, domain from ip_alias_map");
		if (res)
			mysql_free_result(res);
		res = (MYSQL_RES *) 0;
		if (mysql_query(&mysql[1], SqlBuf))
		{
			if(mysql_errno(&mysql[1]) == ER_NO_SUCH_TABLE)
				create_table(ON_LOCAL, "ip_alias_map", IP_ALIAS_TABLE_LAYOUT);
			else
			{
				mysql_perror("vshow_ip_map: %s", SqlBuf);
				return(-1);
			}
			return (0);
		}
		if (!(res = mysql_store_result(&mysql[1])))
		{
			mysql_perror("vshow_ip_map: mysql_store_result");
			return (0);
		}
	} else
	if (more == 0)
		return (0);
	if ((row = mysql_fetch_row(res)))
	{
		scopy(ip, row[0], 18);
		scopy(domain, row[1], MAX_BUFF);
		more = 1;
		return (1);
	}
	more = 0;
	mysql_free_result(res);
	res = (MYSQL_RES *) 0;
	return (0);
}
#endif

void
getversion_vshow_ip_map_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
