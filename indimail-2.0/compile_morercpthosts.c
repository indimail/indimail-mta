/*
 * $Log: compile_morercpthosts.c,v $
 * Revision 2.2  2017-03-13 13:38:04+05:30  Cprogrammer
 * use PREFIX for bin programs
 *
 * Revision 2.1  2005-12-29 22:40:21+05:30  Cprogrammer
 * use getEnvConfigStr to set variables from environment variables
 *
 * Revision 1.3  2001-11-24 12:17:58+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.2  2001-11-20 10:53:42+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.1  2001-10-24 18:14:54+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

#ifndef	lint
static char     sccsid[] = "$Id: compile_morercpthosts.c,v 2.2 2017-03-13 13:38:04+05:30 Cprogrammer Exp mbhangui $";
#endif

/*
 * compile the morercpthosts file using qmail-newmrh program
 */
int
compile_morercpthosts()
{
	int             pid;

	pid = vfork();
	if (pid == 0)
	{
		execl(PREFIX"/bin/qmail-newmrh", "qmail-newmrh", NULL);
		exit(127);
	} else
		wait(&pid);
	return (0);
}

void
getversion_compile_morercpthosts_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
