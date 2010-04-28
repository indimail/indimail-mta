/*
 * $Log: getactualpath.c,v $
 * Revision 2.4  2009-02-18 21:25:33+05:30  Cprogrammer
 * removed compiler warnings
 *
 * Revision 2.3  2009-02-06 11:37:28+05:30  Cprogrammer
 * ignore return value of chdir
 *
 * Revision 2.2  2008-09-14 21:02:23+05:30  Cprogrammer
 * added error messages for all calls
 *
 * Revision 2.1  2002-08-10 18:38:42+05:30  Cprogrammer
 * function to get the actual path (after resolving links etc)
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: getactualpath.c,v 2.4 2009-02-18 21:25:33+05:30 Cprogrammer Stab mbhangui $";
#endif

#include <unistd.h>
#include <errno.h>
#include <string.h>

char    *
getactualpath(char *path)
{
	char            buffer1[MAX_BUFF];
	static char     buffer2[MAX_BUFF];

	if (!getcwd(buffer1, MAX_BUFF))
	{
		error_stack(stderr, "getcwd: %s\n", strerror(errno));
		return ((char *) 0);
	}
	if (chdir(path))
	{
		error_stack(stderr, "chdir:%s: %s\n", path, strerror(errno));
		return ((char *) 0);
	}
	if (!getcwd(buffer2, MAX_BUFF))
	{
		error_stack(stderr, "getcwd: %s\n", strerror(errno));
		if (chdir(buffer1) == -1) ;
		return ((char *) 0);
	}
	if (chdir(buffer1) == -1) ;
	return (buffer2);
}

void
getversion_getactualpath_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
