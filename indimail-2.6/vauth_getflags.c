/*
 * $Log: vauth_getflags.c,v $
 * Revision 2.4  2008-05-28 16:39:02+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 2.3  2002-08-11 18:40:57+05:30  Cprogrammer
 * removed PWD_FLAG from select as passwd is now not updated in hostcntrl
 * removed redundant code for selecting from indibak if indibak does not exist
 *
 * Revision 2.2  2002-08-03 04:35:09+05:30  Cprogrammer
 * mysql string fix
 *
 * Revision 2.1  2002-05-13 02:28:07+05:30  Cprogrammer
 * changed data type to unsigned long
 *
 * Revision 1.4  2002-03-29 22:45:03+05:30  Cprogrammer
 * changed queries to use proper index
 * mysql_free_result() when function returns
 *
 * Revision 1.3  2001-11-30 01:06:49+05:30  Cprogrammer
 * added creation of table indibak if not existing
 *
 * Revision 1.2  2001-11-29 20:56:40+05:30  Cprogrammer
 * conditional compilation for distributed architecture
 *
 * Revision 1.1  2001-11-29 13:25:07+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: vauth_getflags.c,v 2.4 2008-05-28 16:39:02+05:30 Cprogrammer Stab mbhangui $";
#endif

#ifdef CLUSTERED_SITE
#include <stdlib.h>
#include <string.h>
#include <mysqld_error.h>

static struct passwd **SqlPtr;
struct passwd *
vauth_getflags(char *domain, int first)
{
	static int      more, flag;
	int             err;
	unsigned long   num;
	static MYSQL_RES *res1, *res2;
	MYSQL_ROW       row;
	char            SqlBuf[SQL_BUF_SIZE];

	if (first == 1 || !flag)
	{
		if ((err = vauth_open((char *) 0)) != 0)
			return ((struct passwd *) 0);
		snprintf(SqlBuf, SQL_BUF_SIZE, "select pw_name, pw_passwd, pw_uid from %s where pw_uid in (%d, %d) \
			and pw_domain=\"%s\"", default_table, ADD_FLAG, DEL_FLAG, domain);
		if (mysql_query(&mysql[1], SqlBuf))
		{
			mysql_perror("vauth_getflags: %s", SqlBuf);
			return ((struct passwd *) 0);
		}
		if (!(res1 = mysql_store_result(&mysql[1])))
		{
			mysql_perror("vauth_getflags: mysql_store_result");
			return ((struct passwd *) 0);
		}
		snprintf(SqlBuf, SQL_BUF_SIZE, "select pw_name, pw_passwd, pw_uid from %s where pw_uid in (%d, %d) \
			and pw_domain=\"%s\"", inactive_table, ADD_FLAG, DEL_FLAG, domain);
		if (mysql_query(&mysql[1], SqlBuf))
		{
			if(mysql_errno(&mysql[0]) == ER_NO_SUCH_TABLE)
			{
				snprintf(SqlBuf, SQL_BUF_SIZE, "CREATE TABLE IF NOT EXISTS %s ( %s )", inactive_table, SMALL_TABLE_LAYOUT);
				if (mysql_query(&mysql[1], SqlBuf))
				{
					mysql_perror("vauth_getflags: %s", SqlBuf);
					mysql_free_result(res1);
					res1 = (MYSQL_RES *) 0;
					res2 = (MYSQL_RES *) 0;
					return ((struct passwd *) 0);
				}
				res2 = (MYSQL_RES *) 0;
			} else
			{
				mysql_perror("vauth_getflags: %s", SqlBuf);
				mysql_free_result(res1);
				res1 = (MYSQL_RES *) 0;
				res2 = (MYSQL_RES *) 0;
				return ((struct passwd *) 0);
			}
		} else
		if (!(res2 = mysql_store_result(&mysql[1])))
		{
			mysql_free_result(res1);
			res1 = (MYSQL_RES *) 0;
			mysql_perror("vauth_getflags: mysql_store_result");
			return ((struct passwd *) 0);
		}
		if(!(num = (res2 ? (mysql_num_rows(res1) + mysql_num_rows(res2)) : mysql_num_rows(res1))))
		{
			mysql_free_result(res1);
			if(res2)
				mysql_free_result(res2);
			res1 = (MYSQL_RES *) 0;
			res2 = (MYSQL_RES *) 0;
			return ((struct passwd *) 0);
		}
		if(SqlPtr)
		{
			for(more = 0;SqlPtr[more];more++)
			{
				free(SqlPtr[more]->pw_name);
				free(SqlPtr[more]->pw_passwd);
				free(SqlPtr[more]);
			}
			free(SqlPtr);
			SqlPtr = (struct passwd **) 0;
		}
		if(!(SqlPtr = (struct passwd **) calloc(1, sizeof(struct passwd *) * (num + 1))))
		{
			perror("malloc");
			mysql_free_result(res1);
			if(res2)
				mysql_free_result(res2);
			res1 = (MYSQL_RES *) 0;
			res2 = (MYSQL_RES *) 0;
			return ((struct passwd *) 0);
		}
		for(more = 0;(row = mysql_fetch_row(res1));more++)
		{
			if(!(SqlPtr[more] = (struct passwd *) malloc(sizeof(struct passwd))))
			{
				perror("malloc");
				mysql_free_result(res1);
				if(res2)
					mysql_free_result(res2);
				res1 = (MYSQL_RES *) 0;
				res2 = (MYSQL_RES *) 0;
				for(more--;more != -1;more--)
					free(SqlPtr[more]);
				free(SqlPtr);
				SqlPtr = (struct passwd **) 0;
				return ((struct passwd *) 0);
			}
			SqlPtr[more]->pw_name = malloc(slen(row[0]) + 1);
			SqlPtr[more]->pw_passwd = malloc(slen(row[1]) + 1);
			scopy(SqlPtr[more]->pw_name, row[0], MAX_BUFF);
			scopy(SqlPtr[more]->pw_passwd, row[1], MAX_BUFF);
			SqlPtr[more]->pw_uid = atoi(row[2]);
		}
		for(;res2 && (row = mysql_fetch_row(res2));more++)
		{
			if(!(SqlPtr[more] = (struct passwd *) malloc(sizeof(struct passwd))))
			{
				perror("malloc");
				mysql_free_result(res1);
				if(res2)
					mysql_free_result(res2);
				res1 = (MYSQL_RES *) 0;
				res2 = (MYSQL_RES *) 0;
				for(more--;more != -1;more--)
					free(SqlPtr[more]);
				free(SqlPtr);
				SqlPtr = (struct passwd **) 0;
				return ((struct passwd *) 0);
			}
			SqlPtr[more]->pw_name = malloc(slen(row[0]) + 1);
			SqlPtr[more]->pw_passwd = malloc(slen(row[1]) + 1);
			scopy(SqlPtr[more]->pw_name, row[0], MAX_BUFF);
			scopy(SqlPtr[more]->pw_passwd, row[1], MAX_BUFF);
			SqlPtr[more]->pw_uid = atoi(row[2]);
		}
		flag++;
		mysql_free_result(res1);
		if(res2)
			mysql_free_result(res2);
		res1 = (MYSQL_RES *) 0;
		res2 = (MYSQL_RES *) 0;
		SqlPtr[more] = (struct passwd *) 0;
		more = 1;
		return(SqlPtr[0]);
	} 
	if(SqlPtr[more])
		return(SqlPtr[more++]);
	for(more = 0;SqlPtr[more];more++)
	{
		free(SqlPtr[more]->pw_name);
		free(SqlPtr[more]->pw_passwd);
		free(SqlPtr[more]);
	}
	more = flag = 0;
	free(SqlPtr);
	SqlPtr = (struct passwd **) 0;
	return ((struct passwd *) 0);
}
#endif

void
getversion_vauth_getflags_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
