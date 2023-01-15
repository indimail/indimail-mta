/*
 * $Log: uacl.c,v $
 * Revision 1.8  2023-01-15 23:32:07+05:30  Cprogrammer
 * out() changed to have varargs
 *
 * Revision 1.7  2021-08-29 23:27:08+05:30  Cprogrammer
 * define functions as noreturn
 *
 * Revision 1.6  2021-06-13 17:36:34+05:30  Cprogrammer
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
#include <noreturn.h>
#include "qregex.h"
#include "control.h"
#include "matchregex.h"
#include "mail_acl.h"
#include "wildmat.h"
#include "varargs.h"

#define FATAL "uacl: fatal: "

void
#ifdef  HAVE_STDARG_H
out(char *s1, ...)
#else
out(va_alist)
va_dcl
#endif
{
	va_list         ap;
	char           *str;
#ifndef HAVE_STDARG_H
	char           *s1;
#endif

#ifdef HAVE_STDARG_H
	va_start(ap, s1);
#else
	va_start(ap);
	s1 = va_arg(ap, char *);
#endif

	if (substdio_puts(subfdout, s1) == -1)
		_exit(1);
	while (1) {
		str = va_arg(ap, char *);
		if (!str)
			break;
		if (substdio_puts(subfdout, str) == -1)
			_exit(1);
	}
	va_end(ap);
}

void
flush()
{
	if (substdio_flush(subfdout) == -1)
		strerr_die2sys(111, FATAL, "write: ");
	return;
}

no_return void
die_control()
{
	substdio_putsflush(subfderr, "uacl: unable to read controls (#4.3.0)\n");
	substdio_flush(subfdout);
	_exit(111);
}

no_return void
die_nomem()
{
	substdio_putsflush(subfderr, "uacl: out of memory\n");
	substdio_flush(subfdout);
	_exit(111);
}

no_return void
die_usage()
{
	substdio_putsflush(subfderr, "usage: uacl sender recipient\n");
	substdio_flush(subfdout);
	_exit(111);
}

no_return void
die_regex(char *str)
{
	substdio_puts(subfderr, "uacl: regex failed: ");
	substdio_puts(subfderr, str);
	substdio_puts(subfderr, "\n");
	substdio_flush(subfdout);
	substdio_flush(subfderr);
	_exit(111);
}

int
main(int argc, char **argv)
{
	char           *x;
	int             i, qregex = 0, acclistok = 0;
	stralloc        acclist = { 0 };

	if (argc != 3)
		die_usage();
	if ((x = env_get("QREGEX")))
		scan_int(x, &qregex);
	else {
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
	static char    *x = "$Id: uacl.c,v 1.8 2023-01-15 23:32:07+05:30 Cprogrammer Exp mbhangui $";

	x = sccsidwildmath;
	x++;
}
