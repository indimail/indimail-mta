/*
 * $Log: report.c,v $
 * Revision 1.8  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.7  2024-02-07 22:58:37+05:30  Cprogrammer
 * modified error messages
 *
 * Revision 1.6  2023-12-06 17:02:13+05:30  Cprogrammer
 * added comment on report format for qmail-rspawn
 *
 * Revision 1.5  2021-08-29 23:27:08+05:30  Cprogrammer
 * define functions as noreturn
 *
 * Revision 1.4  2021-08-28 23:07:59+05:30  Cprogrammer
 * moved dtype enum delivery variable from variables.h to getDomainToken.h
 *
 * Revision 1.3  2021-05-30 00:16:51+05:30  Cprogrammer
 * renamed local to local_delivery
 *
 * Revision 1.2  2021-05-26 07:36:54+05:30  Cprogrammer
 * fixed extra colon char in error messages
 *
 * Revision 1.1  2021-05-23 06:35:28+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <unistd.h>
#include <substdio.h>
#include <subfd.h>
#include <strerr.h>
#include <noreturn.h>
#include "report.h"
#include "getDomainToken.h"

extern dtype    delivery;

/*-
 * qmail-rspawn doesn't use exit code of qmail-remote. It needs a report in
 * the following format
 * "[r,h,s]recipient_report\0[K,Z,D]message_report\0"
 *
 * recipient_report start with one of the letters r, h, s
 * as below
 * r - Recipient report: acceptance.
 * s - Recipient report: temporary rejection.
 * h - Recipient report: permanent rejection.
 *
 * message_report start with one of the letters K, Z, D
 * as below
 * K - Message report: success.
 * Z - Message report: temporary failure.
 * D - Message report: permanent failure.
 *
 * Examples of qmail-remote report
 *
 * Success
 * "rFrom <xxx@example.com> RCPT <yyy@example.org>\0\n"
 * "KHost example.com accepted message\0\n"
 *
 * temp failure
 * "sFrom <xxx@example.com> RCPT <yyy@example.org>\0\n"
 * "ZTemporary failure accepting message\0\n"
 *
 * perm failure
 * "hFrom <xxx@example.com> RCPT <yyy@example.org>\0\n"
 * "Dexample.org does not like recipient\0\n"
 *
 * qmail-lspawn uses the exit code of qmail-local
 * 0   - Success
 * 111 - Temporary failure
 * 100 - Permanent failure
 */
no_return void
report(int errCode, const char *s1, const char *s2, const char *s3, const char *s4, const char *s5, const char *s6)
{
	if (delivery == local_delivery) /*- strerr_die does not return */
		strerr_die6x(errCode, s1, s2, s3, s4, s5, s6);
	if (!errCode) { /*- should never happen */
		if (substdio_put(subfdoutsmall, "r\0Kspawn accepted message.\n\0", 28) == -1)
			_exit(111);
		if (s1) {
			if (substdio_puts(subfdoutsmall, s1) == -1 ||
					substdio_put(subfdoutsmall, "\n", 1) == -1)
				_exit(111);
		} else
		if (substdio_put(subfdoutsmall, "spawn said: 250 ok notification queued\n\0", 41) == -1)
			_exit(111);
	} else {
		/*- h - hard, s - soft */
		if (substdio_put(subfdoutsmall, errCode == 111 ? "s" : "h", 1) == -1)
			_exit(111);
		if (s1 && substdio_puts(subfdoutsmall, s1) == -1)
			_exit(111);
		if (s2 && substdio_puts(subfdoutsmall, s2) == -1)
			_exit(111);
		if (s3 && substdio_puts(subfdoutsmall, s3) == -1)
			_exit(111);
		if (s4 && substdio_puts(subfdoutsmall, s4) == -1)
			_exit(111);
		if (s5 && substdio_puts(subfdoutsmall, s5) == -1)
			_exit(111);
		if (s6 && substdio_puts(subfdoutsmall, s6) == -1)
			_exit(111);
		if (substdio_put(subfdoutsmall, "\0", 1) == -1)
			_exit(111);
		if (delivery == local_delivery) {
			if (substdio_puts(subfdoutsmall,
					errCode == 111 ?  "Zspawn-filter(local): filter execution deferred" : "Drefusing to run filter\n") == -1)
				_exit(111);
		} else
		if (delivery == remote_delivery) {
			if (substdio_puts(subfdoutsmall,
					errCode == 111 ?  "Zspawn-filter(remote): filter execution deferred" : "Drefusing to run filter\n") == -1)
				_exit(111);
		}
		if (substdio_put(subfdoutsmall, "\0", 1) == -1)
			_exit(111);
	}
	substdio_flush(subfdoutsmall);
	/*- For qmail-rspawn to stop complaining unable to run qmail-remote */
	_exit(delivery == remote_delivery ? 0 : errCode);
}

void
getversion_report_c()
{
	const char     *x = "$Id: report.c,v 1.8 2024-05-09 22:03:17+05:30 mbhangui Exp mbhangui $";

	x = sccsidreporth;
	x = sccsidgetdomainth;
	x++;
}
