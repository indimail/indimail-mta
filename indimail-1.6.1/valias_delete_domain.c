/*
 * $Log: valias_delete_domain.c,v $
 * Revision 2.4  2008-05-28 16:38:19+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 2.3  2004-01-06 17:18:07+05:30  Cprogrammer
 * use real domain
 *
 * Revision 2.2  2003-07-02 18:30:31+05:30  Cprogrammer
 * return success if table not present
 *
 * Revision 2.1  2002-10-27 21:31:33+05:30  Cprogrammer
 * used generic function create_table()
 *
 * Revision 1.8  2002-02-23 20:24:20+05:30  Cprogrammer
 * corrected bug with ER_NO_SUCH_TABLE check
 *
 * Revision 1.7  2001-12-21 02:22:27+05:30  Cprogrammer
 * create table if errno is ER_NO_SUCH_TABLE
 *
 * Revision 1.6  2001-12-02 18:48:28+05:30  Cprogrammer
 * create table when valias not existing
 *
 * Revision 1.5  2001-11-30 00:33:12+05:30  Cprogrammer
 * removed '\n'
 *
 * Revision 1.4  2001-11-24 12:20:33+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.3  2001-11-20 10:56:28+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.2  2001-11-14 19:25:46+05:30  Cprogrammer
 * distributed arch change
 *
 * Revision 1.1  2001-10-24 18:15:19+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: valias_delete_domain.c,v 2.4 2008-05-28 16:38:19+05:30 Cprogrammer Stab mbhangui $";
#endif

#ifdef VALIAS
#include <mysqld_error.h>
int
valias_delete_domain(char *domain)
{
	int             err;
	char           *real_domain;
	char            SqlBuf[SQL_BUF_SIZE];

	if(!domain || !*domain)
		return(1);
	if ((err = vauth_open((char *) 0)) != 0)
		return (err);
	if(!(real_domain = vget_real_domain(domain)))
		real_domain = domain;
	snprintf(SqlBuf, SQL_BUF_SIZE, "delete low_priority from valias where domain = \"%s\"",
		real_domain);
	if (mysql_query(&mysql[1], SqlBuf))
	{
		if(mysql_errno(&mysql[1]) == ER_NO_SUCH_TABLE)
		{
			if(create_table(ON_LOCAL, "valias", VALIAS_TABLE_LAYOUT))
				return(-1);
			return(0);
		}
		mysql_perror("valias_delete_domain: %s", SqlBuf);
		return (-1);
	}
	return (0);
}
#endif

void
getversion_valias_delete_domain_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
