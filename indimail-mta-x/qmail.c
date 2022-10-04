/*
 * $Id: qmail.c,v 1.33 2022-10-04 23:43:37+05:30 Cprogrammer Exp mbhangui $
 */
#include <unistd.h>
#include <substdio.h>
#include <wait.h>
#include <scan.h>
#include <fd.h>
#include <env.h>
#include <stralloc.h>
#include <error.h>
#include "qmail.h"
#include "auto_prefix.h"

/*- open the queue */
int
qmail_open(struct qmail *qq)
{
	int             pim[2];
	int             pie[2];
	int             pic[2], e, errfd; /* custom error message from custom error patch by Flavio Curti */
	char           *x, *binqqargs[2] = { 0, 0 };
	stralloc        q = {0};

	if (!(x = env_get("ERROR_FD")))
		errfd = CUSTOM_ERR_FD;
	else
		scan_int(x, &errfd);
	if (pipe(pim) == -1)
		return -1;
	if (pipe(pie) == -1) {
		e = errno;
		close(pim[0]);
		close(pim[1]);
		errno = e;
		return -1;
	}
	if (errfd != -1 && pipe(pic) == -1) { /* pipe for custom error */
		e = errno;
		close(pim[0]);
		close(pim[1]);
		close(pie[0]);
		close(pie[1]);
		errno = e;
		return -1;
	}
	switch (qq->pid = vfork())
	{
	case -1:
		e = errno;
		close(pim[0]);
		close(pim[1]);
		close(pie[0]);
		close(pie[1]);
		if (errfd != -1) {
			close(pic[0]);
			close(pic[1]);
		}
		errno = e;
		return -1;
	case 0:
		close(pim[1]);
		close(pie[1]);
		if (errfd != -1)
			close(pic[0]); /*- we want to write error message */
		if (fd_move(0, pim[0]) == -1)
			_exit(120);
		if (fd_move(1, pie[0]) == -1)
			_exit(120);
		if (errfd != -1) {
			if (fd_move(errfd, pic[1]) == -1)
				_exit(120);
		}
		if (chdir("/") == -1)
			_exit(63);
		if (!(x = env_get("NULLQUEUE"))) {
			if (!stralloc_copys(&q, auto_prefix) ||
					!stralloc_catb(&q, "/sbin/qmail-nullqueue", 21) ||
					!stralloc_0(&q))
				_exit(51);
			binqqargs[0] = q.s;
		}
		if (!x)
			x = env_get("QMAILQUEUE");
		if (!x) {
			if (!stralloc_copys(&q, auto_prefix) ||
					!stralloc_catb(&q, "/sbin/qmail-queue", 17) ||
					!stralloc_0(&q))
				_exit(51);
			binqqargs[0] = q.s;
		} else
			binqqargs[0] = x;
		execv(*binqqargs, binqqargs);
		_exit(120);
	}
	qq->fdm = pim[1];
	close(pim[0]);
	qq->fde = pie[1];
	close(pie[0]);
	if (errfd != -1) {
		qq->fdc = pic[0];
		close(pic[1]); /*- we want to read error message */
	} else
		qq->fdc = -1;
	substdio_fdbuf(&qq->ss, write, qq->fdm, qq->buf, sizeof(qq->buf));
	qq->flagerr = 0;
	return 0;
}

unsigned long
qmail_qp(struct qmail *qq)
{
	return qq->pid;
}

void
qmail_fail(struct qmail *qq)
{
	qq->flagerr = 1;
}

void
qmail_put(struct qmail *qq, char *s, unsigned int len)
{
	if (!qq->flagerr) {
		if (substdio_put(&qq->ss, s, len) == -1)
			qq->flagerr = 1;
	}
}

void
qmail_puts(struct qmail *qq, char *s)
{
	if (!qq->flagerr) {
		if (substdio_puts(&qq->ss, s) == -1)
			qq->flagerr = 1;
	}
}

void
qmail_from(struct qmail *qq, char *s)
{
	if (substdio_flush(&qq->ss) == -1)
		qq->flagerr = 1;
	close(qq->fdm);
	substdio_fdbuf(&qq->ss, write, qq->fde, qq->buf, sizeof(qq->buf));
	qmail_put(qq, "F", 1);
	qmail_puts(qq, s);
	qmail_put(qq, "", 1);
}

void
qmail_to(struct qmail *qq, char * s)
{
	qmail_put(qq, "T", 1);
	qmail_puts(qq, s);
	qmail_put(qq, "", 1);
}

char           *
qmail_close(struct qmail *qq)
{
	int             wstat, exitcode, len = 0;
	char            ch;
	static char     errstr[1024];

	qmail_put(qq, "", 1);
	if (!qq->flagerr && substdio_flush(&qq->ss) == -1)
		qq->flagerr = 1;
	close(qq->fde);

	/* read custom error */
	if (qq->fdc != -1) {
		substdio_fdbuf(&qq->ss, read, qq->fdc, qq->buf, sizeof(qq->buf));
		while (substdio_bget(&qq->ss, &ch, 1) && len < (sizeof(errstr) - 1)) {
			errstr[len] = ch;
			len++;
		}
		if (len > 0)
			errstr[len] = 0; /* add termination */
		close(qq->fdc);
	}
	if (wait_pid(&wstat, qq->pid) != qq->pid)
		return "Zqq waitpid surprise (#4.3.0)";
	if (wait_crashed(wstat))
		return "Zqq crashed (#4.3.0)";
	exitcode = wait_exitcode(wstat);
	switch (exitcode)
	{
	case 115: /*- compatibility */
	case 11:
		return "Dqq envelope address too long (#5.1.3)";
	case 31:
		return "Dqq mail server permanently rejected message (#5.3.0)";
	case 32:
		return "Dqq spam or junk mail threshold exceeded (#5.7.1)"; /*- qmail-spamfiter */
	case 33:
		return "Dqq message contains virus (#5.7.1)";
	case 34:
		return "Dqq message contains banned attachment (#5.7.1)";
	case 35:
		return "Dqq private key file does not exist (#5.3.5)";
	case 50:
		return "Zqq unable to get privilege to run virus scanner (#4.3.0)"; /*- qhpsi */
	case 51:
		return "Zqq out of memory (#4.3.0)";
	case 52:
		return "Zqq timeout (#4.3.0)";
	case 53:
		return "Zqq write error or disk full (#4.3.0)";
	case 0:
		if (!qq->flagerr)
			return "";
		/*- fall through */
	case 54:
		return "Zqq read error (#4.3.0)";
	case 55:
		return "Zqq unable to read configuration (#4.3.0)";
	case 56:
		return "Zqq trouble making network connection (#4.3.0)";
	case 57:
		return "Zqq unable to open shared object/plugin (#4.3.0)";
	case 58:
		return "Zqq unable to resolve symbol in shared object/plugin (#4.3.0)";
	case 59:
		return "Zqq unable to close shared object/plugin (#4.3.0)";
	case 60:
		return "Zqq trouble creating pipes/sockets (#4.3.0)";
	case 61:
		return "Zqq trouble in home directory (#4.3.0)";
	case 62:
		return "Zqq unable to access mess file (#4.3.0)";
	case 63:
		return "Zqq trouble doing cd to root directory (#4.3.0)";
	case 64:
		return "Zqq trouble syncing message to disk (#4.3.0)";
	case 65:
		return "Zqq trouble creating files in intd. (#4.3.0)";
	case 66:
		return "Zqq trouble linking todofn to intdfn (#4.3.0)";
	case 67:
		return "Zqq trouble linking messfn to pidfn (#4.3.0)";
	case 68:
		return "Zqq trouble creating temporary files (#4.3.0)";
	case 69:
		return "Zqq trouble syncing dir to disk (#4.3.0)";
	case 70:
		return "Zqq trouble with pid file (#4.3.0)";
	case 71:
		return "Zqq mail server temporarily rejected message (#4.3.0)";
	case 72:
		return "Zqq connection to mail server timed out (#4.4.1)";
	case 73:
		return "Zqq connection to mail server rejected (#4.4.1)";
	case 74:
		return "Zqq communication with mail server failed (#4.4.2)";
	case 75:
		return "Zqq unable to exec (#4.3.0)";
	case 76:
		return "Zqq temporary problem with SPAM filter (#4.3.0)";
	case 77: /*- thanks to problem repoted by peter cheng */
		return "Zqq unable to run QHPSI scanner (#4.3.0)";
	case 78:
		return "Zqq trouble getting uids/gids (#4.3.0)";
	case 79:
		return "Zqq envelope format error (#4.3.0)";
	case 80:
		return "Zqq trouble removing intdfn";
	case 91:
		/*- fall through */
	case 81:
		return "Zqq internal bug (#4.3.0)";
	case 87: /*-*/
		return "Zqq mail system incorrectly configured. (#4.3.5)";
	case 82: /*- compatability with simscan, notqmail, etc */
	case 120:
		return "Zqq unable to exec qq (#4.3.0)";
	case 121: /*-*/
		return "Zqq unable to fork (#4.3.0)";
	case 122: /*-*/
		return "Zqq waitpid surprise (#4.3.0)";
	case 123: /*-*/
		return "Zqq crashed (#4.3.0)";
	case 88: /*- custom error */
		if (qq->fdc != -1 && len > 2)
			return errstr;
		return "Zqq temporary problem (#4.3.0)";
	default:
		if ((exitcode >= 11) && (exitcode <= 40))
			return "Dqq permanent problem (#5.3.0)";
		return "Zqq temporary problem (#4.3.0)";
	}
}

void
getversion_qmail_c()
{
	static char    *x = "$Id: qmail.c,v 1.33 2022-10-04 23:43:37+05:30 Cprogrammer Exp mbhangui $";

	x++;
}

/*
 * $Log: qmail.c,v $
 * Revision 1.33  2022-10-04 23:43:37+05:30  Cprogrammer
 * set ERROR_FD to -1 to disable custom error
 *
 * Revision 1.32  2022-04-03 18:09:00+05:30  Cprogrammer
 * refactored return codes
 *
 * Revision 1.31  2022-03-30 22:53:05+05:30  Cprogrammer
 * include error.h for errno
 *
 * Revision 1.30  2022-03-28 10:08:14+05:30  Cprogrammer
 * added new error codes
 * save and restore errno
 *
 * Revision 1.29  2022-03-05 13:31:03+05:30  Cprogrammer
 * use auto_prefix for binary paths
 *
 * Revision 1.28  2020-12-07 16:08:20+05:30  Cprogrammer
 * added exit code 79 as duplicate to 91 for Envelope format error
 *
 * Revision 1.27  2020-11-24 13:46:41+05:30  Cprogrammer
 * removed exit.h
 *
 * Revision 1.26  2020-05-12 12:11:46+05:30  Cprogrammer
 * c89 prototypes
 * fix integer signedness error in qmail_put() (CVE-2005-1515)
 *
 * Revision 1.25  2016-06-03 09:57:41+05:30  Cprogrammer
 * moved qmail-mullqueue, qmail-queue to sbin
 *
 * Revision 1.24  2009-11-13 23:05:29+05:30  Cprogrammer
 * report QHPSI error when exit code is 77
 *
 * Revision 1.23  2009-04-22 15:23:55+05:30  Cprogrammer
 * added scan.h
 *
 * Revision 1.22  2009-04-22 13:42:10+05:30  Cprogrammer
 * made fd for custom error configurable through env variable ERROR_FD
 *
 * Revision 1.21  2009-04-03 11:42:35+05:30  Cprogrammer
 * increased size of errstr
 *
 * Revision 1.20  2009-03-22 09:24:05+05:30  Cprogrammer
 * removed domainkey, dkim specific error messages. domainkey, dkim will now use
 * custom error (exit code 88)
 *
 * Revision 1.19  2005-06-11 21:31:44+05:30  Cprogrammer
 * added custom message support
 *
 * Revision 1.18  2005-05-16 16:33:44+05:30  Cprogrammer
 * added case where mess file is not accessible
 *
 * Revision 1.17  2005-04-24 22:43:11+05:30  Cprogrammer
 * added codes for running external scanners
 *
 * Revision 1.16  2005-04-01 23:03:49+05:30  Cprogrammer
 * added exit codes for domainkeys
 *
 * Revision 1.15  2004-10-22 20:28:13+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.14  2004-10-22 15:36:40+05:30  Cprogrammer
 * removed readwrite.h
 *
 * Revision 1.13  2004-09-19 14:37:00+05:30  Cprogrammer
 * added cases for virus and banned attachments
 *
 * Revision 1.12  2004-05-03 22:10:34+05:30  Cprogrammer
 * unset QMAILQUEUE back to original for multisession SMTP
 *
 * Revision 1.11  2004-01-10 09:44:11+05:30  Cprogrammer
 * case 76 added for problem with SPAM filter
 *
 * Revision 1.10  2003-12-20 01:34:08+05:30  Cprogrammer
 * corrected message for exec failure and fork failure
 *
 * Revision 1.9  2003-12-16 00:31:41+05:30  Cprogrammer
 * added RCS log
 *
 */
