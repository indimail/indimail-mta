/*
 * $Id: svstatd.c,v 1.2 2026-08-31 13:28:09+05:30 Cprogrammer Exp mbhangui $
 *
 * © 2026 Manvendra Bhangui
 * All intellectual property developed during the term of this agreement, including but not limited
 * to svstatd.c, shall be and remain the sole and exclusive property of Manvendra Bhangui
 */

#include <unistd.h>
#include <ctype.h>
#include <sys/wait.h>
#include <stralloc.h>
#include <str.h>
#include <signal.h>
#include <fmt.h>
#include <substdio.h>
#include <subfd.h>
#include <getln.h>
#include <strerr.h>
#include <error.h>
#include <makeargs.h>

#define FATAL "svstatd: fatal: "

stralloc        line = { 0 };
int             i, match;
const char     *(svstat[]) = { "svstat", 0, (char *) NULL};
const char     *(svup[]) = { "svc", "-u", 0, (char *) NULL};
const char     *(svdn[]) = { "svc", "-d", 0, (char *) NULL};

int
main(int argc, char **argv)
{
	int             status, action = -1, ret;
	pid_t           pid;
	const char     *prog, *prog_args;
	const char    **ptr;
	char            strnum1[FMT_ULONG], strnum2[FMT_ULONG];

	if (getln(subfdinsmall, &line, &match, '\n') == -1)
		strerr_die2sys(111, FATAL, "unable to read input: ");
	if (!line.len || !match) {
		if (substdio_puts(subfdout, FATAL) == -1 ||
				substdio_put(subfdout, "invalid input [", 15) == -1 ||
				substdio_put(subfdout, line.s, line.len) == -1 ||
				substdio_flush(subfdout) == -1)
			strerr_die2x(111, FATAL, "Unable to write to descriptor 2");
		strerr_die4x(111, FATAL, "invalid input1 [", line.s, "]: ");
	}
	if (!stralloc_0(&line)) {
		if (substdio_puts(subfdout, FATAL) == -1 ||
				substdio_put(subfdout, "out of memory\n", 14) == -1 ||
				substdio_flush(subfdout) == -1)
			strerr_die2x(111, FATAL, "Unable to write to descriptor 2");
		strerr_die2x(111, FATAL, "out of memory");
	}
	line.s[line.len - 2] = 0;
	i = str_chr(line.s, ' ');
	if (line.s[i])
		line.s[i] = '/';
	if (line.s[0] == '/')
		action = 1;
	else
	if (!str_diffn(line.s, "up|", 3))
		action = 2;
	else
	if (!str_diffn(line.s, "dn|", 3))
		action = 3;
	if (action == -1 || !line.s[i]) {
		line.s[i] = ' ';
		if (substdio_puts(subfdout, FATAL) == -1 ||
				substdio_put(subfdout, "invalid input [", 15) == -1 ||
				substdio_put(subfdout, line.s, line.len) == -1 ||
				substdio_put(subfdout, "]\n", 2) == -1 ||
				substdio_flush(subfdout) == -1)
			strerr_die2x(111, FATAL, "Unable to write to descriptor 2");
		strerr_die4x(111, FATAL, "invalid input2 [", line.s, "]: ");
	}
	switch(action)
	{
	case 1:
		svstat[1] = line.s;
		prog_args = line.s;
		prog = *svstat;
		ptr = svstat;
		break;
	case 2:
		svup[2] = line.s + 3;
		prog_args = line.s + 3;
		prog = *svup;
		ptr = svup;
		break;
	case 3:
		svdn[2] = line.s + 3;
		prog_args = line.s + 3;
		prog = *svdn;
		ptr = svdn;
		break;
	}
	if (access(prog_args, X_OK)) {
		if (substdio_puts(subfdout, FATAL) == -1 ||
				substdio_put(subfdout, "unable to access ", 17) == -1 ||
				substdio_puts(subfdout, prog_args) == -1 ||
				substdio_flush(subfdout) == -1)
			strerr_die2x(111, FATAL, "Unable to write to descriptor 2");
		strerr_die4sys(111, FATAL, "unable to access [", prog_args, "]: ");
	}
	switch ((pid = fork()))
	{
	case -1:
		break;
	case 0:
		execvp(prog, (char **) ptr);
		if (substdio_puts(subfdout, FATAL) == -1 ||
				substdio_put(subfdout, "unable to exec ", 15) == -1 ||
				substdio_puts(subfdout, prog) == -1 ||
				substdio_puts(subfdout, prog_args) == -1 ||
				substdio_flush(subfdout) == -1)
			strerr_die2x(111, FATAL, "Unable to write to descriptor 2");
		strerr_die6sys(111, FATAL, "unable to exec ", prog, " ", prog_args, ": ");
		break;
	default:
		break;
	}
	for (;;) {
		if (!(i = waitpid(pid, &status, 0)))
			break;
		else
		if (i == -1) {
#ifdef ERESTART
			if (errno == error_intr || errno == error_restart)
#else
			if (errno == error_intr)
#endif
				continue;
			if (errno == ECHILD)
				break;
			strerr_die2sys(111, FATAL, "waitpid: ");
		}
		if (WIFSTOPPED(status)) {
			strnum1[fmt_ulong(strnum1, pid)] = 0;
			strnum2[fmt_uint(strnum2, WIFSTOPPED(status) ? WSTOPSIG(status) : SIGCONT)] = 0;
			strerr_warn3(strnum1, WIFSTOPPED(status) ? " stopped by signal " : " started by signal ", strnum2, 0);
			continue;
		}
	}
	if (WIFSIGNALED(status)) {
		strnum1[fmt_ulong(strnum1, pid)] = 0;
		strnum2[fmt_uint(strnum2, WTERMSIG(status))] = 0;
		strerr_warn3(strnum1, ": killed by signal ", strnum2, 0);
		ret = -1;
	} else
	if (WIFEXITED(status)) {
		ret = WEXITSTATUS(status);
		strnum1[fmt_ulong(strnum1, pid)] = 0;
		strnum2[fmt_uint(strnum2, ret < 0 ? 0 - ret : ret)] = 0;
		strerr_warn4(strnum1, ": normal exit return status", ret < 0 ? " -" : " ", strnum2, 0);
	} else
		ret = -1;
	if (action == 1)
		_exit(ret);
	svstat[1] = line.s + 3;
	prog_args = line.s + 3;
	prog = *svstat;
	ptr = svstat;
	execvp(prog, (char **) ptr);
	if (substdio_puts(subfdout, FATAL) == -1 ||
			substdio_put(subfdout, "unable to exec ", 15) == -1 ||
			substdio_puts(subfdout, prog) == -1 ||
			substdio_puts(subfdout, prog_args) == -1 ||
			substdio_flush(subfdout) == -1)
		strerr_die2x(111, FATAL, "Unable to write to descriptor 2");
	strerr_die6sys(111, FATAL, "unable to exec ", prog, " ", prog_args, ": ");
}

/*
 * $Log: svstatd.c,v $
 * Revision 1.2  2026-08-31 13:28:09+05:30  Cprogrammer
 * added feature to start/stop service
 *
 * Revision 1.1  2026-01-14 18:51:19+05:30  Cprogrammer
 * Initial revision
 *
 */
