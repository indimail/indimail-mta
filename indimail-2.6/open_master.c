/*
 * $Log: open_master.c,v $
 * Revision 2.10  2019-03-18 18:21:12+05:30  Cprogrammer
 * return 2 if hostcntrl not configured
 *
 * Revision 2.9  2017-03-13 14:04:53+05:30  Cprogrammer
 * replaced qmaildir with sysconfdir
 *
 * Revision 2.8  2016-05-17 17:09:39+05:30  Cprogrammer
 * use control directory set by configure
 *
 * Revision 2.7  2010-03-07 09:58:51+05:30  Cprogrammer
 * return error of host.master is not present
 *
 * Revision 2.6  2010-02-20 11:31:17+05:30  Cprogrammer
 * added RCS log
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: open_master.c,v 2.10 2019-03-18 18:21:12+05:30 Cprogrammer Exp mbhangui $";
#endif

#ifdef CLUSTERED_SITE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

int
open_master()
{
	char           *ptr, *sysconfdir, *controldir;
	char            host_path[MAX_BUFF], master_host[MAX_BUFF];
	FILE           *fp;

	if ((ptr = (char *) getenv("MASTER_HOST")) != (char *) 0)
		return (open_central_db(ptr));
	getEnvConfigStr(&controldir, "CONTROLDIR", CONTROLDIR);
	if (*controldir == '/') {
		if (snprintf(host_path, MAX_BUFF, "%s/host.master", controldir) == -1)
			host_path[MAX_BUFF - 1] = 0;
	} else {
		getEnvConfigStr(&sysconfdir, "SYSCONFDIR", SYSCONFDIR);
		if (snprintf(host_path, MAX_BUFF, "%s/%s/host.master", sysconfdir, controldir) == -1)
			host_path[MAX_BUFF - 1] = 0;
	}
	if (!(fp = fopen(host_path, "r")))
	{
		if (errno == 2)
			return (2);
		fprintf(stderr, "%s: %s\n", host_path, strerror(errno));
		return (1);
	} else
	{
		if (fscanf(fp, "%s", master_host) == 1)
		{
			master_host[MAX_BUFF - 1] = 0;
			fclose(fp);
			return (open_central_db(master_host));
		}
		fclose(fp);
	}
	return (1);
}
#endif

void
getversion_open_master_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
