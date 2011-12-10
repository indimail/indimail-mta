/*
 * $Log: lockTable.c,v $
 * Revision 2.2  2008-05-28 16:36:49+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 2.1  2003-02-09 00:45:05+05:30  Cprogrammer
 * function to lock/unlock tables
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: lockTable.c,v 2.2 2008-05-28 16:36:49+05:30 Cprogrammer Stab mbhangui $";
#endif

/*
 * flag  0 - Unlock
 *       1 - Lock
 */
int
lockTable(int which, char *table_name, int flag)
{
	char            SqlBuf[SQL_BUF_SIZE];

#ifdef CLUSTERED_SITE
	if ((which == ON_MASTER ? open_master() : vauth_open((char *) 0)))
	{
		fprintf(stderr, "lockTable: Failed to open %s db\n", which == ON_MASTER ? "master" : "local");
		return (-1);
	}
#else
	which = ON_LOCAL;
	if (vauth_open((char *) 0))
	{
		fprintf(stderr, "lockTable: Failed to open local db\n");
		return (-1);
	}
#endif
	if (flag)
		snprintf(SqlBuf, SQL_BUF_SIZE, "lock tables %s write", table_name);
	else
		snprintf(SqlBuf, SQL_BUF_SIZE, "unlock tables");
	if (mysql_query(which == ON_MASTER ? &mysql[0] : &mysql[1], SqlBuf))
	{
		fprintf(stderr, "lockTable: mysql_query: %s: %s\n", SqlBuf, mysql_error(which == ON_MASTER ? &mysql[0] : &mysql[1]));
		return (1);
	}
	return (0);
}

void
getversion_lockTable_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
