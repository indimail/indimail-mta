/*
 * $Log: vfstab_update.c,v $
 * Revision 2.5  2008-05-28 17:41:27+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 2.4  2003-03-05 00:20:06+05:30  Cprogrammer
 * added status updation fix
 *
 * Revision 2.3  2002-10-27 21:40:34+05:30  Cprogrammer
 * used generic function create_table()
 *
 * Revision 2.2  2002-08-11 00:32:25+05:30  Cprogrammer
 * added pathToFilesystem() to correctly determine the filesystem
 * insert record if update fails
 *
 * Revision 2.1  2002-08-07 18:44:09+05:30  Cprogrammer
 * function to update fstab entries
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: vfstab_update.c,v 2.5 2008-05-28 17:41:27+05:30 Cprogrammer Stab mbhangui $";
#endif

#include <mysqld_error.h>

int
vfstab_update(char *filesystem, char *mdahost, long user_quota, long size_quota, int status)
{
	int             err;
	char            SqlBuf[SQL_BUF_SIZE], statusbuf[28];
	char           *ptr;

#ifdef CLUSTERED_SITE
	if (open_master())
	{
		fprintf(stderr, "vfstab_update: Failed to open Master Db\n");
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
		fprintf(stderr, "vfstab_update: %s: Not a filesystem\n", filesystem);
		return (-1);
	}
	if (status == 0 || status == 1)
		snprintf(statusbuf, sizeof(statusbuf), ",status=%d", status);
	else
	{
		*statusbuf = ' ';
		*(statusbuf + 1) = 0;
	}
	if (user_quota > 0 && size_quota > 0)
	{
		snprintf(SqlBuf, SQL_BUF_SIZE, 
			"update low_priority fstab set max_users=%ld, max_size=%ld %s where filesystem = \"%s\" and host = \"%s\"", 
			user_quota, size_quota, statusbuf, ptr, mdahost);
	} else
	if (user_quota > 0)
	{
		snprintf(SqlBuf, SQL_BUF_SIZE, 
			"update low_priority fstab set max_users=%ld %s where filesystem = \"%s\" and host = \"%s\"", 
			user_quota, statusbuf, ptr, mdahost);
	} else
	if (size_quota > 0)
	{
		snprintf(SqlBuf, SQL_BUF_SIZE, 
			"update low_priority fstab set max_size=%ld %s where filesystem = \"%s\" and host = \"%s\"", 
			size_quota, statusbuf, ptr, mdahost);
	}
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
			fprintf(stderr, "vfstab_update: No rows selected\n");
			return (-1);
		} else
		{
#ifdef CLUSTERED_SITE
			fprintf(stderr, "vfstab_update: %s: %s\n", SqlBuf, mysql_error(&mysql[0]));
#else
			fprintf(stderr, "vfstab_update: %s: %s\n", SqlBuf, mysql_error(&mysql[1]));
#endif
			return (-1);
		}
	}
#ifdef CLUSTERED_SITE
	if ((err = mysql_affected_rows(&mysql[0])) == -1)
	{
		fprintf(stderr, "vfstab_update: mysql_affected_rows: %s\n", mysql_error(&mysql[0]));
		return (-1);
	}
#else
	if ((err = mysql_affected_rows(&mysql[1])) == -1)
	{
		fprintf(stderr, "vfstab_update: mysql_affected_rows: %s\n", mysql_error(&mysql[1]));
		return (-1);
	}
#endif
	/*-
	if (!err)
		return (vfstab_insert(ptr, mdahost, FS_ONLINE, user_quota, size_quota));
	*/
	return (0);
}

void
getversion_vfstab_update_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
