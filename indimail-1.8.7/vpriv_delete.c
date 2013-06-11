/*
 * $Log: vpriv_delete.c,v $
 * Revision 2.4  2008-09-08 09:58:23+05:30  Cprogrammer
 * removed mysql_escape
 *
 * Revision 2.3  2008-06-13 10:58:58+05:30  Cprogrammer
 * compile vpriv_delete() only if CLUSTERED_SITE defined
 *
 * Revision 2.2  2008-05-28 17:41:55+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 2.1  2003-09-14 01:58:44+05:30  Cprogrammer
 * function to delete privileges
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: vpriv_delete.c,v 2.4 2008-09-08 09:58:23+05:30 Cprogrammer Stab mbhangui $";
#endif

#ifdef CLUSTERED_SITE
#include <stdlib.h>
#include <string.h>
#include <mysqld_error.h>

int
vpriv_delete(char *user, char *program)
{
	int             err;
	char            SqlBuf[SQL_BUF_SIZE];

	if (!user || !*user)
		return (1);
	if (open_master())
	{
		fprintf(stderr, "vpriv_delete: Failed to open Master Db\n");
		return (-1);
	}
	if (program && *program)
	{
		snprintf(SqlBuf, SQL_BUF_SIZE, 
			"delete low_priority from vpriv where user = \"%s\" and program = \"%s\"", 
			user, program);
	} else
		snprintf(SqlBuf, SQL_BUF_SIZE, 
			"delete low_priority from vpriv where user = \"%s\"", user);
	if (mysql_query(&mysql[0], SqlBuf))
	{
		if (mysql_errno(&mysql[0]) == ER_NO_SUCH_TABLE)
		{
			if (create_table(ON_MASTER, "vpriv", PRIV_CMD_LAYOUT))
				return (-1);
			if (verbose)
				printf("No program line %s for user %s\n", program, user);
			return (0);
		} else
		{
			fprintf(stderr, "vpriv_delete: %s: %s\n", SqlBuf, mysql_error(&mysql[0]));
			return (-1);
		}
	}
	if ((err = mysql_affected_rows(&mysql[0])) == -1)
	{
		fprintf(stderr, "mysql_affected_rows: %s\n", mysql_error(&mysql[0]));
		return (-1);
	}
	if (!verbose)
		return (0);
	if (err && verbose)
		printf("Deleted program %s for user %s (%d entries)\n", program, user, err);
	else
	if (verbose)
		printf("No Program %s for user %s\n",  program, user);
	return (0);
}
#endif

void
getversion_vpriv_delete_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
