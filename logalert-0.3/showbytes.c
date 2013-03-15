/*
 * $Log: $
 */
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include "common.h"

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
	printf("%d\n", bytes);
	fflush(stdout);
	_exit (0);
}
