/*
 * $Log: condtomaildir.c,v $
 * Revision 1.7  2022-04-04 11:08:07+05:30  Cprogrammer
 * use USE_FSYNC, USE_FDATASYNC, USE_SYNCDIR to set sync to disk feature
 *
 * Revision 1.6  2021-08-29 23:27:08+05:30  Cprogrammer
 * define functions as noreturn
 *
 * Revision 1.5  2020-11-24 13:44:40+05:30  Cprogrammer
 * removed exit.h
 *
 * Revision 1.4  2014-05-22 14:44:34+05:30  Cprogrammer
 * removed duplicate sig.h
 *
 * Revision 1.3  2004-10-22 20:24:04+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.2  2004-10-22 15:34:44+05:30  Cprogrammer
 * removed readwrite.h
 *
 * Revision 1.1  2004-07-17 20:51:06+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <unistd.h>
#include <sys/stat.h>
#include <signal.h>
#include <sig.h>
#include <byte.h>
#include <open.h>
#include <substdio.h>
#include <strerr.h>
#include <error.h>
#include <fmt.h>
#include <env.h>
#include <str.h>
#include <wait.h>
#include <seek.h>
#include <now.h>
#include <env.h>
#include <pathexec.h>
#include <noreturn.h>
#ifdef USE_FSYNC
#include <fcntl.h>
#include "syncdir.h"
#endif

#define sig_ignore(s) (sig_catch((s),sig_ignorehandler))
#define FATAL "condtomaildir: fatal: "

void            (*sig_ignorehandler) () = SIG_IGN;

static char     fntmptph[80 + FMT_ULONG * 2];

void
tryunlinktmp()
{
	unlink(fntmptph);
}

no_return void
sigalrm()
{
	tryunlinktmp();
	strerr_die1x(111, "Timeout on maildir delivery. (#4.3.0)");
}

int
doit(char *dir)
{
	pid_t           pid;
	time_t          tm;
	int             loop, fd;
	struct stat     st;
	substdio        ss, ssout;
	char           *rpline, *dtline, *s;
	char            buf[SUBSTDIO_INSIZE], outbuf[SUBSTDIO_OUTSIZE], host[64],
					fnnewtph[80 + FMT_ULONG * 2];

	sig_catch(SIGALRM, sigalrm);
	if (chdir(dir) == -1) {
		if (error_temp(errno))
			return (1);
		return (2);
	}
	pid = getpid();
	host[0] = '\0';
	gethostname(host, sizeof(host));
	for (loop = 0;; ++loop) {
		tm = now();
		s = fntmptph;
		s += fmt_str(s, "tmp/");
		s += fmt_ulong(s, tm);
		*s++ = '.';
		s += fmt_ulong(s, pid);
		*s++ = '.';
		s += fmt_strn(s, host, sizeof(host));
		*s++ = 0;
		if (stat(fntmptph, &st) == -1)
			if (errno == error_noent)
				break;
		if (loop == 2)
			return (1);
		sleep(2);
	}
	str_copy(fnnewtph, fntmptph);
	byte_copy(fnnewtph, 3, "new");
	alarm(86400);
	if ((fd = open_excl(fntmptph)) == -1)
		return (1);
	if (!(rpline = env_get("RPLINE")))
		strerr_die2x(100, FATAL, "RPLINE not set");
	if (!(dtline = env_get("DTLINE")))
		strerr_die2x(100, FATAL, "DTLINE not set");
	substdio_fdbuf(&ss, read, 0, buf, sizeof(buf));
	substdio_fdbuf(&ssout, write, fd, outbuf, sizeof(outbuf));
	if (substdio_puts(&ssout, rpline) == -1)
		goto fail;
	if (substdio_puts(&ssout, dtline) == -1)
		goto fail;
	switch (substdio_copy(&ssout, &ss))
	{
	case -2:
		tryunlinktmp();
		return (4);
	case -3:
		goto fail;
	}

	if (substdio_flush(&ssout) == -1)
		goto fail;
#ifdef USE_FSYNC
	if ((use_fsync > 0 || use_fdatasync > 0) && (use_fdatasync ? fdatasync : fsync) (fd) == -1)
		goto fail;
#else
	if (fsync(fd) == -1)
		goto fail;
#endif
	if (close(fd) == -1)
		goto fail;
	if (link(fntmptph, fnnewtph) == -1)
		goto fail;
	tryunlinktmp();
#if !defined(SYNCDIR_H) && defined(USE_FSYNC) && defined(LINUX)
	if (use_syncdir > 0) {
		if ((fd = open(fnnewtph, O_RDONLY)) == -1) {
			if (errno != error_noent)
				goto fail;
		} else
		if ((use_fdatasync ? fdatasync : fsync) (fd) == -1 || close(fd) == -1)
			goto fail;
	}
#endif
	return (0);

fail:
	tryunlinktmp();
	return (1);
}

int
main(int argc, char **argv, char **envp)
{
	char           *dir;
	int             pid, wstat, r;
#ifdef USE_FSYNC
	char           *ptr;
#endif

	if (!argv[1] || !argv[2])
		strerr_die1x(100, "condtomaildir: usage: condtomaildir dir program [ arg ... ]");
#ifdef USE_FSYNC
	ptr = env_get("USE_FSYNC");
	use_fsync = (ptr && *ptr) ? 1 : 0;
	ptr = env_get("USE_FDATASYNC");
	use_fdatasync = (ptr && *ptr) ? 1 : 0;
	ptr = env_get("USE_SYNCDIR");
	use_syncdir = (ptr && *ptr) ? 1 : 0;
#endif

	if ((pid = fork()) == -1)
		strerr_die2sys(111, FATAL, "unable to fork: ");
	if (pid == 0) {
		pathexec_run(argv[2], argv + 2, envp);
		if (error_temp(errno))
			_exit(111);
		_exit(100);
	}
	if (wait_pid(&wstat, pid) == -1)
		strerr_die2x(111, FATAL, "wait failed");
	if (wait_crashed(wstat))
		strerr_die2x(111, FATAL, "child crashed");
	switch (wait_exitcode(wstat))
	{
	case 0:
		break;
	case 111:
		strerr_die2x(111, FATAL, "temporary child error");
	default:
		_exit(0);
	}
	if (seek_begin(0) == -1)
		strerr_die2sys(111, FATAL, "unable to rewind: ");
	umask(077);
	sig_ignore(SIGPIPE);
	dir = argv[1];
	switch ((r = doit(dir)))
	{
	case 0:
		break;
	case 2:
		strerr_die1x(111, "Unable to chdir to maildir. (#4.2.1)");
	case 3:
		strerr_die1x(111, "Timeout on maildir delivery. (#4.3.0)");
	case 4:
		strerr_die1x(111, "Unable to read message. (#4.3.0)");
	default:
		strerr_die1x(111, "Temporary error on maildir delivery. (#4.3.0)");
	}
	strerr_die1x(99, "condtomaildir");
	/*- Not reached */
	return (0);
}

void
getversion_condtomaildir_c()
{
	static char    *x = "$Id: condtomaildir.c,v 1.7 2022-04-04 11:08:07+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
