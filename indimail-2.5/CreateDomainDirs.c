/*
 * $Log: CreateDomainDirs.c,v $
 * Revision 2.3  2005-12-29 22:40:43+05:30  Cprogrammer
 * use getEnvConfigStr to set variables from environment variables
 *
 * Revision 2.2  2004-06-20 16:31:49+05:30  Cprogrammer
 * renamed ISOCOR_BASE_PATH to BASE_PATH
 *
 * Revision 2.1  2004-05-17 14:00:42+05:30  Cprogrammer
 * changed VPOPMAIL to INDIMAIL
 *
 * Revision 1.3  2001-11-24 12:16:36+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.2  2001-11-20 10:53:13+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.1  2001-10-24 18:14:56+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>

#ifndef	lint
static char     sccsid[] = "$Id: CreateDomainDirs.c,v 2.3 2005-12-29 22:40:43+05:30 Cprogrammer Stab mbhangui $";
#endif

int
CreateDomainDirs(domain, uid, gid)
	char           *domain;
	uid_t           uid;
	gid_t           gid;
{
	int             i, len;
	char           *base_path, *DirName;
	char           *FileSystems[] = {
		"A2E",
		"F2K",
		"L2P",
		"Q2S",
		"T2Zsym",
		"",
	};

	if (!domain || !*domain)
		return (1);
	getEnvConfigStr(&base_path, "BASE_PATH", BASE_PATH);
	umask(INDIMAIL_UMASK);
	for(i = 0;*FileSystems[i];i++)
	{
		len = slen(base_path) + slen(FileSystems[i]) + slen(domain) + 3;
		if((DirName = (char *) malloc(len)) != (char *) NULL)
			snprintf(DirName, len, "%s/%s/%s", base_path, FileSystems[i], domain);
		else
			perror("malloc");
		if(r_mkdir(DirName, INDIMAIL_DIR_MODE, uid, gid))
			fprintf(stderr, "r_mkdir: %s: %s\n", DirName, strerror(errno));
		free(DirName);
	}
	return (0);
}

void
getversion_CreateDomainDirs_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
