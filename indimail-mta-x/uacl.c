/*
 * $Log: uacl.c,v $
 * Revision 1.6  2021-06-12 20:02:15+05:30  Cprogrammer
 * removed chdir(auto_qmail)
 *
 * Revision 1.5  2021-05-23 07:11:36+05:30  Cprogrammer
 * include wildmat.h for wildmat_internal
 *
 * Revision 1.4  2010-11-05 06:29:38+05:30  Cprogrammer
 * moved mail_acl() to mail_acl.c
 *
 * Revision 1.3  2010-01-20 11:26:32+05:30  Cprogrammer
 * new logic for access list
 *
 * Revision 1.2  2010-01-19 13:27:58+05:30  Cprogrammer
 * display error for chdir instead of 'unable to read controls'
 *
 * Revision 1.1  2010-01-19 13:24:58+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <unistd.h>
#include <scan.h>
#include <str.h>
#include <fmt.h>
#include <env.h>
#include <subfd.h>
#include <strerr.h>
#include "qregex.h"
#include "control.h"
#include "matchregex.h"
#include "mail_acl.h"
#include "wildmat.h"

#define FATAL "uacl: fatal: "

/*- accesslist */
int             acclistok = 0;
static stralloc acclist = { 0 };
int             qregex = 0;

void
out(char *str)
{
	if (!str || !*str)
		return;
	if (substdio_puts(subfdout, str) == -1)
		strerr_die2sys(111, FATAL, "write: ");
	return;
}

void
flush()
{
	if (substdio_flush(subfdout) == -1)
		strerr_die2sys(111, FATAL, "write: ");
	return;
}

void
die_control()
{
	substdio_putsflush(subfderr, "uacl: unable to read controls (#4.3.0)\n");
	substdio_flush(subfdout);
	_exit(111);
}

void
die_nomem()
{
	substdio_putsflush(subfderr, "uacl: out of memory\n");
	substdio_flush(subfdout);
	_exit(111);
}

void
die_usage()
{
	substdio_putsflush(subfderr, "usage: uacl sender recipient\n");
	substdio_flush(subfdout);
	_exit(111);
}

void
die_regex(char *str)
{
	substdio_puts(subfderr, "uacl: regex failed: ");
	substdio_puts(subfderr, str);
	substdio_puts(subfderr, "\n");
	substdio_flush(subfdout);
	substdio_flush(subfderr);
	_exit(111);
}

void
die_chdir()
{
	substdio_putsflush(subfderr, "uacl: fatal: unable to chdir to home\n");
	_exit(111);
}

int
main(int argc, char **argv)
{
	char           *x;
	int             i;

	if (argc != 3)
		die_usage();
	if ((x = env_get("QREGEX")))
		scan_int(x, &qregex);
	else
	{
		if (control_readint(&qregex, "qregex") == -1)
			die_control();
		if (qregex && !env_put("QREGEX=1"))
			die_nomem();
	}
	acclistok = control_readfile(&acclist, (x = env_get("ACCESSLIST")) && *x ? x : "accesslist", 0);
	i = acclistok ? mail_acl(&acclist, qregex, argv[1], argv[2], 1) : 0;
	flush();
	return (i ? 100 : 0);
}

void
getversion_uacl_c()
{
	static char    *x = "$Id: uacl.c,v 1.6 2021-06-12 20:02:15+05:30 Cprogrammer Exp mbhangui $";

	x = sccsidwildmath;
	x++;
}
