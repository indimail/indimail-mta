/*
 * $Log: vadd_ip_map.c,v $
 * Revision 2.5  2008-09-08 09:54:31+05:30  Cprogrammer
 * formatting of long lines
 *
 * Revision 2.4  2008-05-28 15:39:29+05:30  Cprogrammer
 * removed leftover cdb code
 *
 * Revision 2.3  2008-05-28 15:19:55+05:30  Cprogrammer
 * mysql module default
 *
 * Revision 2.2  2004-05-22 22:31:00+05:30  Cprogrammer
 * Renamed ip_addr to ipaddr
 *
 * Revision 2.1  2002-10-27 21:30:29+05:30  Cprogrammer
 * used generic function create_table()
 *
 * Revision 1.7  2002-08-03 04:32:52+05:30  Cprogrammer
 * mysql string fix
 *
 * Revision 1.6  2002-02-23 20:24:10+05:30  Cprogrammer
 * corrected bug with ER_NO_SUCH_TABLE check
 *
 * Revision 1.5  2001-12-21 02:21:56+05:30  Cprogrammer
 * create table when errno is ER_NO_SUCH_TABLE
 *
 * Revision 1.4  2001-11-24 12:20:22+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.3  2001-11-20 10:56:18+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.2  2001-11-14 19:25:30+05:30  Cprogrammer
 * vauth_open for distributed arch change
 *
 * Revision 1.1  2001-10-24 18:15:18+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: vadd_ip_map.c,v 2.5 2008-09-08 09:54:31+05:30 Cprogrammer Stab mbhangui $";
#endif

#ifdef IP_ALIAS_DOMAINS
/*
 * Add an ip to domain mapping
 * It will remove any duplicate entry before adding it
 *
 */
#include <mysqld_error.h>
int
vadd_ip_map(char *ip, char *domain)
{
	char            SqlBuf[SQL_BUF_SIZE];

	if (!ip || !*ip)
		return (-1);
	if (!domain || !*domain)
		return (-1);
	if (vauth_open((char *) 0) != 0)
		return (-1);
	snprintf(SqlBuf, SQL_BUF_SIZE,
		"replace low_priority into ip_alias_map ( ipaddr, domain ) values ( \"%s\", \"%s\" )", 
		ip, domain);
	if (mysql_query(&mysql[1], SqlBuf))
	{
		if(mysql_errno(&mysql[1]) == ER_NO_SUCH_TABLE)
		{
			if(create_table(ON_LOCAL, "ip_alias_map", IP_ALIAS_TABLE_LAYOUT))
				return(-1);
			if (!mysql_query(&mysql[1], SqlBuf))
				return(0);
		}
		mysql_perror("vadd_ip_map: mysql_query: %s", SqlBuf);
		return (-1);
	}
	return (0);
}
#endif

void
getversion_vadd_ip_map_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
