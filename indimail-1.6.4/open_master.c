/*
 * $log:$
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: open_master.c,v 2.5 2009-02-18 21:32:46+05:30 Cprogrammer Stab mbhangui $";
#endif

#ifdef CLUSTERED_SITE
#include <stdio.h>
#include <stdlib.h>

int
open_master()
{
	char           *ptr, *qmaildir, *controldir;
	char            host_path[MAX_BUFF], master_host[MAX_BUFF];
	FILE           *fp;

	if((ptr = (char *) getenv("MASTER_HOST")) != (char *) 0)
		return (open_central_db(ptr));
	getEnvConfigStr(&qmaildir, "QMAILDIR", QMAILDIR);
	getEnvConfigStr(&controldir, "CONTROLDIR", "control");
	if (snprintf(host_path, MAX_BUFF, "%s/%s/host.master", qmaildir, controldir) == -1)
		host_path[MAX_BUFF - 1] = 0;
	if (!(fp = fopen(host_path, "r")))
		return (open_central_db(MASTER_HOST));
	else
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
