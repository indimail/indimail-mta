/*
 * $Log: vwrite_dir_control.c,v $
 * Revision 2.4  2009-02-18 21:36:29+05:30  Cprogrammer
 * check chown for error
 *
 * Revision 2.3  2008-05-28 15:38:13+05:30  Cprogrammer
 * removed cdb code
 *
 * Revision 2.2  2004-06-25 09:02:50+05:30  Cprogrammer
 * corrected incorrect initialization of ptr
 *
 * Revision 2.1  2004-06-24 20:05:37+05:30  Cprogrammer
 * fix for condition when domain is null
 *
 * Revision 1.11  2002-08-03 04:40:42+05:30  Cprogrammer
 * mysql string fix
 *
 * Revision 1.10  2002-02-23 20:27:47+05:30  Cprogrammer
 * corrected bug with ER_NO_SUCH_TABLE check
 *
 * Revision 1.9  2001-12-22 18:19:42+05:30  Cprogrammer
 * create table on if mysql_errno is ER_NO_SUCH_TABLE
 *
 * Revision 1.8  2001-12-03 01:39:40+05:30  Cprogrammer
 * printing of error message if chown fails
 *
 * Revision 1.7  2001-12-03 01:33:55+05:30  Cprogrammer
 * code beautification
 *
 * Revision 1.6  2001-12-03 00:09:38+05:30  Cprogrammer
 * code to change owner to indimail for the .filesystem file
 *
 * Revision 1.5  2001-12-02 18:53:03+05:30  Cprogrammer
 * code for adding filesystems to .filesystems
 *
 * Revision 1.4  2001-11-24 12:22:46+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.3  2001-11-20 11:02:25+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.2  2001-11-14 19:30:05+05:30  Cprogrammer
 * distributed arch change
 *
 * Revision 1.1  2001-10-24 18:15:48+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <mysqld_error.h>

#ifndef	lint
static char     sccsid[] = "$Id: vwrite_dir_control.c,v 2.4 2009-02-18 21:36:29+05:30 Cprogrammer Stab mbhangui $";
#endif

int
vwrite_dir_control(char *table_name, vdir_type * vdir, char *domain, uid_t dummy1, gid_t dummy2)
{
	char            SqlBuf[SQL_BUF_SIZE], tmpbuf[MAX_BUFF];
	uid_t           uid = 0;
	gid_t           gid = 0;
	char           *ptr = (char *) 0;

	if (domain && *domain && !(ptr = vget_assign(domain, NULL, 0, &uid, &gid)))
	{
		fprintf(stderr, "%s: No such domain\n", domain);
		return(-1);
	}
	if (vauth_open((char *) 0) != 0)
		return (-1);
	snprintf(SqlBuf, SQL_BUF_SIZE, "replace low_priority into dir_control%s ( \
	domain, cur_users, \
	level_cur, level_max, \
	level_start0, level_start1, level_start2, \
	level_end0, level_end1, level_end2, \
	level_mod0, level_mod1, level_mod2, \
	level_index0, level_index1, level_index2, the_dir ) \
	values (\"%s\", %ld, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, \"%s\")", 
	table_name, 
	domain, vdir->cur_users, 
	vdir->level_cur, vdir->level_max, 
	vdir->level_start[0], vdir->level_start[1], vdir->level_start[2], 
	vdir->level_end[0], vdir->level_end[1], vdir->level_end[2], 
	vdir->level_mod[0], vdir->level_mod[1], vdir->level_mod[2], 
	vdir->level_index[0], vdir->level_index[1], vdir->level_index[2], vdir->the_dir);
	if (mysql_query(&mysql[1], SqlBuf))
	{
		if (mysql_errno(&mysql[1]) == ER_NO_SUCH_TABLE)
		{
			vcreate_dir_control(table_name, domain);
			if (mysql_query(&mysql[1], SqlBuf))
			{
				mysql_perror("vwrite_dir_control: mysql_query: %s", SqlBuf);
				return (-1);
			}
		} else
		{
			mysql_perror("vwrite_dir_control: mysql_query: %s", SqlBuf);
			return (-1);
		}
	}
	if (ptr)
	{
		snprintf(tmpbuf, MAX_BUFF, "%s/.filesystems", ptr);
		if (update_file(tmpbuf, table_name, 0640))
		{
			perror(tmpbuf);
			if (chown(tmpbuf, uid, gid) == -1);
			return(-1);
		}
		if (chown(tmpbuf, uid, gid))
		{
			fprintf(stderr, "chown: %s: %s\n", tmpbuf, strerror(errno));
			return(0);
		}
	}
	return (0);
}

void
getversion_vwrite_dir_control_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
