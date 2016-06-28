/*
 * $Log: vdel_dir_control.c,v $
 * Revision 2.4  2009-02-18 21:35:50+05:30  Cprogrammer
 * check return value of fscanf
 *
 * Revision 2.3  2009-01-15 08:56:07+05:30  Cprogrammer
 * change for once_only flag in remove_line
 *
 * Revision 2.2  2008-05-28 15:30:14+05:30  Cprogrammer
 * removed leftover code of cdb
 *
 * Revision 2.1  2008-05-28 15:28:44+05:30  Cprogrammer
 * mysql module default
 *
 * Revision 1.9  2002-08-03 04:37:28+05:30  Cprogrammer
 * mysql string fix
 *
 * Revision 1.8  2002-03-03 17:18:12+05:30  Cprogrammer
 * replaced strncat with scat
 *
 * Revision 1.7  2001-12-14 10:35:07+05:30  Cprogrammer
 * non-existence of .filesystems not an error
 *
 * Revision 1.6  2001-12-02 20:22:54+05:30  Cprogrammer
 * added code for CDB
 *
 * Revision 1.5  2001-12-02 18:50:28+05:30  Cprogrammer
 * code to remove entries from dir_control after reading .filesystems
 *
 * Revision 1.4  2001-11-24 12:21:41+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.3  2001-11-20 11:00:11+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.2  2001-11-14 19:28:33+05:30  Cprogrammer
 * distributed arch change
 *
 * Revision 1.1  2001-10-24 18:15:32+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: vdel_dir_control.c,v 2.4 2009-02-18 21:35:50+05:30 Cprogrammer Stab mbhangui $";
#endif

int
vdel_dir_control(char *domain)
{
	int             err;
	char           *ptr;
	char            SqlBuf[SQL_BUF_SIZE], tmpbuf[MAX_BUFF], buffer[MAX_BUFF];
	FILE           *fp;

	if ((err = vauth_open((char *) 0)) != 0)
		return (err);
	if(!(ptr = vget_assign(domain, NULL, 0, 0, 0)))
	{
		fprintf(stderr, "%s: No such domain\n", domain);
		return(-1);
	}
	snprintf(tmpbuf, MAX_BUFF, "%s/.filesystems", ptr);
	if(!(fp = fopen(tmpbuf, "r")))
		return(0);
	for(;;)
	{
		if (fscanf(fp, "%s", buffer) != 1)
		{
			if(ferror(fp))
				break;
		}
		if(feof(fp))
			break;
		snprintf(SqlBuf, SQL_BUF_SIZE, "delete low_priority from dir_control%s where domain = \"%s\"", 
			buffer, domain);
		if (mysql_query(&mysql[1], SqlBuf))
		{
			mysql_perror("vdel_dir_control: dir_control%s: %s", ptr, SqlBuf);
			continue;
		}
		remove_line(buffer, tmpbuf, 0, 0640);
	}
	fclose(fp);
	return (0);
}

void
getversion_vdel_dir_control_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
