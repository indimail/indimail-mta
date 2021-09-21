/*
 * $Log: qbase64.c,v $
 * Revision 1.8  2021-08-29 23:27:08+05:30  Cprogrammer
 * define functions as noreturn
 *
 * Revision 1.7  2010-03-03 11:00:41+05:30  Cprogrammer
 * remove newline
 *
 * Revision 1.6  2010-03-03 09:34:09+05:30  Cprogrammer
 * renamed b64encode to base64, combining encoding and decoding
 *
 * Revision 1.5  2004-10-22 20:18:29+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.4  2004-07-17 21:16:25+05:30  Cprogrammer
 * added RCS log
 *
 */
#include <unistd.h>
#include <stralloc.h>
#include <base64.h>
#include <getln.h>
#include <sgetopt.h>
#include <error.h>
#include <noreturn.h>

static char     ssinbuf[1024];
static char     ssoutbuf[512];
static char     sserrbuf[512];
static substdio ssin = SUBSTDIO_FDBUF(read, 0, ssinbuf, sizeof ssinbuf);
static substdio ssout = SUBSTDIO_FDBUF(write, 1, ssoutbuf, sizeof ssoutbuf);
static substdio sserr = SUBSTDIO_FDBUF(write, 2, sserrbuf, sizeof(sserrbuf));

void
logerr(char *s)
{
	if (substdio_puts(&sserr, s) == -1)
		_exit(1);
}

void
logerrf(char *s)
{
	if (substdio_puts(&sserr, s) == -1)
		_exit(1);
	if (substdio_flush(&sserr) == -1)
		_exit(1);
}

no_return void
my_error(char *s1, char *s2, int exit_val)
{
	logerr(s1);
	logerr(": ");
	if (s2) {
		logerr(s2);
		logerr(": ");
	}
	logerr(error_str(errno));
	logerrf("\n");
	_exit(exit_val);
}

int
main(int argc, char **argv)
{
	int             match, opt, encode = 1, ignore_newline = 0;
	stralloc        user = { 0 };
	stralloc        userout = { 0 };

	while ((opt = getopt(argc, argv, "di")) != opteof) {
		switch (opt) {
		case 'd':
			encode = 0;
			break;
		case 'i':
			ignore_newline = 1;
			break;
		}
	}
	for (opt = -1;;) {
		if (getln(&ssin, &user, &match, '\n') == -1)
			my_error("base64: read", 0, 2);
		if (!match && user.len == 0)
			break;
		if (match && ignore_newline)
			user.len--; /*- remove new line */
		if (encode)
			opt = b64encode(&user, &userout);
		else
			opt = b64decode((const unsigned char *) user.s, user.len, &userout);
		if (substdio_bput(&ssout, userout.s, userout.len) == -1)
			my_error("base64: write", 0, 3);
		if (substdio_bput(&ssout, "\n", 1))
			my_error("base64: write", 0, 3);
		if (substdio_flush(&ssout) == -1)
			my_error("base64: write", 0, 3);
	}
	_exit (opt);
}

void
getversion_qbase64_c()
{
	static char    *x = "$Id: qbase64.c,v 1.8 2021-08-29 23:27:08+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
