/*
 * $Log: SqlServer.c,v $
 * Revision 2.5  2010-02-26 10:52:27+05:30  Cprogrammer
 * return host in host:user:password:port/socket format
 *
 * Revision 2.4  2008-05-28 16:37:57+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 2.3  2003-01-17 01:05:50+05:30  Cprogrammer
 * added function MdaServer()
 *
 * Revision 2.2  2002-08-03 00:34:29+05:30  Cprogrammer
 * added additonal argument domain as search is not unique on ip mdahost alone
 *
 * Revision 2.1  2002-04-12 02:08:09+05:30  Cprogrammer
 * Use RelayHosts instead of relayhosts to avoid duplicate allocation in OpenDatabases() and SqlServer()
 *
 * Revision 1.1  2002-03-29 12:11:14+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: SqlServer.c,v 2.5 2010-02-26 10:52:27+05:30 Cprogrammer Stab mbhangui $";
#endif

#include <string.h>
#include <stdlib.h>

char *
SqlServer(char *mdahost, char *domain)
{
	DBINFO        **rhostsptr;
	int             total, len;
	char            port_buf[18];
	static char    *hostbuf;

	if(!RelayHosts && !(RelayHosts = LoadDbInfo_TXT(&total)))
	{
		perror("LoadDbInfo_TXT");
		return((char *) 0);
	}
	for (rhostsptr = RelayHosts;*rhostsptr;rhostsptr++)
	{
		if(!strncmp((*rhostsptr)->domain, domain, DBINFO_BUFF) && !strncmp((*rhostsptr)->mdahost, mdahost, DBINFO_BUFF))
		{
			len = strlen((*rhostsptr)->server) + strlen((*rhostsptr)->user) +
					strlen((*rhostsptr)->password) + 3;
			if (strncmp((*rhostsptr)->server, "localhost", 10) && (*rhostsptr)->port != 3306)
			{
				snprintf(port_buf, sizeof(port_buf), "%d", (*rhostsptr)->port);
				len += (strlen(port_buf) + 1);
			} else
				*port_buf = 0;
			if (!(hostbuf = (char *) realloc(hostbuf, len)))
			{
				perror("SqlServer: realloc");
				return((char *) 0);
			}
			if (*port_buf)
				snprintf(hostbuf, len, "%s:%s:%s:%s", (*rhostsptr)->server, (*rhostsptr)->user,
						(*rhostsptr)->password, port_buf);
			else
				snprintf(hostbuf, len, "%s:%s:%s", (*rhostsptr)->server, (*rhostsptr)->user,
						(*rhostsptr)->password);
			return (hostbuf);
		}
	}
	return((char *) 0);
}

char *
MdaServer(char *sqlhost, char *domain)
{
	DBINFO        **rhostsptr;
	int             total;

	if(!RelayHosts && !(RelayHosts = LoadDbInfo_TXT(&total)))
	{
		perror("LoadDbInfo_TXT");
		return((char *) 0);
	}
	for (rhostsptr = RelayHosts;*rhostsptr;rhostsptr++)
	{
		if(!strncmp((*rhostsptr)->domain, domain, DBINFO_BUFF) && !strncmp((*rhostsptr)->server, sqlhost, DBINFO_BUFF))
			return((*rhostsptr)->mdahost);
	}
	return((char *) 0);
}

void
getversion_SqlServer_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
