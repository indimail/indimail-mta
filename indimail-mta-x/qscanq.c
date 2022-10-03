/*
 * $Log: qscanq.c,v $
 * Revision 1.11  2022-10-03 12:25:19+05:30  Cprogrammer
 * fixed return exit codes
 *
 * Revision 1.10  2021-06-27 10:38:40+05:30  Cprogrammer
 * uidnit new argument to disable/enable error on missing uids
 *
 * Revision 1.9  2021-06-12 19:24:08+05:30  Cprogrammer
 * minor optimization by removing call to extra env_get
 *
 * Revision 1.8  2020-09-16 19:06:34+05:30  Cprogrammer
 * fix compiler warning for FreeBSD
 *
 * Revision 1.7  2020-06-08 22:51:53+05:30  Cprogrammer
 * handle chdir error
 *
 * Revision 1.6  2017-05-04 20:20:40+05:30  Cprogrammer
 * close passwd, group database
 *
 * Revision 1.5  2014-07-27 15:16:57+05:30  Cprogrammer
 * change to qscand uid if run through non qscand user
 *
 * Revision 1.4  2005-04-25 22:53:49+05:30  Cprogrammer
 * added error check for setreuid()
 *
 * Revision 1.3  2004-10-22 20:29:47+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.2  2004-09-28 22:22:33+05:30  Cprogrammer
 * use SCANDIR to select the virus scan directory
 *
 * Revision 1.1  2004-09-24 14:04:54+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "substdio.h"
#include "auto_spool.h"
#include "auto_uids.h"
#include "auto_fnlen.h"
#include "mkfn.h"
#include "auto_retries.h"
#include "wait.h"
#include "sig.h"
#include "strerr.h"
#include "env.h"
#include "exitcodes.h"

#define FATAL "qscanq: fatal: "

int             do_cleanq();

extern char    *const auto_qstdin[];
int             flaglog = 0;
char            fn[FN_BYTES];
static char     sserrbuf[512];
static substdio sserr = SUBSTDIO_FDBUF(write, 2, sserrbuf, sizeof(sserrbuf));

void
err(s)
	char           *s;
{
	if (!flaglog)
		return;
	if (substdio_putsflush(&sserr, s) == -1)
		strerr_die2sys(QQ_XTEMP, FATAL, "unable to write stderr: ");
}

/*
 * Global so cleanup routine can find it. 
 */
void
reset_sticky(char *scandir)
{
	if (chdir(scandir) == -1) {
		if (flaglog)
			strerr_die4sys(QQ_XTEMP, FATAL, "unable to chdir to ", (char *) scandir, ": ");
		_exit(QQ_XTEMP);	/*- temporary refusal to handle messages */
	}
	if (chmod(fn, 0700) == -1)
		;
}

void
real_cleanup(char *scandir)
{
	reset_sticky(scandir);
	do_cleanq();
}

int
main(int argc, char *argv[])
{
	int             c = 1, qstat = -1;
	uid_t           uid;
	pid_t           pid = 0;
	char           *ptr;

	if (uidinit(1, 1) == -1) {
		if (flaglog)
			strerr_die2sys(QQ_XTEMP, FATAL, "unable to get uid/gid: ");
		_exit(QQ_XTEMP);
	}
	/*- Check whether we should be logging errors */
	if (env_get("DEBUG"))
		flaglog = 1;
	umask(0);

	uid = getuid();
	if (uid != auto_uidc && setreuid(auto_uidc, auto_uidc)) {
		if (flaglog)
			strerr_die2sys(QQ_XTEMP, FATAL, "setreuid failed: ");
		_exit(QQ_XTEMP);
	}
	if (!(ptr = env_get("SCANDIR")))
		ptr = (char *) auto_spool;
	/*- [ cwd := spool folder ] */
	if (chdir(ptr) < 0) {
		if (flaglog)
			strerr_die4sys(QQ_XTEMP, FATAL, "unable to chdir to ", (char *) auto_spool, ": ");
		_exit(QQ_XTEMP);	/*- temporary refusal to handle messages */
	}

	/*- [fn := name timestamp.ppid.n of existing directory w/sticky bit set ] */
	do {
		mkfn(fn, c++);
		if (!mkdir(fn, 0700) && !chmod(fn, 01700))
			break;
		sleep(1);
	} while (c < MAX_RETRIES);
	if (c == MAX_RETRIES) {
		if (flaglog)
			strerr_die4sys(QQ_XTEMP, FATAL, "unable to mkdir ", fn, ": ");
		_exit(QQ_XTEMP);
	}
	/*- [ cwd := fn ] */
	if (chdir(fn) < 0) {
		if (flaglog)
			strerr_die4sys(QQ_XTEMP, FATAL, "unable to chdir to ", fn, ": ");
		rmdir(fn);				/*- if rmdir fails, bummer.  */
		reset_sticky(ptr);
		_exit(QQ_XTEMP);
	}
	/*- [ cwd := fn/work */
	if (mkdir("work", 0777) == -1) {
		if (flaglog)
			strerr_die4sys(QQ_XTEMP, FATAL, "unable to mkdir ", fn, "/work: ");
		reset_sticky(ptr);
		_exit(QQ_XTEMP);
	}
	if (chdir("work") == -1) {
		if (flaglog)
			strerr_die4sys(QQ_XTEMP, FATAL, "unable to chdir to ", fn, "/work: ");
		rmdir("work");
		real_cleanup(ptr);
		_exit(QQ_XTEMP);
	}
	/*
	 * Run a non-privileged process to do actual scanning 
	 */
	switch (pid = vfork())
	{
	case -1:
		return 0;
	case 0: /*- Run with indimail uid */
		if (setreuid(getuid(), getuid())) {
			if (flaglog)
				strerr_die2sys(QQ_XTEMP, FATAL, "setreuid failed: ");
			_exit(QQ_XTEMP);
		}
		execv(*auto_qstdin, argv);
		if (flaglog)
			strerr_die2sys(QQ_XTEMP, FATAL, "execv failed: ");
		real_cleanup(ptr);
		_exit(QQ_XTEMP); /*- hopefully never reached */ ;
	}
	/*- Catch the exit status */
	if (wait_pid(&qstat, pid) == -1) {
		if (flaglog)
			strerr_die2sys(QQ_XTEMP, FATAL, "waidpid failed: ");
		real_cleanup(ptr);
		_exit(QQ_XTEMP);
	}
	real_cleanup(ptr);
	if (wait_crashed(qstat)) {
		if (flaglog)
			err("qscanq-stdin crashed.\n");
		_exit(QQ_XTEMP);
	}
	_exit(wait_exitcode(qstat));
}

void
getversion_qscanq_c()
{
	static char    *x = "$Id: qscanq.c,v 1.11 2022-10-03 12:25:19+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
