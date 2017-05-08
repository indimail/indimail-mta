/*
 * $Log: get_local_hostid.c,v $
 * Revision 2.5  2017-03-13 13:43:49+05:30  Cprogrammer
 * replaced qmaildir with sysconfdir
 *
 * Revision 2.4  2016-05-17 17:09:39+05:30  Cprogrammer
 * use control directory set by configure
 *
 * Revision 2.3  2008-06-03 19:42:56+05:30  Cprogrammer
 * padded hostid with leading zeros to avoid problem with user addition
 *
 * Revision 2.2  2005-12-29 22:44:42+05:30  Cprogrammer
 * use getEnvConfigStr to set variables from environment variables
 *
 * Revision 2.1  2002-08-25 22:32:39+05:30  Cprogrammer
 * made control dir configurable
 *
 * Revision 1.1  2002-03-29 18:22:17+05:30  Cprogrammer
 * Initial revision
 *
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: get_local_hostid.c,v 2.5 2017-03-13 13:43:49+05:30 Cprogrammer Stab mbhangui $";
#endif


char           *
get_local_hostid()
{
	static char     hostid[MAX_BUFF];
	char            TmpFname[MAX_BUFF];
	char           *sysconfdir, *controldir, *ptr;
	FILE           *fp;
	long            hostidno;

	getEnvConfigStr(&controldir, "CONTROLDIR", CONTROLDIR);
	if (*controldir == '/')
		snprintf(TmpFname, MAX_BUFF, "%s/hostid", controldir);
	else {
		getEnvConfigStr(&sysconfdir, "SYSCONFDIR", SYSCONFDIR);
		snprintf(TmpFname, MAX_BUFF, "%s/%s/hostid", sysconfdir, controldir);
	}
	if ((fp = fopen(TmpFname, "r")))
	{
		if (!fgets(hostid, MAX_BUFF - 1, fp))
		{
			fclose(fp);
			return((char *) 0);
		}
		if ((ptr = strchr(hostid, '\n')))
			*ptr = 0;
		fclose(fp);
		return(hostid);
	}
	if ((hostidno = gethostid()) == -1)
		return((char *) 0);
	snprintf(hostid, MAX_BUFF, "%08lx", hostidno);
	return(hostid);
}

void
getversion_get_local_hostid_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
