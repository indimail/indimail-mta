/* 
 * $Id: waitfor.c,v 1.1 2026-07-12 20:01:51+05:30 Cprogrammer Exp mbhangui $
 */
#define _GNU_SOURCE
#include <poll.h>
#include <stdlib.h>
#include <signal.h>
#ifdef __linux__
#include <sys/syscall.h>
#endif
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#if defined(__FreeBSD__) || defined(__OpenBSD__)
#include <sys/event.h>
#include <fmt.h>
#endif
#include <subfd.h>
#include <substdio.h>
#include <scan.h>
#include <strerr.h>
#include <error.h>
#include <qprintf.h>
#include <sgetopt.h>
#if defined(__sun__)
#include <stralloc.h>
#endif

#define FATAL "waitfor: fatal: "
#define WARN  "waitfor: warn: "

#if defined(__linux__)
/* pidfd_open may not be in older glibc headers */
#if !defined(__NR_pidfd_open)
#  if defined(__x86_64__)
#    define __NR_pidfd_open 434
#  elif defined(__aarch64__)
#    define __NR_pidfd_open 434
#  elif defined(__i386__)
#    define __NR_pidfd_open 434
#  endif
#endif
#endif

#ifdef __linux__
/*- wrapper for the system call. See pidfd_open(2) */
static int
pidfd_open(pid_t pid, unsigned int flags)
{
    return syscall(__NR_pidfd_open, pid, flags);
}
#endif

int
main(int argc, char *argv[])
{
	pid_t           target_pid;
	int             c, verbose = 0, i = 1;
#ifdef __linux__
	int             pidfd, ready;
	struct pollfd   pfd;
#endif
#if defined(__sun__)
	stralloc        path;
#endif
#if defined(__FreeBSD__) || defined(__OpenBSD__)
	char            strnum[FMT_ULONG];
	int             kq, nevents;
    struct kevent   changelist;
    struct kevent   eventlist;
#endif

	if (argc == 1)
		strerr_die2x(100, FATAL, "USAGE: waitfor [-v] pid");
	while ((c = getopt(argc, argv, "v")) != opteof) {
		switch (c)
		{
		case 'v':
			verbose = 1;
			break;
		default:
			strerr_die2x(100, FATAL, "USAGE: waitfor [-v] pid");
		}
	}

	if (argc - optind < 1)
		strerr_die2x(100, FATAL, "USAGE: waitfor [-v] pid");
	scan_int(argv[optind], &target_pid);

#if defined(__linux__)
	if ((pidfd = pidfd_open(target_pid, 0L)) == -1)
		strerr_die4sys(111, FATAL, "unable to open pid ", argv[optind], ": ");
	pfd.fd = pidfd;
	pfd.events = POLLIN; /* The kernel signals POLLIN when the process terminates */

	/*-
	 * poll() blocks here with 0% CPU consumption until an event happens
	 * -1 means no timeout; wait indefinitely
	 */
	for (;;) {
		if ((ready = poll(&pfd, 1, -1)) == -1) {
			if (errno == error_intr)
				continue;
			strerr_die2sys(111, FATAL, "poll :");
		} else
		if (ready)
			break;
	}
	close(pidfd);

	/* Double-check that it was a read readiness event triggered (process termination) */
	if (pfd.revents & POLLIN) {
		if (verbose && subprintf(subfdout, "PID %d terminated!\n", target_pid) == -1)
			strerr_die1sys(111, "unable to write to descriptor 1: ");
		i = 0;
	} else
	if (verbose && subprintf(subfdout, "PID %d unblocked due to alternative event flags: %d\n",
				target_pid, pfd.revents) == -1)
		strerr_die1sys(111, "unable to write to descriptor 1: ");
	if (verbose && substdio_flush(subfdout) == -1)
		strerr_die1sys(i, "unable to write to descriptor 1: ");
#elif defined(__FreeBSD__) || defined(__OpenBSD__)
    if ((kq = kqueue()) == -1)
		strerr_die2sys(111, FATAL, "kqueue :");

    /* EVFILT_PROC targets processes via their PID passed as the 'ident' */
    EV_SET(&changelist, target_pid, EVFILT_PROC, EV_ADD | EV_ENABLE, NOTE_EXIT, 0, NULL);

    /* Register the event with the kernel */
    if (kevent(kq, &changelist, 1, NULL, 0, NULL) == -1) {
        close(kq);
		if (errno == error_srch) {
			strnum[fmt_ulong(strnum, target_pid)] = 0;
            strerr_die4x(111, FATAL, "Process with PID ", strnum, " does not exist.");
        } else 
            strerr_die2sys(111, FATAL, "kevent registration failed");
    }

    /* Block until the target process exits */
    if ((nevents = kevent(kq, NULL, 0, &eventlist, 1, NULL)) == -1) {
        close(kq);
		strerr_die2sys(111, FATAL, "kevent:");
    }
    if (eventlist.fflags & NOTE_EXIT) {
		if (verbose && subprintf(subfdout, "PID %d terminated!\n", target_pid) == -1)
			strerr_die1sys(111, "unable to write to descriptor 1: ");
		i = 0;
	} else
	if (verbose && subprintf(subfdout, "PID %d unblocked due to alternative event flags: %d\n",
			target_pid, eventlist.fflags) == -1)
		strerr_die1sys(111, "unable to write to descriptor 1: ");
	if (verbose && substdio_flush(subfdout) == -1)
		strerr_die1sys(i, "unable to write to descriptor 1: ");
#elif defined(__sun__)
    qsnprintf(&path, sizeof(path), "/proc/%d", (int) target_pid);
    for (;;) {
        struct stat st;
        if (stat(path, &st) < 0) {
            if (errno == ENOENT)
				i = 0;
			else
				i = 1;
			break;
        }
        usleep(100000);   /* 100 ms */
    }
    if (!i) {
		if (verbose && subprintf(subfdout, "PID %d terminated!\n", target_pid) == -1)
			strerr_die1sys(111, "unable to write to descriptor 1: ");
	} else
	if (verbose && subprintf(subfdout, "PID %d unblocked due to alternative event flags: %d\n",
				target_pid, pfd.revents) == -1)
		strerr_die1sys(111, "unable to write to descriptor 1: ");
	if (verbose && substdio_flush(subfdout) == -1)
		strerr_die1sys(i, "unable to write to descriptor 1: ");
#else
    for (;;) {
        if (kill(target_pid, 0) < 0) {
            if (errno == ESRCH)
				i = 0;
			else
				i = errno;
        }
        usleep(100000);   /* 100 ms */
    }
    if (!i) {
		if (verbose && subprintf(subfdout, "PID %d terminated!\n", target_pid) == -1)
			strerr_die1sys(111, "unable to write to descriptor 1: ");
	} else
	if (verbose && subprintf(subfdout, "PID %d unblocked due to error errno=[%d]\n",
			target_pid, i) == -1)
		strerr_die1sys(111, "unable to write to descriptor 1: ");
	if (verbose && substdio_flush(subfdout) == -1)
		strerr_die1sys(i, "unable to write to descriptor 1: ");
#endif
	return i;
}

/*
 * $Log: waitfor.c,v $
 * Revision 1.1  2026-07-12 20:01:51+05:30  Cprogrammer
 * Initial revision
 *
 */
