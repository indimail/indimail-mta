/*
 * $Log: is_distributed_domain.c,v $
 * Revision 2.7  2008-11-06 15:37:50+05:30  Cprogrammer
 * added cache_reset option
 *
 * Revision 2.6  2008-05-28 16:36:06+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 2.5  2003-10-24 00:42:20+05:30  Cprogrammer
 * treat all domains as non distributed if mcd file is absent
 *
 * Revision 2.4  2003-10-16 00:01:17+05:30  Cprogrammer
 * set dist_flag to -1 on error
 *
 * Revision 2.3  2003-02-01 14:10:54+05:30  Cprogrammer
 * use LoadDbInfo_TXT() to find distributed_domain for efficiency
 *
 * Revision 2.2  2002-09-11 20:39:24+05:30  Cprogrammer
 * print error message if unable to open mcd file
 *
 * Revision 2.1  2002-08-25 22:48:57+05:30  Cprogrammer
 * made control dir configurable
 *
 * Revision 1.8  2002-03-18 22:23:31+05:30  Cprogrammer
 * domain line in mcdfile has 3rd column to specify if a domain is distributed
 *
 * Revision 1.7  2001-12-12 19:25:16+05:30  Cprogrammer
 * added code to skip comments and blank lines
 *
 * Revision 1.6  2001-12-12 13:43:58+05:30  Cprogrammer
 * is_distributed_domain uses the same file as tcpserver
 *
 * Revision 1.5  2001-11-29 22:59:09+05:30  Cprogrammer
 * replaced strncpy with scopy
 *
 * Revision 1.4  2001-11-28 23:39:03+05:30  Cprogrammer
 * check to prevent core dump in subsequent strncmp()
 *
 * Revision 1.3  2001-11-28 22:59:23+05:30  Cprogrammer
 * improved efficiency by returning from a static variable when same domain is passed
 *
 * Revision 1.2  2001-11-24 12:19:17+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.1  2001-11-22 22:53:46+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: is_distributed_domain.c,v 2.7 2008-11-06 15:37:50+05:30 Cprogrammer Stab mbhangui $";
#endif

#ifdef CLUSTERED_SITE
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#ifdef QUERY_CACHE
static char     _cacheSwitch = 1;
#endif

int
is_distributed_domain(char *Domain)
{
	static char     savedomain[MAX_BUFF];
	static int      dist_flag;
	int             total;
	DBINFO        **rhostsptr;

	if (!Domain || !*Domain)
		return (0);
#ifdef QUERY_CACHE
	if (_cacheSwitch && (char *) getenv("QUERY_CACHE") && dist_flag != -1 && *savedomain 
		&& !strncmp(Domain, savedomain, MAX_BUFF))
		return (dist_flag);
	else
		scopy(savedomain, Domain, MAX_BUFF);
	if (!_cacheSwitch)
		_cacheSwitch = 1;
#else
	scopy(savedomain, Domain, MAX_BUFF);
#endif
	if (!RelayHosts && !(RelayHosts = LoadDbInfo_TXT(&total)))
	{
		if (errno == 2)
			return (dist_flag = 0);
		fprintf(stderr, "is_distributed_domain: LoadDbInfo_TXT: %s\n", strerror(errno));
		dist_flag = -1;
		return (-1);
	}
	for (dist_flag = 0, rhostsptr = RelayHosts;*rhostsptr;rhostsptr++)
	{
		if (!strncmp((*rhostsptr)->domain, Domain, DBINFO_BUFF))
			return ((dist_flag = (*rhostsptr)->distributed));
	}
	return (dist_flag = 0);
}

#ifdef QUERY_CACHE
void
is_distributed_domain_cache(char cache_switch)
{
	_cacheSwitch = cache_switch;
	return;
}
#endif
#else
int
is_distributed_domain(char *Domain)
{
	return (0);
}
#endif

void
getversion_is_distributed_domain_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
