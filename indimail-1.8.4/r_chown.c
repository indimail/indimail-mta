/*
 * $Log: r_chown.c,v $
 * Revision 1.3  2001-11-24 12:19:55+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.2  2001-11-20 10:55:47+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.1  2001-10-24 18:15:07+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#ifndef	lint
static char     sccsid[] = "$Id: r_chown.c,v 1.3 2001-11-24 12:19:55+05:30 Cprogrammer Stab mbhangui $";
#endif

/*
 * Recursive change ownership utility 
 */
int
r_chown(path, owner, group)
	char           *path;
	uid_t           owner;
	gid_t           group;
{
	DIR            *entry;
	struct dirent  *dp;
	struct stat     statbuf;
	int             len;
	char           *file_name;

	if(chown(path, owner, group))
		return (-1);
	if(!(entry = opendir(path)))
		return (-1);
	for(file_name = 0;;)
	{
		if(!(dp = readdir(entry)))
			break;
		if(!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, ".."))
			continue;
		len = slen(path) + slen(dp->d_name) + 2;
		if(!(file_name = (char *) realloc(file_name, (len * sizeof(char)))))
		{
			closedir(entry);
			return (-1);
		}
		snprintf(file_name, len, "%s/%s", path, dp->d_name);
		if(lstat(file_name, &statbuf))
			continue;
		if (S_ISDIR(statbuf.st_mode))
		{
			if(r_chown(file_name, owner, group))
			{
				closedir(entry);
				return (-1);
			}
		}
		else
		if(chown(file_name, owner, group))
		{
			closedir(entry);
			return (-1);
		}
	}
	closedir(entry);
	free(file_name);
	return (0);
}

void
getversion_r_chown_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
