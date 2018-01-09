/*
 * $Log: qmail-cat.c,v $
 * Revision 1.7  2009-10-03 15:02:59+05:30  Cprogrammer
 * process all arguments on command line
 * bug fix in my_error - 2nd arg never printed
 *
 * Revision 1.6  2009-03-31 09:07:55+05:30  Cprogrammer
 * open files other than fifo in O_RDONLY
 *
 * Revision 1.5  2004-10-27 17:52:37+05:30  Cprogrammer
 * stralloc_0() not needed if substdio_bput() is used instead of substdio_bputs()
 *
 * Revision 1.4  2004-10-22 20:28:10+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.3  2004-07-15 23:31:56+05:30  Cprogrammer
 * fixed compilation warning
 *
 * Revision 1.2  2004-01-10 09:45:07+05:30  Cprogrammer
 * close file descriptor
 *
 * Revision 1.1  2004-01-07 21:09:03+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "substdio.h"
#include "stralloc.h"
#include "getln.h"
#include "error.h"

static char     ssinbuf[1024];
static substdio ssin = SUBSTDIO_FDBUF(read, 0, ssinbuf, sizeof ssinbuf);
static char     ssoutbuf[512];
static substdio ssout = SUBSTDIO_FDBUF(write, 1, ssoutbuf, sizeof ssoutbuf);
static char     sserrbuf[512];
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

void
my_error(char *s1, char *s2, int exit_val)
{
	logerr(s1);
	logerr(": ");
	if (s2)
	{
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
	stralloc        line = { 0 };
	struct stat     st;
	int             i, match, fd, mode = O_RDONLY;

	if (argc == 1) {
		logerr("USAGE: ");
		logerr(argv[0]);
		logerrf(" filename\n");
		_exit(1);
	}
	for (i = 1;i < argc;i++) {
		if (stat(argv[i], &st) == -1)
			my_error("qmail-cat: stat", argv[i], 1);
		if ((st.st_mode & S_IFMT) == S_IFIFO)
			mode = O_RDWR;
		if ((fd = open(argv[i], mode)) == -1)
			my_error("qmail-cat: open", argv[i], 1);
		if (dup2(fd, 0))
			my_error("qmail-cat: dup2", 0, 1);
		if (fd && close(fd))
			my_error("qmail-cat: close", 0, 1);
		for (;;) {
			if (getln(&ssin, &line, &match, '\n') == -1)
				my_error("qmail-cat: read", 0, 2);
			if (!match && line.len == 0)
				break;
			if (substdio_bput(&ssout, line.s, line.len) == -1)
				my_error("qmail-cat: write", 0, 3);
			if (substdio_flush(&ssout) == -1)
				my_error("qmail-cat: write", 0, 3);
		}
	}
	if (substdio_flush(&ssout) == -1)
		my_error("qmail-cat: write", 0, 3);
	_exit(0);
	/*- Not reached */
	return(0);
}

void
getversion_qmail_cat_c()
{
	static char    *x = "$Id: qmail-cat.c,v 1.7 2009-10-03 15:02:59+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
