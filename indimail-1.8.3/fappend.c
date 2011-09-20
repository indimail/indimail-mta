/*
 * $Log: fappend.c,v $
 * Revision 1.2  2001-11-20 11:02:35+05:30  Cprogrammer
 * Added getversion_fappend_c()
 *
 * Revision 1.1  2001-10-24 18:14:59+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#ifndef	lint
static char     sccsid[] = "$Id: fappend.c,v 1.2 2001-11-20 11:02:35+05:30 Cprogrammer Stab mbhangui $";
#endif

/* function to append a file to another file */
int
fappend(readfile, appndfile, mode, perm, uid, gid)
	char           *readfile, *appndfile, *mode;
	mode_t          perm;
	uid_t           uid;
	gid_t           gid;
{
	char            buffer[2048];
	int             fin, fout;
#ifdef SUN41
	int             num;
#else
	unsigned        num;
#endif

#if defined(CYGWIN) || defined(WindowsNT)
	if ((fin = open(readfile, O_RDONLY|O_BINARY, 0)) == -1)
#else
	if ((fin = open(readfile, O_RDONLY, 0)) == -1)
#endif
		return (-1);
	if (*mode == 'w')
	{
#if defined(CYGWIN) || defined(WindowsNT)
		if((fout = open(appndfile, O_CREAT|O_TRUNC|O_BINARY|O_WRONLY, perm)) == -1)
			return(-1);
#else
		if((fout = open(appndfile, O_CREAT|O_TRUNC|O_WRONLY, perm)) == -1)
			return(-1);
#endif
	} else
	{
#if defined(CYGWIN) || defined(WindowsNT)
	if ((fout = open(appndfile, O_CREAT|O_APPEND|O_BINARY|O_WRONLY, perm)) == -1)
#else
	if ((fout = open(appndfile, O_CREAT|O_APPEND|O_WRONLY, perm)) == -1)
#endif
		return (-1);
	}
	if(chown(appndfile, uid, gid))
		fprintf(stderr, "chown: %s (%d %d): %s\n", appndfile, (int) uid, (int) gid, strerror(errno));
	while ((num = read(fin, buffer, sizeof(buffer))) >= 1)
		if (write(fout, buffer, num) != num)
		{
			(void) close(fin);
			(void) close(fout);
			return (-1);
		}
	(void) close(fin);
	(void) close(fout);
	return (0);
}

void
getversion_fappend_c()
{
	printf("%s\n", sccsid);
}
