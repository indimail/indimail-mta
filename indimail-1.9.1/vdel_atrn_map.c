/*
 * $Log: vdel_atrn_map.c,v $
 * Revision 2.2  2008-05-28 17:40:30+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 2.1  2003-07-04 11:30:23+05:30  Cprogrammer
 * delete user domain mapping for ODMR
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: vdel_atrn_map.c,v 2.2 2008-05-28 17:40:30+05:30 Cprogrammer Stab mbhangui $";
#endif

int
vdel_atrn_map(char *user, char *domain, char *domain_list)
{
	char            SqlBuf[SQL_BUF_SIZE];

	if (!user || !*user)
		return (-1);
	if (!domain || !*domain)
		return (-1);
	if (vauth_open((char *) 0) != 0)
		return (-1);
	snprintf(SqlBuf, SQL_BUF_SIZE, 
		"delete low_priority from atrn_map where pw_name = \"%s\" and pw_domain = \"%s\" and domain_list = \"%s\"", 
		user, domain, domain_list);
	if (mysql_query(&mysql[1], SqlBuf))
	{
		mysql_perror("vdel_atrn_map: %s", SqlBuf);
		return (-1);
	}
	return (0);
}

void
getversion_vdel_atrn_map_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
