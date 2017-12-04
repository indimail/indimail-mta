/*
 * $Log: findmdahost.c,v $
 * Revision 2.16  2010-06-07 18:43:27+05:30  Cprogrammer
 * added addition argument to indicate connection to all MySQL database or just one
 * return number of dbinfo records in the additional argument
 *
 * Revision 2.15  2010-04-24 09:29:23+05:30  Cprogrammer
 * default port already set in GetSmtproute() or get_smtp_service_port()
 *
 * Revision 2.14  2010-03-07 11:10:35+05:30  Cprogrammer
 * no need of checking host.cntrl. Check mcdinfo instead
 *
 * Revision 2.13  2009-09-23 21:21:32+05:30  Cprogrammer
 * record error when mysql_ping reports MySQL server has gone away
 *
 * Revision 2.12  2008-07-20 18:09:38+05:30  Cprogrammer
 * added error message
 *
 * Revision 2.11  2008-05-28 16:35:26+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 2.10  2006-03-02 20:42:24+05:30  Cprogrammer
 * While processing if one MySQL host is down, continue if the user entry is not on the
 * >> 'down' MySQL h
 *
 * Revision 2.9  2005-12-29 22:44:23+05:30  Cprogrammer
 * use getEnvConfigStr to set variables from environment variables
 *
 * Revision 2.8  2003-02-01 14:08:24+05:30  Cprogrammer
 * bug fix - get_smtp_service_port() should not be called for non-distributed domains
 *
 * Revision 2.7  2002-12-29 18:58:24+05:30  Cprogrammer
 * use control file smtproute if hostcntrl is absent
 *
 * Revision 2.6  2002-10-28 23:29:40+05:30  Cprogrammer
 * removed duplicate assignment statements
 *
 * Revision 2.5  2002-10-28 17:56:49+05:30  Cprogrammer
 * force reconnection to mysql if mysql_ping() fails
 *
 * Revision 2.4  2002-10-06 00:00:06+05:30  Cprogrammer
 * added Error message if mysql_ping fails
 *
 * Revision 2.3  2002-08-31 15:57:32+05:30  Cprogrammer
 * take domain as the DEFAULT_DOMAIN if email does not contain domain
 *
 * Revision 2.2  2002-08-25 22:32:31+05:30  Cprogrammer
 * made control dir configurable
 *
 * Revision 2.1  2002-07-27 10:53:41+05:30  Cprogrammer
 * moved OpenDatabases() into distributed part to increase efficiency
 *
 * Revision 1.2  2002-04-10 04:43:34+05:30  Cprogrammer
 * do ping before vauth_init
 *
 * Revision 1.1  2002-04-10 02:59:05+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: findmdahost.c,v 2.16 2010-06-07 18:43:27+05:30 Cprogrammer Stab mbhangui $";
#endif

#ifdef CLUSTERED_SITE
#include <string.h>
#include <stdlib.h>
#include <errno.h>

char           *
findmdahost(char *email, int *total)
{
	int             is_dist, count, port, connect_all;
	char            user[MAX_BUFF], domain[MAX_BUFF];
	static char     mailhost[MAX_BUFF];
	char           *ptr, *cptr, *real_domain, *ip;
	DBINFO        **rhostsptr;
	MYSQL         **mysqlptr;
	struct passwd *pw;

	for (cptr = user, ptr = email;*ptr && *ptr != '@';*cptr++ = *ptr++);
	*cptr = 0;
	if (*ptr)
		ptr++;
	else
		getEnvConfigStr(&ptr, "DEFAULT_DOMAIN", DEFAULT_DOMAIN);
	for (cptr = domain;*ptr;*cptr++ = *ptr++);
	*cptr = 0;
	if (!*domain)
		return ((char *) 0);
	if (!(real_domain = vget_real_domain(domain)))
		real_domain = domain;
	if ((is_dist = is_distributed_domain(real_domain)) == -1)
	{
		fprintf(stderr, "%s: is_distributed_domain failed\n", real_domain);
		return ((char *) 0);
	}
	if (is_dist)
	{
		if (!(ip = findhost(email, 0)))
			return ((char *) 0);
		else
			return (ip);
	} 
	/*- reach here if non-distributed */
	if (!total)
		connect_all = 1;
	else
		connect_all = *total;
	if (connect_all)
	{
		if (OpenDatabases())
			return ((char *) 0);
	} else
	{
		if (RelayHosts)
		{
			for (count = 0, rhostsptr = RelayHosts;*rhostsptr;rhostsptr++, count++);
			*total = count;
		} else
		{
			if (!(RelayHosts = LoadDbInfo_TXT(total)))
			{
				perror("findmdahost: LoadDbInfo_TXT");
				return ((char *) 0);
			}
			for (rhostsptr = RelayHosts;*rhostsptr;rhostsptr++)
				(*rhostsptr)->fd = -1;
		}
		if (!MdaMysql)
		{
			if (!(MdaMysql = (MYSQL **) calloc(1, sizeof(MYSQL *) * (*total))))
			{
				fprintf(stderr, "findmdahost: calloc: %d Bytes: %s\n",
					*total * (int) sizeof(MYSQL *), strerror(errno));
				return ((char *) 0);
			}
			for (mysqlptr = MdaMysql, rhostsptr = RelayHosts;*rhostsptr;mysqlptr++, rhostsptr++)
				*mysqlptr = (MYSQL *) 0;
		}
	}
	for (count= 1, mysqlptr = MdaMysql, rhostsptr = RelayHosts;*rhostsptr;mysqlptr++, rhostsptr++, count++)
	{
		/*- for non distributed only one entry is there in mcd file */
		if (!strncmp(real_domain, (*rhostsptr)->domain, DBINFO_BUFF))
			break;
	}
	if (*rhostsptr)
	{
		if (!connect_all && !*mysqlptr && !((*mysqlptr) = (MYSQL *) calloc(1, sizeof(MYSQL))))
		{
			fprintf(stderr, "findmdahost: calloc: %d Bytes: %s",
				(int) sizeof(MYSQL *), strerror(errno));
			(*rhostsptr)->fd = -1;
			return ((char *) 0);
		}
		if ((*rhostsptr)->fd == -1)
		{
			if (connect_db(rhostsptr, mysqlptr))
			{
				fprintf(stderr, "%d: %s Failed db %s@%s for user %s port %d\n", count, (*rhostsptr)->domain, 
					(*rhostsptr)->database, (*rhostsptr)->server, (*rhostsptr)->user, (*rhostsptr)->port);
				(*rhostsptr)->fd = -1;
				return ((char *) 0);
			} else
				(*rhostsptr)->fd = (*mysqlptr)->net.fd;
		}
		if (mysql_ping(*mysqlptr))
		{
			fprintf(stderr, "mysql_ping: (%s) %s: Reconnecting... %s@%s user %s port %d\n",
					mysql_error(*mysqlptr), (*rhostsptr)->domain, (*rhostsptr)->database,
					(*rhostsptr)->server, (*rhostsptr)->user, (*rhostsptr)->port);
			mysql_close(*mysqlptr);
			if (connect_db(rhostsptr, mysqlptr))
			{
				fprintf(stderr, "%d: %s Failed db %s@%s for user %s port %d\n", count, (*rhostsptr)->domain, 
					(*rhostsptr)->database, (*rhostsptr)->server, (*rhostsptr)->user, (*rhostsptr)->port);
				(*rhostsptr)->fd = -1;
				return ((char *) 0);
			} else
				(*rhostsptr)->fd = (*mysqlptr)->net.fd;
		}
		vauth_init(1, *mysqlptr);
		if (!(pw = vauth_getpw(user, real_domain)))
		{
			is_open = 0; /* prevent closing of connection by vclose */
			return ((char *) 0);
		} else
		{
			is_open = 0; /* prevent closing of connection by vclose */
			if (is_dist == 1)
				port = get_smtp_service_port(0, real_domain, (*rhostsptr)->mdahost);
			else
				port = GetSmtproute(real_domain);
			if (port == -1)
				return ((char *) 0);
			snprintf(mailhost, MAX_BUFF, "%s:%s:%d", domain, (*rhostsptr)->mdahost, port);
			return (mailhost);
		}
	} else
		userNotFound = 1;
	return ((char *) 0);
}
#endif

void
getversion_findmdahost_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
