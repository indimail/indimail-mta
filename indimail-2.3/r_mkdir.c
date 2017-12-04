/*
 * $Log: r_mkdir.c,v $
 * Revision 2.1  2002-08-25 22:33:41+05:30  Cprogrammer
 * added chmod()
 *
 * Revision 1.3  2001-11-24 12:19:57+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.2  2001-11-20 10:55:49+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.1  2001-10-24 18:15:08+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#ifndef	lint
static char     sccsid[] = "$Id: r_mkdir.c,v 2.1 2002-08-25 22:33:41+05:30 Cprogrammer Stab mbhangui $";
#endif

int
r_mkdir(dir, perm, uid, gid)
	char           *dir;
	mode_t          perm;
	uid_t           uid;
	gid_t           gid;
{
	char           *ptr, *cptr, *path;
	int             len;

	if(!access(dir, F_OK))
		return(0);
	len = slen(dir);
	if(!(path = (char *) malloc(len + 1)))
		return(1);
	(void) scopy(path, dir, len + 1);
	if(*path == '/')
		ptr = path + 1;
	else
		ptr = path;
	for(;*ptr && (cptr = strchr(ptr, '/'));)
	{
		ptr = cptr + 1;
		if(!cptr || !*ptr)
			break;
		*cptr = 0;
		if (access(path, F_OK) && (mkdir(path, perm) || chown(path, uid, gid) || chmod(path, perm)))
		{
			*cptr = '/';
			free(path);
			return(1);
		}
		*cptr = '/';
	}
	(void) free(path);
	if (access(dir, F_OK) && (mkdir(dir, perm) || chown(dir, uid, gid) || chmod(dir, perm)))
		return(1);
	return(0);
}

void
getversion_r_mkdir_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
