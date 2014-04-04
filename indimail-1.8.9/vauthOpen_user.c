/*
 * $Log: vauthOpen_user.c,v $
 * Revision 2.11  2010-06-07 18:44:12+05:30  Cprogrammer
 * pass additional connect_all argument to findmdahost()
 *
 * Revision 2.10  2010-05-07 13:56:50+05:30  Cprogrammer
 * include stdlib.h to fix compiler warnng
 *
 * Revision 2.9  2010-05-01 14:19:57+05:30  Cprogrammer
 * connect to only one MySQL database if connect_all = 0
 *
 * Revision 2.8  2009-09-23 21:22:54+05:30  Cprogrammer
 * record error when mysql_ping reports MySQL server has gone away
 *
 * Revision 2.7  2008-11-07 11:13:59+05:30  Cprogrammer
 * BUG - mdahost was getting clobbered
 *
 * Revision 2.6  2008-05-28 16:39:45+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 2.5  2006-03-02 20:41:25+05:30  Cprogrammer
 * While processing if one MySQL host is down, continue if the user entry is not on the
 * >> 'down' MySQL h
 *
 * Revision 2.4  2005-12-29 22:51:52+05:30  Cprogrammer
 * use getEnvConfigStr to set variables from environment variables
 *
 * Revision 2.3  2002-10-28 17:58:31+05:30  Cprogrammer
 * force reconnection to mysql if mysql_ping() fails
 *
 * Revision 2.2  2002-10-06 00:01:49+05:30  Cprogrammer
 * added error logging for mysql_ping()
 *
 * Revision 2.1  2002-08-31 15:57:37+05:30  Cprogrammer
 * take domain as the DEFAULT_DOMAIN if email does not contain domain
 *
 * Revision 1.2  2002-04-10 04:43:53+05:30  Cprogrammer
 * do ping before vauth_init
 *
 * Revision 1.1  2002-04-10 03:00:33+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: vauthOpen_user.c,v 2.11 2010-06-07 18:44:12+05:30 Cprogrammer Stab mbhangui $";
#endif

#ifdef CLUSTERED_SITE
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

int
vauthOpen_user(char *email, int connect_all)
{
	int             count, total = 0;
	char            user[MAX_BUFF], domain[MAX_BUFF], mdahost[MAX_BUFF];
	char           *ptr, *cptr, *real_domain;
	DBINFO        **rhostsptr;
	MYSQL         **mysqlptr;

	for (cptr = user, ptr = email;*ptr && *ptr != '@';*cptr++ = *ptr++);
	*cptr = 0;
	if (*ptr)
		ptr++;
	else
		getEnvConfigStr(&ptr, "DEFAULT_DOMAIN", DEFAULT_DOMAIN);
	for (cptr = domain;*ptr;*cptr++ = *ptr++);
	*cptr = 0;
	if (!(ptr = findmdahost(email, &total)))
		return(-1);
	for (;*ptr && *ptr != ':';ptr++);
	if (!*ptr++)
		return(-1);
	for (cptr = mdahost;*ptr && *ptr != ':';*cptr++ = *ptr++);
	*cptr = 0;
	if (connect_all)
	{
		if (OpenDatabases())
			return(-1);
	} else
	{
		if (RelayHosts)
		{
			for (count = 0, rhostsptr = RelayHosts, mysqlptr = MdaMysql;*rhostsptr;mysqlptr++, rhostsptr++, count++);
			total = count;
		} else
		{
			total = 0;
			if (!(RelayHosts = LoadDbInfo_TXT(&total)))
			{
				perror("vauthOpen_user: LoadDbInfo_TXT");
				return(-1);
			}
			for (rhostsptr = RelayHosts; *rhostsptr; rhostsptr++)
				(*rhostsptr)->fd = -1;
		}
		if (!MdaMysql)
		{
			if (!(MdaMysql = (MYSQL **) calloc(1, sizeof(MYSQL *) * (total))))
			{
				fprintf(stderr, "vauthOpen_user: calloc: %d Bytes: %s\n",
					total * (int) sizeof(MYSQL *), strerror(errno));
				return (-1);
			}
			for (rhostsptr = RelayHosts, mysqlptr = MdaMysql;*rhostsptr;mysqlptr++, rhostsptr++)
				*mysqlptr = (MYSQL *) 0;
		}
	}
	if (!(real_domain = vget_real_domain(domain)))
		real_domain = domain;
	for (count = 1, rhostsptr = RelayHosts, mysqlptr = MdaMysql;*rhostsptr;mysqlptr++, rhostsptr++, count++)
	{
		if (!strncmp(real_domain, (*rhostsptr)->domain, DBINFO_BUFF) &&
				!strncmp(mdahost, (*rhostsptr)->mdahost, DBINFO_BUFF))
			break;
	}
	if (*rhostsptr)
	{
		if (!connect_all && !*mysqlptr && !((*mysqlptr) = (MYSQL *) calloc(1, sizeof(MYSQL))))
		{
			fprintf(stderr, "vauthOpen_user: calloc: %d Bytes: %s",
				(int) sizeof(MYSQL *), strerror(errno));
			(*rhostsptr)->fd = -1;
			return (-1);
		}
		if ((*rhostsptr)->fd == -1)
		{
			if (connect_db(rhostsptr, mysqlptr))
			{
				fprintf(stderr, "%d: %s Failed db %s@%s for user %s port %d\n",
					count, (*rhostsptr)->domain, (*rhostsptr)->database, (*rhostsptr)->server,
					(*rhostsptr)->user, (*rhostsptr)->port);
				(*rhostsptr)->fd = -1;
				return(-1);
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
				fprintf(stderr, "%d: %s Failed db %s@%s for user %s port %d\n",
					count, (*rhostsptr)->domain, (*rhostsptr)->database, (*rhostsptr)->server,
					(*rhostsptr)->user, (*rhostsptr)->port);
				(*rhostsptr)->fd = -1;
				return(-1);
			} else
				(*rhostsptr)->fd = (*mysqlptr)->net.fd;
		}
		vauth_init(1, *mysqlptr);
		return(0);
	} else
		userNotFound = 1;
	return(-1);
}
#endif

void
getversion_vauthOpen_user_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
