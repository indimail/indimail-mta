/*
 * $Log: showbytes.c,v $
 * Revision 1.2  2013-05-15 00:16:21+05:30  Cprogrammer
 * fixed warnings
 *
 * Revision 1.1  2013-03-15 09:30:20+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include "common.h"

#ifndef	lint
static char     sccsid[] = "$Id: showbytes.c,v 1.2 2013-05-15 00:16:21+05:30 Cprogrammer Exp mbhangui $";
#endif

int
main(int argc, char **argv)
{
	umdir_t bytes;
	int fd;

	if (argc != 2) {
		write(2, "usage: showbytes statusfile\n", 28);
		_exit (1);
	}
	if ((fd = open(argv[1], O_RDONLY)) == -1) {
		perror(argv[1]);
		_exit (1);
	}
	if (lseek(fd, sizeof(pid_t), SEEK_SET) == -1) {
		perror("lseek");
		_exit (1);
	}
	if (read(fd, &bytes, sizeof(umdir_t)) == -1) {
		perror("read");
		_exit (1);
	}
	close(fd);
	printf("%d\n", (int) bytes);
	fflush(stdout);
	_exit (0);
}

void
getversion_showbytes_c()
{
	printf("%s\n", sccsid);
}
