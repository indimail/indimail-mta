/*
 * $Log: vpriv_insert.c,v $
 * Revision 2.4  2008-09-08 09:58:29+05:30  Cprogrammer
 * removed mysql_escape
 *
 * Revision 2.3  2008-06-13 10:59:14+05:30  Cprogrammer
 * compile vpriv_insert only if CLUSTERED_SITE defined
 *
 * Revision 2.2  2008-05-28 17:41:58+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 2.1  2003-09-14 01:59:01+05:30  Cprogrammer
 * function to add privileges
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: vpriv_insert.c,v 2.4 2008-09-08 09:58:29+05:30 Cprogrammer Stab mbhangui $";
#endif

#ifdef CLUSTERED_SITE
#include <stdlib.h>
#include <string.h>
#include <mysqld_error.h>

int
vpriv_insert(char *user, char *program, char *cmdargs)
{
	int             err;
	char            SqlBuf[SQL_BUF_SIZE];

	if (!user || !*user || !program || !*program)
		return (1);
	if (open_master())
	{
		fprintf(stderr, "vpriv_insert: Failed to open Master Db\n");
		return (-1);
	}
	snprintf(SqlBuf, SQL_BUF_SIZE, 
		"insert low_priority into vpriv ( user, program, cmdswitches ) values ( \"%s\", \"%s\", \"%s\")", 
		user, program, cmdargs && *cmdargs ? cmdargs : "*");
	if (mysql_query(&mysql[0], SqlBuf))
	{
		if (mysql_errno(&mysql[0]) == ER_NO_SUCH_TABLE)
		{
			if (create_table(ON_MASTER, "vpriv", PRIV_CMD_LAYOUT))
				return (-1);
			if (mysql_query(&mysql[0], SqlBuf))
			{
				fprintf(stderr, "vpriv_insert: %s: %s\n", SqlBuf, mysql_error(&mysql[0]));
				return (-1);
			}
		} else
		{
			fprintf(stderr, "vpriv_insert: %s: %s\n", SqlBuf, mysql_error(&mysql[0]));
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
	if (err)
		printf("Added Privilege for %s [%s %s]\n", user, program, cmdargs);
	else
		printf("No Privilege added for %s\n", user);
	return (0);
}
#endif

void
getversion_vpriv_insert_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
