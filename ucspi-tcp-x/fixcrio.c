/*
 * $Log: fixcrio.c,v $
 * Revision 1.4  2021-08-30 12:47:59+05:30  Cprogrammer
 * define funtions as noreturn
 *
 * Revision 1.3  2021-05-12 21:01:51+05:30  Cprogrammer
 * replace pathexec() with upathexec()
 *
 * Revision 1.2  2020-11-27 17:32:22+05:30  Cprogrammer
 * removed exit.h
 *
 * Revision 1.1  2005-01-22 01:02:34+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <unistd.h>
#include <signal.h>
#include <sig.h>
#include <strerr.h>
#include <byte.h>
#include <fd.h>
#include <noreturn.h>
#include "iopause.h"
#include "upathexec.h"

#define FATAL "fixcrio: fatal: "

static char     prebuf[256];
static char     leftbuf[512];
static char     rightbuf[512];
static int      leftstatus = 0;
static int      leftlen;
static int      leftpos;
static int      leftflagcr = 0;
static int      rightstatus = 0;
static int      rightlen;
static int      rightpos;
static int      rightflagcr = 0;

no_return void
doit(int fdleft, int fdright)
{
	struct taia     stamp;
	struct taia     deadline;
	iopause_fd      x[4];
	int             xlen;
	iopause_fd     *io0;
	iopause_fd     *ioleft;
	iopause_fd     *io1;
	iopause_fd     *ioright;
	int             r;
	int             i;
	char            ch;

	for (;;) {
		xlen = 0;
		io0 = 0;
		if (leftstatus == 0) {
			io0 = &x[xlen++];
			io0->fd = 0;
			io0->events = IOPAUSE_READ;
		}
		ioleft = 0;
		if (leftstatus == 1) {
			ioleft = &x[xlen++];
			ioleft->fd = fdleft;
			ioleft->events = IOPAUSE_WRITE;
		}
		ioright = 0;
		if (rightstatus == 0) {
			ioright = &x[xlen++];
			ioright->fd = fdright;
			ioright->events = IOPAUSE_READ;
		}
		io1 = 0;
		if (rightstatus == 1) {
			io1 = &x[xlen++];
			io1->fd = 1;
			io1->events = IOPAUSE_WRITE;
		}
		taia_now(&stamp);
		taia_uint(&deadline, 3600);
		taia_add(&deadline, &stamp, &deadline);
		iopause(x, xlen, &deadline, &stamp);
		if (io0 && io0->revents) {
			if ((r = read(0, prebuf, sizeof prebuf)) <= 0) {
				leftstatus = -1;
				close(fdleft);
			} else {
				leftstatus = 1;
				leftpos = 0;
				leftlen = 0;
				for (i = 0; i < r; ++i) {
					ch = prebuf[i];
					if (ch == '\n')
						if (!leftflagcr)
							leftbuf[leftlen++] = '\r';
					leftbuf[leftlen++] = ch;
					leftflagcr = (ch == '\r');
				}
			}
		}
		if (ioleft && ioleft->revents) {
			if ((r = write(fdleft, leftbuf + leftpos, leftlen - leftpos)) == -1)
				break;
			leftpos += r;
			if (leftpos == leftlen)
				leftstatus = 0;
		}
		if (ioright && ioright->revents) {
			if ((r = read(fdright, prebuf, sizeof prebuf)) <= 0)
				break;
			rightstatus = 1;
			rightpos = 0;
			rightlen = 0;
			for (i = 0; i < r; ++i) {
				ch = prebuf[i];
				if (ch == '\n')
					if (!rightflagcr)
						rightbuf[rightlen++] = '\r';
				rightbuf[rightlen++] = ch;
				rightflagcr = (ch == '\r');
			}
		}
		if (io1 && io1->revents) {
			if ((r = write(1, rightbuf + rightpos, rightlen - rightpos)) == -1)
				break;
			rightpos += r;
			if (rightpos == rightlen)
				rightstatus = 0;
		}
	}
	_exit(0);
}

int
main(int argc, char **argv, char **envp)
{
	int             piin[2];
	int             piout[2];

	if (argc < 2)
		strerr_die1x(100, "fixcrio: usage: fixcrio program [ arg ... ]");
	if (pipe(piin) == -1)
		strerr_die2sys(111, FATAL, "unable to create pipe: ");
	if (pipe(piout) == -1)
		strerr_die2sys(111, FATAL, "unable to create pipe: ");
	switch (fork())
	{
	case -1:
		strerr_die2sys(111, FATAL, "unable to fork: ");
	case 0:
		sig_ignore(sig_pipe);
		close(piin[0]);
		close(piout[1]);
		doit(piin[1], piout[0]);
	}
	close(piin[1]);
	close(piout[0]);
	if (fd_move(0, piin[0]) == -1)
		strerr_die2sys(111, FATAL, "unable to move descriptors: ");
	if (fd_move(1, piout[1]) == -1)
		strerr_die2sys(111, FATAL, "unable to move descriptors: ");
	upathexec_run(argv[1], argv + 1, envp);
	strerr_die4sys(111, FATAL, "unable to run ", argv[1], ": ");
	/*- Not reached */
	return(0);
}

void
getversion_fixcrio_c()
{
	const char     *x = "$Id: fixcrio.c,v 1.4 2021-08-30 12:47:59+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
