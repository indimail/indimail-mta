/*
 * $Log: vdel_ip_map.c,v $
 * Revision 2.2  2008-05-28 15:30:54+05:30  Cprogrammer
 * removed cdb code
 *
 * Revision 2.1  2004-05-22 22:31:04+05:30  Cprogrammer
 * renamed ip_addr to ipaddr
 *
 * Revision 1.5  2002-08-03 04:38:04+05:30  Cprogrammer
 * mysql string fix
 *
 * Revision 1.4  2001-11-24 12:21:43+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.3  2001-11-20 11:00:12+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.2  2001-11-14 19:28:38+05:30  Cprogrammer
 * distributed arch change
 *
 * Revision 1.1  2001-10-24 18:15:34+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: vdel_ip_map.c,v 2.2 2008-05-28 15:30:54+05:30 Cprogrammer Stab mbhangui $";
#endif

#ifdef IP_ALIAS_DOMAINS
int
vdel_ip_map(char *ip, char *domain)
{
	char            SqlBuf[SQL_BUF_SIZE];

	if (!ip || !*ip)
		return (-1);
	if (!domain || !*domain)
		return (-1);
	if (vauth_open((char *) 0) != 0)
		return (-1);
	snprintf(SqlBuf, SQL_BUF_SIZE, "delete low_priority from ip_alias_map where ipaddr=\"%s\" and domain = \"%s\"", 
		ip, domain);
	if (mysql_query(&mysql[1], SqlBuf))
	{
		mysql_perror("vdel_ip_map: %s", SqlBuf);
		return (-1);
	}
	return (0);
}
#endif

void
getversion_vdel_ip_map_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
