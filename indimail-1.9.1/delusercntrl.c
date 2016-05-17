/*
 * $Log: delusercntrl.c,v $
 * Revision 2.7  2016-05-17 15:38:40+05:30  Cprogrammer
 * fixed comments
 *
 * Revision 2.6  2009-02-06 11:37:14+05:30  Cprogrammer
 * fixed potential buffer overflow
 *
 * Revision 2.5  2008-11-07 17:05:17+05:30  Cprogrammer
 * do not treat no rows deleted as an error
 *
 * Revision 2.4  2008-09-08 09:34:20+05:30  Cprogrammer
 * removed mysql_escape
 *
 * Revision 2.3  2008-05-28 16:35:09+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 2.2  2004-05-17 00:47:35+05:30  Cprogrammer
 * added force flag to bypass distributed domain check
 *
 * Revision 2.1  2002-08-05 00:13:16+05:30  Cprogrammer
 * added mysql_escape for user
 *
 * Revision 1.7  2002-08-03 04:26:54+05:30  Cprogrammer
 * mysql string fix
 *
 * Revision 1.6  2001-12-22 18:06:50+05:30  Cprogrammer
 * changed error string for open_master failure
 *
 * Revision 1.5  2001-12-19 16:27:58+05:30  Cprogrammer
 * replaced mysql[1] with mysql[0]
 *
 * Revision 1.4  2001-12-11 11:31:00+05:30  Cprogrammer
 * open connection to master for updates
 *
 * Revision 1.3  2001-11-30 00:12:28+05:30  Cprogrammer
 * used variable cntrl_table from hostcntrl table
 *
 * Revision 1.2  2001-11-29 00:31:27+05:30  Cprogrammer
 * replaced snprintf with fprintf for error display on stdout
 *
 * Revision 1.1  2001-11-28 22:51:34+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: delusercntrl.c,v 2.7 2016-05-17 15:38:40+05:30 Cprogrammer Exp mbhangui $";
#endif

#ifdef CLUSTERED_SITE
#include <stdlib.h>
#include <string.h>
#include <errno.h>

/*
 * 1 - Mysql Error (or) Assignment Error
 * 0 - Success
 */

int
delusercntrl(char *user, char *domain, int force)
{
	char            SqlBuf[SQL_BUF_SIZE];
	int             err;

	if (!user || !*user || !domain || !*domain)
		return (1);
	/*
	 *  Check if Domain is distributed or not , by checking hostcntrl table
	 */
	if (!force)
	{
		if((err = is_distributed_domain(domain)) == -1)
			return(1);
		if(!err)
			return(0);
	}
	if (open_master())
	{
		fprintf(stderr, "delusercntrl: Failed to open Master Db\n");
		return (1);
	}
	snprintf(SqlBuf, SQL_BUF_SIZE, 
		"delete low_priority from %s where pw_name=\"%s\" and pw_domain=\"%s\"", cntrl_table, user, domain);
	if (mysql_query(&mysql[0], SqlBuf))
	{
		fprintf(stderr, "delusercntrl: mysql error: %s\n", mysql_error(&mysql[0]));
		return(1);
	}
	err = mysql_affected_rows(&mysql[0]);
	return(err == -1 ? 1 : 0);
}
#endif
void
getversion_delusercntrl_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
