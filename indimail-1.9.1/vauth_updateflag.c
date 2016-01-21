/*
 * $Log: vauth_updateflag.c,v $
 * Revision 2.5  2009-02-06 11:40:39+05:30  Cprogrammer
 * fixed potential buffer overflow
 *
 * Revision 2.4  2008-09-08 09:56:27+05:30  Cprogrammer
 * removed mysql_escape
 *
 * Revision 2.3  2008-05-28 16:39:58+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 2.2  2002-08-05 01:11:00+05:30  Cprogrammer
 * added mysql_escape for username
 *
 * Revision 2.1  2002-08-03 04:36:26+05:30  Cprogrammer
 * mysql string fix
 *
 * Revision 1.3  2001-11-29 20:57:26+05:30  Cprogrammer
 * conditional compilation for distributed architecture
 * code for deleting entry if flag passed is -1
 *
 * Revision 1.2  2001-11-28 23:28:25+05:30  Cprogrammer
 * removed hardcoding of indimail table
 *
 * Revision 1.1  2001-11-28 22:54:34+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: vauth_updateflag.c,v 2.5 2009-02-06 11:40:39+05:30 Cprogrammer Stab mbhangui $";
#endif

#ifdef CLUSTERED_SITE
#include <stdlib.h>
#include <string.h>

int
vauth_updateflag(char *user, char *domain, int flag)
{
	int             err;
	char            SqlBuf[SQL_BUF_SIZE];

	if (!user || !*user || !domain || !*domain)
		return (-1);
	if (vauth_open(0))
		return (-1);
	if(flag == -1)
		snprintf(SqlBuf, SQL_BUF_SIZE, 
		"delete low_priority from %s where  pw_name = \"%s\" and pw_domain = \"%s\"",  
			default_table, user, domain);
	else
		snprintf(SqlBuf, SQL_BUF_SIZE, 
		"update low_priority %s set pw_uid = %d where pw_name = \"%s\" and pw_domain = \"%s\"",  
			default_table, flag, user, domain);
	if (mysql_query(&mysql[1], SqlBuf))
	{
		mysql_perror("vauth_updateflag: %s", SqlBuf);
		return (-1);
	}
	err = mysql_affected_rows(&mysql[1]);
	if(!err)
	{
		if(flag == -1)
			snprintf(SqlBuf, SQL_BUF_SIZE, 
			"delete low_priority from %s where  pw_name = \"%s\" and pw_domain = \"%s\"",  
				inactive_table, user, domain);
		else
			snprintf(SqlBuf, SQL_BUF_SIZE, 
			"update low_priority %s set pw_uid = %d where pw_name = \"%s\" and pw_domain = \"%s\"",  
				inactive_table, flag, user, domain);
		if (mysql_query(&mysql[1], SqlBuf))
		{
			mysql_perror("vauth_updateflag: %s", SqlBuf);
			return (-1);
		}
		err = mysql_affected_rows(&mysql[1]);
	}
	return(((err == -1 || !err) ?  1 : 0));
}
#endif

void
getversion_vauth_updateflag_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
	return;
}
