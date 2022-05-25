#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <wait.h>
#include <getopt.h>
#include <errno.h>
#include <sys/types.h>

char onezerotwofour[] = 
	"000000000000000000000000000000000000000000000000000000000000000\n"
	"000000000000000000000000000000000000000000000000000000000000000\n"
	"000000000000000000000000000000000000000000000000000000000000000\n"
	"000000000000000000000000000000000000000000000000000000000000000\n"
	"000000000000000000000000000000000000000000000000000000000000000\n"
	"000000000000000000000000000000000000000000000000000000000000000\n"
	"000000000000000000000000000000000000000000000000000000000000000\n"
	"000000000000000000000000000000000000000000000000000000000000000\n"
	"000000000000000000000000000000000000000000000000000000000000000\n"
	"000000000000000000000000000000000000000000000000000000000000000\n"
	"000000000000000000000000000000000000000000000000000000000000000\n"
	"000000000000000000000000000000000000000000000000000000000000000\n"
	"000000000000000000000000000000000000000000000000000000000000000\n"
	"000000000000000000000000000000000000000000000000000000000000000\n"
	"000000000000000000000000000000000000000000000000000000000000000\n"
	"000000000000000000000000000000000000000000000000000000000000000\n";

void
do_exit()
{
	_exit(111);
}

void
open_mail(char *from, char *to, int n)
{
	int             len, fd;
	FILE           *fp;

	if (!(fp = tmpfile())) {
		perror("tmpfile");
		exit(111);
	}
	fd = fileno(fp);
	fprintf(fp, "From: %s\n", from);
	fprintf(fp, "To: %s\n", to);
	fprintf(fp, "Subject: Test message number %d\n\n", n);
	fprintf(fp, "this is a test mesage.\n");
	fprintf(fp, "%s", onezerotwofour);
	fflush(fp);
	if (dup2(fd, 0) == -1) {
		perror("dup2");
		exit(111);
	}
	if (fd)
		close(fd);
	lseek(0, 0, SEEK_SET);
}

char           *usage = "usage: %s [ -q qmail-inject-path] -n mail_count -p process_count -f sender -t recipient\n";

int
main(int argc, char **argv)
{
	int             i, j, k, mail_count, process_count, remainder, mails_per_process, status, debug = 0;
	char           *from, *to, *ptr;
	char           *binqqargs[6] = { "/usr/bin/qmail-inject", "-a", "-f", 0, 0, 0 };
	pid_t           pid;

	ptr = strrchr(argv[0], '/');
	if (ptr)
		ptr++;
	else
		ptr = argv[0];
	from = to = (char *) NULL;
	mail_count = process_count = 0;
	while ((i = getopt(argc, argv, "q:dn:p:f:t:")) != -1) {
		switch (i) {
		case 'd':
			debug = 1;
			break;
		case 'n':
			mail_count = atoi(optarg);
			break;
		case 'p':
			process_count = atoi(optarg);
			break;
		case 'f':
			from = optarg;
			break;
		case 't':
			to = optarg;
			break;
		case 'q':
			binqqargs[0] = optarg;
			break;
		default:
			fprintf(stderr, usage, ptr);
			exit(100);
			break;
		}
	}

	if (!mail_count || !process_count || !from || !to) {
		fprintf(stderr, usage, ptr);
		exit(100);
	}

	binqqargs[3] = from;
	binqqargs[4] = to;
	mails_per_process = mail_count / process_count;
	remainder = mail_count % process_count;

	if (debug) {
		printf("mail per process = %d, remainder = %d\n", mails_per_process, remainder);
		fflush(stdout);
	}
	for (i = 0; i < process_count; i++) {
		k = mails_per_process * i + 1;
		for (j = k; j < k + mails_per_process; j++) {
			if (debug) {
				printf("%d\n", j);
				continue;
			}
			switch (pid = fork())
			{
			case -1:
				perror("fork");
				do_exit();
			case 0:
				open_mail(from, to, j);
				execv(*binqqargs, binqqargs);
				fprintf(stderr, "execv: %s: %s\n", *binqqargs, strerror(errno));
				/*- printf("%d %d %d\n", k + 1, k + mails_per_process, j); -*/
				exit(0);
			default:
				break;
			}
		}
	}
	if (remainder) {
		for (i = j; i < j + remainder; i++) {
			if (debug) {
				printf("%d\n", j);
				continue;
			}
			switch (pid = fork())
			{
			case -1:
				perror("fork");
				do_exit();
			case 0:
				open_mail(from, to, i);
				execv(*binqqargs, binqqargs);
				fprintf(stderr, "execv: %s: %s\n", *binqqargs, strerror(errno));
				/*- printf("%d %d\n", k + 1, k + remainder); -*/
				exit(0);
			default:
				break;
			}
		}
	}
	if (debug)
		return 0;
	while (1) {
		switch (wait(&status)) {
		case -1:
			if (errno == EINTR)
				continue;
			else
			if (errno == ECHILD)
				exit(0);
		default:
			break;
		}
	}
	return 0;
}
