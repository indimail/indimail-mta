/*
 * $Log: mktempfile.c,v $
 * Revision 1.1  2021-06-09 19:32:47+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <unistd.h>
#include <fcntl.h>
#include <stralloc.h>
#include <env.h>
#include <substdio.h>
#include <error.h>
#include <fmt.h>
#include "mktempfile.h"

int
mktempfile(int seekfd)
{
	char            inbuf[2048], outbuf[2048], strnum[FMT_ULONG];
	char           *tmpdir;
	static stralloc tmpFile = { 0 };
	struct substdio ssin;
	struct substdio ssout;
	int             fd;

	if (lseek(seekfd, 0, SEEK_SET) == 0)
		return (0);
	if (errno == EBADF)
		_exit(54);
	if (!(tmpdir = env_get("TMPDIR")))
		tmpdir = "/tmp";
	if (!stralloc_copys(&tmpFile, tmpdir) ||
			!stralloc_cats(&tmpFile, "/qmailFilterXXX") ||
			!stralloc_catb(&tmpFile, strnum, fmt_ulong(strnum, (unsigned long) getpid())) ||
			!stralloc_0(&tmpFile))
		_exit(51);
	if ((fd = open(tmpFile.s, O_RDWR | O_EXCL | O_CREAT, 0600)) == -1)
		return (-1);
	unlink(tmpFile.s);
	substdio_fdbuf(&ssout, write, fd, outbuf, sizeof (outbuf));
	substdio_fdbuf(&ssin, read, seekfd, inbuf, sizeof (inbuf));
	switch (substdio_copy(&ssout, &ssin))
	{
	case -2: /*- read error */
		close(fd);
		_exit(54);
	case -3: /*- write error */
		close(fd);
		_exit(53);
	}
	if (substdio_flush(&ssout) == -1) {
		close(fd);
		_exit(68);
	}
	if (fd != seekfd) {
		if (dup2(fd, seekfd) == -1) {
			close(fd);
			_exit(68);
		}
		close(fd);
	}
	if (lseek(seekfd, 0, SEEK_SET) != 0) {
		close(seekfd);
		_exit(54);
	}
	return (0);
}

#ifndef	lint
void
getversion_mktempfile_c()
{
	static char    *x = "$Id: mktempfile.c,v 1.1 2021-06-09 19:32:47+05:30 Cprogrammer Exp mbhangui $";

	x = sccsidmktempfileh;
	x++;
}
#endif
