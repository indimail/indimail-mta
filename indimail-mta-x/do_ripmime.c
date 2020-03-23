/*
 * $Log: do_ripmime.c,v $
 * Revision 1.1  2008-08-03 18:27:30+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <sys/types.h>
#include <unistd.h>
#include "wait.h"
#include "sig.h"
#include "exitcodes.h"
#include "strerr.h"

extern int      flaglog;
extern int      alarm_flag;
extern pid_t    pid;
extern char    *auto_ripmime_cmd[];

int
do_ripmime()
{
	int             rmstat = -1;

	/*- Defer alarms */
	sig_alarmblock();
	/*- Launch ripmime */
	switch (pid = vfork())
	{
	case -1:
		if (flaglog)
			strerr_warn1("qscanq-stdin: fatal: vfork failed: ", &strerr_sys);
		return 0;
	case 0:
		close(1); /*- Don't let it fiddle with envelope */
		close(2); /*- Don't let it squawk */
		execve(*auto_ripmime_cmd, auto_ripmime_cmd, 0);
		if (flaglog)
			strerr_warn1("qscanq-stdin: fatal: could not execve MIME extractor: ", &strerr_sys);
		_exit(QQ_XBUGS); /*- hopefully never reached */ ;
	}
	/*- Allow alarms */
	sig_alarmunblock();
	/*- Catch the exit status */
	if (wait_pid(&rmstat, pid) == -1)
		return 0;
	pid = 0;
	if (wait_crashed(rmstat))
		return 0;
	if (wait_exitcode(rmstat) != 0)
		return 0;
	return 1;
}

void
getversion_do_ripmime_c()
{
	static char    *x = "$Id: do_ripmime.c,v 1.1 2008-08-03 18:27:30+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
