/*
 * $Log: qscanq-stdin.c,v $
 * Revision 1.9  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.8  2021-08-29 23:27:08+05:30  Cprogrammer
 * define functions as noreturn
 *
 * Revision 1.7  2021-06-12 19:25:48+05:30  Cprogrammer
 * removed chdir(auto_qmail)
 *
 * Revision 1.6  2021-06-09 19:36:55+05:30  Cprogrammer
 * use qmulti() instead of exec of qmail-multi
 *
 * Revision 1.5  2020-11-24 13:47:46+05:30  Cprogrammer
 * removed exit.h
 *
 * Revision 1.4  2016-06-03 09:58:33+05:30  Cprogrammer
 * moved qmail-multi to sbin
 *
 * Revision 1.3  2005-02-14 23:06:19+05:30  Cprogrammer
 * unset VIRUSCHECK to prevent recursive calls to qscanq
 *
 * Revision 1.2  2004-10-22 20:29:47+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.1  2004-09-22 23:26:46+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <substdio.h>
#include <open.h>
#include <fd.h>
#include <wait.h>
#include <seek.h>
#include <sig.h>
#include <env.h>
#include <strerr.h>
#include <noreturn.h>
#include "mkfn.h"
#include "exitcodes.h"
#include "auto_fnlen.h"
#include "auto_ageout.h"
#include "qmulti.h"

#define FATAL "qscanq: fatal: "

int             do_ripmime();
int             do_scan();
int             do_cleanq();

static char     buf1[256];
static substdio ss1 = SUBSTDIO_FDBUF(write, 1, buf1, sizeof(buf1));
static int      alarm_flag;
int             flaglog = 0;
pid_t           cmd_pid;

no_return void
die_write()
{
	do_cleanq();
	_exit(53);
}

no_return void
die_read()
{
	do_cleanq();
	_exit(54);
}

void
err(s)	/*- was named puts, but Solaris pwd.h includes stdio.h. dorks.  */
	char           *s;
{
	if (!flaglog)
		return;
	if (substdio_puts(&ss1, s) == -1)
		_exit(111);
}

/*- Signal handler just sets a flag.  */
void
alarm_handler(int sig)
{
	if (cmd_pid)
		kill(cmd_pid, SIGKILL);
	else
		_exit(QQ_XTEMP);
}

/*
 * The current directory is now "work", under a subdirectory of the
 * virus scanning queue. A message is on stdin, and its envelope on
 * file descriptor 2. Our environment is empty. Our job is to extract
 * the MIME parts of the message, and run the virus scanner in cwd.
 */
int
main(int argc, char *argv[])
{
	int             fdout = -1, n = 0;
	char            inbuf[2048], outbuf[256], fn[FN_BYTES];
	struct substdio ssin, ssout;

	cmd_pid = 0;
	/*- Set an alarm handler */
	alarm_flag = 0;
	sig_alarmcatch(alarm_handler);
	alarm(MAX_AGE);
	/*- Check whether we should be logging errors */
	if (env_get("DEBUG"))
		flaglog = 1;
	/*
	 * Set up stdin with seekable copy of message
	 * [ fn := timestamp.ppid.n ]
	 */
	for(;;) {
		mkfn(fn, n++);
		/*- [ stdin := copied to a file named timestamp.ppid.n in cwd ] */
		if ((fdout = open(fn, O_RDWR|O_EXCL|O_CREAT, 0644)) < 0) {
			if (flaglog)
				strerr_die2sys(QQ_XTEMP, FATAL, "could not create file 'msg': ");
			do_cleanq();
			_exit(QQ_XWRTE);
		}
		if (n == 1)
			unlink(fn);
		substdio_fdbuf(&ssout, write, fdout, outbuf, sizeof(outbuf));
		substdio_fdbuf(&ssin, read, 0, inbuf, sizeof(inbuf));
		switch (substdio_copy(&ssout, &ssin))
		{
		case -2:
			die_read();
		case -3:
			die_write();
		}
		if (substdio_flush(&ssout) == -1)
			die_write();
		if (n == 2) {
			close(fdout);
			break;
		}
		if (fd_move(0, fdout) < 0) {
			if (flaglog)
				strerr_die2sys(QQ_XTEMP, FATAL, "fdmove failed: ");
			do_cleanq();
			_exit(QQ_XREAD);
		}
		seek_begin(0);
	}
	/*
	 * Run ripmime now, to extract the message parts.
	 * [ stdin := extracted to files under cwd ]
	 */
	seek_begin(0);
	if (!do_ripmime()) {
		err("MIME extraction failed\n");
		do_cleanq();
		_exit(QQ_XREAD);
	}
	/*-
	 * Run antivir now, to check the current directory for viruses.
	 */
	seek_begin(0);
	switch (do_scan())
	{
	case (EX_ALLOK):
		break;
	case (EX_BADEX): /*- Found a banned extension */
		do_cleanq();
		_exit(QQ_BADEX); /*- drop the message */
	case (EX_VIRUS): /*- Found a virus */
		do_cleanq();
		_exit(QQ_VIRUS); /*- drop the message */
	default: /*- some problem w/scan */
		err("some problem occurred while scanning the message\n");
		do_cleanq();
		_exit(QQ_XTEMP);
	}
	/*- all done!  */
	do_cleanq();
	/*
	 * unset VIRUSCHECK to prevent
	 * qscanq getting called recursively
	 * by qmail-multi
	 */
	if (!env_unset("VIRUSCHECK"))
		_exit(51);
	if (argc > 1)
		execv(argv[1], argv + 1);
	else
		return (qmulti(0, argc, argv));
	_exit(QQ_XTEMP);
}

#ifndef lint
void
getversion_qscanq_stdin_c()
{
	const char     *x = "$Id: qscanq-stdin.c,v 1.9 2024-05-09 22:03:17+05:30 mbhangui Exp mbhangui $";

	x++;
}
#endif
