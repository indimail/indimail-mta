/*
 * $Log: report.c,v $
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
#include "report.h"
#include "variables.h"

extern dtype    delivery;

void
report(int errCode, char *s1, char *s2, char *s3, char *s4, char *s5, char *s6)
{
	if (delivery == local_delivery) /*- strerr_die does not return */
		strerr_die6x(errCode, s1, s2, s3, s4, s5, s6);
	if (!errCode) { /*- should never happen */
		if (substdio_put(subfdoutsmall, "r\0Kspawn accepted message.\n", 28) == -1)
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
		if (substdio_puts(subfdoutsmall, 
			errCode == 111 ?  "Zspawn said: Message deferred" : "Dspawn said: Giving up on filter\n") == -1)
			_exit(111);
		if (substdio_put(subfdoutsmall, "\0", 1) == -1)
			_exit(111);
	}
	substdio_flush(subfdoutsmall);
	/*- For qmail-rspawn to stop complaining unable to run qmail-remote */
	_exit(0);
}

void
getversion_report_c()
{
	static char    *x = "$Id: report.c,v 1.3 2021-05-30 00:16:51+05:30 Cprogrammer Exp mbhangui $";

	x = sccsidreporth;
	x++;
}
