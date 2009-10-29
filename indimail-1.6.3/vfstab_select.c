/*
 * $Log: vfstab_select.c,v $
 * Revision 2.6  2009-10-14 20:47:33+05:30  Cprogrammer
 * use strtoll() instead of atol()
 *
 * Revision 2.5  2008-05-28 17:41:21+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 2.4  2003-07-25 09:24:36+05:30  Cprogrammer
 * removed blank line
 *
 * Revision 2.3  2002-10-27 21:39:49+05:30  Cprogrammer
 * used generic function create_table()
 *
 * Revision 2.2  2002-08-11 01:01:51+05:30  Cprogrammer
 * changed high_priority selects to low_priority selects
 *
 * Revision 2.1  2002-08-09 20:24:56+05:30  Cprogrammer
 * function to select fstab entries
 *
 */

#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: vfstab_select.c,v 2.6 2009-10-14 20:47:33+05:30 Cprogrammer Stab mbhangui $";
#endif

#define XOPEN_SOURCE = 600
#include <stdlib.h>
#include <mysqld_error.h>

char           *
vfstab_select(char *host, int *status, long *max_users, long *cur_users, long *max_size, long *cur_size)
{
	static char     FileSystem[MAX_BUFF];
	char            SqlBuf[SQL_BUF_SIZE];
	static MYSQL_RES *res;
	MYSQL_ROW       row;

	if(!res)
	{
#ifdef CLUSTERED_SITE
		if(open_central_db(0))
			return ((char *) 0);
#else
		if(vauth_open(0))
			return ((char *) 0);
#endif
		if(host && *host)
			snprintf(SqlBuf, SQL_BUF_SIZE, 
			"select filesystem,host,status,max_users,cur_users,max_size,cur_size from fstab where \
			host = \"%s\" order by filesystem", host);
		else
			snprintf(SqlBuf, SQL_BUF_SIZE, 
			"select filesystem,host,status,max_users,cur_users,max_size,cur_size from fstab order by filesystem");
#ifdef CLUSTERED_SITE
		if (mysql_query(&mysql[0], SqlBuf))
#else
		if (mysql_query(&mysql[1], SqlBuf))
#endif
		{
#ifdef CLUSTERED_SITE
			if(mysql_errno(&mysql[0]) == ER_NO_SUCH_TABLE)
#else
			if(mysql_errno(&mysql[1]) == ER_NO_SUCH_TABLE)
#endif
			{
#ifdef CLUSTERED_SITE
				if(create_table(ON_MASTER, "fstab", FSTAB_TABLE_LAYOUT))
#else
				if(create_table(ON_LOCAL, "fstab", FSTAB_TABLE_LAYOUT))
#endif
					return((char *) 0);
				return ((char *) 0);
			} else
			{
#ifdef CLUSTERED_SITE
				fprintf(stderr, "vfstab_select: %s: %s\n", SqlBuf, mysql_error(&mysql[0]));
#else
				fprintf(stderr, "vfstab_select: %s: %s\n", SqlBuf, mysql_error(&mysql[1]));
#endif
				return ((char *) 0);
			}
		}
#ifdef CLUSTERED_SITE
		if(!(res = mysql_store_result(&mysql[0])))
#else
		if(!(res = mysql_store_result(&mysql[1])))
#endif
			return ((char *) 0);
	} 
	if ((row = mysql_fetch_row(res)))
	{
		scopy(FileSystem, row[0], MAX_BUFF);
		if(host)
			scopy(host, row[1], MAX_BUFF);
		if(status)
			*status = atoi(row[2]);
		if(max_users)
			*max_users = atol(row[3]);
		if(cur_users)
			*cur_users = atol(row[4]);
		if(max_size)
			*max_size = strtoll(row[5], 0, 0);
		if(cur_size)
			*cur_size = strtoll(row[6], 0, 0);
		return (FileSystem);
	}
	mysql_free_result(res);
	res = (MYSQL_RES *) 0;
	return ((char *) 0);
}

void
getversion_vfstab_select_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
