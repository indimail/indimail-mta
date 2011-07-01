/*
 * $Log: count_dir.c,v $
 * Revision 2.4  2009-10-17 20:16:53+05:30  Cprogrammer
 * include_trash wrongly defined as int
 *
 * Revision 2.3  2009-10-15 08:22:27+05:30  Cprogrammer
 * added option INCLUDE_TRASH to include trash in quota calculations
 *
 * Revision 2.2  2008-06-24 21:45:40+05:30  Cprogrammer
 * porting for 64 bit
 *
 * Revision 2.1  2002-10-16 23:42:55+05:30  Cprogrammer
 * replaced skip of system files with a function skip_system_files()
 *
 * Revision 1.8  2002-03-04 12:50:27+05:30  Cprogrammer
 * corrected compilation if USE_MAILDIRQUOTA was not defined
 *
 * Revision 1.7  2002-02-24 03:24:27+05:30  Cprogrammer
 * Change for incorporating MAILDROP Maildir Quota
 *
 * Revision 1.6  2002-02-23 23:54:05+05:30  Cprogrammer
 * corrected bug where any folder or file ending with 'T' was being treated as trash
 *
 * Revision 1.5  2001-12-19 16:27:37+05:30  Cprogrammer
 * emails marked as deleted not to be counted in quota calculation
 *
 * Revision 1.4  2001-11-24 12:18:00+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.3  2001-11-20 10:54:05+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.2  2001-11-02 16:29:57+05:30  Cprogrammer
 * Removed Trash folder in the Quota calculation for folders
 *
 * Revision 1.1  2001-10-24 18:14:55+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"
#include <stdio.h>
#include <errno.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>

#ifndef	lint
static char     sccsid[] = "$Id: count_dir.c,v 2.4 2009-10-17 20:16:53+05:30 Cprogrammer Stab mbhangui $";
#endif

mdir_t count_dir(char *dir_name, mdir_t *mailcount)
{
	DIR            *entry;
	struct dirent  *dp;
	struct stat     statbuf;
	mdir_t          file_size, len, flen, count;
	int             is_trash;
	char           *tmpstr, *ptr, *include_trash;

	if (!dir_name || !*dir_name)
		return (0);
	if (!(entry = opendir(dir_name)))
		return (0);
	if ((include_trash = (char *) getenv("INCLUDE_TRASH")))
		is_trash = 0;
	else
	{
		if (strstr(dir_name, "/Maildir/.Trash"))
			is_trash = 1;
		else
			is_trash = 0;
	}
	if (mailcount)
		*mailcount = 0;
	for (file_size = 0, tmpstr = 0;;)
	{
		if (!(dp = readdir(entry)))
			break;
		if (!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, ".."))
			continue;
		else
		if (skip_system_files(dp->d_name))
			continue;
		flen = slen(dp->d_name);
		len = flen + slen(dir_name) + 2;
		if (!(tmpstr = (char *) realloc(tmpstr, (len * sizeof(char)))))
		{
			fprintf(stderr, "realloc: %d bytes: %s\n", (int) len, strerror(errno));
			closedir(entry);
			return(-1);
		}
		snprintf(tmpstr, len, "%s/%s", dir_name, dp->d_name);
		if ((ptr = strstr(tmpstr, ",S=")) != NULL)
		{
			ptr += 3;
			file_size += atoi(ptr);
			if (mailcount)
				(*mailcount)++;
		} else
		if (!stat(tmpstr, &statbuf))
		{
			if (S_ISDIR(statbuf.st_mode))
			{
				file_size += count_dir(tmpstr, &count);
				if (mailcount)
					*mailcount += count;
			} else
			{
				if (!include_trash && (*(dp->d_name + flen - 1) == 'T' || is_trash))
					continue;
				if (mailcount)
					(*mailcount)++;
				file_size += statbuf.st_size;
			}
		}
	}
	closedir(entry);
	free(tmpstr);
	return (file_size);
}

void
getversion_count_dir_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
