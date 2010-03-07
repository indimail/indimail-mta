/*
 * $Log: vfstab_status.c,v $
 * Revision 2.6  2008-05-28 17:41:24+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 2.5  2002-10-27 21:40:11+05:30  Cprogrammer
 * used generic function create_table()
 *
 * Revision 2.4  2002-10-11 20:07:21+05:30  Cprogrammer
 * fixed memory leak
 *
 * Revision 2.3  2002-08-11 11:46:19+05:30  Cprogrammer
 * return error if 0 rows are updated
 *
 * Revision 2.2  2002-08-11 00:32:13+05:30  Cprogrammer
 * added pathToFilesystem() to correctly determine the filesystem
 *
 * Revision 2.1  2002-08-07 18:43:47+05:30  Cprogrammer
 * function to alter filesystem status
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: vfstab_status.c,v 2.6 2008-05-28 17:41:24+05:30 Cprogrammer Stab mbhangui $";
#endif

#include <stdlib.h>
#include <mysqld_error.h>

int
vfstab_status(char *filesystem, char *mdahost, int status)
{
	int             err;
	char            SqlBuf[SQL_BUF_SIZE];
	char           *ptr;
	MYSQL_RES      *res;
	MYSQL_ROW       row;

#ifdef CLUSTERED_SITE
	if (open_master())
	{
		fprintf(stderr, "vfstab_status: Failed to open Master Db\n");
		return (-1);
	}
#else
	if (vauth_open(0))
	{
		fprintf(stderr, "Failed to open local Db\n");
		return (-1);
	}
#endif
	if (!(ptr = pathToFilesystem(filesystem)))
	{
		fprintf(stderr, "vfstab_status: %s: Not a filesystem\n", filesystem);
		return (-1);
	}
	if (status == -1)
	{
		snprintf(SqlBuf, SQL_BUF_SIZE, 
			"select high_priority status from fstab where filesystem = \"%s\" and host = \"%s\"", 
			ptr, mdahost);
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
				fprintf(stderr, "vfstab_status: No rows selected\n");
				return (-1);
			} else
			{
#ifdef CLUSTERED_SITE
				fprintf(stderr, "vfstab_status: %s: %s\n", SqlBuf, mysql_error(&mysql[0]));
#else
				fprintf(stderr, "vfstab_status: %s: %s\n", SqlBuf, mysql_error(&mysql[1]));
#endif
			}
		}
#ifdef CLUSTERED_SITE
		if (!(res = mysql_store_result(&mysql[0])))
#else
		if (!(res = mysql_store_result(&mysql[1])))
#endif
		{
#ifdef CLUSTERED_SITE
			fprintf(stderr, "vfstab_status: mysql_store_result: %s\n", mysql_error(&mysql[0]));
#else
			fprintf(stderr, "vfstab_status: mysql_store_result: %s\n", mysql_error(&mysql[1]));
#endif
			return (-1);
		}
		if (!(row = mysql_fetch_row(res)))
		{
			fprintf(stderr, "vfstab_status: No rows selected\n");
			mysql_free_result(res);
			return (-1);
		}
		status = (atoi(row[0]) == FS_ONLINE ? FS_OFFLINE : FS_ONLINE);
		mysql_free_result(res);
	} /*- if (status == -1) */
	snprintf(SqlBuf, SQL_BUF_SIZE, 
		"update low_priority fstab set status=%d where filesystem = \"%s\" and host = \"%s\"", 
		status, ptr, mdahost);
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
			fprintf(stderr, "vfstab_status: No rows selected\n");
			return (1);
		} else
		{
#ifdef CLUSTERED_SITE
			fprintf(stderr, "vfstab_status: %s: %s\n", SqlBuf, mysql_error(&mysql[0]));
#else
			fprintf(stderr, "vfstab_status: %s: %s\n", SqlBuf, mysql_error(&mysql[1]));
#endif
			return (-1);
		}
	}
#ifdef CLUSTERED_SITE
	if ((err = mysql_affected_rows(&mysql[0])) == -1)
	{
		fprintf(stderr, "vfstab_status: mysql_affected_rows: %s\n", mysql_error(&mysql[0]));
		return (-1);
	}
#else
	if ((err = mysql_affected_rows(&mysql[1])) == -1)
	{
		fprintf(stderr, "vfstab_status: mysql_affected_rows: %s\n", mysql_error(&mysql[1]));
		return (-1);
	}
#endif
	if (!err)
	{
		fprintf(stderr, "vfstab_status: No rows selected\n");
		return (1);
	}
	return (status);
}

void
getversion_vfstab_status_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
