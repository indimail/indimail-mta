/*
 * $Id: qmail-spamfilter.c,v 1.11 2024-05-09 22:03:17+05:30 mbhangui Exp mbhangui $
 */
#include <unistd.h>
#include <fcntl.h>
#include <substdio.h>
#include <stralloc.h>
#include <sig.h>
#include <env.h>
#include <getEnvConfig.h>
#include <scan.h>
#include <fmt.h>
#include <wait.h>
#include <error.h>
#include <makeargs.h>
#include <mktempfile.h>
#include <noreturn.h>
#include "auto_prefix.h"
#include "qmulti.h"
#include "qmail.h"

no_return void
sigalrm()
{
	/*- thou shalt not clean up here */
	_exit(QQ_TIMEOUT);
}

no_return void
sigbug()
{
	_exit(QQ_INTERNAL_BUG);
}

int
main(int argc, char **argv)
{
	int             wstat, filt_exitcode, queueexitcode, n, ham_code = 1,
					spam_code = 0, unsure_code = 2;
	unsigned long   death;
	int             pipefd[2], recpfd[2];
	pid_t           filt_pid, queuepid;
	struct substdio ssin, ssout;
	stralloc        spamfilterargs = { 0 }, q = { 0 };
	char            inbuf[2048], outbuf[2048];
	const char     *ptr, *spamf;
	char          **Argv;

	if (chdir("/") == -1)
		_exit(QQ_CHDIR);
	sig_pipeignore();
	sig_miscignore();
	sig_alarmcatch(sigalrm);
	sig_bugcatch(sigbug);
	getEnvConfiguLong(&death, "DEATH", DEATH);
	alarm(death);
	if ((ptr = env_get("VIRUSCHECK")) && *ptr) {
		scan_int(ptr, &n);
		if (1 < n && 8 > n) {
			if (!stralloc_copys(&q, auto_prefix) ||
					!stralloc_catb(&q, "/sbin/qscanq", 12) ||
					!stralloc_0(&q))
			execv(q.s, argv);
			_exit(QQ_EXEC_FAILED);
		}
	}
	ptr = (env_get("RELAYCLIENT") || env_get("AUTHINFO")) ? "" : 0;
	if (!(spamf = env_get("SPAMFILTER")) || (ptr && env_get("RELAYCLIENT_NOSPAMFILTER")))
		return (qmulti("SPAMQUEUE", argc, argv)); /*- Does not return */
	if (pipe(pipefd) == -1)
		_exit(QQ_PIPE_SOCKET);
	switch ((filt_pid = fork())) /*- spam filter */
	{
	case -1:
		close(0);
		close(1);
		close(pipefd[0]);
		close(pipefd[1]);
		_exit(QQ_FORK_ERR);
	case 0:
		if (!stralloc_copys(&spamfilterargs, spamf) ||
				!stralloc_0(&spamfilterargs))
			_exit(QQ_OUT_OF_MEMORY);
		if (!(Argv = makeargs(spamfilterargs.s)))
			_exit(QQ_OUT_OF_MEMORY);
		/*- Mail content read from fd 0 */
		if (mktempfile(0))
			_exit(QQ_TMP_FILES);
		if (dup2(pipefd[1], 1) == -1)
			_exit(QQ_DUP_ERR);
		if (close(pipefd[0]) == -1)
			_exit(QQ_PIPE_SOCKET);
		if (pipefd[1] != 1)
			close(pipefd[1]);
		execv(*Argv, Argv);
		_exit(QQ_EXEC_FAILED);
	default:
		break;
	}
	if (pipe(recpfd) == -1) {
		close(0);
		close(1);
		close(pipefd[0]);
		close(pipefd[1]);
		close(recpfd[0]);
		close(recpfd[1]);
		_exit(QQ_PIPE_SOCKET);
	}
	close(0);
	switch ((queuepid = fork()))
	{
	case -1:
		close(1);
		close(pipefd[0]);
		close(pipefd[1]);
		close(recpfd[0]);
		close(recpfd[1]);
		wait_pid(&wstat, filt_pid);
		_exit(QQ_FORK_ERR);
	case 0:
		/*-
		 * Mail content read from pipfd[0]
		 * which has been filtered through SPAMFILTER
		 * Envelope information can be read through recpfd[0]
		 */
		if (dup2(pipefd[0], 0) == -1)
			_exit(QQ_DUP_ERR);
		if (close(pipefd[1]) == -1)
			_exit(QQ_PIPE_SOCKET);
		if (dup2(recpfd[0], 1) == -1)
			_exit(QQ_DUP_ERR);
		if (close(recpfd[1]) == -1)
			_exit(QQ_PIPE_SOCKET);
		if (pipefd[0] != 0)
			close(pipefd[0]);
		if (recpfd[0] != 1)
			close(recpfd[0]);
		return (qmulti("SPAMQUEUE", argc, argv));
	default:
		close(pipefd[0]);
		close(pipefd[1]);
		close(recpfd[0]);
		break;
	}
	if (wait_pid(&wstat, filt_pid) != filt_pid) {
		close(1);
		close(recpfd[1]);
		wait_pid(&wstat, queuepid);
		_exit(QQ_WAITPID_SURPRISE);
	}
	if (wait_crashed(wstat)) {
		close(1);
		close(recpfd[1]);
		wait_pid(&wstat, queuepid);
		_exit(QQ_CRASHED);
	}
	/*
	 * Process message if exit code is 0, 1, 2
	 */
	filt_exitcode = wait_exitcode(wstat);
	if (!(ptr = env_get("SPAMEXITCODE")))
		spam_code = 0; /*- default for bogofilter */
	else
		scan_int(ptr, &spam_code);
	if (!(ptr = env_get("HAMEXITCODE")))
		ham_code = 1; /*- default for bogofilter */
	else
		scan_int(ptr, &ham_code);
	if (!(ptr = env_get("UNSUREEXITCODE")))
		unsure_code = 2; /*- default for bogofilter */
	else
		scan_int(ptr, &unsure_code);
	if (spam_code == filt_exitcode) { /* Message is SPAM */
		if ((n = rewrite_envelope(recpfd[1])) > 1) { /*- Some error */
			close(1);
			close(recpfd[1]);
			wait_pid(&wstat, queuepid);
			_exit(n);
		}
		ptr = env_get("REJECTSPAM");
		if (n == 1 || ptr) {
			/*- REJECTSPAM takes precedence over spam notifications */
			if (ptr && *ptr > '0') {
				(void) discard_envelope();
				close(1);
				close(recpfd[1]);
				wait_pid(&wstat, queuepid);
				if (*ptr == '1')
					_exit(QQ_SPAM_THRESHOLD); /*- bounce */
				else
					_exit(0); /*- blackhole */
			} else /*- spam notification - envelope has been rewritten */ if (n == 1) {
				(void) discard_envelope();
				goto finish;
			}
		}
	} else
	if (filt_exitcode != ham_code && filt_exitcode != unsure_code) {
		close(1);
		close(recpfd[1]);
		wait_pid(&wstat, queuepid);
		_exit(QQ_TEMP_SPAM_FILTER); /*- treat this as temp problem with spam filter */
	}

	/*- Write envelope to qmail-queue */
	substdio_fdbuf(&ssout, write, recpfd[1], outbuf, sizeof (outbuf));
	/*- Read envelope from qmail-smtpd */
	substdio_fdbuf(&ssin, read, 1, inbuf, sizeof (inbuf));
	switch (substdio_copy(&ssout, &ssin))
	{
	case -2: /*- read error */
		close(1);
		close(recpfd[1]);
		_exit(QQ_READ_ERR);
	case -3: /*- write error */
		close(1);
		close(recpfd[1]);
		_exit(QQ_WRITE_ERR);
	}
	if (substdio_flush(&ssout) == -1)
		_exit(QQ_WRITE_ERR);
finish:
	close(1);
	close(recpfd[1]);
	if (wait_pid(&wstat, queuepid) != queuepid)
		_exit(QQ_WAITPID_SURPRISE);
	if (wait_crashed(wstat))
		_exit(QQ_CRASHED);
	_exit(queueexitcode = wait_exitcode(wstat));
	/*- Not reached */
	return (0);
}

#ifndef	lint
void
getversion_qmail_spamfilter_c()
{
	const char     *x = "$Id: qmail-spamfilter.c,v 1.11 2024-05-09 22:03:17+05:30 mbhangui Exp mbhangui $";

	x = sccsidmakeargsh;
	x = sccsidmktempfileh;
	x++;
}
#endif

/*
 * $Log: qmail-spamfilter.c,v $
 * Revision 1.11  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.10  2024-01-07 01:44:35+05:30  Cprogrammer
 * bypass antispam filter when RELAYCLIENT and RELAYCLIENT_NOSPAMFILTER is set
 *
 * Revision 1.9  2023-12-25 09:31:02+05:30  Cprogrammer
 * made DEATH configurable
 *
 * Revision 1.8  2023-10-26 23:14:32+05:30  Cprogrammer
 * added HAMEXITCODE, UNSUREEXITCODE
 *
 * Revision 1.7  2023-10-25 13:26:09+05:30  Cprogrammer
 * rewind descriptor 0 regardless of MAKE_SEEKABLE setting
 *
 * Revision 1.6  2022-10-17 19:44:59+05:30  Cprogrammer
 * use exit codes defines from qmail.h
 *
 * Revision 1.5  2022-04-03 18:44:36+05:30  Cprogrammer
 * refactored qmail_open() error codes
 *
 * Revision 1.4  2022-03-05 13:35:30+05:30  Cprogrammer
 * use auto_prefix/sbin for qscanq path
 *
 * Revision 1.3  2022-01-30 09:14:32+05:30  Cprogrammer
 * removed chdir auto_qmail
 *
 * Revision 1.2  2021-08-29 23:27:08+05:30  Cprogrammer
 * define functions as noreturn
 *
 * Revision 1.1  2021-06-15 12:16:52+05:30  Cprogrammer
 * Initial revision
 *
 */
