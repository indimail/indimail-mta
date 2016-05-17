/*
 * $Log: updusercntrl.c,v $
 * Revision 2.6  2016-05-17 15:38:46+05:30  Cprogrammer
 * fixed comment
 *
 * Revision 2.5  2009-02-06 11:40:20+05:30  Cprogrammer
 * fixed potential buffer overflow
 *
 * Revision 2.4  2008-09-08 09:54:14+05:30  Cprogrammer
 * removed mysql_escape
 *
 * Revision 2.3  2008-08-02 09:08:55+05:30  Cprogrammer
 * use new function error_stack
 *
 * Revision 2.2  2008-05-28 16:38:07+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 2.1  2004-05-17 00:58:27+05:30  Cprogrammer
 * function for updating hostcntrl
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: updusercntrl.c,v 2.6 2016-05-17 15:38:46+05:30 Cprogrammer Exp mbhangui $";
#endif

#ifdef CLUSTERED_SITE
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <mysqld_error.h>

/*
 * 1 - Mysql Error (or) Assignment Error
 * 0 - Success
 */

int
updusercntrl(char *user, char *domain, char *hostid, int force)
{
	char            SqlBuf[SQL_BUF_SIZE];
	int             err;

	if (!user || !*user || !domain || !*domain)
		return (1);
	/*
	 *  Check if Domain is distributed or not , by checking table hostcntrl
	 */
	if (force == 0)
	{
		if ((err = is_distributed_domain(domain)) == -1)
			return (1);
		else
		if (!err)
			return (0);
	}
	if (open_master())
	{
		error_stack(stderr, "updusercntrl: Failed to open Master Db\n");
		return (1);
	}
	snprintf(SqlBuf, SQL_BUF_SIZE, 
		"update low_priority %s set host = \"%s\" where pw_name = \"%s\" and pw_domain = \"%s\"", 
		cntrl_table, hostid, user, domain);
	if (mysql_query(&mysql[0], SqlBuf))
	{
		if (mysql_errno(&mysql[0]) == ER_NO_SUCH_TABLE)
		{
			create_table(ON_MASTER, cntrl_table, CNTRL_TABLE_LAYOUT);
			return (0);
		}
		fprintf(stderr, "updusercntrl: %s: %s\n", SqlBuf, mysql_error(&mysql[0]));
		return (1);
	}
	return (0);
}
#endif
void
getversion_updusercntrl_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
