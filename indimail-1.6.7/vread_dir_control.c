/*
 * $Log: vread_dir_control.c,v $
 * Revision 2.2  2008-05-28 17:42:18+05:30  Cprogrammer
 * removed USE_MYSQL, removed cdb code
 *
 * Revision 2.1  2003-07-02 18:40:04+05:30  Cprogrammer
 * return initialized structure if dir_control table is missing
 *
 * Revision 1.9  2002-08-03 04:39:30+05:30  Cprogrammer
 * mysql string fix
 *
 * Revision 1.8  2002-02-23 20:26:49+05:30  Cprogrammer
 * corrected bug with ER_NO_SUCH_TABLE check
 *
 * Revision 1.7  2001-12-22 18:18:22+05:30  Cprogrammer
 * create table only if mysql_errno is ER_NO_SUCH_TABLE
 *
 * Revision 1.6  2001-12-03 01:34:34+05:30  Cprogrammer
 * code beautification
 *
 * Revision 1.5  2001-12-02 18:52:16+05:30  Cprogrammer
 * code changes to incorporate filesystem specific storage
 *
 * Revision 1.4  2001-11-24 12:22:24+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.3  2001-11-20 11:02:11+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.2  2001-11-14 19:29:30+05:30  Cprogrammer
 * distributed arch change
 *
 * Revision 1.1  2001-10-24 18:15:44+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"
#include <string.h>
#include <stdlib.h>

#ifndef	lint
static char     sccsid[] = "$Id: vread_dir_control.c,v 2.2 2008-05-28 17:42:18+05:30 Cprogrammer Stab mbhangui $";
#endif

#include <mysqld_error.h>
int
vread_dir_control(char *filename, vdir_type *vdir, char *domain)
{
	int             found = 0;
	char            SqlBuf[SQL_BUF_SIZE];
	MYSQL_RES      *res;
	MYSQL_ROW       row;

	if (vauth_open((char *) 0) != 0)
		return (-1);
	snprintf(SqlBuf, SQL_BUF_SIZE, "select high_priority %s from dir_control%s where domain = \"%s\"", 
		DIR_CONTROL_SELECT, filename, domain);
	if (mysql_query(&mysql[1], SqlBuf))
	{
		if(mysql_errno(&mysql[1]) == ER_NO_SUCH_TABLE)
		{
			if(vcreate_dir_control(filename, domain))
				return(-1);
			init_dir_control(vdir);
			return (0);
		} else
		{
			mysql_perror("vread_dir_control: %s", SqlBuf);
			return (-1);
		}
	}
	if (!(res = mysql_store_result(&mysql[1])))
	{
		mysql_perror("vread_dir_control: mysql_store_result");
		return (1);
	}
	if ((row = mysql_fetch_row(res)))
	{
		found = 1;
		vdir->cur_users = atol(row[0]);
		vdir->level_cur = atoi(row[1]);
		vdir->level_max = atoi(row[2]);

		vdir->level_start[0] = atoi(row[3]);
		vdir->level_start[1] = atoi(row[4]);
		vdir->level_start[2] = atoi(row[5]);

		vdir->level_end[0] = atoi(row[6]);
		vdir->level_end[1] = atoi(row[7]);
		vdir->level_end[2] = atoi(row[8]);

		vdir->level_mod[0] = atoi(row[9]);
		vdir->level_mod[1] = atoi(row[10]);
		vdir->level_mod[2] = atoi(row[11]);
		vdir->level_index[0] = atoi(row[12]);
		vdir->level_index[1] = atoi(row[13]);
		vdir->level_index[2] = atoi(row[14]);
		scopy(vdir->the_dir, row[15], MAX_DIR_NAME);
	}
	mysql_free_result(res);
	if (found == 0)
		init_dir_control(vdir);
	return (0);
}

void
getversion_vread_dir_control_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
