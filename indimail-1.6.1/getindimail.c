/*
 * $Log: getindimail.c,v $
 * Revision 2.9  2008-05-28 16:35:42+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 2.8  2005-01-01 00:23:09+05:30  Cprogrammer
 * added comments for SqlPtr
 *
 * Revision 2.7  2004-09-20 20:09:49+05:30  Cprogrammer
 * skip group entries for inactivation
 *
 * Revision 2.6  2003-11-15 23:05:06+05:30  Cprogrammer
 * Bug. variable more incremented before assigning to SqlPtr
 *
 * Revision 2.5  2003-05-31 12:09:58+05:30  Cprogrammer
 * compare on length when checking skip gecos
 *
 * Revision 2.4  2003-03-30 23:16:15+05:30  Cprogrammer
 * added logic to skip entries with specific gecos
 *
 * Revision 2.3  2002-08-03 04:31:45+05:30  Cprogrammer
 * mysql string fix
 *
 * Revision 2.2  2002-08-03 00:36:32+05:30  Cprogrammer
 * select of pw_name changed to lower case to prevent vdeloldusers LocateUser() case comparision problem
 *
 * Revision 2.1  2002-05-13 02:27:07+05:30  Cprogrammer
 * changed data type to unsigned long
 *
 * Revision 1.10  2002-03-03 17:13:31+05:30  Cprogrammer
 * replaced strcat with scat
 * corrected wrong sizes used in scopy
 *
 * Revision 1.9  2001-12-24 00:56:35+05:30  Cprogrammer
 * allocated one more byte to save the 1st char of the username
 *
 * Revision 1.8  2001-12-08 14:43:46+05:30  Cprogrammer
 * removed printf statement
 *
 * Revision 1.7  2001-12-03 09:39:52+05:30  Cprogrammer
 * stored the directory also in SqlPtr
 *
 * Revision 1.6  2001-12-02 20:21:34+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.5  2001-11-30 01:06:33+05:30  Cprogrammer
 * set count to -1 indicating error
 *
 * Revision 1.4  2001-11-30 00:13:11+05:30  Cprogrammer
 * set variable userNotFound if records not found
 *
 * Revision 1.3  2001-11-24 20:23:49+05:30  Cprogrammer
 * add option to return no of users in variable count
 *
 * Revision 1.2  2001-11-24 12:19:07+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.1  2001-11-23 00:12:01+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: getindimail.c,v 2.9 2008-05-28 16:35:42+05:30 Cprogrammer Stab mbhangui $";
#endif

#include <stdlib.h>
#include <string.h>

char  **
getindimail(char *domain, int sortit, char **skipGecos, unsigned long *count)
{
	static char   **SqlPtr;
	char          **ptr;
	unsigned long   more, num, dlen;
	static MYSQL_RES *res;
	MYSQL_ROW       row;
	int             skip;
	char            SqlBuf[SQL_BUF_SIZE];

	*count = 0;
	if (vauth_open((char *) 0))
	{
		*count = -1;
		return ((char **) 0);
	}
	if(skipGecos)
		snprintf(SqlBuf, SQL_BUF_SIZE, "select lower(pw_name),pw_dir,pw_gecos from %s where pw_domain=\"%s\"", 
			default_table, domain);
	else
		snprintf(SqlBuf, SQL_BUF_SIZE, "select lower(pw_name),pw_dir from %s where pw_domain=\"%s\"", 
			default_table, domain);
	if (sortit == 1)
		scat(SqlBuf, " order by pw_name", SQL_BUF_SIZE);
	if (mysql_query(&mysql[1], SqlBuf))
	{
		mysql_perror("pw_name_getall: %s", SqlBuf);
		*count = -1;
		return ((char **) 0);
	}
	if (!(res = mysql_store_result(&mysql[1])))
	{
		mysql_perror("pw_name_getall: mysql_store_result");
		*count = -1;
		return ((char **) 0);
	}
	if(!(num = mysql_num_rows(res)))
	{
		mysql_free_result(res);
		res = (MYSQL_RES *) 0;
		return ((char **) 0);
	}
	if(!(SqlPtr = (char **) calloc(1, sizeof(char *) * (num + 1))))
	{
		perror("malloc");
		mysql_free_result(res);
		res = (MYSQL_RES *) 0;
		*count = -1;
		return ((char **) 0);
	}
	for(more = 0;(row = mysql_fetch_row(res));)
	{
		if(skipGecos)
		{
			for(skip = 0, ptr = skipGecos;*ptr;ptr++)
			{
				if(!memcmp(*ptr, row[2], strlen(*ptr) + 1))
				{
					skip = 1;
					break;
				} else
				if (!strncmp("MailGroup", *ptr, 10) && !memcmp("MailGroup ", row[2], 10))
				{
					skip = 1;
					break;
				}
			}
			if(skip)
				continue;
		}
		if(!(SqlPtr[more] = malloc((num = slen(row[0])) + (dlen = slen(row[1])) + 3)))
		{
			perror("malloc");
			mysql_free_result(res);
			res = (MYSQL_RES *) 0;
			*count = -1;
			return ((char **) 0);
		}
		/*
		 * How pw_name and pw_dir is stored in SqlPtr
		 *
		 * pw_name
		 * NULL
		 * First Char of pw_name
		 * pw_dir
		 */
		scopy(SqlPtr[more], row[0], num + 1);
		*(SqlPtr[more] + num + 1) = *(row[0]); /*- Save the first char in username */
		scopy(SqlPtr[more] + num + 2, row[1], dlen + 1);
		more++;
	} /*- for(more = 0;(row = mysql_fetch_row(res));) */
	*count = more;
	mysql_free_result(res);
	SqlPtr[more] = (char *) 0;
	return(SqlPtr);
}

void
getversion_getindimail_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
