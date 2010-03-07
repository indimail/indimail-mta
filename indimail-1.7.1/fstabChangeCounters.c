/*
 * $Log: fstabChangeCounters.c,v $
 * Revision 2.5  2008-05-28 16:35:28+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 2.4  2002-10-27 21:15:04+05:30  Cprogrammer
 * used generic function create_table()
 *
 * Revision 2.3  2002-08-11 01:01:34+05:30  Cprogrammer
 * removed low priority updates
 *
 * Revision 2.2  2002-08-11 00:26:17+05:30  Cprogrammer
 * added pathToFilesystem() to correctly determine the filesystem
 *
 * Revision 2.1  2002-08-07 19:30:38+05:30  Cprogrammer
 * function to change cur_users and cur_size
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: fstabChangeCounters.c,v 2.5 2008-05-28 16:35:28+05:30 Cprogrammer Stab mbhangui $";
#endif

#include <errno.h>
#include <string.h>
#include <mysqld_error.h>

int
fstabChangeCounter(char *filesystem, char *mdahost, long user_count, long size_count)
{
	int             err;
	char           *ptr;
	char            SqlBuf[SQL_BUF_SIZE];

#ifdef CLUSTERED_SITE
	if (open_master())
	{
		fprintf(stderr, "fstabChangeCounter: Failed to open Master Db\n");
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
		fprintf(stderr, "vauth_active: pathToFilesystem: %s: %s\n", filesystem, strerror(errno));
		return (-1);
	}
	if (user_count && size_count)
	{
		snprintf(SqlBuf, SQL_BUF_SIZE, 
			"update fstab set cur_users=cur_users+%ld, cur_size=cur_size+%ld where filesystem = \"%s\" \
			and host = \"%s\"", user_count, size_count, ptr, mdahost);
	} else
	if (user_count)
	{
		snprintf(SqlBuf, SQL_BUF_SIZE, 
			"update fstab set cur_users=cur_users+%ld where filesystem = \"%s\" and host = \"%s\"", 
			user_count, ptr, mdahost);
	} else
	if (size_count)
	{
		snprintf(SqlBuf, SQL_BUF_SIZE, 
			"update fstab set cur_size=cur_size+%ld where filesystem = \"%s\" and host = \"%s\"", 
			size_count, ptr, mdahost);
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
			fprintf(stderr, "fstabChangeCounter: No rows selected\n");
			return (-1);
		} else
		{
#ifdef CLUSTERED_SITE
			fprintf(stderr, "fstabChangeCounter: %s: %s\n", SqlBuf, mysql_error(&mysql[0]));
#else
			fprintf(stderr, "fstabchangeCounter: %s: %s\n", SqlBuf, mysql_error(&mysql[1]));
#endif
			return (-1);
		}
	}
#ifdef CLUSTERED_SITE
	if ((err = mysql_affected_rows(&mysql[0])) == -1)
	{
		fprintf(stderr, "fstabChangeCounter: mysql_affected_rows: %s\n", mysql_error(&mysql[0]));
		return (-1);
	}
#else
	if ((err = mysql_affected_rows(&mysql[1])) == -1)
	{
		fprintf(stderr, "fstabChangeCounter: mysql_affected_rows: %s\n", mysql_error(&mysql[1]));
		return (-1);
	}
#endif
	return (0);
}

void
getversion_fstabChangeCounters_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
