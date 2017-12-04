/*
 * $Log: vupd_ip_map.c,v $
 * Revision 2.3  2008-05-28 15:37:38+05:30  Cprogrammer
 * removed cdb code
 *
 * Revision 2.2  2004-05-22 22:34:57+05:30  Cprogrammer
 * renamed ip_addr to ipaddr
 *
 * Revision 2.1  2003-03-05 00:21:41+05:30  Cprogrammer
 * update ip map
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: vupd_ip_map.c,v 2.3 2008-05-28 15:37:38+05:30 Cprogrammer Stab mbhangui $";
#endif

#ifdef IP_ALIAS_DOMAINS
int
vupd_ip_map(char *ip, char *domain)
{
	char            SqlBuf[SQL_BUF_SIZE];

	if (!ip || !*ip)
		return (-1);
	if (!domain || !*domain)
		return (-1);
	if (vauth_open((char *) 0) != 0)
		return (-1);
	snprintf(SqlBuf, SQL_BUF_SIZE, "update low_priority ip_alias_map set domain=\"%s\" where ipaddr = \"%s\"",
		domain, ip);
	if (mysql_query(&mysql[1], SqlBuf))
	{
		mysql_perror("vupd_ip_map: %s", SqlBuf);
		return (-1);
	}
	return (0);
}
#endif

void
getversion_vupd_ip_map_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
