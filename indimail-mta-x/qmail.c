/*
 * $Log: qmail.c,v $
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
#include <unistd.h>
#include "substdio.h"
#include "wait.h"
#include "scan.h"
#include "fd.h"
#include "qmail.h"
#include "auto_qmail.h"
#include "env.h"

/*- open the queue */
int
qmail_open(struct qmail *qq)
{
	int             pim[2];
	int             pie[2];
	int             pic[2], errfd; /* custom message */
	char           *x, *binqqargs[2] = { 0, 0 };

	if (pipe(pim) == -1)
		return -1;
	if (pipe(pie) == -1) {
		close(pim[0]);
		close(pim[1]);
		return -1;
	}
	if (pipe(pic) == -1) {
		close(pim[0]);
		close(pim[1]);
		close(pie[0]);
		close(pie[1]);
		return -1;
	}
	switch (qq->pid = vfork())
	{
	case -1:
		close(pim[0]);
		close(pim[1]);
		close(pie[0]);
		close(pie[1]);
		close(pic[0]);
		close(pic[1]);
		return -1;
	case 0:
		close(pim[1]);
		close(pie[1]);
		close(pic[0]); /*- we want to receive data */
		if (fd_move(0, pim[0]) == -1)
			_exit(120);
		if (fd_move(1, pie[0]) == -1)
			_exit(120);
		if (!(x = env_get("ERROR_FD")))
			errfd = CUSTOM_ERR_FD;
		else
			scan_int(x, &errfd);
		if (fd_move(errfd, pic[1]) == -1)
			_exit(120);
		if (chdir(auto_qmail) == -1)
			_exit(61);
		if (!binqqargs[0] && env_get("NULLQUEUE"))
			binqqargs[0] = "sbin/qmail-nullqueue";
		if (!binqqargs[0])
			binqqargs[0] = env_get("QMAILQUEUE");
		if (!binqqargs[0])
			binqqargs[0] = "sbin/qmail-queue";
		execv(*binqqargs, binqqargs);
		_exit(120);
	}
	qq->fdm = pim[1];
	close(pim[0]);
	qq->fde = pie[1];
	close(pie[0]);
	qq->fdc = pic[0];
	close(pic[1]);
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
	substdio_fdbuf(&qq->ss, read, qq->fdc, qq->buf, sizeof(qq->buf));
	while (substdio_bget(&qq->ss, &ch, 1) && len < (sizeof(errstr) - 1))
	{
		errstr[len] = ch;
		len++;
	}
	if (len > 0)
		errstr[len] = 0; /* add str-term */
	close(qq->fdc);
	if (wait_pid(&wstat, qq->pid) != qq->pid)
		return "Zqq waitpid surprise (#4.3.0)";
	if (wait_crashed(wstat))
		return "Zqq crashed (#4.3.0)";
	exitcode = wait_exitcode(wstat);
	switch (exitcode)
	{
	case 115: /*- compatibility */
	case 11:
		return "Denvelope address too long for qq (#5.1.3)";
	case 31:
		return "Dmail server permanently rejected message (#5.3.0)";
	case 32: /*-*/
		return "DSPAM or junk mail threshold exceeded (#5.7.1)";
	case 33: /*-*/
		return "DMessage contains virus (#5.7.1)";
	case 34: /*-*/
		return "DMessage contains banned attachment (#5.7.1)";
	case 35: /*-*/
		return "DPrivate key file does not exist (#5.3.5)";
	case 50: /*-*/
		return "Zunable to set uid/gid (#4.3.0)";
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
	case 57: /*-*/
		return "Zunable to open shared object/plugin (#4.3.0)";
	case 58: /*-*/
		return "Zunable to resolve symbol in shared object/plugin (#4.3.0)";
	case 59: /*-*/
		return "Zunable to close shared object/plugin (#4.3.0)";
	case 60: /*-*/
		return "Zqq trouble creating pipes/sockets (#4.3.0)";
	case 61:
		return "Zqq trouble in home directory (#4.3.0)";
	case 62: /*-*/
		return "Zqq unable to access mess file (#4.3.0)";
	case 63:
	case 64:
	case 65:
	case 66:
		return "Zqq trouble creating files in queue (#4.3.0)";
	case 67: /*-*/
		return "Zqq trouble getting uids/gids (#4.3.0)";
	case 68: /*-*/
		return "Zqq trouble creating temporary files (#4.3.0)";
	case 71:
		return "Zmail server temporarily rejected message (#4.3.0)";
	case 72:
		return "Zconnection to mail server timed out (#4.4.1)";
	case 73:
		return "Zconnection to mail server rejected (#4.4.1)";
	case 74:
		return "Zcommunication with mail server failed (#4.4.2)";
	case 75: /*-*/
		return "Zunable to exec (#4.3.0)";
	case 76: /*-*/
		return "Ztemporary problem with SPAM filter (#4.3.0)";
	case 77: /*- thanks to problem repoted by peter cheng */
		return "Zqq unable to run QHPSI scanner (#4.3.0)";
	case 91:
		/*- fall through */
	case 81:
		return "Zqq internal bug (#4.3.0)";
	case 87: /*-*/
		return "Zmail system incorrectly configured. (#4.3.5)";
	case 88: /*- custom error */
		if (len > 2)
			return errstr;
	case 120:
		return "Zunable to exec qq (#4.3.0)";
	case 121: /*-*/
		return "Zunable to fork (#4.3.0)";
	case 122: /*-*/
		return "Zqq waitpid surprise (#4.3.0)";
	case 123: /*-*/
		return "Zqq crashed (#4.3.0)";
	default:
		if ((exitcode >= 11) && (exitcode <= 40))
			return "Dqq permanent problem (#5.3.0)";
		return "Zqq temporary problem (#4.3.0)";
	}
}

void
getversion_qmail_c()
{
	static char    *x = "$Id: qmail.c,v 1.27 2020-11-24 13:46:41+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
