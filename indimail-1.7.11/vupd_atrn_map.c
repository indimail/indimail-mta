/*
 * $Log: vupd_atrn_map.c,v $
 * Revision 2.3  2008-09-08 09:59:32+05:30  Cprogrammer
 * formatting of long lines
 *
 * Revision 2.2  2008-05-28 17:43:02+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 2.1  2003-07-04 11:30:58+05:30  Cprogrammer
 * update user's domain list for ODMR
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: vupd_atrn_map.c,v 2.3 2008-09-08 09:59:32+05:30 Cprogrammer Stab mbhangui $";
#endif

int
vupd_atrn_map(char *user, char *domain, char *old_domain, char *domain_list)
{
	char            SqlBuf[SQL_BUF_SIZE];

	if (!user || !*user)
		return (-1);
	if (!domain || !*domain)
		return (-1);
	if (vauth_open((char *) 0) != 0)
		return (-1);
	snprintf(SqlBuf, SQL_BUF_SIZE,
		"update low_priority atrn_map set domain_list=\"%s\" where pw_name=\"%s\" and \
		pw_domain=\"%s\" and domain_list=\"%s\"",
		domain_list, user, domain, old_domain);
	if (mysql_query(&mysql[1], SqlBuf))
	{
		mysql_perror("vupd_atrn_map: %s", SqlBuf);
		return (-1);
	}
	return (0);
}

void
getversion_vupd_atrn_map_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
