/*
 * $Log: do_scan.c,v $
 * Revision 1.16  2020-04-01 16:13:35+05:30  Cprogrammer
 * added header for MakeArgs() function
 *
 * Revision 1.15  2018-05-18 17:39:05+05:30  Cprogrammer
 * BUG - break out of loop if file extension matches a line in badext
 *
 * Revision 1.14  2018-05-11 14:37:51+05:30  Cprogrammer
 * BUG - fixed prohibited extensions scanning
 *
 * Revision 1.13  2016-05-17 19:44:58+05:30  Cprogrammer
 * use auto_control, set by conf-control to set control directory
 *
 * Revision 1.12  2009-05-01 10:38:48+05:30  Cprogrammer
 * change for errstr argument to address_match()
 *
 * Revision 1.11  2009-04-29 21:03:17+05:30  Cprogrammer
 * check address_match() for failure
 *
 * Revision 1.10  2009-04-29 08:59:07+05:30  Cprogrammer
 * change for cdb argument to address_match()
 *
 * Revision 1.9  2009-04-29 08:24:03+05:30  Cprogrammer
 * change for address_match() function
 *
 * Revision 1.8  2008-08-03 18:25:33+05:30  Cprogrammer
 * use proper proto
 *
 * Revision 1.7  2007-12-20 13:54:42+05:30  Cprogrammer
 * removed compilation warning
 *
 * Revision 1.6  2005-02-18 17:54:30+05:30  Cprogrammer
 * Facility to specify name of scanned file by %s in SCANCMD env variable
 *
 * Revision 1.5  2004-10-22 20:24:40+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.4  2004-09-27 15:30:26+05:30  Cprogrammer
 * moved env_unset("VIRUSCHECK") to qmail-multi.c
 *
 * Revision 1.3  2004-09-23 22:54:44+05:30  Cprogrammer
 * corrected problem with permissions
 *
 * Revision 1.2  2004-09-22 22:24:55+05:30  Cprogrammer
 * added SCANCMD to select virus scanner
 * add code to reject bad attachments
 *
 * Revision 1.1  2004-09-20 11:08:52+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <dirent.h>
#include "auto_qmail.h"
#include "auto_control.h"
#include "wait.h"
#include "variables.h"
#include "scan.h"
#include "sig.h"
#include "env.h"
#include "exitcodes.h"
#include "strerr.h"
#include "qregex.h"
#include "control.h"
#include "str.h"
#include "MakeArgs.h"

extern int      flaglog;
extern pid_t    pid;
extern int      alarm_flag;
extern char    *auto_scancmd[];

int             extok = 0;
static stralloc ext = { 0 };
struct constmap mapext;
int             brpok = 0;
static stralloc brp = { 0 };

void
die_nomem()
{
	_exit(51);
}

void
die_control()
{
	_exit(55);
}

void
die(e)
	int             e;
{
	_exit(e);
}

int
scan_badattachments(char *dir_name)
{
	DIR            *dir;
	struct dirent  *dp;
	char           *x;
	int             match;
	static stralloc cdir = { 0 };
	static stralloc addr = { 0 };

	if (!dir_name || !*dir_name)
		return (0);
	if (!(dir = opendir(dir_name)))
		die(61);
	if (!controldir) {
		if (!(controldir = env_get("CONTROLDIR")))
			controldir = auto_control;
	}
	if (*controldir != '/') {
		if (!stralloc_copys(&cdir, auto_qmail))
			die_nomem();
		if (!stralloc_append(&cdir, "/"))
			die_nomem();
		if (!stralloc_cats(&cdir, controldir))
			die_nomem();
	} else
	if (!stralloc_copys(&cdir, controldir))
		die_nomem();
	if (!stralloc_0(&cdir))
		die_nomem();
	if (symlink(cdir.s, "control"))
		die(68);
	if ((extok = control_readfile(&ext, (x = env_get("BADEXT")) && *x ? x : "badext", 0)) == -1)
		die_control();
	if (extok && !constmap_init(&mapext, ext.s, ext.len, 0))
		die_nomem();
	if ((brpok = control_readfile(&brp, (x = env_get("BADEXTPATTERNS")) && *x ? x : "badextpatterns", 0)) == -1)
		die_control();
	unlink("control");
	setdotChar('.');
	for (match = 0; !match;) {
		if(!(dp = readdir(dir)))
			break;
		if (!str_diff(dp->d_name, ".") || !str_diff(dp->d_name, ".."))
			continue;
		if (!stralloc_copys(&addr, dp->d_name))
			die_nomem();
		if (!stralloc_0(&addr))
			die_nomem();
		/*- badext, badextpatterns */
		switch (address_match(0, &addr, extok ? &ext : 0, extok ? &mapext : 0, brpok ? &brp : 0, 0))
		{
		case 0:
			match = 0;
			break;
		case 1:
			match = 1;
			break;
		case -1:
			die_nomem();
		case -2:
			die_control();
		default:
			die(71);
		}
	}
	setdotChar('@');
	closedir(dir);
	return (match);
}

int
do_scan()
{
	int             avstat = -1;
	char          **scancmd;
	char           *ptr;
	unsigned long   u, i;

	/*- Defer alarms */
	sig_alarmblock();
	if (!(ptr = env_get("VIRUSCHECK")))
		return(EX_ALLOK);
	if (!*ptr)
		return(EX_ALLOK);
	scan_ulong(ptr, &u);
	switch (u)
	{
	case 3:
	case 4:
	case 5:
	case 7:
		if (scan_badattachments("."))
			return(EX_BADEX);
	default:
		break;
	}
	switch (u)
	{
	case 2:
	case 4:
	case 5:
	case 6:
		/*- Launch antivir */
		switch (pid = vfork())
		{
		case -1: /*- Can't launch; temporary failure */
			if (flaglog)
				strerr_warn1("qscanq-stdin: fatal: vfork failed: ", &strerr_sys);
			return EX_TMPERR;
		case 0:
			close(0); /*- Don't let it fiddle with message */
			close(1); /*- Don't let it fiddle with envelope */
			if ((ptr = env_get("SCANCMD"))) {
				if (!(scancmd = MakeArgs(ptr)))
					_exit(51);
				for (i = 1;scancmd[i];i++) {
					if (!str_diffn(scancmd[i], "%s", 2))
						scancmd[i] = ".";
				}
			} else
				scancmd = auto_scancmd;
			execve(*scancmd, scancmd, 0);
			if (flaglog)
				strerr_warn1("qscanq-stdin: fatal: failed to execve scanner: ", &strerr_sys);
			_exit(QQ_XBUGS); /*- hopefully never reached */ ;
		}
		/*- Allow alarms */
		sig_alarmunblock();
		/*- Catch the exit status */
		if (wait_pid(&avstat, pid) == -1) {
			if (flaglog)
				strerr_warn1("qscanq-stdin: waitpid failed: ", &strerr_sys);
			return EX_TMPERR;
		}
		if (wait_crashed(avstat)) {
			if (flaglog)
				strerr_warn1("qscanq-stdin: virus scanner crashed: ", &strerr_sys);
			return EX_TMPERR;
		}
		return (wait_exitcode(avstat));
	default:
		break;
	} /*- switch (u) */
	return(EX_ALLOK);
}

void
getversion_do_scan_c()
{
	static char    *x = "$Id: do_scan.c,v 1.16 2020-04-01 16:13:35+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
