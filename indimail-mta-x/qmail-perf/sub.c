/*
 * create sub directories as per conf-split
 * create files in these directories
 */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>

char two56bytes[] =  "0000000000000000"
              "0000000000000000"
              "0000000000000000"
              "0000000000000000"
              "0000000000000000"
              "0000000000000000"
              "0000000000000000"
              "0000000000000000"
              "0000000000000000"
              "0000000000000000"
              "0000000000000000"
              "0000000000000000"
              "0000000000000000"
              "0000000000000000"
              "0000000000000000"
              "0000000000000000";

int
main(int argc, char **argv)
{
	int count, i, j, k, s, fd, sub;
	char fn[82];
	struct timeval  tmval1, tmval2;

	if (argc != 4) {
		fprintf(stderr, "sub sync conf-split no-of-files-to-create\n");
		return 1;
	}
	gettimeofday(&tmval1, 0);
	s = atoi(argv[1]); /*- sync */
	j = atoi(argv[2]); /*- conf split */
	k = atoi(argv[3]); /*- no of files */
	for (i = 0; i < j; i++) {
		sprintf(fn, "%d", i);
		if (access(fn, F_OK) && mkdir(fn, 0755) == -1) {
			fprintf(stderr, "mkdir: %s: %s\n", fn, strerror(errno));
			return 1;
		}
	}
	for (i = 0; i < k; i++) {
		if (j) {
			sub = i % j;
			sprintf(fn, "%d/%d.f", sub, i);
		} else
			sprintf(fn, "%d.f", i);
		if ((fd = open(fn, O_CREAT|O_WRONLY, 0644)) == -1) {
			perror(fn);
			return 1;
		}
		if (write(fd, two56bytes, 256) == -1) {
			fprintf(stderr, "write: %s: %s\n", fn, strerror(errno));
			return 1;
		}
		if (s) {
			if (fsync(fd) == -1 || close(fd) == -1) {
				fprintf(stderr, "fsync/close: %s: %s\n", fn, strerror(errno));
				return 1;
			}
		} else {
			if (close(fd) == -1) {
				fprintf(stderr, "close: %s: %s\n", fn, strerror(errno));
				return 1;
			}
		}
	}
	gettimeofday(&tmval2, 0);
	printf ("%f\n", (tmval2.tv_sec + ((double) tmval2.tv_usec / 1000000)) - (tmval1.tv_sec + ((double) tmval1.tv_usec / 1000000)));
	return 0;
}
