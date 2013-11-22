/*
 * $Log: vauth_getall.c,v $
 * Revision 2.4  2008-05-28 16:39:00+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 2.3  2008-05-28 15:23:39+05:30  Cprogrammer
 * removed ldap, cdb module
 *
 * Revision 2.2  2002-06-26 03:19:48+05:30  Cprogrammer
 * VLDAP_BASEDN changed to LDAP_BASEDN
 *
 * Revision 2.1  2002-05-13 02:27:58+05:30  Cprogrammer
 * changed data type to unsigned long
 *
 * Revision 1.9  2002-03-03 17:14:04+05:30  Cprogrammer
 * replaced strcat with scat
 * corrected wrong sizes used in scopy
 *
 * Revision 1.8  2001-12-23 11:44:46+05:30  Cprogrammer
 * allocate one more struct in storeSql() to prevent core dump
 *
 * Revision 1.7  2001-12-14 10:34:34+05:30  Cprogrammer
 * treat no existent of indibak not as an error
 * Funtion FreeSqlPtr to free memory
 *
 * Revision 1.6  2001-12-09 00:16:47+05:30  Cprogrammer
 * code restructuring done
 *
 * Revision 1.5  2001-12-02 20:22:50+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.4  2001-11-24 12:20:56+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.3  2001-11-20 11:06:32+05:30  Cprogrammer
 * *** empty log message ***
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: vauth_getall.c,v 2.4 2008-05-28 16:39:00+05:30 Cprogrammer Stab mbhangui $";
#endif

#include <stdlib.h>
#include <string.h>
#include <mysqld_error.h>

static int      storeSql(int *, int, MYSQL_RES *);
static void     FreeSqlPtr();

static struct passwd **SqlPtr;
struct passwd *
vauth_getall(char *domain, int first, int sortit)
{
	char           *domstr;
	static int      more, flag;
	int             err;
	unsigned long   num1, num2;
	static MYSQL_RES *res;
	char            SqlBuf[SQL_BUF_SIZE];

	if (first == 1 || !flag)
	{
		if ((err = vauth_open((char *) 0)) != 0)
			return ((struct passwd *) 0);
		if(SqlPtr)
			FreeSqlPtr();
		/*-- Active Table --------*/
		if (site_size == LARGE_SITE)
		{
			domstr = vauth_munch_domain(domain);
			snprintf(SqlBuf, SQL_BUF_SIZE, LARGE_GETALL, domstr);
		}
		else
			snprintf(SqlBuf, SQL_BUF_SIZE, SMALL_GETALL, default_table, domain);
		if (sortit == 1)
			scat(SqlBuf, " order by pw_name", SQL_BUF_SIZE);
		if (mysql_query(&mysql[1], SqlBuf))
		{
			mysql_perror("vauth_getall: %s", SqlBuf);
			return ((struct passwd *) 0);
		}
		if (!(res = mysql_store_result(&mysql[1])))
		{
			mysql_perror("vauth_getall: mysql_store_result");
			return ((struct passwd *) 0);
		}
		num1 = num2 = more = 0;
		if((num1 = mysql_num_rows(res)) > 0)
			storeSql(&more, num1, res);
		mysql_free_result(res);
		res = (MYSQL_RES *) 0;
		/*-- Inactive Table ------*/
		snprintf(SqlBuf, SQL_BUF_SIZE, SMALL_GETALL, inactive_table, domain);
		if (sortit == 1)
			scat(SqlBuf, " order by pw_name", SQL_BUF_SIZE);
		if(!mysql_query(&mysql[1], SqlBuf))
		{
			if (!(res = mysql_store_result(&mysql[1])))
			{
				mysql_perror("vauth_getall: mysql_store_result");
				if(SqlPtr)
					FreeSqlPtr();
				return ((struct passwd *) 0);
			}
			if((num2 = mysql_num_rows(res)) > 0)
				storeSql(&more, num1 + num2, res);
			mysql_free_result(res);
			res = (MYSQL_RES *) 0;
		} else
		if(mysql_errno(&mysql[1]) != ER_NO_SUCH_TABLE)
		{
			if(SqlPtr)
				FreeSqlPtr();
			return ((struct passwd *) 0);
		}
		if(!(num1 + num2))
			return ((struct passwd *) 0);
		flag++;
		SqlPtr[more] = (struct passwd *) 0;
		more = 0;
	} /*- if (first == 1 || !flag) */
	if(SqlPtr[more])
		return(SqlPtr[more++]);
	if(SqlPtr)
		FreeSqlPtr();
	more = flag = 0;
	SqlPtr = (struct passwd **) 0;
	return ((struct passwd *) 0);
}

static int
storeSql(int *more, int num, MYSQL_RES *res)
{
	MYSQL_ROW       row;
	int             len;

	if(!(SqlPtr = (struct passwd **) realloc(SqlPtr, sizeof(struct passwd *) * (num + 1))))
	{
		perror("malloc");
		return(1);
	}
	for(;(row = mysql_fetch_row(res));(*more)++)
	{
		if(!(SqlPtr[*more] = (struct passwd *) malloc(sizeof(struct passwd))))
		{
			perror("malloc");
			return(1);
		}
		SqlPtr[*more]->pw_name = malloc(len = (slen(row[0]) + 1));
		scopy(SqlPtr[*more]->pw_name, row[0], len);
		SqlPtr[*more]->pw_passwd = malloc(len = (slen(row[1]) + 1));
		scopy(SqlPtr[*more]->pw_passwd, row[1], len);
		SqlPtr[*more]->pw_gecos = malloc(len = (slen(row[4]) + 1));
		scopy(SqlPtr[*more]->pw_gecos, row[4], len);
		SqlPtr[*more]->pw_dir = malloc(len = (slen(row[5]) + 1));
		scopy(SqlPtr[*more]->pw_dir, row[5], len);
		SqlPtr[*more]->pw_shell = malloc(len = (slen(row[6]) + 1));
		scopy(SqlPtr[*more]->pw_shell, row[6], len);
		SqlPtr[*more]->pw_uid = atoi(row[2]);
		SqlPtr[*more]->pw_gid = atoi(row[3]);
	}
	SqlPtr[num] = (struct passwd *) 0;
	return(0);
}

static void
FreeSqlPtr()
{
	int             more;
	
	if(SqlPtr)
	{
		for(more = 0;SqlPtr[more];more++)
		{
			free(SqlPtr[more]->pw_name);
			free(SqlPtr[more]->pw_passwd);
			free(SqlPtr[more]->pw_gecos);
			free(SqlPtr[more]->pw_dir);
			free(SqlPtr[more]->pw_shell);
			free(SqlPtr[more]);
		}
		free(SqlPtr);
		SqlPtr = (struct passwd **) 0;
	}
	return;
}

void
getversion_vauth_getall_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
