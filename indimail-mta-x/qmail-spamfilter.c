/*
 * $Log: qmail-spamfilter.c,v $
 * Revision 1.2  2021-08-29 23:27:08+05:30  Cprogrammer
 * define functions as noreturn
 *
 * Revision 1.1  2021-06-15 12:16:52+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <unistd.h>
#include <fcntl.h>
#include <substdio.h>
#include <stralloc.h>
#include <sig.h>
#include <env.h>
#include <scan.h>
#include <fmt.h>
#include <wait.h>
#include <error.h>
#include "auto_qmail.h"
#include <makeargs.h>
#include <mktempfile.h>
#include <noreturn.h>
#include "qmulti.h"

#define DEATH 86400	/*- 24 hours; _must_ be below q-s's OSSIFIED (36 hours) */

no_return void
sigalrm()
{
	/*- thou shalt not clean up here */
	_exit(52);
}

no_return void
sigbug()
{
	_exit(81);
}

int
main(int argc, char **argv)
{
	int             wstat, filt_exitcode, queueexitcode, n;
	int             pipefd[2], recpfd[2];
	pid_t           filt_pid, queuepid;
	struct substdio ssin, ssout;
	stralloc        spamfilterargs = { 0 };
	char            inbuf[2048], outbuf[2048];
	char           *ptr, *makeseekable, *spamf;
	char          **Argv;

	if (chdir(auto_qmail) == -1)
		_exit(61);
	sig_pipeignore();
	sig_miscignore();
	sig_alarmcatch(sigalrm);
	sig_bugcatch(sigbug);
	alarm(DEATH);
	if ((ptr = env_get("VIRUSCHECK")) && *ptr) {
		scan_int(ptr, &n);
		if (1 < n && 8 > n) {
			execv("sbin/qscanq", argv);
			_exit(75);
		}
	}
	if (!(spamf = env_get("SPAMFILTER")) || env_get("RELAYCLIENT"))
		return (qmulti("SPAMQUEUE", argc, argv)); /*- Does not return */
	if (pipe(pipefd) == -1)
		_exit(60);
	switch ((filt_pid = fork())) /*- spam filter */
	{
	case -1:
		close(0);
		close(1);
		close(pipefd[0]);
		close(pipefd[1]);
		_exit(121);
	case 0:
		if (!stralloc_copys(&spamfilterargs, spamf) ||
				!stralloc_0(&spamfilterargs))
			_exit(51);
		if (!(Argv = makeargs(spamfilterargs.s)))
			_exit(51);
		/*- Mail content read from fd 0 */
		makeseekable = env_get("MAKE_SEEKABLE");
		if (makeseekable && mktempfile(0))
			_exit(68);
		if (dup2(pipefd[1], 1) == -1 || close(pipefd[0]) == -1)
			_exit(60);
		if (pipefd[1] != 1)
			close(pipefd[1]);
		execv(*Argv, Argv);
		_exit(75);
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
		_exit(60);
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
		_exit(120);
	case 0:
		/*- 
		 * Mail content read from pipfd[0]
		 * which has been filtered through SPAMFILTER
		 * Envelope information can be read through recpfd[0]
		 */
		if (dup2(pipefd[0], 0) == -1 || close(pipefd[1]) == -1)
			_exit(60);
		if (dup2(recpfd[0], 1) == -1 || close(recpfd[1]) == -1)
			_exit(60);
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
		_exit(122);
	}
	if (wait_crashed(wstat)) {
		close(1);
		close(recpfd[1]);
		wait_pid(&wstat, queuepid);
		_exit(123);
	}
	/*
	 * Process message if exit code is 0, 1, 2
	 */
	switch (filt_exitcode = wait_exitcode(wstat))
	{
	case 0: /*- SPAM */
	case 1: /*- HAM */
	case 2: /*- Unsure */
		if ((ptr = env_get("SPAMEXITCODE"))) {
			scan_int(ptr, &n);
			if (n == filt_exitcode) { /*- Message is SPAM */
				if ((n = rewrite_envelope(recpfd[1])) > 1) { /*- Some error */
					close(1);
					close(recpfd[1]);
					wait_pid(&wstat, queuepid);
					_exit(n);
				}
				if (n == 1 || (ptr = env_get("REJECTSPAM"))) {
					/*- REJECTSPAM takes precedence over spam notifications */
					if (ptr && *ptr > '0') {
						(void) discard_envelope();
						close(1);
						close(recpfd[1]);
						wait_pid(&wstat, queuepid);
						if (*ptr == '1')
							_exit(32); /*- bounce */
						else
							_exit(0); /*- blackhole */
					} else /*- spam notification - envelope has been rewritten */ if (n == 1) {
						(void) discard_envelope();
						goto finish;
					}
				}
			}
		}
		break;
	default: /*- should not happen normally */
		close(1);
		close(recpfd[1]);
		wait_pid(&wstat, queuepid);
		_exit(76); /*- treat this as temp problem with spam filter */
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
		_exit(54);
	case -3: /*- write error */
		close(1);
		close(recpfd[1]);
		_exit(53);
	}
	if (substdio_flush(&ssout) == -1)
		_exit(53);
finish:
	close(1);
	close(recpfd[1]);
	if (wait_pid(&wstat, queuepid) != queuepid)
		_exit(122);
	if (wait_crashed(wstat))
		_exit(123);
	_exit(queueexitcode = wait_exitcode(wstat));
	/*- Not reached */
	return (0);
}

#ifndef	lint
void
getversion_qmail_spamfilter_c()
{
	static char    *x = "$Id: qmail-spamfilter.c,v 1.2 2021-08-29 23:27:08+05:30 Cprogrammer Exp mbhangui $";

	x = sccsidqmultih;
	x = sccsidmakeargsh;
	x = sccsidmktempfileh;
	x++;
}
#endif
