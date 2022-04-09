#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <getopt.h>
#include <sys/select.h>

ssize_t
timeoutread(long t, int fd, char *buf, size_t len)
{
	fd_set          rfds;
	struct timeval  tv;

	tv.tv_sec = t;
	tv.tv_usec = 0;

	FD_ZERO(&rfds);
	FD_SET(fd, &rfds);

	if (select(fd + 1, &rfds, (fd_set *) 0, (fd_set *) 0, &tv) == -1)
		return -1;
	if (FD_ISSET(fd, &rfds))
		return read(fd, buf, len);

	errno = ETIMEDOUT;
	return -1;
}

int
main(int argc, char **argv)
{
	int fd, n, opt, timeout = 5;
	char buffer[10];

	while ((opt = getopt(argc, argv, "t:")) != -1) {
		switch (opt)
		{
		case 't':
		timeout = atoi(optarg);
			break;
		}
	}
	if (optind + 1 !=argc) {
		fprintf(stderr, "usage: readfifo [-t timeout] fifo_path\n");
		return 1;
	}
	if ((fd = open(argv[optind], O_RDONLY|O_NDELAY, 0)) == -1) {
		perror(argv[1]);
		return 1;
	}
	if ((n = timeoutread(timeout, fd, buffer, 1)) == -1)
		return 1;
	return 0;
}
