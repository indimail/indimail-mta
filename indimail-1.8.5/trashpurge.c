/*
 * $Log: trashpurge.c,v $
 * Revision 2.2  2007-12-22 00:40:50+05:30  Cprogrammer
 * added to delete trash files older than a period
 *
 * Revision 2.1  2006-01-23 21:49:46+05:30  Cprogrammer
 * added mailboxpurge function
 *
 * Revision 1.2  2001-12-24 00:57:03+05:30  Cprogrammer
 * code revamped
 *
 * Revision 1.1  2001-12-03 04:16:46+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"
#include <errno.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>

#ifndef	lint
static char     sccsid[] = "$Id: trashpurge.c,v 2.2 2007-12-22 00:40:50+05:30 Cprogrammer Stab mbhangui $";
#endif

long
mailboxpurge(char *dir, char *mailbox, long age, int fast_option)
{
	char            tmpbuf[MAX_BUFF];
	char           *file_name, *ptr;
	DIR            *entry;
	struct dirent  *dp;
	struct stat     statbuf;
	int             i, len;
	long            deleted, tmpdate;
	time_t          tmval;
	char           *MailDirNames[] = {
		"cur",
		"new",
		"tmp",
	};

	if (access(dir, F_OK))
		return(0);
	tmval = time(0);
	for (deleted = 0l, file_name = 0, i = 0; i < 3; i++)
	{
		snprintf(tmpbuf, MAX_BUFF, "%s/Maildir/%s/%s", dir, mailbox, MailDirNames[i]);
		if (!(entry = opendir(tmpbuf)))
		{
			if (errno != 2)
				fprintf(stderr, "opendir: %s: %s\n", tmpbuf, strerror(errno));
			continue;
		}
		for (;;)
		{
			if (!(dp = readdir(entry)))
				break;
			if (!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, ".."))
				continue;
			len = slen(tmpbuf) + slen(dp->d_name) + 2;
			if (!(file_name = (char *) realloc(file_name, (len * sizeof(char)))))
			{
				fprintf(stderr, "realloc: %d bytes: %s\n", len, strerror(errno));
				closedir(entry);
				break;
			}
			snprintf(file_name, len, "%s/%s", tmpbuf, dp->d_name);
			if (!fast_option)
			{
				if (lstat(file_name, &statbuf))
				{
					fprintf(stderr, "lstat: %s: %s\n", file_name, strerror(errno));
					continue;
				}
				if (S_ISDIR(statbuf.st_mode))
					continue;
			}
			if (age)
			{
				tmpdate = atol(dp->d_name); /*- If the this fails do not delete the file */
				if ((age && ((tmval - tmpdate)/86400 <= age)) || (tmpdate == 0L))
					continue;
			}
			if (!unlink(file_name))
			{
				if (fast_option)
				{
					if (!(ptr = strchr(dp->d_name, '='))) 
					{
						if (lstat(file_name, &statbuf))
						{
							fprintf(stderr, "lstat: %s: %s\n", file_name, strerror(errno));
							continue;
						}
						deleted += statbuf.st_size;
					} else
						deleted += atol(ptr + 1);
				} else
					deleted += statbuf.st_size;
			} else
				fprintf(stderr, "unlink: %s: %s\n", file_name, strerror(errno));
		} /*- for (;;) */
		closedir(entry);
	}
	free(file_name);
	return(deleted);
}

long
trashpurge(char *dir)
{
	return(mailboxpurge(dir, ".Trash", -1, 0));
}

void
getversion_trashpurge_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
