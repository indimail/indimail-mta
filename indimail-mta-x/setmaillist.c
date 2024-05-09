/*
 * $Log: setmaillist.c,v $
 * Revision 1.4  2021-08-29 23:27:08+05:30  Cprogrammer
 * define functions as noreturn
 *
 * Revision 1.3  2004-10-22 20:30:17+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.2  2004-10-22 15:39:14+05:30  Cprogrammer
 * removed readwrite.h
 *
 * Revision 1.1  2004-10-21 22:47:18+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <unistd.h>
#include <sys/stat.h>
#include <substdio.h>
#include <subfd.h>
#include <byte.h>
#include <strerr.h>
#include <stralloc.h>
#include <getln.h>
#include <open.h>
#include <noreturn.h>

#define FATAL "setmaillist: fatal: "

int             rename(const char *, const char *);

no_return void
usage()
{
	strerr_die1x(100, "setmaillist: usage: setmaillist list.bin list.tmp");
}

no_return void
writeerr(const char *s)
{
	strerr_die4sys(111, FATAL, "unable to write to ", s, ": ");
}

void
out(substdio *ss, const char *s, int len, char *fntmp)
{
	if (substdio_put(ss, s, len) == -1)
		writeerr(fntmp);
}

int
main(int argc, char **argv)
{
	stralloc        line = { 0 };
	int             match, fd;
	char           *fnbin, *fntmp;
	char            buf[1024];
	substdio        ss;

	umask(033);
	if (!(fnbin = argv[1]))
		usage();
	if (!(fntmp = argv[2]))
		usage();
	if ((fd = open_trunc(fntmp)) == -1)
		strerr_die4sys(111, FATAL, "unable to create ", fntmp, ": ");
	substdio_fdbuf(&ss, write, fd, buf, sizeof buf);
	do {
		if (getln(subfdinsmall, &line, &match, '\n') == -1)
			strerr_die2sys(111, FATAL, "unable to read input: ");
		while (line.len) {
			if (line.s[line.len - 1] != '\n' && line.s[line.len - 1] != ' ' && line.s[line.len - 1] != '\t')
				break;
			--line.len;
		}

		if (byte_chr(line.s, line.len, '\0') != line.len)
			strerr_die2x(111, FATAL, "NUL in input");
		if (line.len && line.s[0] != '#') {
			if ((line.s[0] == '.') || (line.s[0] == '/'))
			{
				out(&ss, line.s, line.len, fntmp);
				out(&ss, "", 1, fntmp);
			} else {
				if (line.len > 800)
					strerr_die2x(111, FATAL, "addresses must be under 800 bytes");
				if (line.s[0] != '&')
					out(&ss, "&", 1, fntmp);
				out(&ss, line.s, line.len, fntmp);
				out(&ss, "", 1, fntmp);
			}
		}
	} while (match);
	if (substdio_flush(&ss) == -1)
		writeerr(fntmp);
	if (fsync(fd) == -1)
		writeerr(fntmp);
	if (close(fd) == -1)
		writeerr(fntmp);				/*- NFS stupidity */
	if (rename(fntmp, fnbin) == -1)
		strerr_die6sys(111, FATAL, "unable to move ", fntmp, " to ", fnbin, ": ");
	_exit(0);
	/*- Not reached */
	return(0);
}

void
getversion_setmaillist_c()
{
	const char     *x = "$Id: setmaillist.c,v 1.4 2021-08-29 23:27:08+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
