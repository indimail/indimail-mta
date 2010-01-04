/*
 * $Log: SqlServer.c,v $
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
static char     sccsid[] = "$Id: SqlServer.c,v 2.4 2008-05-28 16:37:57+05:30 Cprogrammer Stab mbhangui $";
#endif

#include <string.h>
char *
SqlServer(char *mdahost, char *domain)
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
		if(!strncmp((*rhostsptr)->domain, domain, DBINFO_BUFF) && !strncmp((*rhostsptr)->mdahost, mdahost, DBINFO_BUFF))
			return((*rhostsptr)->server);
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
