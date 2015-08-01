/*
 * $Log: vcreate_dir_control.c,v $
 * Revision 2.6  2009-02-18 21:35:44+05:30  Cprogrammer
 * check chown for error
 *
 * Revision 2.5  2009-02-06 11:40:59+05:30  Cprogrammer
 * ignore return value of chown
 *
 * Revision 2.4  2008-09-08 09:56:53+05:30  Cprogrammer
 * use integer columns as integers
 *
 * Revision 2.3  2008-05-28 17:40:25+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 2.2  2004-06-24 20:04:31+05:30  Cprogrammer
 * fix for adding users to local domains
 *
 * Revision 2.1  2002-10-27 21:36:07+05:30  Cprogrammer
 * used generic function create_table()
 *
 * Revision 1.10  2002-08-03 04:37:23+05:30  Cprogrammer
 * mysql string fix
 *
 * Revision 1.9  2001-12-22 18:16:22+05:30  Cprogrammer
 * create table if it is non existent
 *
 * Revision 1.8  2001-12-03 01:39:12+05:30  Cprogrammer
 * code beautification and printing of error message if chown fails
 *
 * Revision 1.7  2001-12-03 00:12:35+05:30  Cprogrammer
 * included unistd.h
 *
 * Revision 1.6  2001-12-03 00:11:45+05:30  Cprogrammer
 * code for changing owner of .filesystem to indimail
 *
 * Revision 1.5  2001-12-02 18:50:09+05:30  Cprogrammer
 * added code to update .filesystems file
 *
 * Revision 1.4  2001-11-24 12:21:30+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.3  2001-11-20 11:00:01+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.2  2001-11-14 19:27:53+05:30  Cprogrammer
 * distributed arch change
 *
 * Revision 1.1  2001-10-24 18:15:29+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: vcreate_dir_control.c,v 2.6 2009-02-18 21:35:44+05:30 Cprogrammer Stab mbhangui $";
#endif

#include <string.h>
#include <unistd.h>
#include <errno.h>

int
vcreate_dir_control(char *filename, char *domain)
{
	char            SqlBuf[SQL_BUF_SIZE], tmpbuf[MAX_BUFF];
	uid_t           uid;
	gid_t           gid;
	char           *ptr = (char *) 0;

	if (domain && *domain && !(ptr = vget_assign(domain, NULL, 0, &uid, &gid)))
	{
		fprintf(stderr, "%s: No such domain\n", domain);
		return(-1);
	}
	if (vauth_open((char *) 0) != 0)
		return(1);
	snprintf(tmpbuf, sizeof(tmpbuf), "dir_control%s", filename);
	if(create_table(ON_LOCAL, tmpbuf, DIR_CONTROL_TABLE_LAYOUT))
		return(1);
	snprintf(SqlBuf, SQL_BUF_SIZE, "replace low_priority into dir_control%s (\
	domain, cur_users, \
	level_cur, level_max, \
	level_start0, level_start1, level_start2, \
	level_end0, level_end1, level_end2, \
	level_mod0, level_mod1, level_mod2, \
	level_index0, level_index1, level_index2, the_dir ) \
	values (\"%s\", 0, 0, %d, 0, 0, 0, %d, %d, %d, 0, 2, 4, 0, 0, 0, \"\")", 
	filename, domain, MAX_DIR_LEVELS, MAX_DIR_LIST - 1, MAX_DIR_LIST - 1, MAX_DIR_LIST - 1);
	if (mysql_query(&mysql[1], SqlBuf))
	{
		mysql_perror("vcreate_dir_control: %s", SqlBuf);
		return(1);
	}
	if (ptr)
	{
		snprintf(tmpbuf, MAX_BUFF, "%s/.filesystems", ptr);
		if (update_file(tmpbuf, filename, 0640))
		{
			if (!getuid() || !geteuid())
				if (chown(tmpbuf, uid, gid) == -1)
				{
					fprintf(stderr, "chown: %s: %s\n", tmpbuf, strerror(errno));
					return(-1);
				}
			return(-1);
		}
		if (!getuid() || !geteuid())
			if (chown(tmpbuf, uid, gid))
			{
				fprintf(stderr, "chown: %s: %s\n", tmpbuf, strerror(errno));
				return(0);
			}
	}
	return(0);
}

void
getversion_vcreate_dir_control_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
