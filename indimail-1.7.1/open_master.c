/*
 * $Log: open_master.c,v $
 * Revision 2.7  2010-03-07 09:58:51+05:30  Cprogrammer
 * return error of host.master is not present
 *
 * Revision 2.6  2010-02-20 11:31:17+05:30  Cprogrammer
 * added RCS log
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: open_master.c,v 2.7 2010-03-07 09:58:51+05:30 Cprogrammer Stab mbhangui $";
#endif

#ifdef CLUSTERED_SITE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

int
open_master()
{
	char           *ptr, *qmaildir, *controldir;
	char            host_path[MAX_BUFF], master_host[MAX_BUFF];
	FILE           *fp;

	if ((ptr = (char *) getenv("MASTER_HOST")) != (char *) 0)
		return (open_central_db(ptr));
	getEnvConfigStr(&qmaildir, "QMAILDIR", QMAILDIR);
	getEnvConfigStr(&controldir, "CONTROLDIR", "control");
	if (snprintf(host_path, MAX_BUFF, "%s/%s/host.master", qmaildir, controldir) == -1)
		host_path[MAX_BUFF - 1] = 0;
	if (!(fp = fopen(host_path, "r")))
	{
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
