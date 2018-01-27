/*
 * $Log: valias_update.c,v $
 * Revision 2.6  2008-09-08 09:55:14+05:30  Cprogrammer
 * removed mysql_escape
 *
 * Revision 2.5  2008-05-28 16:38:31+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 2.4  2004-01-06 17:18:27+05:30  Cprogrammer
 * use real domain
 *
 * Revision 2.3  2003-07-02 18:36:32+05:30  Cprogrammer
 * logic corretion if table valias is missing
 *
 * Revision 2.2  2002-10-27 21:33:01+05:30  Cprogrammer
 * used generic function create_table()
 *
 * Revision 2.1  2002-08-05 01:03:07+05:30  Cprogrammer
 * added mysql_escape for alias, alias_line and old_alias
 *
 * Revision 1.8  2002-08-03 04:33:48+05:30  Cprogrammer
 * mysql string fix
 *
 * Revision 1.7  2002-02-23 20:24:37+05:30  Cprogrammer
 * corrected bug with ER_NO_SUCH_TABLE check
 *
 * Revision 1.6  2001-12-27 00:49:51+05:30  Cprogrammer
 * added verbose messages
 *
 * Revision 1.5  2001-12-22 18:10:04+05:30  Cprogrammer
 * create table on if mysql_errno is ER_NO_SUCH_TABLe
 *
 * Revision 1.4  2001-12-09 23:56:29+05:30  Cprogrammer
 * valias_update now checks the old valias line
 *
 * Revision 1.3  2001-12-03 01:57:37+05:30  Cprogrammer
 * changed update to low priority
 *
 * Revision 1.2  2001-12-02 01:12:17+05:30  Cprogrammer
 * changed erroneous error message from valias_insert to valias_update
 *
 * Revision 1.1  2001-12-01 23:08:41+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: valias_update.c,v 2.6 2008-09-08 09:55:14+05:30 Cprogrammer Stab mbhangui $";
#endif

#ifdef VALIAS
#include <stdlib.h>
#include <string.h>
#include <mysqld_error.h>

int
valias_update(char *alias, char *domain, char *old_alias, char *alias_line)
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
	while (*alias_line == ' ' && *alias_line != 0)
		++alias_line;
	snprintf(SqlBuf, SQL_BUF_SIZE, 
		"update low_priority valias set valias_line=\"%s\" where alias=\"%s\" and \
		domain=\"%s\" and valias_line=\"%s\"",
		alias_line, alias, real_domain, old_alias);
	if (mysql_query(&mysql[1], SqlBuf))
	{
		if(mysql_errno(&mysql[1]) == ER_NO_SUCH_TABLE)
		{
			fprintf(stderr, "No alias line %s for alias %s@%s\n",
				alias_line, alias, real_domain);
			if(create_table(ON_LOCAL, "valias", VALIAS_TABLE_LAYOUT))
				return(-1);
			return (1);
		}
		mysql_perror("valias_update: %s", SqlBuf);
		return (-1);
	}
	if((err = mysql_affected_rows(&mysql[1])) == -1)
	{
		mysql_perror("mysql_affected_rows");
		return(-1);
	}
	if(!verbose)
		return (0);
	if(err)
		printf("Updated alias line %s for alias %s@%s (%d entries)\n", alias_line, alias, real_domain, err);
	else
		fprintf(stderr, "No alias line %s for alias %s@%s\n", alias_line, alias, real_domain);
	return (0);
}
#endif

void
getversion_valias_update_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
