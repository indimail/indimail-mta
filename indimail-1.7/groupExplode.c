/*
 * $Log: $
 */
#include "indimail.h"
#include <stdlib.h>

#ifndef	lint
static char     sccsid[] = "$Id: $";
#endif

char *
groupExplode(char *email)
{
	char            user[MAX_BUFF], domain[MAX_BUFF];
	char           *ptr, *cptr, *real_domain;
	int             valias_count;
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
	if (vauthOpen_user(email))
#else
	if (vauth_open((char *) 0))
#endif
	{
		if (userNotFound)
		{
#ifdef CLUSTERED_SITE
#ifdef VALIAS
			/*-
			 * No need of checking further
			 * valias uses addusecntrl() to add aliases
			 * to hostcntrl
			 */
			if (is_distributed_domain(real_domain))
			{
				is_open = 0;
				return((char *) 0);
			}
#endif
			if(OpenDatabases())
				return((char *) 0);
			/*
			 * These are all individual domains
			 * cycle through all the MySQL databases for each MDA hosts
			 * to locate the alias (group)
			 */
			for (count = 1, mysqlptr = MdaMysql, rhostsptr = RelayHosts;*rhostsptr;mysqlptr++, rhostsptr++)
			{
				if (!strncmp((*rhostsptr)->domain, real_domain, DBINFO_BUFF))
				{
					if((*rhostsptr)->fd == -1)
					{
						if (connect_db(rhostsptr, mysqlptr))
						{
							fprintf(stderr, "%d: %s Failed db %s@%s for user %s port %d\n", count, (*rhostsptr)->domain, 
								(*rhostsptr)->database, (*rhostsptr)->server, (*rhostsptr)->user, (*rhostsptr)->port);
							(*rhostsptr)->fd = -1;
							is_open = 0;
							return((char *) 0);
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
							return((char *) 0);
						} else
							(*rhostsptr)->fd = (*mysqlptr)->net.fd;
					}
					vauth_init(1, *mysqlptr);
					if ((valias_count = valiasCount(user, real_domain)) == -1)
					{
						is_open = 0;
						userNotFound = 0;
						return(-1);
					} else
					if (valias_count > 0)
					{
						is_open = 0;
						return(4);
					}
				}
			} /*- for (count = 1, mysqlptr = MdaMysql, rhostsptr = RelayHosts;*rhostsptr;mysqlptr++, rhostsptr++) */
			is_open = 0;
#else
			if((valias_count = valiasCount(user, real_domain)) == -1)
				return(-1);
			else
			if(valias_count > 0)
				return(4);
#endif
			return(1);
		} else
			return(-1);
	} /*- if (vauth_open((char *) 0)) */
	if (!vauth_getpw(user, real_domain))
	{
		if (userNotFound)
		{
			if ((valias_count = valiasCount(user, real_domain)) == -1)
			{
				is_open = 0;
				userNotFound = 0;
				return(-1);
			} else
			if (valias_count > 0)
			{
#ifdef CLUSTERED_SITE
				is_open = 0;
#endif
				return(4);
			}
#ifdef CLUSTERED_SITE
			is_open = 0;
#endif
			return(1);
		} else
		{
#ifdef CLUSTERED_SITE
			is_open = 0;
#endif
			return(-1);
		}
	}
#ifdef CLUSTERED_SITE
	is_open = 0;
#endif
	if (is_inactive)
		return(2);
	else
	if (is_overquota)
		return(3);
	return(0);
}
#endif

void
getversion_groupExplode_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
