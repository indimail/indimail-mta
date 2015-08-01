/*
 * $Log: is_already_running.c,v $
 * Revision 2.2  2010-08-15 20:49:53+05:30  Cprogrammer
 * fixed double fclose()
 *
 * Revision 2.1  2009-02-18 21:27:18+05:30  Cprogrammer
 * check return value of fscanf
 *
 * Revision 1.1  2001-12-13 00:28:28+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <stdio.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>

#ifndef lint
static char     sccsid[] = "$Id: is_already_running.c,v 2.2 2010-08-15 20:49:53+05:30 Cprogrammer Stab mbhangui $";
#endif

#define MAX_BUFF 1024
/*
 * 0 - not running
 * 1 - already running
 */
int
is_already_running(char *pgname)
{
	char            filename[MAX_BUFF];
	FILE           *fp;
	int             pid;

	snprintf(filename, MAX_BUFF, "/tmp/%s.PID", pgname);
	if ((fp = fopen(filename, "r")) != (FILE *) 0)
	{
		if (fscanf(fp, "%d", &pid) == 1)
		{
			if (pid && !kill(pid, 0))
			{
				fclose(fp);
				return (pid);
			}
		}
		fclose(fp);
	}
	if ((fp = fopen(filename, "w")) != (FILE *) 0)
	{
		fprintf(fp, "%d\n", (int) getpid());
		fclose(fp);
		return (0);
	}
	return (-1);
}

void
getversion_is_already_running_c()
{
	printf("%s\n", sccsid);
	return;
}
