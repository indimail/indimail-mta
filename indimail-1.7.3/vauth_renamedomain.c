/*
 * $Log: vauth_renamedomain.c,v $
 * Revision 2.9  2010-03-07 09:28:37+05:30  Cprogrammer
 * check return value of is_distributed_domain for error
 *
 * Revision 2.8  2009-02-18 21:34:49+05:30  Cprogrammer
 * check return value of fscanf
 *
 * Revision 2.7  2009-02-06 11:40:33+05:30  Cprogrammer
 * ignore return value of fscanf
 *
 * Revision 2.6  2008-09-17 22:47:43+05:30  Cprogrammer
 * added dbinfo table
 *
 * Revision 2.5  2008-05-28 16:39:48+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 2.4  2003-06-19 00:14:12+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 2.3  2002-08-03 04:35:46+05:30  Cprogrammer
 * mysql string fix
 *
 * Revision 2.2  2002-05-09 00:37:22+05:30  Cprogrammer
 * added more tables for data to be renamed when changing domain names
 *
 * Revision 2.1  2002-05-05 22:23:53+05:30  Cprogrammer
 * function to rename domains in indimail, indibak and hostcntrl
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: vauth_renamedomain.c,v 2.9 2010-03-07 09:28:37+05:30 Cprogrammer Stab mbhangui $";
#endif

#include <mysqld_error.h>

static int rename_data(int, char *, char *, char *, char *);

int
vauth_renamedomain(char *OldDomain, char *NewDomain, char *domdir)
{
	char            SqlBuf[SQL_BUF_SIZE], tmpbuf[MAX_BUFF], buffer[MAX_BUFF];
	char           *ptr1, *ptr2;
	FILE           *fp;
	int             err = 0;
#ifdef CLUSTERED_SITE
	int             is_dist = 0;
#endif

	if (vauth_open((char *) 0))
		return (-1);
	if (site_size == LARGE_SITE)
	{
		ptr1 = vauth_munch_domain(OldDomain);
		ptr2 = vauth_munch_domain(NewDomain);
		snprintf(SqlBuf, SQL_BUF_SIZE, "rename table %s to %s", ptr1, ptr2);
		if (mysql_query(&mysql[1], SqlBuf))
		{
			mysql_perror("vauth_renamedomain: %s", SqlBuf);
			return (-1);
		}
	} else
	if (rename_data(ON_LOCAL, default_table, "pw_domain", NewDomain, OldDomain))
		err = 1;
	else
#ifdef CLUSTERED_SITE
	if ((is_dist = is_distributed_domain(OldDomain)) == -1)
	{
		fprintf(stderr, "%s: is_distributed_domain failed\n", OldDomain);
		return (-1);
	}
	if (is_dist && rename_data(ON_MASTER, cntrl_table, "pw_domain", NewDomain, OldDomain))
		err = 1;
	else
	if (rename_data(ON_MASTER, "smtp_port", "domain", NewDomain, OldDomain))
		err = 1;
	else
	if (rename_data(ON_MASTER, "dbinfo", "domain", NewDomain, OldDomain))
		err = 1;
	else
#endif
	if (rename_data(ON_LOCAL, inactive_table, "pw_domain", NewDomain, OldDomain))
		err = 1;
	else
	if (rename_data(ON_LOCAL, "valias", "domain", NewDomain, OldDomain))
		err = 1;
	else
	if (rename_data(ON_LOCAL, "dir_control", "domain", NewDomain, OldDomain))
		err = 1;
	snprintf(tmpbuf, MAX_BUFF, "%s/.filesystems", domdir);
	if ((fp = fopen(tmpbuf, "r")) != (FILE *) 0)
	{
		for(;;)
		{
			if (fscanf(fp, "%s", buffer) != 1)
			{
				if (ferror(fp))
				{
					err = 1;
					break;
				}
			}
			if (feof(fp))
				break;
			snprintf(tmpbuf, MAX_BUFF, "dir_control%s", buffer);
			if (rename_data(ON_LOCAL, tmpbuf, "domain", NewDomain, OldDomain))
				err = 1;
		}
		fclose(fp);
	}
	if (rename_data(ON_LOCAL, "lastauth", "domain", NewDomain, OldDomain))
		err = 1;
	else
	if (rename_data(ON_LOCAL, "userquota", "domain", NewDomain, OldDomain))
		err = 1;
	else
	if (rename_data(ON_LOCAL, "vlimit", "domain", NewDomain, OldDomain))
		err = 1;
	return (err);
}

static int
rename_data(int which, char *tablename, char *column_name, char *NewDomain, char *OldDomain)
{
	char            SqlBuf[SQL_BUF_SIZE];

	if (which != ON_MASTER && which != ON_LOCAL)
		return (-1);
	snprintf(SqlBuf, SQL_BUF_SIZE, "update %s set %s=\"%s\" where %s=\"%s\"", 
		tablename, column_name, NewDomain, column_name, OldDomain);
#ifdef CLUSTERED_SITE
	if ((which == ON_MASTER ? open_master() : vauth_open((char *) 0)))
	{
		fprintf(stderr, "rename_data: Failed to open %s db\n", which == ON_MASTER ? "master" : "local");
		return (-1);
	}
#else
	which = ON_LOCAL;
	if (vauth_open((char *) 0))
	{
		fprintf(stderr, "rename_data: Failed to open local db\n");
		return (-1);
	}
#endif
	if (mysql_query(which == ON_MASTER ? &mysql[0] : &mysql[1], SqlBuf))
	{
		if (mysql_errno(which == ON_MASTER ? &mysql[0] : &mysql[1]) == ER_NO_SUCH_TABLE)
			return (0);
		fprintf(stderr, "rename_data: %s: %s\n", tablename, mysql_error(which == ON_MASTER ? &mysql[0] : &mysql[1]));
		return (-1);
	}
	return (0);
}

void
getversion_vauth_renamedomain_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
