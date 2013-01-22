/*
 * $Log: valias_insert.c,v $
 * Revision 2.19  2010-03-07 09:28:26+05:30  Cprogrammer
 * check return value of is_distributed_domain for error
 *
 * Revision 2.18  2010-02-16 09:28:38+05:30  Cprogrammer
 * return error when insert fails
 *
 * Revision 2.17  2009-11-25 12:54:04+05:30  Cprogrammer
 * do not allow empty alias_line
 *
 * Revision 2.16  2009-10-17 20:18:40+05:30  Cprogrammer
 * added missing argument real_domain
 *
 * Revision 2.15  2009-09-15 12:09:20+05:30  Cprogrammer
 * report the address being added instead of the alias
 *
 * Revision 2.14  2009-09-13 12:47:13+05:30  Cprogrammer
 * added option to ignore requirement of destination email address to be local
 *
 * Revision 2.13  2008-12-16 18:36:32+05:30  Cprogrammer
 * BUG - alias entries were not getting added on hostcntrl for distributed domains
 *
 * Revision 2.12  2008-09-08 09:54:52+05:30  Cprogrammer
 * removed mysql_escape
 *
 * Revision 2.11  2008-05-28 15:43:31+05:30  Cprogrammer
 * removed leftover cdb code
 *
 * Revision 2.10  2008-05-28 15:21:25+05:30  Cprogrammer
 * mysql module default
 *
 * Revision 2.9  2004-05-19 20:06:01+05:30  Cprogrammer
 * new logic for replacing '.' with ':'
 *
 * Revision 2.8  2004-05-17 01:08:19+05:30  Cprogrammer
 * force flag argument added to addusercntrl()
 *
 * Revision 2.7  2004-05-17 00:50:34+05:30  Cprogrammer
 * added hostid argument to addusercntrl()
 *
 * Revision 2.6  2004-01-06 17:18:11+05:30  Cprogrammer
 * use real domain
 *
 * Revision 2.5  2002-12-31 20:57:26+05:30  Cprogrammer
 * use is_distributed_domain() before inserting into hostcntrl
 *
 * Revision 2.4  2002-10-27 21:31:55+05:30  Cprogrammer
 * used generic function create_table()
 *
 * Revision 2.3  2002-08-05 01:00:31+05:30  Cprogrammer
 * added mysql_escape for alias and alias_line
 *
 * Revision 2.2  2002-08-03 04:33:20+05:30  Cprogrammer
 * mysql string fix
 *
 * Revision 2.1  2002-05-05 22:19:45+05:30  Cprogrammer
 * use islocalif () to determine if mailstore is local
 *
 * Revision 1.12  2002-03-29 22:09:43+05:30  Cprogrammer
 * compare hostid instead of ip address
 *
 * Revision 1.11  2002-02-23 20:24:24+05:30  Cprogrammer
 * corrected bug with ER_NO_SUCH_TABLE check
 *
 * Revision 1.10  2002-01-09 23:09:17+05:30  Cprogrammer
 * replace '.' with ':' when creating .qmail files
 *
 * Revision 1.9  2001-12-27 00:49:44+05:30  Cprogrammer
 * added verbose messages
 *
 * Revision 1.8  2001-12-21 02:22:38+05:30  Cprogrammer
 * create table when errno is ER_NO_SUCH_TABLE
 *
 * Revision 1.7  2001-12-19 16:29:55+05:30  Cprogrammer
 * code to add the alias to hostcntrl
 *
 * Revision 1.6  2001-12-03 01:57:19+05:30  Cprogrammer
 * changed insert to low_priority
 *
 * Revision 1.5  2001-12-02 01:11:52+05:30  Cprogrammer
 * removed unecessary snprintf
 *
 * Revision 1.4  2001-11-24 12:20:35+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.3  2001-11-20 10:56:29+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.2  2001-11-14 19:25:52+05:30  Cprogrammer
 * distributed arch change
 *
 * Revision 1.1  2001-10-24 18:15:20+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: valias_insert.c,v 2.19 2010-03-07 09:28:26+05:30 Cprogrammer Stab mbhangui $";
#endif

#ifdef VALIAS
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <mysqld_error.h>

int
valias_insert(char *alias, char *domain, char *alias_line, int ignore)
{
	int             err;
	char            SqlBuf[SQL_BUF_SIZE];
	char           *ptr, *real_domain;
#ifdef CLUSTERED_SITE
	char           *mailstore;
	char            emailid[MAX_BUFF];
#endif

	if (!domain || !*domain)
		return (1);
	if (!alias_line || !*alias_line)
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
		snprintf(emailid, MAX_BUFF, "%s@%s", alias, real_domain);
		if (!(mailstore = findhost(emailid, 1)))
		{
			/*
			 * Get IP-Address of the Local machine 
			 */
			if (!(ptr = get_local_hostid()))
			{
				fprintf(stderr, "valias_insert: could not get local ip: %s\n", strerror(errno));
				return (-1);
			}
			if (addusercntrl(alias, real_domain, ptr, "alias", 0))
			{
				fprintf(stderr, "valias_insert: Could not insert into central database\n");
				return (1);
			}
		}
		if ((ptr = strchr(alias_line, '@')) && (mailstore = findhost(alias_line, 1)) != (char *) 0)
		{
			if ((ptr = strrchr(mailstore, ':')) != (char *) 0)
				*ptr = 0;
			for (;*mailstore && *mailstore != ':';mailstore++);
			mailstore++;
			if (!ignore && !islocalif (mailstore))
			{
				fprintf(stderr, "%s@%s not local (mailstore %s)\n", alias_line, real_domain, mailstore);
				return (1);
			}
		}
	}
#endif
	if ((err = vauth_open((char *) 0)) != 0)
		return (err);
	while (*alias_line == ' ' && *alias_line != 0)
		++alias_line;
	snprintf(SqlBuf, SQL_BUF_SIZE, 
		"insert low_priority into valias ( alias, domain, valias_line ) values ( \"%s\", \"%s\", \"%s\")", 
		alias, real_domain, alias_line);
	if (mysql_query(&mysql[1], SqlBuf))
	{
		if (mysql_errno(&mysql[1]) == ER_NO_SUCH_TABLE)
		{
			if (create_table(ON_LOCAL, "valias", VALIAS_TABLE_LAYOUT))
				return (-1);
			if (mysql_query(&mysql[1], SqlBuf))
			{
				mysql_perror("valias_insert: %s", SqlBuf);
				return (-1);
			}
		} else
		{
			mysql_perror("valias_insert: %s", SqlBuf);
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
	if (err)
		printf("Added alias %s@%s\n", alias, real_domain);
	else
		printf("No alias %s@%s\n", alias, real_domain);
	return (err ? 0 : 1);
}
#endif

void
getversion_valias_insert_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
