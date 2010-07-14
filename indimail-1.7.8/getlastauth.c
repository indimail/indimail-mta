/*
 * $Log: getlastauth.c,v $
 * Revision 2.7  2008-09-08 09:42:43+05:30  Cprogrammer
 * changes for using mysql_real_escape_string
 *
 * Revision 2.6  2008-05-28 16:35:44+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 2.5  2002-12-02 01:47:44+05:30  Cprogrammer
 * added webm as a service
 *
 * Revision 2.4  2002-08-03 04:28:26+05:30  Cprogrammer
 * mysql string fix
 *
 * Revision 2.3  2002-08-01 11:09:35+05:30  Cprogrammer
 * select in lower case to avoid problems when user logs in with upper case
 *
 * Revision 2.2  2002-07-09 14:46:12+05:30  Cprogrammer
 * removed user addition entries from calculation as users are now added to indibak
 *
 * Revision 2.1  2002-05-13 02:26:42+05:30  Cprogrammer
 * changed data type to unsigned long
 *
 * Revision 1.11  2002-03-03 17:12:51+05:30  Cprogrammer
 * replaced strcat with scat
 * corrected wrong sizes used in snprintf, and scopy
 *
 * Revision 1.10  2002-01-09 23:12:52+05:30  Cprogrammer
 * option added for low memory clients
 *
 * Revision 1.9  2001-12-09 02:18:31+05:30  Cprogrammer
 * added 'add' also in criteria for determining active users
 *
 * Revision 1.8  2001-12-08 14:43:33+05:30  Cprogrammer
 * corrected syntax error with sql statement
 *
 * Revision 1.7  2001-12-08 00:34:24+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.6  2001-12-02 20:21:21+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.5  2001-11-30 01:06:17+05:30  Cprogrammer
 * set count to -1 indicating error
 *
 * Revision 1.4  2001-11-28 22:58:50+05:30  Cprogrammer
 * stdlib.h added
 *
 * Revision 1.3  2001-11-24 20:22:53+05:30  Cprogrammer
 * Added options to
 * 1. return either active or inactive users
 * 2. return the no of active or inactive users in variable count
 *
 * Revision 1.2  2001-11-24 12:19:05+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.1  2001-11-20 11:12:00+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: getlastauth.c,v 2.7 2008-09-08 09:42:43+05:30 Cprogrammer Stab mbhangui $";
#endif

#include <string.h>
#include <time.h>
#include <stdlib.h>

/*
 * Type 1 : Get Active   User List
 * Type 2 : Get Inactive User List
 */
char  **
getlastauth(char *Domain, int Age, int sortit, int Type, unsigned long *count)
{
	char            SqlBuf[SQL_BUF_SIZE];
	static char   **SqlPtr;
	time_t          tmval;
	unsigned long   num, more;
	MYSQL_RES      *res;
	MYSQL_ROW       row;

	*count = 0;
	if (vauth_open((char *) 0) != 0)
	{
		*count = -1;
		return ((char **) 0);
	}
	/*
	 * get the time 
	 */
	tmval = time(NULL);
	/*
	 * subtract the age 
	 */
	tmval = tmval - (86400 * Age);
	if(Type == 1)
		snprintf(SqlBuf, SQL_BUF_SIZE, "select distinct lower(user) from lastauth where domain = \"%s\" and \
			(service=\"pop3\" or service=\"imap\" or service=\"webm\") and UNIX_TIMESTAMP(timestamp) >= %ld", Domain, tmval);
	else
		snprintf(SqlBuf, SQL_BUF_SIZE, "select distinct lower(user) from lastauth where domain = \"%s\" and \
			(service=\"pop3\" or service=\"imap\" or service=\"webm\") and UNIX_TIMESTAMP(timestamp) < %ld", Domain, tmval);
	if (sortit == 1)
		scat(SqlBuf, " order by user", SQL_BUF_SIZE);
	if (mysql_query(&mysql[1], SqlBuf))
	{
		mysql_perror("getlastauth: %s", SqlBuf);
		*count = -1;
		return ((char **) 0);
	}
#ifdef LOW_MEM
	if (!(res = mysql_use_result(&mysql[1])))
#else
	if (!(res = mysql_store_result(&mysql[1])))
#endif
	{
		mysql_perror("getlastauth: mysql_store_result");
		*count = -1;
		return ((char **) 0);
	}
	if(!(num = mysql_num_rows(res)))
	{
		mysql_free_result(res);
		return ((char **) 0);
	}
	if(!(SqlPtr = (char **) calloc(1, sizeof(char *) * (num + 1))))
	{
		perror("malloc");
		mysql_free_result(res);
		*count = -1;
		return ((char **) 0);
	}
	for(more = 0;(row = mysql_fetch_row(res));more++)
	{
		if(!(SqlPtr[more] = malloc(num = (slen(row[0]) + 1))))
		{
			perror("malloc");
			mysql_free_result(res);
			*count = -1;
			return ((char **) 0);
		}
		scopy(SqlPtr[more], row[0], num);
	}
#ifdef LOW_MEM
	if(!mysql_eof(res))
	{
		perror("malloc");
		mysql_free_result(res);
		*count = -1;
		return ((char **) 0);
	}
#endif
	*count = more;
	mysql_free_result(res);
	SqlPtr[more] = (char *) 0;
	return(SqlPtr);
}

void
getversion_getlastauth_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
