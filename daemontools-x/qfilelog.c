/*
 * $Log: qfilelog.c,v $
 * Revision 1.5  2021-08-30 12:04:53+05:30  Cprogrammer
 * define funtions as noreturn
 *
 * Revision 1.4  2020-09-16 19:03:09+05:30  Cprogrammer
 * fix compiler warning for FreeBSD
 *
 * Revision 1.3  2020-06-08 22:51:43+05:30  Cprogrammer
 * quench compiler warning
 *
 * Revision 1.2  2004-10-22 20:28:04+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.1  2004-07-27 23:00:14+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <noreturn.h>

#define PAUSE sleep(60)

void
errmsg(const char *m)
{
	if (write(2, m, strlen(m)) == -1)
		;
}

void
warn_sys(const char *m)
{
	errmsg("qfilelog: warning: ");
	errmsg(m);
	errmsg(": ");
	errmsg(strerror(errno));
	errmsg("\n");
}

void
warn(const char *m)
{
	errmsg("qfilelog: warning: ");
	errmsg(m);
	errmsg(".\n");
}

no_return void
fatal(const char *m)
{
	errmsg("qfilelog: fatal error: ");
	errmsg(m);
	errmsg("\n");
	exit(1);
}

#define BUFSIZE 4096
static int      fd;
static char    *filename;
static char     buf[BUFSIZE];

ssize_t
do_read(void)
{
	ssize_t         rd = read(0, buf, BUFSIZE);
	if (rd == -1)
		rd = 0;
	return rd;
}

void
do_write(ssize_t rd)
{
	ssize_t         offset = 0;
	ssize_t         wr;
	while (offset < rd) {
		wr = write(fd, buf + offset, rd - offset);
		if (wr == -1) {
			warn_sys("Can't write, pausing");
			PAUSE;
		} else
			offset += wr;
	}
}

void
do_close(void)
{
	if (fd >= 0) {
		while (fsync(fd) == -1) {
			warn_sys("Error syncing the file, pausing");
			PAUSE;
		}
		while (close(fd) == -1) {
			warn_sys("Error closing the file, pausing");
			PAUSE;
		}
	}
}

void
do_open(void)
{
	do {
		fd = open(filename, O_WRONLY | O_CREAT | O_APPEND, 0666);
		if (fd == -1) {
			warn_sys("Error opening the output file, pausing");
			PAUSE;
		}
	}
	while (fd == -1);
}

no_return void
loop(void)
{
	ssize_t         rd;
	for (;;) {
		if ((rd = do_read()))
			do_write(rd);
	}
}

void
catch_hup(int flag)
{
	signal(SIGHUP, catch_hup);
	do_close();
	do_open();
}

no_return void
catch_int(int flag)
{
	do_close();
	exit(0);
}

int
main(int argc, char *argv[])
{
	if (argc < 2) {
		fatal("usage: filelog filename\n");
		return 1;
	}
	filename = argv[1];
	signal(SIGHUP, catch_hup);
	signal(SIGINT, catch_int);
	signal(SIGTERM, catch_int);
	fd = -1;
	do_open();
	loop();
}

void
getversion_qfilelog_c()
{
	static char    *x = "$Id: qfilelog.c,v 1.5 2021-08-30 12:04:53+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
