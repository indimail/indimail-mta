/*
 * $Log: FifoCreate.c,v $
 * Revision 2.7  2009-02-18 09:07:00+05:30  Cprogrammer
 * check chown error
 *
 * Revision 2.6  2009-02-06 11:37:19+05:30  Cprogrammer
 * ignore return value of chown
 *
 * Revision 2.5  2004-05-17 14:01:00+05:30  Cprogrammer
 * changed VPOPMAIL to INDIMAIL
 *
 * Revision 2.4  2003-07-31 09:46:05+05:30  Cprogrammer
 * bug fix - incorrect handling of errno
 *
 * Revision 2.3  2002-12-24 00:40:20+05:30  Cprogrammer
 * corrected problem if fifo was not an absolute path
 *
 * Revision 2.2  2002-12-23 20:52:39+05:30  Cprogrammer
 * fifoname variable was not correctly reset with '/' char in all cases after returning from the function
 *
 * Revision 2.1  2002-07-04 00:30:24+05:30  Cprogrammer
 * return error if file exists and is not a fifo
 *
 * Revision 1.2  2002-04-10 02:56:41+05:30  Cprogrammer
 * set ownership to indimail by default for any fifo creation
 *
 * Revision 1.1  2002-04-08 23:29:48+05:30  Cprogrammer
 * Initial revision
 *
 */

#ifndef	lint
static char     sccsid[] = "$Id: FifoCreate.c,v 2.7 2009-02-18 09:07:00+05:30 Cprogrammer Stab mbhangui $";
#endif

#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include "indimail.h"

int
FifoCreate(char *fifoname)
{
	char           *ptr;
	struct stat     statbuf;
	int             status;

	errno = 0;
	if (!(status = mkfifo(fifoname, INDIMAIL_QMAIL_MODE)))
	{
		if (!getuid() || !geteuid())
		{
			if (indimailuid == -1 || indimailgid == -1)
				GetIndiId(&indimailuid, &indimailgid);
			if (chown(fifoname, indimailuid, indimailgid))
				return (-1);
		}
		errno = 0;
		return (0);
	} else
	if (errno == EEXIST)
	{
		if (stat(fifoname, &statbuf))
			return(-1);
		if (S_ISFIFO(statbuf.st_mode))
		{
			if (!getuid() || !geteuid())
			{
				if (indimailuid == -1 || indimailgid == -1)
					GetIndiId(&indimailuid, &indimailgid);
				if (chown(fifoname, indimailuid, indimailgid))
					return (-1);
			}
			return (0);
		}
		errno = EEXIST;
		return(-1);
	} else
	if (errno == ENOENT)
	{
		if ((ptr = strrchr(fifoname, '/')))
			*ptr = 0;
		if (access(fifoname, F_OK))
		{
			if (indimailuid == -1 || indimailgid == -1)
				GetIndiId(&indimailuid, &indimailgid);
			if (r_mkdir(fifoname, 0755, indimailuid, indimailgid))
			{
				if (ptr)
					*ptr = '/';
				return (-1);
			}
			if (ptr)
				*ptr = '/';
			if (mkfifo(fifoname, INDIMAIL_QMAIL_MODE))
				return (-1);
			else
			{
				if (!getuid() || !geteuid())
					if (chown(fifoname, indimailuid, indimailgid))
						return (-1);
				return(0);
			}
		}
		if (ptr)
			*ptr = '/';
		return (-1);
	}
	return (-1);
}

void
getversion_FifoCreate_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
