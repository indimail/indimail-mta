/*
 * $Log: dbinfoDel.c,v $
 * Revision 2.5  2008-09-08 09:33:50+05:30  Cprogrammer
 * formatting of long line
 *
 * Revision 2.4  2008-05-28 16:34:32+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 2.3  2005-12-29 22:41:03+05:30  Cprogrammer
 * use getEnvConfigStr to set variables from environment variables
 *
 * Revision 2.2  2003-12-08 20:04:47+05:30  Cprogrammer
 * do not delete if table is missing - create dbinfo table only
 * return success for no rows also
 *
 * Revision 2.1  2002-12-09 00:15:30+05:30  Cprogrammer
 * function to delete mysql server definition from dbinfo
 *
 */
#include "indimail.h"
#include <mysqld_error.h>

#ifndef	lint
static char     sccsid[] = "$Id: dbinfoDel.c,v 2.5 2008-09-08 09:33:50+05:30 Cprogrammer Stab mbhangui $";
#endif

#ifdef CLUSTERED_SITE
int
dbinfoDel(char *domain, char *mdahost)
{
	char            SqlBuf[SQL_BUF_SIZE], mcdFile[MAX_BUFF];
	char           *mcdfile, *qmaildir, *controldir;
	int             err;

	getEnvConfigStr(&qmaildir, "QMAILDIR", QMAILDIR);
	getEnvConfigStr(&controldir, "CONTROLDIR", "control");
	getEnvConfigStr(&mcdfile, "MCDFILE", MCDFILE);
	if(*mcdfile == '/')
		scopy(mcdFile, mcdfile, MAX_BUFF);
	else
		snprintf(mcdFile, MAX_BUFF, "%s/%s/%s", qmaildir, controldir, mcdfile);
	if (open_master())
	{
		fprintf(stderr, "dbinfoDel: Failed to open Master Db\n");
		return(-1);
	}
	snprintf(SqlBuf, SQL_BUF_SIZE,
		"delete low_priority from dbinfo where filename=\"%s\" and domain=\"%s\" and mdahost=\"%s\"",
		mcdFile, domain, mdahost);
	if (mysql_query(&mysql[0], SqlBuf))
	{
		if(mysql_errno(&mysql[0]) == ER_NO_SUCH_TABLE)
		{
			if(create_table(ON_MASTER, "dbinfo", DBINFO_TABLE_LAYOUT))
				return(-1);
			return(0);
		} else
		{
			fprintf(stderr, "dbinfoDel: mysql_query: %s: %s\n", SqlBuf, mysql_error(&mysql[0]));
			return (-1);
		}
	}
	if((err = mysql_affected_rows(&mysql[0])) == -1)
	{
		fprintf(stderr, "dbinfoDel: mysql_affected_rows: %s\n", mysql_error(&mysql[0]));
		return(-1);
	}
	if (verbose)
		printf("%s %s\n", err ? "Deleted Domain" : "No MCD for domain" , domain);
	return (0);
}
#endif

void
getversion_dbinfoDel_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
