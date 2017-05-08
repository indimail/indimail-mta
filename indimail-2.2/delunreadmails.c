/*
 * $Log: delunreadmails.c,v $
 * Revision 1.3  2001-12-29 11:04:43+05:30  Cprogrammer
 * added switch to delete unread mails from cur
 *
 * Revision 1.2  2001-12-24 00:56:21+05:30  Cprogrammer
 * code revamped
 *
 * Revision 1.1  2001-12-03 04:16:56+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"
#include <stdlib.h>
#include <pwd.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>

#ifndef	lint
static char     sccsid[] = "$Id: delunreadmails.c,v 1.3 2001-12-29 11:04:43+05:30 Cprogrammer Stab mbhangui $";
#endif

long
Delunreadmails(char *dir, int type, int days)
{
	char            tmpbuf[MAX_BUFF];
	char           *file_name;
	time_t          curtime;
	DIR            *entry;
	struct dirent  *dp;
	long            deleted;
	struct stat     statbuf;
	int             len;

	if(type == 1)
		snprintf(tmpbuf, MAX_BUFF, "%s/Maildir/new", dir);
	else
		snprintf(tmpbuf, MAX_BUFF, "%s/Maildir/cur", dir);
	if (!(entry = opendir(tmpbuf)))
	{
		if(errno == 2)
			return(0);
		fprintf(stderr, "opendir: %s: %s\n", tmpbuf, strerror(errno));
		return (-1);
	}
	curtime = time(0);
	for(file_name = 0, deleted = 0l;;)
	{
		if(!(dp = readdir(entry)))
			break;
		if(!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, ".."))
			continue;
		len = slen(tmpbuf) + slen(dp->d_name) + 2;
		if(!(file_name = (char *) realloc(file_name, (len * sizeof(char)))))
		{
			fprintf(stderr, "realloc: %d bytes: %s\n", len, strerror(errno));
			closedir(entry);
			return(-1);
		}
		snprintf(file_name, len, "%s/%s", tmpbuf, dp->d_name);
		if(type == 2 && file_name[len - 2] != ',')
				continue;
		if(lstat(file_name, &statbuf))
		{
			fprintf(stderr, "lstat: %s: %s\n", file_name, strerror(errno));
			continue;
		}
		if (!S_ISDIR(statbuf.st_mode) && ((curtime - statbuf.st_mtime) > days * 86400))
		{
			if(verbose)
				printf("Removing File %s\n", file_name);
			if(!unlink(file_name))
				deleted += statbuf.st_size;
			else
				fprintf(stderr, "unlink: %s: %s\n", file_name, strerror(errno));
		}
	}
	closedir(entry);
	free(file_name);
	return(deleted);
}

void
getversion_delunreadmails_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
	return;
}
