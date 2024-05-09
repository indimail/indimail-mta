/*
 * $Id: test-recipients.c,v 1.4 2024-05-09 22:03:17+05:30 mbhangui Exp mbhangui $
 */
#include <unistd.h>
#include <substdio.h>
#include <subfd.h>
#include <sgetopt.h>
#include <env.h>
#include <scan.h>
#include <str.h>
#include <stralloc.h>
#include <strerr.h>
#include <timeoutwrite.h>
#include "indimail_stub.h"
#include "recipients.h"
#include "variables.h"
#include "auto_control.h"
#include "auto_sysconfdir.h"

#define FATAL "test-recipients: fatal: "

static stralloc libfn = { 0 };
void           *phandle;
int             timeout = 60;

ssize_t
safewrite(int fd, char *buf, int len)
{
	int             r;

	if ((r = timeoutwrite(timeout, fd, buf, len)) <= 0)
		_exit(1);
	return r;
}

const char     *
load_virtual()
{
	const char     *ptr, *errstr;
	int             i;

	/*- load virtual package library */
	if (!(ptr = env_get("VIRTUAL_PKG_LIB"))) {
		if (!controldir) {
			if (!(controldir = env_get("CONTROLDIR")))
				controldir = auto_control;
		}
		if (!libfn.len) {
			if (!stralloc_copys(&libfn, controldir) ||
					(libfn.s[libfn.len - 1] != '/' && !stralloc_append(&libfn, "/")) ||
					!stralloc_catb(&libfn, "libindimail", 11) ||
					!stralloc_0(&libfn))
				strerr_die2x(111, FATAL, "out of memory");
		}
		ptr = libfn.s;
	}
	loadLibrary(&phandle, ptr, &i, &errstr);
	if (i)
		strerr_die3x(111, FATAL, "error loading libindimail: ", errstr);
	return ptr;
}

int
check_recipient_sql(char *addr, int len)
{
	const char     *ptr, *errstr;
	void           *(*inquery) (char, char *, char *);

	if (!(ptr = load_virtual()))
		return -1;
	if (!(inquery = getlibObject(ptr, &phandle, "inquery", &errstr)))
		strerr_die3x(111, FATAL, "error loading symbom inquery: ", errstr);
	if ((ptr = (*inquery) (USER_QUERY, addr, 0))) {
		if (*ptr == 4)	   /*- allow aliases */
			return (0);
		return (*ptr);
	}
	strerr_die2x(111, FATAL, "database error");
	/*- Not Reached */
	return (0);
}

int
main(int argc, char **argv)
{
	int             i, do_recipient = 0, do_sql = 0;
	char           *arg, *sysconfdir;

	while ((i = getopt(argc, argv, "rst:")) != opteof) {
		switch (i)
		{
		case 't':
			scan_int(optarg, &timeout);
			break;
		case 'r':
			do_recipient = 1;
			break;
		case 's':
			do_sql = 1;
			break;
		default:
			strerr_die1x(100, "test-recipients -r | -s addr");
		}
	}
	if ((!do_recipient && !do_sql) || (do_recipient && do_sql))
		strerr_die1x(100, "test-recipients -r | -s addr");
	if (optind + 1 != argc)
		strerr_die1x(100, "test-recipients -r | -s addr");
	arg = argv[optind++];
	if (do_sql == 1) {
		/*
		 * This function returns
		 *  0: User is fine
		 *  1: User is not present
		 *  2: User is Inactive
		 *  3: User is overquota
		 * -1: System Error
		 */
		switch(check_recipient_sql(arg, str_len(arg)))
		{
		case 0:
			strerr_die1x(1, "test-recipients: user found in MySQL");
		case 1:
			strerr_die2x(1, FATAL, "user not found");
		case 2:
			strerr_die2x(1, FATAL, "user is inactive");
		case 3:
			strerr_die2x(1, FATAL, "user is overquota");
		}
	} else
	if (do_recipient == 1) {
		sysconfdir = (sysconfdir = env_get("SYSCONFDIR")) ? sysconfdir : auto_sysconfdir;
		if (chdir(sysconfdir) == -1)
			strerr_die4sys(111, FATAL, "unable to chdir to ", sysconfdir, ": ");
		if (recipients_init() == -1)
			strerr_die2x(111, FATAL, "unable to initialize recipients extension");
		/*-
		 * return -3: problem with PAVM
		 * return -2: out of memory
		 * return -1: error reading control file
		 * return  0: address not found; fatal
		 * return  1: CDB lookup
		 * return  2: PAVM lookup
		 * return  3: Wildcarded domain
		 * return  4: Pass-thru
		 * return 10: none existing control file; pass-thru
		 */
		switch(recipients(arg, str_len(arg)))
		{
		case 0:
			strerr_die2x(1, FATAL, "user not found");
		case 1:
			strerr_die1x(0, "test-recipients: user found in cdb");
		case 2:
			strerr_die1x(0, "test-recipients: user found through PAVM");
		case 3:
			strerr_die1x(0, "test-recipients: user matched through wildcard");
		case 4:
			strerr_die1x(0, "test-recipients: user matched through pass through");
		case 10:
			strerr_die1x(111, "test-recipients: control file missing");
		case -1:
			strerr_die1x(111, "test-recipients: error reading control/recipients");
		case -2:
			strerr_die1x(111, "test-recipients: out of memory");
		case -3:
			strerr_die1x(111, "test-recipients: Error running PAVM");
		case 111:
			strerr_die1x(111, "test-recipients: command configured for PAVM failed");
		default:
			strerr_die1x(111, "test-recipients: unknown/unhandled error in recipients extension");
		}
	}
}
