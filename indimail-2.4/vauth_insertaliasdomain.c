/*
 * $Log: vauth_insertaliasdomain.c,v $
 * Revision 2.3  2008-08-02 09:09:48+05:30  Cprogrammer
 * use new function error_stack
 *
 * Revision 2.2  2008-05-28 16:39:36+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 2.1  2002-10-27 21:34:39+05:30  Cprogrammer
 * used generic function create_table()
 *
 * Revision 1.3  2002-08-03 04:35:34+05:30  Cprogrammer
 * mysql string fix
 *
 * Revision 1.2  2001-12-22 18:14:28+05:30  Cprogrammer
 * changed error string for open_master failure
 *
 * Revision 1.1  2001-12-21 01:23:54+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: vauth_insertaliasdomain.c,v 2.3 2008-08-02 09:09:48+05:30 Cprogrammer Stab mbhangui $";
#endif

#ifdef CLUSTERED_SITE
#include <mysqld_error.h>
int
vauth_insertaliasdomain(char *old_domain, char *new_domain)
{
#ifdef CLUSTERED_SITE
	int             err;
	char            SqlBuf[SQL_BUF_SIZE];
#endif

	if (open_master())
	{
		error_stack(stderr, "vauth_insertaliasdomain: Failed to open Master Db\n");
		return (-1);
	}
	snprintf(SqlBuf, SQL_BUF_SIZE, 
		"insert low_priority into aliasdomain ( alias, domain ) values ( \"%s\", \"%s\")", 
		new_domain, old_domain);
	if (mysql_query(&mysql[0], SqlBuf) && (err = mysql_errno(&mysql[0])) != ER_DUP_ENTRY)
	{
		if (err == ER_NO_SUCH_TABLE)
		{
			if (create_table(ON_MASTER, "aliasdomain", ALIASDOMAIN_TABLE_LAYOUT))
				return (-1);
			if (!mysql_query(&mysql[0], SqlBuf))
				return (0);
		} 
		fprintf(stderr, "vauth_insertaliasdomain: %s: %s\n", SqlBuf, mysql_error(&mysql[0]));
		return (-1);
	}
	return (0);
}
#endif

void
getversion_vauth_insertaliasdomain_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
	return;
}
