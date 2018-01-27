/*
 * $Log: purge_files.c,v $
 * Revision 2.1  2002-10-16 23:43:48+05:30  Cprogrammer
 * replaced skip of system files with a function skip_system_files()
 *
 * Revision 1.7  2002-03-02 02:08:59+05:30  Cprogrammer
 * corrected problems with Folders ending with 'T' not getting purged
 *
 * Revision 1.6  2001-12-19 16:28:28+05:30  Cprogrammer
 * emails marked as deleted not to be counted in quota calculations
 *
 * Revision 1.5  2001-12-03 09:40:08+05:30  Cprogrammer
 * informational message printing only when verbose is set
 *
 * Revision 1.4  2001-11-24 12:19:54+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.3  2001-11-20 10:55:46+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.2  2001-11-14 19:31:06+05:30  Cprogrammer
 * changed ctime to mtime (in case files are restored)
 *
 * Revision 1.1  2001-10-24 18:15:07+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>

#ifndef	lint
static char     sccsid[] = "$Id: purge_files.c,v 2.1 2002-10-16 23:43:48+05:30 Cprogrammer Stab mbhangui $";
#endif

int
purge_files(char *dirname, int days)
{
	char           *tmpstr;
	DIR            *entry;
	struct dirent  *dp;
	struct stat     statbuf;
	int             len;
	time_t          tmval;

	if (chdir(dirname))
	{
		perror(dirname);
		return (1);
	}
	if (!(entry = opendir(dirname)))
	{
		perror(dirname);
		return (1);
	}
	tmval = time(0);
	for (tmpstr = 0;;)
	{
		if (!(dp = readdir(entry)))
			break;
		if (!strncmp(dp->d_name, ".", 1) ||
			!strncmp(dp->d_name, "..", 2))
			continue;
		else
		if(strncmp(dp->d_name, ".Trash", 6) && skip_system_files(dp->d_name))
			continue;
		len = slen(dp->d_name) + slen(dirname) + 2;
		if(!(tmpstr = (char *) realloc(tmpstr, (len * sizeof(char)))))
		{
			fprintf(stderr, "realloc: %d bytes: %s\n", (int) len, strerror(errno));
			closedir(entry);
			return(-1);
		}
		snprintf(tmpstr, len, "%s/%s", dirname, dp->d_name);
		if(stat(tmpstr, &statbuf))
		{
			perror(tmpstr);
			continue;
		}
		if (S_ISDIR(statbuf.st_mode))
			purge_files(tmpstr, days);
		else
		if (((tmval - statbuf.st_mtime) / (3600 * 24)) >= days)
		{
			if(!unlink(tmpstr) && verbose)
				printf("Removed file %s\n", tmpstr);
		}
	}
	closedir(entry);
	free(tmpstr);
	return (0);
}

void
getversion_purge_files_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
