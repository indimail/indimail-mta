/*
 * $Log: vfstab_delete.c,v $
 * Revision 2.4  2008-05-28 17:41:12+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 2.3  2002-10-27 21:39:13+05:30  Cprogrammer
 * used generic function create_table()
 *
 * Revision 2.2  2002-08-11 11:46:01+05:30  Cprogrammer
 * return error if 0 rows are deleted
 *
 * Revision 2.1  2002-08-07 18:42:41+05:30  Cprogrammer
 * function to delete fstab entries
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: vfstab_delete.c,v 2.4 2008-05-28 17:41:12+05:30 Cprogrammer Stab mbhangui $";
#endif

#include <mysqld_error.h>

int
vfstab_delete(char *filesystem, char *mdahost)
{
	int             err;
	char            SqlBuf[SQL_BUF_SIZE];

#ifdef CLUSTERED_SITE
	if (open_master())
	{
		fprintf(stderr, "vfstab_delete: Failed to open Master Db\n");
		return (-1);
	}
#else
	if (vauth_open(0))
	{
		fprintf(stderr, "Failed to open local Db\n");
		return (-1);
	}
#endif
	snprintf(SqlBuf, SQL_BUF_SIZE, 
		"delete low_priority from fstab where filesystem = \"%s\" and host = \"%s\"", 
		filesystem, mdahost);
#ifdef CLUSTERED_SITE
	if (mysql_query(&mysql[0], SqlBuf))
#else
	if (mysql_query(&mysql[1], SqlBuf))
#endif
	{
#ifdef CLUSTERED_SITE
		if (mysql_errno(&mysql[0]) == ER_NO_SUCH_TABLE)
#else
		if (mysql_errno(&mysql[1]) == ER_NO_SUCH_TABLE)
#endif
		{
#ifdef CLUSTERED_SITE
			create_table(ON_MASTER, "fstab", FSTAB_TABLE_LAYOUT);
#else
			create_table(ON_LOCAL, "fstab", FSTAB_TABLE_LAYOUT);
#endif
			fprintf(stderr, "vfstab_delete: No rows selected\n");
			return (1);
		} else
		{
#ifdef CLUSTERED_SITE
			fprintf(stderr, "vfstab_delete: %s: %s\n", SqlBuf, mysql_error(&mysql[0]));
#else
			fprintf(stderr, "vfstab_delete: %s: %s\n", SqlBuf, mysql_error(&mysql[1]));
#endif
			return (-1);
		}
	}
#ifdef CLUSTERED_SITE
	if ((err = mysql_affected_rows(&mysql[0])) == -1)
	{
		fprintf(stderr, "vfstab_delete: mysql_affected_rows: %s\n", mysql_error(&mysql[0]));
		return (-1);
	}
#else
	if ((err = mysql_affected_rows(&mysql[1])) == -1)
	{
		fprintf(stderr, "vfstab_delete: mysql_affected_rows: %s\n", mysql_error(&mysql[1]));
		return (-1);
	}
#endif
	if (!err)
	{
		fprintf(stderr, "vfstab_delete: No rows selected\n");
		return (1);
	}
	return (0);
}

void
getversion_vfstab_delete_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
