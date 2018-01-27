/*
 * $Log: vadd_atrn_map.c,v $
 * Revision 2.3  2008-09-08 09:54:23+05:30  Cprogrammer
 * formatting of long lines
 *
 * Revision 2.2  2008-05-28 16:38:13+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 2.1  2003-07-04 11:29:52+05:30  Cprogrammer
 * add domain maps for ODMR
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: vadd_atrn_map.c,v 2.3 2008-09-08 09:54:23+05:30 Cprogrammer Stab mbhangui $";
#endif

/*
 * Add an domain to user's atrn mapping
 */
#include <mysqld_error.h>
int
vadd_atrn_map(char *user, char *domain, char *domain_list)
{
	char            SqlBuf[SQL_BUF_SIZE];

	if (!user || !*user)
		return (-1);
	if (!domain || !*domain)
		return (-1);
	if (vauth_open((char *) 0) != 0)
		return (-1);
	snprintf(SqlBuf, SQL_BUF_SIZE,
		"insert low_priority into atrn_map ( pw_name, pw_domain, domain_list) values ( \"%s\", \"%s\", \"%s\" )", 
		user, domain, domain_list);
	if (mysql_query(&mysql[1], SqlBuf))
	{
		if(mysql_errno(&mysql[1]) == ER_NO_SUCH_TABLE)
		{
			if(create_table(ON_LOCAL, "atrn_map", ATRN_MAP_LAYOUT))
				return(-1);
			if (!mysql_query(&mysql[1], SqlBuf))
				return(0);
		}
		mysql_perror("vadd_atrn_map: mysql_query: %s", SqlBuf);
		return (-1);
	}
	return (0);
}

void
getversion_vadd_atrn_map_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
