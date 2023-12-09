/*
 * $Id: atrn.c,v 1.10 2023-12-09 11:41:22+05:30 Cprogrammer Exp mbhangui $
 */
#include <case.h>
#include <sig.h>
#include <stralloc.h>
#include <constmap.h>
#include <str.h>
#include <wait.h>
#include <fmt.h>
#include <open.h>
#include <lock.h>
#include <ctype.h>
#include <strerr.h>
#include <sys/types.h>
#include <unistd.h>
#include "rcpthosts.h"
#include "control.h"
#include "auto_libexec.h"
#include "read_assign.h"

extern int      err_child();
extern void     die_nomem();
extern void     die_control();

static char    *binatrnargs[5] = { 0, 0, 0, 0, (char *) 0 };
static stralloc atrn, atrndir, lockfile;

int
atrn_queue(char *arg, char *remoteip)
{
	int             child, flagatrn, len, exitcode, wstat, end_flag, fd;
	char           *cptr, *domain_ptr, *dir;
	char            strnum[FMT_ULONG];
	static int      flagrcpt = 1;
	struct constmap mapatrn;
	stralloc        bin = {0};

	if (flagrcpt)
		flagrcpt = rcpthosts_init();
	if ((flagatrn = control_readfile(&atrn, "etrnhosts", 0)) == -1)
		die_control();
	if (flagrcpt || !flagatrn)
		return -2;
	if (!constmap_init(&mapatrn, atrn.s, atrn.len, 0))
		die_nomem();
	for (cptr = domain_ptr = arg;;cptr++) {
		if (*cptr == ' ' || *cptr == ',' || !*cptr) {
			if (*cptr) {
				end_flag = 0;
				*cptr = 0;
			} else
				end_flag = 1;
			case_lowerb(domain_ptr, len = str_len(domain_ptr)); /*- convert into lower case */
			if (!constmap(&mapatrn, domain_ptr, len))
				return -2;
			if (rcpthosts(domain_ptr, len, 1) != 1)
				return -2;
			if (end_flag)
				break;
			else
				*cptr = ' ';
			domain_ptr = cptr + 1;
		}
	}
	if (!(dir = read_assign("autoturn", NULL, NULL, NULL)))
		return -2;
	if (!stralloc_copys(&atrndir, dir) ||
			!stralloc_append(&atrndir, "/") ||
			!stralloc_cats(&atrndir, arg) ||
			!stralloc_0(&atrndir))
		die_nomem();
	atrndir.len--;
	if (!stralloc_copy(&lockfile, &atrndir) ||
			!stralloc_catb(&lockfile, "/seriallock", 11) ||
			!stralloc_0(&lockfile))
		die_nomem();
	if ((fd = open_append(lockfile.s)) == -1)
		return -5;
	if (lock_exnb(fd) == -1) {
		close(fd);
		unlink(lockfile.s);
		return -4;
	}
	strnum[len = fmt_ulong(strnum, getpid())] = 0;
	if (write(fd, strnum, len) == -1) {
		close(fd);
		unlink(lockfile.s);
		return -1;
	}
	switch (child = fork())
	{
	case -1:
		return -1;
	case 0:
		sig_pipedefault();
		/*- dup 0, 1 to 6,7 for serialsmtp */
		dup2(1, 7); 
		dup2(0, 6);
		if (!stralloc_copys(&bin, auto_libexec) ||
				!stralloc_catb(&bin, "/atrn", 5) ||
				!stralloc_0(&bin))
			strerr_die1x(111, "atrn: fatal: out of memory");
		binatrnargs[0] = bin.s;
		binatrnargs[1] = arg;
		binatrnargs[2] = dir;
		binatrnargs[3] = atrndir.s;
		execv(*binatrnargs, binatrnargs);
		_exit(1);
	}
	if (wait_pid(&wstat, child) == -1)
		return err_child();
	close(fd);
	unlink(lockfile.s);
	if (wait_crashed(wstat))
		return err_child();
	if ((exitcode = wait_exitcode(wstat))) {
		exitcode = 0 - exitcode;
		return (exitcode); /*- no */
	}
	return (0);
}

void
getversion_atrn_c()
{
	static char    *x = "$Id: atrn.c,v 1.10 2023-12-09 11:41:22+05:30 Cprogrammer Exp mbhangui $";

	x++;
}

/*
 * $Log: atrn.c,v $
 * Revision 1.10  2023-12-09 11:41:22+05:30  Cprogrammer
 * use users/cdb to get autoturn director
 * pass etrn/atrn argument, autoturn dir and domain dir as arguments to libexec/etrn, libexec/atrn
 *
 * Revision 1.9  2023-12-03 14:58:12+05:30  Cprogrammer
 * lock dir/seriallock instead of doing it in atrn script
 *
 * Revision 1.8  2023-11-26 12:48:18+05:30  Cprogrammer
 * use auto_libexec for atrn script
 *
 * Revision 1.7  2022-01-30 08:26:55+05:30  Cprogrammer
 * replaced execvp with execv
 *
 * Revision 1.6  2008-05-26 22:19:31+05:30  Cprogrammer
 * removed auto_qmail.h
 *
 * Revision 1.5  2004-10-22 20:21:44+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.4  2004-09-25 23:59:18+05:30  Cprogrammer
 * removed stdio.h
 *
 * Revision 1.3  2003-10-23 01:15:51+05:30  Cprogrammer
 * fixed compilation warnings
 *
 * Revision 1.2  2003-09-27 21:10:21+05:30  Cprogrammer
 * added code to read morercpthosts
 *
 * Revision 1.1  2003-07-05 17:31:33+05:30  Cprogrammer
 * Initial revision
 *
 */
