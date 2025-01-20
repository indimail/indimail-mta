/*
 * $Log: tcpto_clean.c,v $
 * Revision 1.8  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.7  2005-06-29 20:54:53+05:30  Cprogrammer
 * size of buffer changed to TCPTO_BUFSIZ
 *
 * Revision 1.6  2005-06-17 21:51:23+05:30  Cprogrammer
 * increased size of tcpto buffer
 *
 * Revision 1.5  2004-10-22 20:31:37+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.4  2004-10-22 15:39:50+05:30  Cprogrammer
 * removed readwrite.h
 *
 * Revision 1.3  2004-07-17 21:24:44+05:30  Cprogrammer
 * added RCS log
 *
 */
#include <unistd.h>
#include "tcpto.h"
#include "open.h"
#include "substdio.h"

char            tcpto_cleanbuf[TCPTO_BUFSIZ];

void
tcpto_clean()					/*- running from queue/mess */
{
	int             fd;
	int             i;
	substdio        ss;

	if ((fd = open_write("../lock/tcpto")) == -1)
		return;
	substdio_fdbuf(&ss, (ssize_t (*)(int,  char *, size_t)) write, fd, tcpto_cleanbuf, sizeof(tcpto_cleanbuf));
	for (i = 0; i < sizeof(tcpto_cleanbuf); ++i)
		substdio_put(&ss, "", 1);
	substdio_flush(&ss);/*- if it fails, bummer */
	close(fd);
}

void
getversion_tcpto_clean_c()
{
	const char     *x = "$Id: tcpto_clean.c,v 1.8 2024-05-09 22:03:17+05:30 mbhangui Exp mbhangui $";

	x++;
}
