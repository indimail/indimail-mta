/*
 * $Log: mbox2maildir.c,v $
 * Revision 1.7  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.6  2021-08-29 23:27:08+05:30  Cprogrammer
 * define functions as noreturn
 *
 * Revision 1.5  2020-05-11 11:00:57+05:30  Cprogrammer
 * fixed shadowing of global variables by local variables
 *
 * Revision 1.4  2019-07-18 10:50:25+05:30  Cprogrammer
 * removed unecessary header hasflock.h
 *
 * Revision 1.3  2016-06-20 08:32:40+05:30  Cprogrammer
 * minor indentation change
 *
 * Revision 1.2  2008-07-15 19:57:19+05:30  Cprogrammer
 * porting for Mac OS X
 *
 * Revision 1.1  2008-06-06 14:30:51+05:30  Cprogrammer
 * Initial revision
 *
 *
 * mbox2maildir.c
 * Author: Nikola Vladov
 * Converted to substdio by Manvendra Bhangui (mbhangui@gmail.com)
 */
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/file.h>
#include <signal.h>
#include <fmt.h>
#include <lock.h>
#include <substdio.h>
#include <noreturn.h>

int             gettmpfd(char *, char *);

static char     host[68], filename[96], filenamen[96];
static char     buf_1_space[1024];
static char     buf_f_space[8192];	/*- buffer for disk writing */
static int      flagsleep = 0, state = 1, tmpfd = -1, seenat = 0;
static substdio buf_1, buf_f;

void
w2f()
{
	substdio_flush(&buf_1);
}

no_return void
die_clean()
{
	if (tmpfd >= 0)
		unlink(filename);
	w2f();
	_exit(1);
}

void
w2(const char *s)
{
	substdio_put(&buf_1, s, strlen(s));
}

void
w2nl(const char *s)
{
	w2(s);
	w2("\n");
}

void
put(const char *s, int i)
{
	substdio_put(&buf_f, s, i);
}

no_return void
handler(int i)
{
	w2nl("Mbox has been locked 30 seconds straight");
	die_clean();
}

void
safe_close(int fd)
{
	substdio_flush(&buf_f);
	if (fsync(fd) || close(fd) || link(filename, filenamen)) {
		w2(filename);
		w2nl(": close/link error");
		die_clean();
	}
	unlink(filename);
	tmpfd = -1;
	if (flagsleep == -1)
		flagsleep = 1;
}

ssize_t
safe_write(int fd, const char *s, size_t len)
{
	if (len == write(fd, s, len))
		return (len);
	w2(filename);
	w2nl(": write error");
	die_clean();
}

void
safe_open()
{
	if ((tmpfd = gettmpfd(filename, filenamen)) < 0) {
		w2nl("unable to open tmpfd");
		die_clean();
	}
	w2(filename + 4);
	w2(": ");
	substdio_fdbuf(&buf_f, (ssize_t (*)(int,  char *, size_t)) safe_write, tmpfd, buf_f_space, sizeof(buf_f_space));
}

void
blast(char *ch, int r)
{
	int             i;
	const char     *prefix = ">From ";
	const char     *from = prefix + 1;

	for (i = 0; i < r; i++, ch++) {
		switch (state)
		{
		case 0:			/*- .  */
			if (*ch == '\n')
				state = 1;
			break;
		case 1:			/*- \n */
			if (*ch == '\n')
				break;
			if (*ch == 'F') {
				state = 11;
				continue;
			}
			if (*ch == '>') {
				state = 21;
				continue;
			}
			state = 0;
			break;
		case 2:			/*- skip From */
			substdio_put(&buf_1, ch, 1);
			if (*ch == '@')
				seenat = 1;
			if (*ch == '\n') {
				if (!seenat)
					w2nl("**** WARNING: no at sing ****\n");
				state = 1;
			}
			continue;
		case 11:
		case 12:
		case 13:
		case 14:
			if (*ch == from[state - 10]) {
				state += 1;
				if (state == 15) {
					state = 2;
					seenat = 0;
					if (tmpfd >= 0)
						safe_close(tmpfd);
					safe_open();
				}
				continue;
			}
			put(from, state - 10);
			state = 0;
			break;
		case 21:
		case 22:
		case 23:
		case 24:
		case 25:
			if (*ch == prefix[state - 20]) {
				state += 1;
				if (state == 26) {
					state = 0;
					put(from, 5);
				}
				continue;
			}
			put(prefix, state - 20);
			state = 0;
			break;
		}
		put(ch, 1);
	}
}

unsigned int
fmt_ulong0(char *s, unsigned long u, unsigned int n)
{
	unsigned int    len;		/*- DJB */

	len = fmt_ulong(0, u);
	while (len < n) {
		if (s)
			*s++ = '0';
		++len;
	}
	if (s)
		fmt_ulong(s, u);
	return len;
}

void
normalize(struct timeval *now)
{
	while (now->tv_usec < 0) {
		now->tv_sec -= 1;
		now->tv_usec += 1000000;
	}
	while (now->tv_usec > 999999) {
		now->tv_sec += 1;
		now->tv_usec -= 1000000;
	}
}

int
gettmpfd(char *fn, char *fnn)
{
	char           *p;
	int             fd;
	struct timeval  now;

	gettimeofday(&now, 0);
	for (;;) {
		p = fn + 4;
		p += fmt_ulong(p, now.tv_sec);
		*p++ = '.';
		p += fmt_ulong0(p, now.tv_usec, 6);
		*p = '\0';
		if (host[0]) {
			*p++ = '.';
			strcpy(p, host);
		}
		strcpy(fnn + 4, fn + 4);
		fd = open(fn, O_WRONLY | O_CREAT | O_EXCL, 0600);
		if (fd >= 0 || errno != EEXIST)
			break;
		now.tv_usec += 1;
		normalize(&now);
	}
	return (fd);
}

int
main(int argc, char **argv)
{
	int             fd, ds, r, flagkeep = 0;
	char            buf[8192];

	substdio_fdbuf(&buf_1, (ssize_t (*)(int,  char *, size_t)) write, 1, buf_1_space, sizeof(buf_1_space));
	while (argv[1] && argv[1][0] == '-') {
		char           *p = argv[1] + 1;
		for (; *p; p++) {
			switch (*p)
			{
			case 'k':
				flagkeep = 1;
				break;
			case 's':
				flagsleep = -1;
				break;
			default:
				goto usage;
			}
		}
		argc--;
		argv++;
	}
	if (argc < 3) {
usage:
		w2("usage: mbox2maildir [-ks] mbox Maildir\n");
		die_clean();
	}
	strcpy(filename, "tmp/");
	strcpy(filenamen, "new/");

	if ((fd = open(argv[1], O_RDWR)) == -1) {
		w2(argv[1]);
		w2nl(": open error");
		die_clean();
	}
	if (chdir(argv[2])) {
		w2("chdir error: ");
		w2nl(argv[2]);
		die_clean();
	}

	signal(SIGALRM, handler);
	alarm(30);
	if (lock_ex(fd) == -1) {
		w2(argv[1]);
		w2nl(": lock error");
		die_clean();
	}
	alarm(0);
	signal(SIGALRM, SIG_DFL);

	host[0] = '\0';
	gethostname(host, sizeof(host) - 1);
	host[sizeof(host) - 1] = '\0';

	for (;;) {
		if ((r = read(fd, buf, sizeof(buf))) == -1) {
			w2(argv[1]);
			w2nl(": reading error");
			die_clean();
		}
		if (r == 0)
			break;
		blast(buf, r);
	}
	if (tmpfd >= 0)
		safe_close(tmpfd);

	/*
	 * power off
	 */
	if ((ds = open("new/", O_RDONLY)) == -1) {
		w2nl("open new/ error\n");
		die_clean();
	}
	if (fsync(ds) && errno == EIO) {
		w2nl("fsync new/ error\n");
		die_clean();
	}
	close(ds);

	if (!flagkeep) {
		if (ftruncate(fd, 0) || fsync(fd) || close(fd)) {
			w2(argv[1]);
			w2nl(": fsync/close error");
			die_clean();
		}
	} else
		close(fd);

	w2f();
	if (flagsleep == 1)
		sleep(1);
	_exit(0);
}

void
getversion_mbox2maildir_c()
{
	const char     *x = "$Id: mbox2maildir.c,v 1.7 2024-05-09 22:03:17+05:30 mbhangui Exp mbhangui $";

	x++;
}
