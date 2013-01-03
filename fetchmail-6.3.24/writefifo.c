/*
 * $Log: writefifo.c,v $
 * Revision 1.1  2003-12-13 16:05:40+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>

int
main(int argc, char **argv)
{
	int             fd;

	if (argc != 2)
	{
		fprintf(stderr, "writefifo filename\n");
		return (1);
	}
	if ((fd = open(argv[1], O_WRONLY | O_NDELAY)) == -1)
	{
		fprintf(stderr, "open: %s: %s\n", argv[1], strerror(errno));
		return (1);
	}
	if (write(fd, "", 1) == -1)
	{
		fprintf(stderr, "write: %s: %s\n", argv[1], strerror(errno));
		return (1);
	}
	close(fd);
	return (0);
}
