/*
 * $Log: get_Mplexdir.c,v $
 * Revision 2.5  2008-09-14 19:47:12+05:30  Cprogrammer
 * formatted else statements
 *
 * Revision 2.4  2008-07-14 19:34:35+05:30  Cprogrammer
 * added stdlib for malloc functions
 *
 * Revision 2.3  2005-12-29 22:44:49+05:30  Cprogrammer
 * use getEnvConfigStr to set variables from environment variables
 *
 * Revision 2.2  2004-06-20 16:34:59+05:30  Cprogrammer
 * rename ISOCOR_BASE_PATH to BASE_PATH
 *
 * Revision 2.1  2004-05-17 14:01:06+05:30  Cprogrammer
 * changed VPOPMAIL to INDIMAIL
 *
 * Revision 1.3  2001-11-24 12:18:59+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.2  2001-11-20 10:54:55+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.1  2001-10-24 18:14:59+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>

#ifndef	lint
static char     sccsid[] = "$Id: get_Mplexdir.c,v 2.5 2008-09-14 19:47:12+05:30 Cprogrammer Stab mbhangui $";
#endif

char           *
get_Mplexdir(username, domain, creflag, uid, gid)
	char           *username, *domain;
	int             creflag;
	uid_t           uid;
	gid_t           gid;
{
	int             ch, len;
	char           *base_path, *DirName;
	char           *FileSystems[] = {
		"A2E",
		"F2K",
		"L2P",
		"Q2S",
		"T2Zsym",
	};

	if (!*username)
		return ((char *) 0);
	getEnvConfigStr(&base_path, "BASE_PATH", BASE_PATH);
	ch = tolower(*username);
	if (ch >= 'a' && ch <= 'e')
	{
		len = slen(base_path) + slen(FileSystems[0]) + slen(domain) + 3;
		if((DirName = (char *) malloc(len)) != (char *) NULL)
			snprintf(DirName, len, "%s/%s/%s", base_path, FileSystems[0], domain);
		else
			perror("malloc");
	} else
	if (ch >= 'f' && ch <= 'k')
	{
		len = slen(base_path) + slen(FileSystems[1]) + slen(domain) + 3;
		if((DirName = (char *) malloc(len)) != (char *) NULL)
			snprintf(DirName, len, "%s/%s/%s", base_path, FileSystems[1], domain);
		else
			perror("malloc");
	} else
	if (ch >= 'l' && ch <= 'p')
	{
		len = slen(base_path) + slen(FileSystems[2]) + slen(domain) + 3;
		if((DirName = (char *) malloc(len)) != (char *) NULL)
			snprintf(DirName, len, "%s/%s/%s", base_path, FileSystems[2], domain);
		else
			perror("malloc");
	} else
	if (ch >= 'q' && ch <= 's')
	{
		len = slen(base_path) + slen(FileSystems[3]) + slen(domain) + 3;
		if((DirName = (char *) malloc(len)) != (char *) NULL)
			snprintf(DirName, len, "%s/%s/%s", base_path, FileSystems[3], domain);
		else
			perror("malloc");
	} else
	{
		len = slen(base_path) + slen(FileSystems[4]) + slen(domain) + 3;
		if((DirName = (char *) malloc(len)) != (char *) NULL)
			snprintf(DirName, len, "%s/%s/%s", base_path, FileSystems[4], domain);
		else
			perror("malloc");
	}
	if (creflag && DirName)
	{
		umask(INDIMAIL_UMASK);
		r_mkdir(DirName, INDIMAIL_DIR_MODE, uid, gid);
	}
	return (DirName);
}

void
getversion_get_Mplexdir_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
