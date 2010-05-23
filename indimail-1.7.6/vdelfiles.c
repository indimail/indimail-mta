/*
 * $Log: vdelfiles.c,v $
 * Revision 1.6  2001-12-03 04:18:20+05:30  Cprogrammer
 * removed redundant return statement
 *
 * Revision 1.5  2001-11-29 13:21:11+05:30  Cprogrammer
 * added verbose switch
 *
 * Revision 1.4  2001-11-28 23:08:47+05:30  Cprogrammer
 * arguments user and domain added to prevent accidental deletion of files
 *
 * Revision 1.3  2001-11-24 12:21:46+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.2  2001-11-20 11:00:15+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.1  2001-10-24 18:15:33+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"
#include <stdio.h>
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#ifndef	lint
static char     sccsid[] = "$Id: vdelfiles.c,v 1.6 2001-12-03 04:18:20+05:30 Cprogrammer Stab mbhangui $";
#endif

/*
 * vdelfiles : delete a directory tree
 *
 * input: directory to start the deletion
 * output: 
 *         0 on success
 *        -1 on failer
 */
int
vdelfiles(dir, user, domain)
	char           *dir, *user, *domain;
{
	DIR            *entry;
	struct dirent  *dp;
	struct stat     statbuf;
	int             len;
	char           *file_name;

	if (!strcmp(dir, "/") || !strcmp(dir, "/usr") || !strcmp(dir, "/var") || !strcmp(dir, "/mail")) /* safety */
		return(-1);
	if (user && *user)
	{
		if (!strstr(dir, user))
			return(-1);
	}
	if (domain && *domain)
	{
		if (!strstr(dir, domain))
			return(-1);
	}
	if (lstat(dir, &statbuf) == -1)
	{
		if (errno == 2)
			return(0);
		return (-1);
	}
	if (!S_ISDIR(statbuf.st_mode))
	{
		if (verbose)
			printf("Removing File %s\n", dir);
		if (unlink(dir))
		{
			fprintf(stderr, "Failed to remove %s: %s\n", dir, strerror(errno));
			return(1);
		}
		return(0);
	}
	if (!(entry = opendir(dir)))
	{
		fprintf(stderr, "opendir: %s: %s\n", dir, strerror(errno));
		return (-1);
	}
	for (file_name = 0;;)
	{
		if (!(dp = readdir(entry)))
			break;
		if (!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, ".."))
			continue;
		len = slen(dir) + slen(dp->d_name) + 2;
		if (!(file_name = (char *) realloc(file_name, (len * sizeof(char)))))
		{
			fprintf(stderr, "realloc: %d bytes: %s\n", len, strerror(errno));
			closedir(entry);
			return(-1);
		}
		snprintf(file_name, len, "%s/%s", dir, dp->d_name);
		if (lstat(file_name, &statbuf))
		{
			fprintf(stderr, "lstat: %d bytes: %s\n", len, strerror(errno));
			continue;
		}
		if (!S_ISDIR(statbuf.st_mode))
		{
			if (verbose)
				printf("Removing File %s\n", file_name);
			if (unlink(file_name) == -1)
				fprintf(stderr, "unlink: %s: %s\n", file_name, strerror(errno));
			continue;
		} 
		if (vdelfiles(file_name, user, domain) == -1)
		{
			free(file_name);
			closedir(entry);
			return (-1);
		}
	}
	free(file_name);
	closedir(entry);
	if (verbose)
		printf("Removing Dir %s\n", dir);
	rmdir(dir);
	return (0);
}

void
getversion_vdelfiles_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
