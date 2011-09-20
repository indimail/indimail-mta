/*
 * $Log: vget_ip_map.c,v $
 * Revision 2.2  2008-05-28 15:36:50+05:30  Cprogrammer
 * removed cdb code
 *
 * Revision 2.1  2004-05-22 22:33:20+05:30  Cprogrammer
 * renamed ip_addr to ipaddr
 *
 * Revision 1.5  2002-08-03 04:38:10+05:30  Cprogrammer
 * mysql string fix
 *
 * Revision 1.4  2001-11-24 12:21:57+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.3  2001-11-20 11:00:46+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.2  2001-11-14 19:28:48+05:30  Cprogrammer
 * distributed arch change
 *
 * Revision 1.1  2001-10-24 18:15:39+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: vget_ip_map.c,v 2.2 2008-05-28 15:36:50+05:30 Cprogrammer Stab mbhangui $";
#endif

#ifdef IP_ALIAS_DOMAINS
int
vget_ip_map(char *ip, char *domain, int domain_size)
{
	char            SqlBuf[SQL_BUF_SIZE];
	MYSQL_RES      *res;
	MYSQL_ROW       row;
	int             ret = -1;

	if (!ip || !*ip)
		return (-1);
	if (!domain)
		return (-2);
	if (vauth_open((char *) 0) != 0)
		return (-3);
	snprintf(SqlBuf, SQL_BUF_SIZE, "select high_priority domain from ip_alias_map where ipaddr = \"%s\"", ip);
	if (mysql_query(&mysql[1], SqlBuf))
	{
		mysql_perror("vget_ip_map: %s", SqlBuf);
		return (-1);
	}
	if (!(res = mysql_store_result(&mysql[1])))
	{
		mysql_perror("vget_ip_map: mysql_store_result");
		return (-4);
	}
	while ((row = mysql_fetch_row(res)))
	{
		ret = 0;
		scopy(domain, row[0], domain_size);
	}
	mysql_free_result(res);
	return (ret);
}
#endif

void
getversion_vget_ip_map_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
