/*
 * $Log: vfstab_insert.c,v $
 * Revision 2.4  2008-05-28 17:41:15+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 2.3  2002-10-27 21:39:18+05:30  Cprogrammer
 * used generic function create_table()
 *
 * Revision 2.2  2002-08-11 00:32:02+05:30  Cprogrammer
 * added pathToFilesystem() to correctly determine the filesystem
 *
 * Revision 2.1  2002-08-07 18:43:02+05:30  Cprogrammer
 * function to insert fstab entries
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: vfstab_insert.c,v 2.4 2008-05-28 17:41:15+05:30 Cprogrammer Stab mbhangui $";
#endif

#include <mysqld_error.h>

int
vfstab_insert(char *filesystem, char *host, int status, long max_users, long max_size)
{
	char            SqlBuf[SQL_BUF_SIZE];
	char           *ptr;

#ifdef CLUSTERED_SITE
	if (open_master())
	{
		fprintf(stderr, "vfstab_insert: Failed to open Master Db\n");
		return (-1);
	}
#else
	if (vauth_open(0))
	{
		fprintf(stderr, "Failed to open local Db\n");
		return (-1);
	}
#endif
	if(!(ptr = pathToFilesystem(filesystem)))
	{
		fprintf(stderr, "vfstab_insert: %s: Not a filesystem\n", filesystem);
		return(-1);
	}
	snprintf(SqlBuf, SQL_BUF_SIZE, 
		"insert low_priority into fstab (filesystem, host, status, max_users, max_size) \
		values (\"%s\", \"%s\", %d, %ld, %ld)", 
		ptr, host, status, max_users, max_size);
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
				return(-1);
#ifdef CLUSTERED_SITE
			if (!mysql_query(&mysql[0], SqlBuf))
#else
			if (!mysql_query(&mysql[1], SqlBuf))
#endif
				return(0);
		}
#ifdef CLUSTERED_SITE
		fprintf(stderr, "vfstab_insert: %s: %s\n", SqlBuf, mysql_error(&mysql[0]));
#else
		fprintf(stderr, "vfstab_insert: %s: %s\n", SqlBuf, mysql_error(&mysql[1]));
#endif
		return (-1);
	}
	return (0);
}

void
getversion_vfstab_insert_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
