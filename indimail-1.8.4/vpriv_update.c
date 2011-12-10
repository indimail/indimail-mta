/*
 * $Log: vpriv_update.c,v $
 * Revision 2.4  2008-09-08 09:58:40+05:30  Cprogrammer
 * removed mysql_escape
 *
 * Revision 2.3  2008-06-13 10:59:46+05:30  Cprogrammer
 * compile vpriv_update() only if CLUSTERED_SITE defined
 *
 * Revision 2.2  2008-05-28 17:42:03+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 2.1  2003-09-14 01:59:37+05:30  Cprogrammer
 * function to update privileges
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: vpriv_update.c,v 2.4 2008-09-08 09:58:40+05:30 Cprogrammer Stab mbhangui $";
#endif

#ifdef CLUSTERED_SITE
#include <stdlib.h>
#include <string.h>
#include <mysqld_error.h>

int
vpriv_update(char *user, char *program, char *cmdargs)
{
	int             err;
	char            SqlBuf[SQL_BUF_SIZE];

	if (!user || !*user || !program || !*program || !cmdargs || !*cmdargs)
		return (1);
	if (open_master())
	{
		fprintf(stderr, "vpriv_update: Failed to open Master Db\n");
		return (-1);
	}
	snprintf(SqlBuf, SQL_BUF_SIZE, 
		"update low_priority vpriv set cmdswitches=\"%s\" where user=\"%s\" and program=\"%s\"",
		cmdargs, user, program);
	if (mysql_query(&mysql[0], SqlBuf))
	{
		if (mysql_errno(&mysql[0]) == ER_NO_SUCH_TABLE)
		{
			fprintf(stderr, "No program %s for user %s\n", program, user);
			if (create_table(ON_MASTER, "vpriv", PRIV_CMD_LAYOUT))
				return (-1);
			return (1);
		}
		fprintf(stderr, "vpriv_update: %s: %s\n", SqlBuf, mysql_error(&mysql[0]));
		return (-1);
	}
	if ((err = mysql_affected_rows(&mysql[0])) == -1)
	{
		fprintf(stderr, "mysql_affected_rows: %s\n", mysql_error(&mysql[0]));
		return (-1);
	}
	if (!verbose)
		return (0);
	if (err)
		printf("Updated Cmdargs %s for user %s program %s (%d entries)\n", cmdargs, user, program, err);
	else
		fprintf(stderr, "No Program %s for user %s\n", program, user);
	return (0);
}
#endif

void
getversion_vpriv_update_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
