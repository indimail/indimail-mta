/*
 * $Log: do_ripmime.c,v $
 * Revision 1.2  2021-08-30 01:07:47+05:30  Cprogrammer
 * renamed pid as cmd_pid
 *
 * Revision 1.1  2008-08-03 18:27:30+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <sys/types.h>
#include <unistd.h>
#include <wait.h>
#include <sig.h>
#include <strerr.h>
#include "exitcodes.h"

extern int      flaglog;
extern pid_t    cmd_pid;
extern char    *auto_ripmime_cmd[];

int
do_ripmime()
{
	int             rmstat = -1;

	/*- Defer alarms */
	sig_alarmblock();
	/*- Launch ripmime */
	switch ((cmd_pid = vfork()))
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
	if (wait_pid(&rmstat, cmd_pid) == -1)
		return 0;
	cmd_pid = 0;
	if (wait_crashed(rmstat))
		return 0;
	if (wait_exitcode(rmstat) != 0)
		return 0;
	return 1;
}

void
getversion_do_ripmime_c()
{
	const char     *x = "$Id: do_ripmime.c,v 1.2 2021-08-30 01:07:47+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
