/*
 * $Log: $
 */
#include <substdio.h>
#include "haslibrt.h"
#ifdef HASLIBRT
#include <unistd.h>
#include <fcntl.h>
#include <subfd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <error.h>
#include <str.h>
#include <strerr.h>
#include <scan.h>
#include <fmt.h>
#include "send_qload.h"

#define FATAL "setqload: fatal: "
#define WARN  "setqload: warning: "

void
get_queue_number(int *q)
{
	int             shm;

	if ((shm = shm_open("/qscheduler", O_RDONLY, 0644)) == -1)
		strerr_die2sys(111, FATAL, "/qscheduler: ");
	if (read(shm, q, sizeof(int) * 2) == -1)
		strerr_die2sys(111, FATAL, "/qscheduler: read: ");
	return;
}

int
main(int argc, char **argv)
{
	int             i, ret, qnum, qcount, qconf, load;
	int             q[3];
	char            strnum[FMT_ULONG];
	char           *argv0;

	if (argc != 3) {
		i = str_rchr(argv[0], '/');
		argv0 = (argv[0][i] && argv[0][i + 1]) ? argv[0] + i + 1 : argv[0];
		strerr_die4x(100, FATAL, "USAGE: ", argv0, " queue_no load");
	}
	get_queue_number(q);
	qcount = q[0];
	qconf = q[1];
	scan_int(argv[1], &qnum);
	if (qnum > qcount) {
		strnum[fmt_int(strnum, qnum)] = 0;
		strerr_die3x(100, WARN, "queue number cannot be > ", strnum);
	}
	scan_int(argv[2], &load);
	ret = (load > 0 ? send_qload("/qscheduler", qnum, load, 10) : 0);
	if (!ret) {
		strnum[i = fmt_int(strnum, qcount)] = 0;
		if (substdio_put(subfdout, "queue count = ", 14) == -1 ||
				substdio_put(subfdout, strnum, i) == -1 ||
				substdio_put(subfdout, ", queue configured = ", 21) == -1)
			_exit(111);
		strnum[i = fmt_int(strnum, qconf)] = 0;
		if (substdio_put(subfdout, strnum, i) == -1 ||
				substdio_put(subfdout, "\n", 1) == -1 ||
				substdio_flush(subfdout))
			_exit(111);
	}
	return ret;
}
#else
#warning "not compiled with -DHASLIBRT"
#include <substdio.h>
#include <subfd.h>
#include <unistd.h>

static char     sserrbuf[512];
struct substdio sserr;

int
main(argc, argv)
	int             argc;
	char          **argv;
{
	substdio_puts(subfderr, "not compiled with -DLIBRT\n");
	substdio_flush(subfderr);
	_exit(111);
}
#endif

void
getversion_setqload_c()
{
	static char    *x = "$Id: $";

	x++;
}
