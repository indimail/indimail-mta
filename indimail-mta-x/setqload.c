/*
 * $Log: setqload.c,v $
 * Revision 1.4  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.3  2022-08-14 21:58:24+05:30  Cprogrammer
 * fix compilation warning if HASLIBRT is undefined
 *
 * Revision 1.2  2022-04-25 01:26:33+05:30  Cprogrammer
 * fix for OSX
 *
 * Revision 1.1  2022-04-24 19:09:32+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <substdio.h>
#include "haslibrt.h"
#ifdef HASLIBRT
#include <unistd.h>
#include <sgetopt.h>
#include <fcntl.h>
#include <subfd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <error.h>
#include <str.h>
#include <strerr.h>
#include <scan.h>
#include <fmt.h>
#include <evaluate.h>
#include "send_qload.h"

#define FATAL "setqload: fatal: "
#define WARN  "setqload: warn: "

const char     *usage1 = "-q queue_number total";
const char     *usage2 = "-q queue_number lcur/lmax rcur/rmax";
const char     *desc =
	"\n\nwhere\n"
	"   total = lcur/lmax + rcur/rmax\n"
	"   lcur  = cur local   concurrency\n"
	"   lmax  = max local   concurrency\n"
	"   rcur  = cur remote  concurrency\n"
	"   rmax  = max remote  concurrency";

void
get_queue_details(int *q)
{
	int             shm;

	if ((shm = shm_open("/qscheduler", O_RDONLY, 0644)) == -1)
		strerr_die2sys(111, FATAL, "/qscheduler: ");
	if (read(shm, q, sizeof(int) * 2) == -1)
		strerr_die2sys(111, FATAL, "/qscheduler: read: ");
	return;
}

int
compute_expression(struct vartable *vt, const char *s, double *x)
{
	struct val      result;

	switch (evaluate(s, &result, vt))
	{
	case ERROR_SYNTAX:
		strerr_warn2(s, ": syntax error", 0);
		return -1;
	case ERROR_VARNOTFOUND:
		strerr_warn2(s, ": variable not found", 0);
		return -1;
	case ERROR_NOMEM:
		strerr_warn2(s, ": out of memory", 0);
		return -1;
	case ERROR_DIV0:
		strerr_warn2(s, ": division by zero", 0);
		return -1;
	case RESULT_OK:
		*x = (double) ((result.type == T_INT) ? result.ival : result.rval);
		return 0;
	default:
		strerr_warn2(s, ": unknown error", 0);
		return -1;
	}
}

int
main(int argc, char **argv)
{
	int             i, opt, qnum = 0;
	int             q[3];
	double          load, load_l, load_r;
	char            strnum[FMT_DOUBLE];
	char           *argv0;
	struct vartable *vt = {0};

	i = str_rchr(argv[0], '/');
	argv0 = (argv[0][i] && argv[0][i + 1]) ? argv[0] + i + 1 : argv[0];
	load_l = load_r = 0.0;
	while ((opt = getopt(argc, argv, "q:")) != opteof) {
		switch(opt)
		{
		case 'q':
			scan_int(optarg, &qnum);
			break;
		default:
			strerr_warn4("usage: ", argv0, " ", usage1, 0);
			strerr_warn1("       or", 0);
			strerr_warn4("usage: ", argv0, " ", usage2, 0);
			strerr_warn1(desc, 0);
			_exit(100);
		}
	}
	if (!qnum || optind == argc) {
		strerr_warn4("usage: ", argv0, " ", usage1, 0);
		strerr_warn1("       or", 0);
		strerr_warn4("usage: ", argv0, " ", usage2, 0);
		strerr_warn1(desc, 0);
		_exit(100);
	}
	get_queue_details(q);
	if (qnum > q[0]) {
		strnum[fmt_int(strnum, qnum)] = 0;
		strerr_die3x(100, WARN, "queue number cannot be > ", strnum);
	}
	if (!(vt = create_vartable()))
		strerr_die2x(111, argv0, ": out of memory");
	if (optind == argc - 1) {
		if (compute_expression(vt, argv[optind++], &load_l))
			_exit(111);
	} else
	if (optind == argc - 2) {
		if (compute_expression(vt, argv[optind++], &load_l))
			_exit(111);
		if (compute_expression(vt, argv[optind++], &load_r))
			_exit(111);
	}
	if (vt)
		free_vartable(vt);
	load = 100 * (load_l + load_r);
	if (load > 0.0) {
		substdio_put(subfdout, "send load to qscheduler. qload= ", 31);
		strnum[i = fmt_double(strnum, load_l + load_r, 2)] = 0;
		substdio_put(subfdout, strnum, i);
		substdio_put(subfdout, "\n", 1);
		substdio_flush(subfdout);
		return (send_qload("/qscheduler", qnum, load, 10));
	} else {
		strerr_warn2(argv0, ": nothing communicated to qscheduler", 0);
		return 1;
	}
}
#else
#warning "not compiled with -DHASLIBRT"
#include <substdio.h>
#include <subfd.h>
#include <unistd.h>

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
	const char     *x = "$Id: setqload.c,v 1.4 2024-05-09 22:03:17+05:30 mbhangui Exp mbhangui $";

#ifdef HASLIBRT
	x = sccsidevalh;
#endif
	x++;
}
