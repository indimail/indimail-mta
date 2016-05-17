/*
 * $Log: dbinfoAdd.c,v $
 * Revision 2.4  2016-05-17 15:40:09+05:30  Cprogrammer
 * use control directory set by configure
 *
 * Revision 2.3  2008-05-28 16:34:29+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 2.2  2005-12-29 22:40:53+05:30  Cprogrammer
 * use getEnvConfigStr to set variables from environment variables
 *
 * Revision 2.1  2002-12-09 00:14:55+05:30  Cprogrammer
 * function to add mysql parameters to dbinfo
 *
 */
#include "indimail.h"
#include <mysqld_error.h>

#ifndef	lint
static char     sccsid[] = "$Id: dbinfoAdd.c,v 2.4 2016-05-17 15:40:09+05:30 Cprogrammer Exp mbhangui $";
#endif

#ifdef CLUSTERED_SITE
/*
 *
 * File      : controldir/mcdinfo
 * domain    : indicorp.com
 * distFlag  : 0
 * server    : 210.210.122.80
 * mdahost   : 210.210.122.80
 * port      : 3306
 * database  : indimail
 * user      : indimail
 * password  : ssh-1.5-
 * timestamp : 20021130003200
 */
int
dbinfoAdd(char *domain, int distributed, char *sqlserver, char *mdahost, int port, char *database, char *user, char *passwd)
{
	char            SqlBuf[SQL_BUF_SIZE], mcdFile[MAX_BUFF];
	char           *mcdfile, *qmaildir, *controldir;
	int             err;

	getEnvConfigStr(&qmaildir, "QMAILDIR", QMAILDIR);
	getEnvConfigStr(&controldir, "CONTROLDIR", CONTROLDIR);
	getEnvConfigStr(&mcdfile, "MCDFILE", MCDFILE);
	if(*mcdfile == '/')
		scopy(mcdFile, mcdfile, MAX_BUFF);
	else {
		if (*controldir == '/')
			snprintf(mcdFile, MAX_BUFF, "%s/%s", controldir, mcdfile);
		else
			snprintf(mcdFile, MAX_BUFF, "%s/%s/%s", qmaildir, controldir, mcdfile);
	}
	if (open_master())
	{
		fprintf(stderr, "dbinfoAdd: Failed to open Master Db\n");
		return(-1);
	}
	snprintf(SqlBuf, SQL_BUF_SIZE, 
		"insert low_priority into dbinfo ( filename, domain, distributed, server, mdahost, port, dbname, user, passwd ) values \
		( \"%s\", \"%s\", %d, \"%s\", \"%s\", %d, \"%s\", \"%s\", \"%s\")", 
		mcdFile, domain, distributed, sqlserver, mdahost, port, database, user, passwd);
	if (mysql_query(&mysql[0], SqlBuf))
	{
		if(mysql_errno(&mysql[0]) == ER_NO_SUCH_TABLE)
		{
			if(create_table(ON_MASTER, "dbinfo", DBINFO_TABLE_LAYOUT))
				return(-1);
			if (mysql_query(&mysql[0], SqlBuf))
			{
				fprintf(stderr, "dbinfoAdd: mysql_query: %s: %s\n", SqlBuf, mysql_error(&mysql[0]));
				return (-1);
			}
		} else
		{
			fprintf(stderr, "dbinfoAdd: mysql_query: %s: %s\n", SqlBuf, mysql_error(&mysql[0]));
			return (-1);
		}
	}
	if((err = mysql_affected_rows(&mysql[0])) == -1)
	{
		fprintf(stderr, "dbinfoAdd: mysql_affected_rows: %s\n", mysql_error(&mysql[0]));
		return(-1);
	}
	if(!verbose)
		return (!err);
	if(err)
		printf("Added domain %s\n", domain);
	else
		printf("No Domain added %s\n", domain);
	return (!err);
}
#endif

void
getversion_dbinfoAdd_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
