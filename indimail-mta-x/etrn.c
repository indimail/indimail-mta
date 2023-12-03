/*
 * $Id: etrn.c,v 1.19 2023-12-03 12:19:16+05:30 Cprogrammer Exp mbhangui $
 */
#include <ctype.h>
#include <sys/types.h>
#include <unistd.h>
#include <case.h>
#include <sig.h>
#include <stralloc.h>
#include <constmap.h>
#include <str.h>
#include <fmt.h>
#include <wait.h>
#include <error.h>
#include <strerr.h>
#include <open.h>
#include <lock.h>
#include "rcpthosts.h"
#include "etrn.h"
#include "control.h"
#include "variables.h"
#include "auto_libexec.h"
#include "auto_qmail.h"
#include "qcount_dir.h"

int             err_child();
int             err_library();
void            die_nomem();
void            die_control();
static stralloc etrn, maildir1, maildir2, lockfile;

static char    *binetrnargs[5] = { 0, 0, 0, 0, 0 };

int
etrn_queue(char *arg, char *remoteip)
{
	int             child, flagetrn, len, exitcode, wstat, fd;
	size_t          mailcount;
	struct constmap mapetrn;
	static int      flagrcpt = 1;
	char           *dir;
	char            strnum[FMT_ULONG];
	stralloc        bin = {0};

	if (flagrcpt)
		flagrcpt = rcpthosts_init();
	if ((flagetrn = control_readfile(&etrn, "etrnhosts", 0)) == -1)
		die_control();
	if (flagrcpt || !flagetrn)
		return -2;
	if (!constmap_init(&mapetrn, etrn.s, etrn.len, 0))
		die_nomem();
	case_lowerb(arg, len = str_len(arg)); /*- convert into lower case */
	if (!constmap(&mapetrn, arg, len))
		return -2;
	if (rcpthosts(arg, len, 1) != 1)
		return -2;
	if (!stralloc_copys(&maildir1, auto_qmail) ||
			!stralloc_catb(&maildir1, "/autoturn/", 10) ||
			!stralloc_cats(&maildir1, arg) ||
			!stralloc_catb(&maildir1, "/Maildir", 8) ||
			!stralloc_0(&maildir1))
		die_nomem();
	if (!stralloc_copys(&maildir2, auto_qmail) ||
			!stralloc_catb(&maildir2, "/autoturn/", 10) ||
			!stralloc_cats(&maildir2, remoteip) ||
			!stralloc_catb(&maildir2, "/Maildir", 8) ||
			!stralloc_0(&maildir2))
		die_nomem();

	lockfile.len = 0;
	mailcount = 0;
	if (!access(maildir1.s, F_OK))
		qcount_dir(maildir1.s, &mailcount);
	else
	if (errno != error_noent)
		return -1;
	if (!mailcount) {
		if (!access(maildir2.s, F_OK))
			qcount_dir(maildir2.s, &mailcount);
		else
		if (errno != error_noent)
			return -1;
		if (mailcount) {
			dir = maildir2.s;
			if (!stralloc_copy(&lockfile, &maildir2))
				die_nomem();
			lockfile.len -= 8;
			if (!stralloc_catb(&lockfile, "seriallock", 10) ||
					!stralloc_0(&lockfile))
				die_nomem();
		}
	} else {
		dir = maildir1.s;
		if (!stralloc_copy(&lockfile, &maildir1))
			die_nomem();
		lockfile.len -= 8;
		if (!stralloc_catb(&lockfile, "seriallock", 10) ||
				!stralloc_0(&lockfile))
			die_nomem();
	}
	if (!mailcount)
		return -3;
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
		close(1);
		dup2(2, 1);
		if (!stralloc_copys(&bin, auto_libexec) ||
				!stralloc_catb(&bin, "/etrn", 5) ||
				!stralloc_0(&bin))
			strerr_die1x(111, "etrn: fatal: out of memory");
		binetrnargs[0] = bin.s;
		binetrnargs[1] = arg;
		binetrnargs[2] = dir;
		binetrnargs[3] = remoteip;
		execv(*binetrnargs, binetrnargs);
		_exit(1);
	}
	if (wait_pid(&wstat, child) == -1)
		return err_child();
	close(fd);
	unlink(lockfile.s);
	if (wait_crashed(wstat))
		return err_child();
	if ((exitcode = wait_exitcode(wstat))) {
		if (exitcode == 4)
			return (mailcount ? mailcount : -4);
		exitcode = 0 - exitcode;
		return (exitcode); /*- no */
	}
	return 0;
}

void
getversion_etrn_c()
{
	static char    *x = "$Id: etrn.c,v 1.19 2023-12-03 12:19:16+05:30 Cprogrammer Exp mbhangui $";

	if (x)
		x++;
}

/*
 * $Log: etrn.c,v $
 * Revision 1.19  2023-12-03 12:19:16+05:30  Cprogrammer
 * lock dir/seriallock instead of doing it in etrn script
 * moved hostname validation to valid_hname.c
 *
 * Revision 1.18  2022-01-30 08:31:52+05:30  Cprogrammer
 * replaced execvp with execv
 *
 * Revision 1.17  2021-05-26 10:36:55+05:30  Cprogrammer
 * handle access() error other than ENOENT
 *
 * Revision 1.16  2020-03-24 12:58:57+05:30  Cprogrammer
 * use qcount_dir() function to get mail counts
 *
 * Revision 1.15  2019-05-27 20:26:33+05:30  Cprogrammer
 * use VIRTUAL_PKG_LIB env variable if defined
 *
 * Revision 1.14  2019-05-27 12:24:33+05:30  Cprogrammer
 * set full path to libindimail control file
 *
 * Revision 1.13  2019-05-26 12:27:30+05:30  Cprogrammer
 * use libindimail control file to load libindimail if VIRTUAL_PKG_LIB env variable not defined
 *
 * Revision 1.12  2019-04-20 19:48:42+05:30  Cprogrammer
 * change in loadLibrary() interface
 *
 * Revision 1.11  2018-07-01 11:48:57+05:30  Cprogrammer
 * renamed getFunction() to getlibObject()
 *
 * Revision 1.10  2018-01-09 11:36:22+05:30  Cprogrammer
 * load count_dir() using loadLibrary()
 *
 * Revision 1.9  2011-07-29 09:28:21+05:30  Cprogrammer
 * fixed gcc 4.6 warnings
 *
 * Revision 1.8  2008-06-25 23:15:38+05:30  Cprogrammer
 * change for 64 bit port of indimail
 *
 * Revision 1.7  2007-12-20 12:43:59+05:30  Cprogrammer
 * removed compiler warning
 *
 * Revision 1.6  2004-10-22 20:24:55+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.5  2003-10-23 01:19:58+05:30  Cprogrammer
 * fixed compilation warnings
 *
 * Revision 1.4  2003-09-27 21:29:55+05:30  Cprogrammer
 * added code for reading morercpthosts
 *
 * Revision 1.3  2002-09-04 01:49:41+05:30  Cprogrammer
 * added function count_dir()
 *
 * Revision 1.2  2002-08-25 19:44:57+05:30  Cprogrammer
 * exitcodes logic enhanced
 *
 * Revision 1.1  2002-08-25 03:29:02+05:30  Cprogrammer
 * Initial revision
 *
 */
