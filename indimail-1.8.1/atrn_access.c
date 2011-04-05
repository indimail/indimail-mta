/*
 * $Log: atrn_access.c,v $
 * Revision 2.4  2008-09-08 09:20:59+05:30  Cprogrammer
 * removed mysql_escape
 *
 * Revision 2.3  2008-05-28 16:33:37+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 2.2  2004-07-03 23:51:37+05:30  Cprogrammer
 * check return status of parse_email
 *
 * Revision 2.1  2003-07-18 20:03:05+05:30  Cprogrammer
 * access to domains for atrn
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: atrn_access.c,v 2.4 2008-09-08 09:20:59+05:30 Cprogrammer Stab mbhangui $";
#endif

#include <stdlib.h>
#include <string.h>
#include <mysqld_error.h>
int
atrn_access(char *email, char *domain)
{
	char            SqlBuf[SQL_BUF_SIZE], User[MAX_BUFF], Domain[MAX_BUFF];
	static char     Email[MAX_BUFF];
	int             len, num;
	MYSQL_ROW       row;
	static MYSQL_RES *select_res;

	if(!email || !*email)
	{
		*Email = 0;
		if(select_res)
		{
			mysql_free_result(select_res);
			select_res = (MYSQL_RES *) 0;
		}
		return(0);
	}
	if(*Email && strncmp(Email, email, MAX_BUFF))
	{
		*Email = 0;
		if(select_res)
		{
			mysql_free_result(select_res);
			select_res = (MYSQL_RES *) 0;
		}
	}
	if(!*Email)
	{
		strncpy(Email, email, MAX_BUFF);
		if (vauth_open((char *) 0))
		{
			*Email = 0;
			return (-1);
		}
		if (parse_email(email, User, Domain, MAX_BUFF))
		{
			fprintf(stderr, "%s: Email too long\n", email);
			return (1);
		}
		snprintf(SqlBuf, SQL_BUF_SIZE,
			"select high_priority domain_list from atrn_map where pw_name=\"%s\" and pw_domain=\"%s\"",
			User, Domain);
		if (mysql_query(&mysql[1], SqlBuf))
		{
			*Email = 0;
			if(mysql_errno(&mysql[1]) == ER_NO_SUCH_TABLE)
			{
				create_table(ON_LOCAL, "atrn_map", ATRN_MAP_LAYOUT);
				return (1);
			}
			mysql_perror("etrn_access: %s", SqlBuf);
			return (-1);
		}
		if(!(select_res = mysql_store_result(&mysql[1])))
		{
			*Email = 0;
			return (-1);
		}
		if(!(num = mysql_num_rows(select_res)))
		{
			mysql_free_result(select_res);
			select_res = (MYSQL_RES *) 0;
			*Email = 0;
			return(1);
		}
	}
	mysql_data_seek(select_res, 0);
	for(len = strlen(domain);;)
	{
		if (!(row = mysql_fetch_row(select_res)))
			break;
		if(!strncmp(row[0], domain, len))
			return(0);
	}
	return(1);
}

void
getversion_atrn_access_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
