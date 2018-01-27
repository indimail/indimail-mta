/*
 * $Log: signal_process.c,v $
 * Revision 1.3  2001-11-24 12:20:05+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.2  2001-11-20 10:56:00+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.1  2001-10-24 18:15:10+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

#ifndef	lint
static char     sccsid[] = "$Id: signal_process.c,v 1.3 2001-11-24 12:20:05+05:30 Cprogrammer Stab mbhangui $";
#endif

/*
 * Send a signal to a process utility function
 *
 * name    = name of process
 * sig_num = signal number 
 */
int
signal_process(name, sig_num)
	char           *name;
	int             sig_num;
{
	FILE           *ps;
	static char     pid[MAX_BUFF], TmpBuf[MAX_BUFF];
	char           *tmpstr;
	int             out, col;
	int             pid_col = 0;
	pid_t           tmppid;

	memset(pid, 0, MAX_BUFF);
	if (!(ps = popen(PS_COMMAND, "r")))
	{
		perror("popen on ps command");
		return (-1);
	}

	if (fgets(TmpBuf, MAX_BUFF, ps) != NULL)
	{
		col = 0;
		tmpstr = strtok(TmpBuf, TOKENS);
		while (tmpstr != NULL)
		{
			if (strcmp(tmpstr, "PID") == 0)
				pid_col = col;
			tmpstr = strtok(NULL, TOKENS);
			++col;
		}
	}
	out = 0;
	while (fgets(TmpBuf, MAX_BUFF, ps) != NULL && out == 0)
	{
		if (strstr(TmpBuf, name) != NULL)
		{
			tmpstr = strtok(TmpBuf, TOKENS);
			col = 0;
			do
			{
				if (col == pid_col)
				{
					scopy(pid, tmpstr, MAX_BUFF);
					break;
				}
				++col;
				tmpstr = strtok(NULL, TOKENS);
			}
			while (tmpstr != NULL && out == 0);
		}
	}
	pclose(ps);
	tmppid = atoi(pid);
	if (tmppid != 0)
		if (kill(tmppid, sig_num) != 0)
			perror("kill");
	return (0);
}

void
getversion_signal_process_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
