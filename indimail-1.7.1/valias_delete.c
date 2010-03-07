/*
 * $Log: valias_delete.c,v $
 * Revision 2.13  2010-03-07 09:28:23+05:30  Cprogrammer
 * check return value of is_distributed_domain for error
 *
 * Revision 2.12  2008-09-08 09:54:44+05:30  Cprogrammer
 * removed mysql_escape
 *
 * Revision 2.11  2008-05-28 15:42:52+05:30  Cprogrammer
 * removed leftover cdb code
 *
 * Revision 2.10  2008-05-28 15:20:51+05:30  Cprogrammer
 * mysyql module default
 *
 * Revision 2.9  2004-05-19 20:02:37+05:30  Cprogrammer
 * new logic for replacing '.' with ':'
 *
 * Revision 2.8  2004-01-06 17:18:01+05:30  Cprogrammer
 * use real domain
 *
 * Revision 2.7  2002-12-31 20:57:53+05:30  Cprogrammer
 * use is_distributed_domain() before deleting from hostcntrl
 *
 * Revision 2.6  2002-10-27 21:31:05+05:30  Cprogrammer
 * used generic function create_table()
 *
 * Revision 2.5  2002-10-26 21:12:21+05:30  Cprogrammer
 * removed unecessary mysql_query
 *
 * Revision 2.4  2002-10-12 23:03:42+05:30  Cprogrammer
 * condition compilation of clustered code
 *
 * Revision 2.3  2002-08-11 16:53:12+05:30  Cprogrammer
 * display "no alias line" only if verbose is set
 *
 * Revision 2.2  2002-08-11 00:29:36+05:30  Cprogrammer
 * prevented printing alias_line if null
 *
 * Revision 2.1  2002-08-05 00:58:32+05:30  Cprogrammer
 * added mysql_escape for alias
 *
 * Revision 1.12  2002-08-03 04:33:12+05:30  Cprogrammer
 * mysql string fix
 *
 * Revision 1.11  2002-02-23 20:24:17+05:30  Cprogrammer
 * corrected bug with ER_NO_SUCH_TABLE check
 *
 * Revision 1.10  2001-12-27 00:49:36+05:30  Cprogrammer
 * added verbose messages
 *
 * Revision 1.9  2001-12-22 18:07:38+05:30  Cprogrammer
 * changed error string for open_master failure
 *
 * Revision 1.8  2001-12-21 02:22:08+05:30  Cprogrammer
 * create table when mysql_errno is ER_NO_SUCH_TABLE
 *
 * Revision 1.7  2001-12-19 16:29:46+05:30  Cprogrammer
 * code to delete valias from hostcntrl
 *
 * Revision 1.6  2001-12-02 18:48:02+05:30  Cprogrammer
 * print on stdout only when verbose is set
 *
 * Revision 1.5  2001-11-29 13:29:26+05:30  Cprogrammer
 * removed unecessary snprintf calls
 *
 * Revision 1.4  2001-11-24 12:20:32+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.3  2001-11-20 10:56:27+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.2  2001-11-14 19:25:39+05:30  Cprogrammer
 * distributed arch change
 *
 * Revision 1.1  2001-10-24 18:15:19+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: valias_delete.c,v 2.13 2010-03-07 09:28:23+05:30 Cprogrammer Stab mbhangui $";
#endif

#ifdef VALIAS
#include <stdlib.h>
#include <string.h>
#include <mysqld_error.h>

int
valias_delete(char *alias, char *domain, char *alias_line)
{
	int             err;
	char            SqlBuf[SQL_BUF_SIZE];
	char           *real_domain;

	if (!domain || !*domain)
		return (1);
	if (!(real_domain = vget_real_domain(domain)))
		real_domain = domain;
#ifdef CLUSTERED_SITE
	if ((err = is_distributed_domain(real_domain)) == -1)
	{
		fprintf(stderr, "%s: is_distributed_domain failed\n", real_domain);
		return (1);
	} else
	if (err)
	{
		if (open_master())
		{
			fprintf(stderr, "valias_delete: Failed to open Master Db\n");
			return (1);
		}
		snprintf(SqlBuf, SQL_BUF_SIZE, 
			"delete low_priority from %s where pw_name=\"%s\" and pw_domain=\"%s\" and pw_passwd=\"alias\"", 
			cntrl_table, alias, real_domain);
		if (mysql_query(&mysql[0], SqlBuf))
		{
			if (mysql_errno(&mysql[0]) == ER_NO_SUCH_TABLE)
				create_table(ON_MASTER, "hostcntrl", CNTRL_TABLE_LAYOUT);
			else
			{
				fprintf(stderr, "valias_delete: mysql error: %s\n", mysql_error(&mysql[0]));
				return (1);
			}
		}
	}
#endif
	if ((err = vauth_open((char *) 0)) != 0)
		return (err);
	if (alias_line && *alias_line)
		snprintf(SqlBuf, SQL_BUF_SIZE, 
			"delete low_priority from valias where alias = \"%s\" and domain = \"%s\" and valias_line=\"%s\"", 
			alias, real_domain, alias_line);
	else
		snprintf(SqlBuf, SQL_BUF_SIZE, 
			"delete low_priority from valias where alias = \"%s\" and domain = \"%s\"", 
			alias, real_domain);
	if (mysql_query(&mysql[1], SqlBuf))
	{
		if (mysql_errno(&mysql[1]) == ER_NO_SUCH_TABLE)
		{
			if (create_table(ON_LOCAL, "valias", VALIAS_TABLE_LAYOUT))
				return (-1);
			if (verbose)
				printf("No alias line %s for alias %s@%s\n", alias_line ? alias_line : " ", alias, real_domain);
			return (0);
		} else
		{
			mysql_perror("valias_delete: %s", SqlBuf);
			return (-1);
		}
	}
	if ((err = mysql_affected_rows(&mysql[1])) == -1)
	{
		mysql_perror("mysql_affected_rows");
		return (-1);
	}
	if (!verbose)
		return (0);
	if (err && verbose)
		printf("Deleted alias line %s for alias %s@%s (%d entries)\n", alias_line, alias, real_domain, err);
	else
	if (verbose)
		printf("No alias line %s for alias %s@%s\n", alias_line ? alias_line : " ", alias, real_domain);
	return (0);
}
#endif /*- #ifdef VALIAS */

void
getversion_valias_delete_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
