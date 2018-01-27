/*
 * $Log: UserInLookup.c,v $
 * Revision 2.17  2010-05-01 14:14:14+05:30  Cprogrammer
 * added connect_all argument to vauthOpen_user
 *
 * Revision 2.16  2010-03-07 09:27:27+05:30  Cprogrammer
 * check return value of is_distributed_domain for error
 *
 * Revision 2.15  2009-09-23 21:22:50+05:30  Cprogrammer
 * record error when mysql_ping reports MySQL server has gone away
 *
 * Revision 2.14  2008-06-13 10:31:31+05:30  Cprogrammer
 * fixed compilation errors if VALIAS not defined
 *
 * Revision 2.13  2008-05-28 16:38:10+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 2.12  2006-03-02 20:42:43+05:30  Cprogrammer
 * While processing if one MySQL host is down, continue if the user entry is not on the
 * 'down' MySQL host
 *
 * Revision 2.11  2005-12-29 22:50:49+05:30  Cprogrammer
 * use getEnvConfigStr to set variables from environment variables
 *
 * Revision 2.10  2003-12-31 01:24:47+05:30  Cprogrammer
 * return user not present if vauthOpen_user returns userNotFound
 *
 * Revision 2.9  2003-12-26 14:50:43+05:30  Cprogrammer
 * set userNotFound to 0 in case of valias error
 *
 * Revision 2.8  2002-12-27 16:41:21+05:30  Cprogrammer
 * use valiasCount() to determine existence of an alias for a user
 *
 * Revision 2.7  2002-10-28 17:58:02+05:30  Cprogrammer
 * force reconnection to mysql if mysql_ping() fails
 *
 * Revision 2.6  2002-10-12 22:56:56+05:30  Cprogrammer
 * variables count, mysqlptr, rhostsptr defined only in clusted environment
 *
 * Revision 2.5  2002-10-06 00:01:20+05:30  Cprogrammer
 * added code to handle mysql problems
 *
 * Revision 2.4  2002-09-30 19:04:26+05:30  Cprogrammer
 * corrected problem with connection made to wrong mysql server for users not existing in hostcntrl
 *
 * Revision 2.3  2002-09-04 23:00:00+05:30  Cprogrammer
 * added code to return 4 for aliases
 *
 * Revision 2.2  2002-08-31 15:57:26+05:30  Cprogrammer
 * take domain as the DEFAULT_DOMAIN if email does not contain domain
 *
 * Revision 2.1  2002-07-04 00:32:52+05:30  Cprogrammer
 * is_open to be reset only for distributed code
 *
 * Revision 1.1  2002-04-09 14:37:55+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: UserInLookup.c,v 2.17 2010-05-01 14:14:14+05:30 Cprogrammer Stab mbhangui $";
#endif

#include <string.h>

/*
 *  0: User is fine
 *  1: User is not present
 *  2: User is Inactive
 *  3: User is overquota
 * -1: System Error
 */
int
UserInLookup(char *email)
{
	char            user[MAX_BUFF], domain[MAX_BUFF];
	char           *ptr, *cptr, *real_domain;
#ifdef VALIAS
	int             valias_count;
#endif
#ifdef CLUSTERED_SITE
	int             count;
	DBINFO        **rhostsptr;
	MYSQL         **mysqlptr;
#endif

	for (cptr = user, ptr = email;*ptr && *ptr != '@';*cptr++ = *ptr++);
	*cptr = 0;
	if (*ptr)
		ptr++;
	else
		getEnvConfigStr(&ptr, "DEFAULT_DOMAIN", DEFAULT_DOMAIN);
	for (cptr = domain;*ptr;*cptr++ = *ptr++);
	*cptr = 0;
	if (!(real_domain = vget_real_domain(domain)))
		real_domain = domain;
#ifdef CLUSTERED_SITE
	if (vauthOpen_user(email, 1))
#else
	if (vauth_open((char *) 0))
#endif
	{
		if (userNotFound) /*- Maybe user is an alias */
		{
#ifdef CLUSTERED_SITE
#ifdef VALIAS
			/*-
			 * No need of checking further
			 * valias uses addusecntrl() to add aliases
			 * to hostcntrl. so if alias not found in hostcntrl,
			 * it will not be present on the mailstore's alias table
			 */
			if ((count = is_distributed_domain(real_domain)) == -1)
			{
				fprintf(stderr, "%s: is_distributed_domain failed\n", real_domain);
				return (-1);
			} else
			if (count)
			{
				is_open = 0;
				return (1);
			}
#endif
			if (OpenDatabases())
				return (-1);
			for (count = 1, mysqlptr = MdaMysql, rhostsptr = RelayHosts;*rhostsptr;mysqlptr++, rhostsptr++)
			{
				if (!strncmp((*rhostsptr)->domain, real_domain, DBINFO_BUFF))
				{
					if ((*rhostsptr)->fd == -1)
					{
						if (connect_db(rhostsptr, mysqlptr))
						{
							fprintf(stderr, "%d: %s Failed db %s@%s for user %s port %d\n", count, (*rhostsptr)->domain, 
								(*rhostsptr)->database, (*rhostsptr)->server, (*rhostsptr)->user, (*rhostsptr)->port);
							(*rhostsptr)->fd = -1;
							is_open = 0;
							userNotFound = 0;
							return (-1);
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
							is_open = 0;
							userNotFound = 0;
							fprintf(stderr, "%d: %s Failed db %s@%s for user %s port %d\n", count, (*rhostsptr)->domain, 
								(*rhostsptr)->database, (*rhostsptr)->server, (*rhostsptr)->user, (*rhostsptr)->port);
							(*rhostsptr)->fd = -1;
							return (-1);
						} else
							(*rhostsptr)->fd = (*mysqlptr)->net.fd;
					}
#ifdef VALIAS
					vauth_init(1, *mysqlptr);
					if ((valias_count = valiasCount(user, real_domain)) == -1)
					{
						is_open = 0;
						userNotFound = 0;
						return (-1);
					} else
					if (valias_count > 0)
					{
						is_open = 0;
						return (4);
					}
#endif
				}
			} /*- for (count = 1, mysqlptr = MdaMysql, rhostsptr = RelayHosts;*rhostsptr;mysqlptr++, rhostsptr++) */
			is_open = 0;
#else /*- #ifdef CLUSTERED_SITE */
#ifdef VALIAS
			if ((valias_count = valiasCount(user, real_domain)) == -1)
				return (-1);
			else
			if (valias_count > 0)
				return (4);
#endif
#endif /*- #ifdef CLUSTERED_SITE */
			return (1);
		} else /*- if (userNotFound) */
			return (-1);
	}
	if (!vauth_getpw(user, real_domain))
	{
		if (userNotFound)
		{
#ifdef VALIAS
			if ((valias_count = valiasCount(user, real_domain)) == -1)
			{
#ifdef CLUSTERED_SITE
				is_open = 0;
#endif
				userNotFound = 0;
				return (-1);
			} else
			if (valias_count > 0)
			{
#ifdef CLUSTERED_SITE
				is_open = 0;
#endif
				return (4);
			}
#ifdef CLUSTERED_SITE
			is_open = 0;
#endif
#endif
			return (1);
		} else
		{
#ifdef CLUSTERED_SITE
			is_open = 0;
#endif
			return (-1);
		}
	}
#ifdef CLUSTERED_SITE
	is_open = 0;
#endif
	if (is_inactive)
		return (2);
	else
	if (is_overquota)
		return (3);
	return (0);
}

void
getversion_UserInLookup_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
