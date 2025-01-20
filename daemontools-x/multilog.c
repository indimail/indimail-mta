/*
 * $Id: multilog.c,v 1.10 2024-05-09 22:39:36+05:30 mbhangui Exp mbhangui $
 */
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <direntry.h>
#include <alloc.h>
#include <subfd.h>
#include <substdio.h>
#include <strerr.h>
#include <error.h>
#include <open.h>
#include <lock.h>
#include <scan.h>
#include <str.h>
#include <byte.h>
#include <seek.h>
#include <timestamp.h>
#include <wait.h>
#include <coe.h>
#include <env.h>
#include <fd.h>
#include <sig.h>
#include "match.h"
#include "deepsleep.h"
#include "hasunlinkat.h"

#define FATAL "multilog: fatal: "
#define WARNING "multilog: warning: "

int             rename(const char *, const char *);

mode_t          old_umask;
static int      f_sync;

void
pause3(const char *s1, const char *s2, const char *s3)
{
	strerr_warn4(WARNING, s1, s2, s3, &strerr_sys);
	deepsleep(5);
}

void
pause5(const char *s1, const char *s2, const char *s3, const char *s4, const char *s5)
{
	strerr_warn6(WARNING, s1, s2, s3, s4, s5, &strerr_sys);
	deepsleep(5);
}

int             fdstartdir;
int            *f;

void
f_init(char **script)
{
	int             i;
	int             fd;

	for (i = 0; script[i]; ++i);
	if (!(f = (int *) alloc(i * sizeof(*f))))
		strerr_die2x(111, FATAL, "out of memory");
	for (i = 0; script[i]; ++i) {
		fd = -1;
		if (script[i][0] == '=') {
			if (fchdir(fdstartdir) == -1)
				strerr_die2sys(111, FATAL, "unable to switch to starting directory: ");
			if ((fd = open_append(script[i] + 1)) == -1)
				strerr_die4sys(111, FATAL, "unable to create ", script[i] + 1, ": ");
			close(fd);
			if ((fd = open_write(script[i] + 1)) == -1)
				strerr_die4sys(111, FATAL, "unable to write ", script[i] + 1, ": ");
			coe(fd);
		}
		f[i] = fd;
	}
}

struct cyclog
{
	char            buf[512];
	substdio        ss;
	int             fdcurrent;
	unsigned long   bytes;
	unsigned long   num;
	unsigned long   size;
	char           *processor;
	char           *dir;
	int             fddir;
	int             fdlock;
	int             flagselected;
} *c;
int             cnum;
char            fn[40];

int
filesfit(struct cyclog *d)
{
	DIR            *dir;
	direntry       *x;
	int             count;
	int             i;

	if (!(dir = opendir(".")))
		return -1;
	fn[0] = '@';
	fn[1] = 'z';
	fn[2] = 0;
	count = 0;
	for (;;) {
		errno = 0;
		if (!(x = readdir(dir)))
			break;
		if (x->d_name[0] == '@' && str_len(x->d_name) >= 25) {
			++count;
			if (str_diff(x->d_name, fn) < 0) {
				for (i = 0; i < 25; ++i)
					fn[i] = x->d_name[i];
				fn[25] = 0;
			}
		}
	}
	if (errno) {
		closedir(dir);
		return -1;
	}
	closedir(dir);
	if (count < d->num)
		return 1;
	if (!(dir = opendir(".")))
		return -1;
	for (;;) {
		errno = 0;
		if (!(x = readdir(dir)))
			break;
		if (x->d_name[0] == '@' && str_len(x->d_name) >= 25 && str_start(x->d_name, fn)) {
			unlink(x->d_name);
			break;
		}
	}
	if (errno) {
		closedir(dir);
		return -1;
	}
	closedir(dir);
	return 0;
}

void
finish(struct cyclog *d, const char *file, const char *code)
{
	struct stat     st;
#ifdef HASUNLINKAT
	int             fd;
#endif

	for (;;) {
		if (stat(file, &st) == 0)
			break;
		if (errno == error_noent)
			return;
		pause5("unable to stat ", d->dir, "/", file, ", pausing: ");
	}
	if (st.st_nlink == 1) {
		for (;;) {
			timestamp(fn);
			fn[25] = '.';
			fn[26] = code[0];
			fn[27] = 0;
			if (link(file, fn) == 0)
				break;
			pause5("unable to link to ", d->dir, "/", fn, ", pausing: ");
		}
	}
#ifdef HASUNLINKAT
	for (;;) {
		if ((fd = open_read(".")) != -1)
			break;
		pause3("unable to read ", d->dir, ", pausing: ");
	}
	while (unlinkat(fd, file, 0) == -1)
		pause5("unable to remove (unlinkat) ", d->dir, "/", file, ", pausing: ");
	close(fd);
#else
	while (unlink(file) == -1) {
		if (errno == error_noent) /*- don't let removal stuck up in this loop */
			break;
		pause5("unable to remove (unlink) ", d->dir, "/", file, ", pausing: ");
	}
#endif
	for (;;) {
		switch (filesfit(d))
		{
		case 1:
			return;
		case -1:
			pause3("unable to read ", d->dir, ", pausing: ");
		}
	}
}

void
startprocessor(struct cyclog *d)
{
	char           *args[4];
	int             fd;

	sig_uncatch(sig_term);
	sig_uncatch(sig_alarm);
	sig_unblock(sig_term);
	sig_unblock(sig_alarm);
	if ((fd = open_read("previous")) == -1)
		return;
	if (fd_move(0, fd) == -1)
		return;
	if ((fd = open_trunc("processed")) == -1)
		return;
	if (fd_move(1, fd) == -1)
		return;
	if ((fd = open_read("state")) == -1)
		return;
	if (fd_move(4, fd) == -1)
		return;
	if ((fd = open_trunc("newstate")) == -1)
		return;
	if (fd_move(5, fd) == -1)
		return;
	args[0] = (char *) "sh";
	args[1] = (char *) "-c";
	args[2] = d->processor;
	args[3] = 0;
	execve("/bin/sh", args, environ);
}

void
fullcurrent(struct cyclog *d)
{
	int             fd;
	int             pid;
	int             wstat;

	while (fchdir(d->fddir) == -1)
		pause3("unable to switch to ", d->dir, ", pausing: ");
	if (f_sync)
		while (fsync(d->fdcurrent) == -1)
			pause3("unable to write ", d->dir, "/current to disk, pausing: ");
	close(d->fdcurrent);
	while (rename("current", "previous") == -1)
		pause3("unable to rename current to previous in directory ", d->dir, ", pausing: ");
	while ((d->fdcurrent = open_append("current")) == -1)
		pause3("unable to create ", d->dir, "/current, pausing: ");
	coe(d->fdcurrent);
	d->bytes = 0;
	while (fchmod(d->fdcurrent, 0644) == -1)
		pause3("unable to set mode of ", d->dir, "/current, pausing: ");
	while (chmod("previous", 0744) == -1)
		pause3("unable to set mode of ", d->dir, "/previous, pausing: ");
	if (!d->processor)
		finish(d, "previous", "s");
	else {
		for (;;) {
			while ((pid = fork()) == -1)
				pause3("unable to fork for processor in ", d->dir, ", pausing: ");
			if (!pid) {
				startprocessor(d);
				strerr_die4sys(111, FATAL, "unable to run ", d->processor, ": ");
			}
			if (wait_pid(&wstat, pid) == -1)
				pause3("wait failed for processor in ", d->dir, ", pausing: ");
			else
			if (wait_crashed(wstat))
				pause3("processor crashed in ", d->dir, ", pausing: ");
			else
			if (!wait_exitcode(wstat))
				break;
			strerr_warn4(WARNING, "processor failed in ", d->dir, ", pausing", 0);
			deepsleep(5);
		}
		while ((fd = open_append("processed")) == -1)
			pause3("unable to create ", d->dir, "/processed, pausing: ");
		if (f_sync)
			while (fsync(fd) == -1)
				pause3("unable to write ", d->dir, "/processed to disk, pausing: ");
		while (fchmod(fd, 0744) == -1)
			pause3("unable to set mode of ", d->dir, "/processed, pausing: ");
		close(fd);
		while ((fd = open_append("newstate")) == -1)
			pause3("unable to create ", d->dir, "/newstate, pausing: ");
		if (f_sync)
			while (fsync(fd) == -1)
				pause3("unable to write ", d->dir, "/newstate to disk, pausing: ");
		close(fd);
		while (unlink("previous") == -1)
			pause3("unable to remove ", d->dir, "/previous, pausing: ");
		while (rename("newstate", "state") == -1)
			pause3("unable to rename newstate to state in directory ", d->dir, ", pausing: ");
		finish(d, "processed", "s");
	}
}

ssize_t
c_write(int pos, char *buf, int len)
{
	struct cyclog  *d;
	int             w;

	d = c + pos;
	if (d->size > 0 && d->bytes >= d->size)
		fullcurrent(d);
	if (d->size > 0 && len >= d->size - d->bytes)
		len = d->size - d->bytes;

	if (d->size > 0 && d->bytes + len >= d->size - 2000) {
		w = byte_rchr(buf, len, '\n');
		if (w < len)
			len = w + 1;
	}
	for (;;) {
		w = write(d->fdcurrent, buf, len);
		if (w > 0)
			break;
		pause3("unable to write to ", d->dir, "/current, pausing: ");
	}
	d->bytes += w;
	if (d->size > 0 && d->bytes >= d->size - 2000) {
		if (buf[w - 1] == '\n')
			fullcurrent(d);
	}
	return w;
}

void
restart(struct cyclog *d)
{
	struct stat     st;
	int             fd;
	int             flagprocessed;

	if (fchdir(fdstartdir) == -1)
		strerr_die2sys(111, FATAL, "unable to switch to starting directory: ");
	if (old_umask != 022)
		umask(old_umask);
	if (mkdir(d->dir, 0755) && errno != error_exist)
		strerr_die4sys(111, FATAL, "unable to create directory ", d->dir, ": ");
	if (old_umask != 022)
		umask(022);
	d->fddir = open_read(d->dir);
	if ((d->fddir == -1) || (fchdir(d->fddir) == -1))
		strerr_die4sys(111, FATAL, "unable to open directory ", d->dir, ": ");
	coe(d->fddir);
	d->fdlock = open_append("lock");
	if ((d->fdlock == -1) || (lock_exnb(d->fdlock) == -1))
		strerr_die4sys(111, FATAL, "unable to lock directory ", d->dir, ": ");
	coe(d->fdlock);
	if (stat("current", &st) == -1) {
		if (errno != error_noent)
			strerr_die4sys(111, FATAL, "unable to stat ", d->dir, "/current: ");
	} else
	if (st.st_mode & 0100) {
		fd = open_append("current");
		if (fd == -1)
			strerr_die4sys(111, FATAL, "unable to append to ", d->dir, "/current: ");
		if (fchmod(fd, 0644) == -1)
			strerr_die4sys(111, FATAL, "unable to set mode of ", d->dir, "/current: ");
		coe(fd);
		d->fdcurrent = fd;
		d->bytes = st.st_size;
		return;
	}
	unlink("state");
	unlink("newstate");
	flagprocessed = 0;
	if (stat("processed", &st) == -1) {
		if (errno != error_noent)
			strerr_die4sys(111, FATAL, "unable to stat ", d->dir, "/processed: ");
	} else
	if (st.st_mode & 0100)
		flagprocessed = 1;
	if (flagprocessed) {
		unlink("previous");
		finish(d, "processed", "s");
	} else {
		unlink("processed");
		finish(d, "previous", "u");
	}
	finish(d, "current", "u");
	if ((fd = open_trunc("state")) == -1)
		strerr_die4sys(111, FATAL, "unable to write to ", d->dir, "/state: ");
	close(fd);
	if ((fd = open_append("current")) == -1)
		strerr_die4sys(111, FATAL, "unable to write to ", d->dir, "/current: ");
	if (fchmod(fd, 0644) == -1)
		strerr_die4sys(111, FATAL, "unable to set mode of ", d->dir, "/current: ");
	coe(fd);
	d->fdcurrent = fd;
	d->bytes = 0;
}

void
c_init(char **script)
{
	int             i;
	struct cyclog  *d;
	char           *processor;
	unsigned long   num, rot, size;

	cnum = 0;
	for (i = 0; script[i]; ++i) {
		if ((script[i][0] == '.') || (script[i][0] == '/'))
			++cnum;
	}
	if (!(c = (struct cyclog *) alloc(cnum * sizeof(*c))))
		strerr_die2x(111, FATAL, "out of memory");
	d = c;
	processor = 0;
	num = 10;
	size = -1;
	for (i = 0; script[i]; ++i) {
		if (script[i][0] == 's') {
			scan_ulong(script[i] + 1, &size);
			if (size > 0 && size < 4096)
				size = 4096;
			if (size > 16777215)
				size = 16777215;
		} else
		if (script[i][0] == 'n') {
			scan_ulong(script[i] + 1, &num);
			if (num < 2)
				num = 2;
		} else
		if (script[i][0] == 'r') {
			struct itimerval timer;

			scan_ulong(script[i] + 1, &rot);
			if (rot < 1)
				rot = 1;
			timer.it_value.tv_sec     = rot - (time(NULL) % rot);
			timer.it_value.tv_usec    = 0;
			timer.it_interval.tv_sec  = rot;
			timer.it_interval.tv_usec = 0;
			if (setitimer(ITIMER_REAL, &timer, NULL) == -1)
				strerr_die2sys(111,FATAL,"setitimer: ");
		} else
		if (script[i][0] == '!') {
			processor = script[i] + 1;
		} else
		if ((script[i][0] == '.') || (script[i][0] == '/')) {
			d->num = num;
			d->size = size;
			d->processor = processor;
			d->dir = script[i];
			substdio_fdbuf(&d->ss, (ssize_t (*)(int,  char *, size_t)) c_write, d - c, d->buf, sizeof d->buf);
			restart(d);
			++d;
		}
	}
}

void
c_quit(void)
{
	int             j;

	for (j = 0; j < cnum; ++j) {
		substdio_flush(&c[j].ss);
		if (f_sync)
			while (fsync(c[j].fdcurrent) == -1)
				pause3("unable to write ", c[j].dir, "/current to disk, pausing: ");
		while (fchmod(c[j].fdcurrent, 0744) == -1)
			pause3("unable to set mode of ", c[j].dir, "/current, pausing: ");
	}
}

int             flagexitasap = 0;
int             flagforcerotate = 0;
int             flagnewline = 1;
int             flaglog = 1;

void
exitasap(int i)
{
	flagexitasap = 1;
}

void
forcerotate(int i)
{
	flagforcerotate = 1;
}

void
logging(int i)
{
	flaglog = 1;
}

void
nologging(int i)
{
	flaglog = 0;
}

ssize_t
flushread(int fd, char *buf, int len)
{
	int             j;

	for (j = 0; j < cnum; ++j)
		substdio_flush(&c[j].ss);
	if (flagforcerotate) {
		for (j = 0; j < cnum; ++j)
			if (c[j].bytes > 0)
				fullcurrent(&c[j]);
		flagforcerotate = 0;
	}

	if (!len)
		return 0;
	if (flagexitasap) {
		if (flagnewline)
			return 0;
		len = 1;
	}

	sig_unblock(sig_term);
	sig_unblock(sig_alarm);
	sig_unblock(sig_hangup);
	sig_unblock(sig_int);

	len = read(fd, buf, len);

	sig_block(sig_term);
	sig_block(sig_alarm);
	sig_block(sig_hangup);
	sig_block(sig_int);

	if (len <= 0)
		return len;
	flagnewline = (buf[len - 1] == '\n');
	return len;
}

char            inbuf[1024];
substdio        ssin = SUBSTDIO_FDBUF((ssize_t (*)(int,  char *, size_t)) flushread, 0, inbuf, sizeof inbuf);

char            line[1001];
int             linelen;		/*- 0 <= linelen <= 1000 */

void
doit(char **script)
{
	int             flageof;
	char            ch;
	int             j;
	int             i;
	char           *action;
	int             flagselected;
	int             flagtimestamp;

	flagtimestamp = 0;
	if (script[0] && script[0][0] == 't')
		flagtimestamp = 1;
	for (i = 0; i <= 1000; ++i)
		line[i] = '\n';
	linelen = 0;
	flageof = 0;
	for (;;) {
		for (i = 0; i < linelen; ++i)
			line[i] = '\n';
		linelen = 0;
		while (linelen < 1000) {
			if (substdio_get(&ssin, &ch, 1) <= 0) {
				if (!linelen)
					return;
				flageof = 1;
				break;
			}
			if (!linelen && flagtimestamp) {
				timestamp(line);
				line[25] = ' ';
				linelen = 26;
			}
			if (ch == '\n')
				break;
			line[linelen++] = ch;
		}
		if (flaglog)
			flagselected = 1;
		else
			flagselected = 0;
		j = 0;
		for (i = 0; (action = script[i]); ++i) {
			switch (*action)
			{
			case '+':
				if (!flagselected && match(action + 1, line, linelen))
					flagselected = 1;
				break;
			case '-':
				if (flagselected && match(action + 1, line, linelen))
					flagselected = 0;
				break;
			case 'e':
				if (flagselected) {
					if (linelen > 200) {
						substdio_put(subfderr, line, 200);
						substdio_puts(subfderr, "...\n");
					} else {
						substdio_put(subfderr, line, linelen);
						substdio_puts(subfderr, "\n");
					}
					substdio_flush(subfderr);
				}
				break;
			case '=':
				if (flagselected) {
					for (;;) {
						while (seek_begin(f[i]) == -1)
							pause3("unable to move to beginning of ", action + 1, ", pausing: ");
						if (write(f[i], line, 1001) == 1001)
							break;
						pause3("unable to write ", action + 1, ", pausing: ");
					}
				}
				break;
			case '.':
			case '/':
				c[j].flagselected = flagselected;
				++j;
				break;
			}
		}
		for (j = 0; j < cnum; ++j) {
			if (c[j].flagselected)
				substdio_put(&c[j].ss, line, linelen);
		}
		if (linelen == 1000) {
			for (;;) {
				if (substdio_get(&ssin, &ch, 1) <= 0) {
					flageof = 1;
					break;
				}
				if (ch == '\n')
					break;
				for (j = 0; j < cnum; ++j) {
					if (c[j].flagselected)
						substdio_put(&c[j].ss, &ch, 1);
				}
			}
		}
		for (j = 0; j < cnum; ++j) {
			if (c[j].flagselected) {
				ch = '\n';
				substdio_put(&c[j].ss, &ch, 1);
			}
		}
		if (flageof)
			return;
	}
}

int
main(int argc, char **argv)
{

	f_sync = env_get("NOFSYNC") ? 0 : 1;
	old_umask = umask(022);
	if ((fdstartdir = open_read(".")) == -1)
		strerr_die2sys(111, FATAL, "unable to switch to current directory: ");
	coe(fdstartdir);
	sig_block(sig_term);
	sig_block(sig_alarm);
	sig_block(sig_hangup);
	sig_block(sig_int);
	sig_catch(sig_term, exitasap);
	sig_catch(sig_alarm, forcerotate);
	sig_catch(sig_hangup, logging);
	sig_catch(sig_int, nologging);
	++argv;
	f_init(argv);
	c_init(argv);
	doit(argv);
	c_quit();
	_exit(0);
}

void
getversion_multilog_c()
{
	const char     *x = "$Id: multilog.c,v 1.10 2024-05-09 22:39:36+05:30 mbhangui Exp mbhangui $";

	x++;
}

/*
 * $Log: multilog.c,v $
 * Revision 1.10  2024-05-09 22:39:36+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.9  2024-02-18 08:42:01+05:30  Cprogrammer
 * use unlinkat() if available instead of unlink()
 *
 * Revision 1.8  2022-05-22 23:06:19+05:30  Cprogrammer
 * skip fsync() when writing log to disk if env variable NOFSYNC is set
 *
 * Revision 1.7  2008-07-15 19:56:40+05:30  Cprogrammer
 * porting for Mac OS X
 *
 * Revision 1.6  2008-06-22 20:08:02+05:30  Cprogrammer
 * exit if mkdir returns something other than EXIST
 *
 * Revision 1.5  2008-06-07 13:51:26+05:30  Cprogrammer
 * send SIGALRM very NNN seconds
 *
 * Revision 1.4  2004-10-22 20:27:35+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.3  2004-10-09 23:28:27+05:30  Cprogrammer
 * replaced buffer functions with substdio
 *
 * Revision 1.2  2003-12-31 18:36:29+05:30  Cprogrammer
 * fixed compilation warning
 *
 * Revision 1.1  2003-08-07 19:44:39+05:30  Cprogrammer
 * Initial revision
 *
 */
