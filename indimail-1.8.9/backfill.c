/*
 * $Log: backfill.c,v $
 * Revision 2.3  2009-01-15 08:54:58+05:30  Cprogrammer
 * change for once_only flag in remove_line
 *
 * Revision 2.2  2009-01-15 00:11:55+05:30  Cprogrammer
 * added operation for delete
 *
 * Revision 2.1  2009-01-12 10:38:56+05:30  Cprogrammer
 * function to backfill empty slots in dir_control
 *
 */
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <sys/types.h>
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: backfill.c,v 2.3 2009-01-15 08:54:58+05:30 Cprogrammer Stab mbhangui $";
#endif

char *
backfill(char *username, char *domain, char *path, int operation)
{
	char           *filesys_prefix, *ptr = (char *) 0;
	char            filename[MAX_BUFF];
	static char     tmpbuf[MAX_BUFF];
	int             count, len, fd;
	uid_t           uid;
	gid_t           gid;
	FILE           *fp;

	if(!domain || !*domain)
		return ((char *) 0);
	if (!(ptr = vget_assign(domain, NULL, 0, &uid, &gid)))
	{
		fprintf(stderr, "%s: No such domain\n", domain);
		return((char *) 0);
	}
	if (!(filesys_prefix = GetPrefix(username, path)))
		return((char *) 0);
	snprintf(filename, MAX_BUFF, "%s/.%s", ptr, filesys_prefix);
	if (operation == 1) /*- Delete */
	{
		if (!(fp = fopen(filename, "r")))
			return ((char *) 0);
		for (count = 1;;count++)
		{
			if (!fgets(tmpbuf, MAX_BUFF - 2, fp))
			{
				fclose(fp);
				return ((char *) 0);
			}
			if (tmpbuf[(len = strlen(tmpbuf)) - 1] != '\n')
			{
				fprintf(stderr, "Line No %d in %s Exceeds %d chars\n", count, filename, MAX_BUFF - 2);
				fclose(fp);
				return ((char *) 0);
			}
			if ((ptr = strchr(tmpbuf, '#')))
				*ptr = '\0';
			for (ptr = tmpbuf; *ptr && isspace((int) *ptr); ptr++);
			if (!*ptr)
				continue;
			tmpbuf[len - 1] = 0;
			break;
		}
		fclose(fp);
		if (remove_line(ptr, filename, 1, INDIMAIL_QMAIL_MODE) == 1)
		{
			vread_dir_control(filesys_prefix, &vdir, domain);
			if (vdir.cur_users)
				++vdir.cur_users;
			vwrite_dir_control(filesys_prefix, &vdir, domain, uid, gid);
			return (ptr);
		}
	} else
	if (operation == 2) /*- add */
	{
		(void) strncpy(tmpbuf, path, MAX_BUFF);
		if ((ptr = strstr(tmpbuf, username)))
		{
			if (ptr != tmpbuf)
				ptr--;
			if (*ptr == '/')
				*ptr = 0;
		}
		if ((ptr = strstr(tmpbuf, domain)))
		{
			ptr += strlen(domain);
			if (*ptr == '/')
				ptr++;
			if (ptr && *ptr)
			{
#ifdef FILE_LOCKING
				if ((fd = getDbLock(filename, 1)) == -1)
				{
					error_stack(stderr, "get_write_lock: %s: %s\n", filename, strerror(errno));
					return((char *) 0);
				}
#endif
				if (!(fp = fopen(filename, "a")))
				{
#ifdef FILE_LOCKING
					delDbLock(fd, filename, 1);
#endif
					return((char *) 0);
				}
				fprintf(fp, "%s\n", ptr);
				fclose(fp);
#ifdef FILE_LOCKING
				delDbLock(fd, filename, 1);
#endif
				return(ptr);
			}
		}
	}
	return((char *) 0);
}

void
getversion_backfill_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
