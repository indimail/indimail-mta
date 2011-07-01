/*
 * $Log: mdaMysqlConnect.c,v $
 * Revision 2.7  2009-09-23 21:22:15+05:30  Cprogrammer
 * record error when mysql_ping reports MySQL server has gone away
 *
 * Revision 2.6  2008-05-28 16:37:02+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 2.5  2006-03-02 20:38:33+05:30  Cprogrammer
 * While processing if one MySQL host is down, continue if the user entry is not on the
 * 'down' MySQL host
 *
 * Revision 2.4  2002-10-28 17:57:40+05:30  Cprogrammer
 * force reconnection to mysql if mysql_ping() fails
 *
 * Revision 2.3  2002-10-06 00:04:25+05:30  Cprogrammer
 * added code to handle mysql problems
 *
 * Revision 2.2  2002-04-12 12:23:51+05:30  Cprogrammer
 * use domain also to locate the mysql host as multiple domains could be on the same mda
 *
 * Revision 2.1  2002-04-12 01:34:06+05:30  Cprogrammer
 * Function to return MYSQL structure for a given mda host
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: mdaMysqlConnect.c,v 2.7 2009-09-23 21:22:15+05:30 Cprogrammer Stab mbhangui $";
#endif

#include <string.h>
MYSQL **
mdaMysqlConnect(char *mdahost, char *domain)
{
	DBINFO        **rhostsptr;
	MYSQL         **mysqlptr;
	int             count;

	if(OpenDatabases())
		return((MYSQL **) 0);
	for (count = 1, mysqlptr = MdaMysql, rhostsptr = RelayHosts;*rhostsptr;mysqlptr++, rhostsptr++)
	{
		if (!strncmp((*rhostsptr)->domain, domain, DBINFO_BUFF) && !strncmp((*rhostsptr)->mdahost, mdahost, DBINFO_BUFF))
		{
			if ((*rhostsptr)->fd == -1)
			{
				if (connect_db(rhostsptr, mysqlptr))
				{
					fprintf(stderr, "%d: %s Failed db %s@%s for user %s port %d\n", count, (*rhostsptr)->domain, 
						(*rhostsptr)->database, (*rhostsptr)->server, (*rhostsptr)->user, (*rhostsptr)->port);
					(*rhostsptr)->fd = -1;
					return((MYSQL **) 0);
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
					return((MYSQL **) 0);
				} else
					(*rhostsptr)->fd = (*mysqlptr)->net.fd;
			}
			return(mysqlptr);
		}
	}
	return((MYSQL **) 0);
}

void
getversion_mdaMysqlConnect_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
