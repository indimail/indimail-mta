/*
 * $Log: dbinfoUpdate.c,v $
 * Revision 2.5  2016-05-17 17:09:39+05:30  Cprogrammer
 * use control directory set by configure
 *
 * Revision 2.4  2008-09-08 09:34:10+05:30  Cprogrammer
 * formatting of long line
 *
 * Revision 2.3  2008-05-28 16:34:34+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 2.2  2005-12-29 22:41:06+05:30  Cprogrammer
 * use getEnvConfigStr to set variables from environment variables
 *
 * Revision 2.1  2003-01-01 02:33:28+05:30  Cprogrammer
 * function to update dbinfo entries
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: dbinfoUpdate.c,v 2.5 2016-05-17 17:09:39+05:30 Cprogrammer Exp mbhangui $";
#endif

#ifdef CLUSTERED_SITE
#include <mysqld_error.h>

int
dbinfoUpdate(char *domain, int dist, char *sqlserver, char *mdahost, int port, char *database, char *user, char *passwd)
{
	char            SqlBuf[SQL_BUF_SIZE], mcdFile[MAX_BUFF], optbuf[MAX_BUFF];
	char           *mcdfile, *qmaildir, *controldir;
	int             err, len;

	getEnvConfigStr(&controldir, "CONTROLDIR", CONTROLDIR);
	getEnvConfigStr(&mcdfile, "MCDFILE", MCDFILE);
	if(*mcdfile == '/')
		scopy(mcdFile, mcdfile, MAX_BUFF);
	else {
		if (*controldir == '/')
			snprintf(mcdFile, MAX_BUFF, "%s/%s", controldir, mcdfile);
		else {
			getEnvConfigStr(&qmaildir, "QMAILDIR", QMAILDIR);
			snprintf(mcdFile, MAX_BUFF, "%s/%s/%s", qmaildir, controldir, mcdfile);
		}
	}
	if (open_master())
	{
		fprintf(stderr, "dbinfoUpdate: Failed to open Master Db\n");
		return(-1);
	}
	len = 0;
	*optbuf = 0;
	if(dist != -1)
	{
		snprintf(optbuf, sizeof(optbuf), "distributed=%d,", dist);
		len = slen(optbuf);
	}
	if(sqlserver && *sqlserver)
	{
		snprintf(optbuf + len, sizeof(optbuf) - len, "server=\"%s\",", sqlserver);
		len = slen(optbuf);
	}
	if(port != -1)
	{
		snprintf(optbuf + len, sizeof(optbuf) - len, "port=%d,", port);
		len = slen(optbuf);
	}
	if(database && *database)
	{
		snprintf(optbuf + len, sizeof(optbuf) - len, "dbname=\"%s\",", database);
		len = slen(optbuf);
	}
	if(user && *user)
	{
		snprintf(optbuf + len, sizeof(optbuf) - len, "user=\"%s\",", user);
		len = slen(optbuf);
	}
	if(passwd && *passwd)
	{
		snprintf(optbuf + len, sizeof(optbuf) - len, "passwd=\"%s\",", passwd);
		len = slen(optbuf);
	}
	optbuf[len - 1] = 0;
	if(!*optbuf)
	{
		fprintf(stderr, "Invalid Arguments\n");
		return(-1);
	}
	snprintf(SqlBuf, SQL_BUF_SIZE,
		"update low_priority dbinfo set %s where filename=\"%s\" and domain=\"%s\" and mdahost=\"%s\"",
		optbuf, mcdFile, domain, mdahost);
	if (mysql_query(&mysql[0], SqlBuf))
	{
		fprintf(stderr, "dbinfoUpdate: mysql_query: %s: %s\n", SqlBuf, mysql_error(&mysql[0]));
		if(mysql_errno(&mysql[0]) == ER_NO_SUCH_TABLE)
			create_table(ON_MASTER, "dbinfo", DBINFO_TABLE_LAYOUT);
		return (-1);
	}
	if((err = mysql_affected_rows(&mysql[0])) == -1)
	{
		fprintf(stderr, "dbinfoUpdate: mysql_affected_rows: %s\n", mysql_error(&mysql[0]));
		return(-1);
	}
	if(!verbose)
		return (!err);
	if(err)
		printf("Updated File %s, domain %s, mdahost %s\n", mcdFile, domain, mdahost);
	else
		printf("No Update for File %s, domain %s, mdahost %s\n", mcdFile, domain, mdahost);
	return (!err);
}
#endif

void
getversion_dbinfoUpdate_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
