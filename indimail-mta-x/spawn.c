/*
 * $Log: spawn.c,v $
 * Revision 1.31  2021-06-27 10:39:26+05:30  Cprogrammer
 * uidnit new argument to disable/enable error on missing uids
 *
 * Revision 1.30  2021-06-24 12:17:13+05:30  Cprogrammer
 * use uidinit function proto from auto_uids.h
 *
 * Revision 1.29  2021-06-13 17:31:44+05:30  Cprogrammer
 * removed chdir(auto_qmail)
 *
 * Revision 1.28  2021-05-08 12:23:15+05:30  Cprogrammer
 * use /var/indimail/queue if QUEUEDIR is not defined
 *
 * Revision 1.27  2020-11-24 13:48:16+05:30  Cprogrammer
 * removed exit.h
 *
 * Revision 1.26  2020-05-11 11:12:35+05:30  Cprogrammer
 * fixed shadowing of global variables by local variables
 *
 * Revision 1.25  2019-05-27 20:29:48+05:30  Cprogrammer
 * use VIRTUAL_PKG_LIB env variable if defined
 *
 * Revision 1.24  2019-05-27 12:39:35+05:30  Cprogrammer
 * set libfn with full path of libindimail control file
 *
 * Revision 1.23  2019-05-26 12:32:58+05:30  Cprogrammer
 * use libindimail control file to load libindimail if VIRTUAL_PKG_LIB env variable not defined
 *
 * Revision 1.22  2019-04-20 19:53:05+05:30  Cprogrammer
 * changed interface for loadLibrary(), closeLibrary() and getlibObject()
 *
 * Revision 1.21  2018-07-15 12:24:32+05:30  Cprogrammer
 * added new error string for error related to loading libindimail
 *
 * Revision 1.20  2018-01-09 12:35:11+05:30  Cprogrammer
 * use loadLibrary() to access indimail functions()
 *
 * Revision 1.19  2016-02-08 17:29:14+05:30  Cprogrammer
 * set environment variable MESSID
 *
 * Revision 1.18  2010-07-21 21:24:57+05:30  Cprogrammer
 * added envheader code
 *
 * Revision 1.17  2009-12-10 10:46:46+05:30  Cprogrammer
 * display MySQL error
 *
 * Revision 1.16  2009-12-09 23:58:04+05:30  Cprogrammer
 * additional closeflag argument to uidinit
 *
 * Revision 1.15  2008-08-03 18:26:24+05:30  Cprogrammer
 * use proper proto
 *
 * Revision 1.14  2008-07-14 20:57:56+05:30  Cprogrammer
 * fixed compilation on 64 bit Mac OS X
 *
 * Revision 1.13  2005-06-17 21:52:34+05:30  Cprogrammer
 * removed compilation warning
 *
 * Revision 1.12  2004-10-22 20:30:31+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.11  2004-07-27 22:59:18+05:30  Cprogrammer
 * initialize qqeh
 *
 * Revision 1.10  2004-07-17 21:23:22+05:30  Cprogrammer
 * added qqeh code
 * added RCS log
 *
 * Revision 1.9  2003-12-20 02:24:20+05:30  Cprogrammer
 * make compiler happy
 *
 * Revision 1.8  2003-12-07 13:00:34+05:30  Cprogrammer
 * added out of mem message
 *
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "indimail_stub.h"
#include "sig.h"
#include "wait.h"
#include "substdio.h"
#include "byte.h"
#include "str.h"
#include "stralloc.h"
#include "select.h"
#include "coe.h"
#include "open.h"
#include "env.h"
#include "error.h"
#include "alloc.h"
#include "variables.h"
#include "auto_uids.h"
#include "auto_spawn.h"
#include "auto_control.h"
#include "variables.h"

extern int      truncreport_SPAWN;
int             SPAWN(int, int, unsigned long, char *, char *, char *, int);
void            report_SPAWN(substdio *, int, char *, int);
void            initialize_SPAWN(int, char **);

typedef struct delivery
{
	int             used;
	int             fdin;		/*- pipe input */
	int             pid;		/*- zero if child is dead */
	int             wstat;		/*- if !pid: status of child */
	int             fdout;		/*- pipe output, -1 if !pid; delays eof until after death */
	stralloc        output;
} delivery;

static delivery *d;
#ifdef ENABLE_VIRTUAL_PKG
void            *phandle;
#endif

static void
sigchld()
{
	int             wstat;
	int             pid;
	int             i;

	while ((pid = wait_nohang(&wstat)) > 0) {
		for (i = 0; i < auto_spawn; ++i) {
			if (d[i].used) {
				if (d[i].pid == pid) {
					close(d[i].fdout);
					d[i].fdout = -1;
					d[i].wstat = wstat;
					d[i].pid = 0;
				}
			}
		}
	}
}

static int      flagwriting = 1;

static ssize_t
okwrite(fd, buf, n)
	int             fd;
	char           *buf;
	ssize_t         n;
{
	ssize_t         w;

	if (!flagwriting)
		return n;
	if ((w = write(fd, buf, n)) != -1)
		return w;
	if (errno == error_intr)
		return -1;
	flagwriting = 0;
	close(fd);
	return n;
}

static int      flagreading = 1;
static char     outbuf[1024];
static substdio ssout;

static int      stage = 0;	/*- reading 0:delnum 1:delnum 2:messid 3:sender 4:qqeh 5:envh 6:recip */
static int      flagabort = 0;	/*- if 1, everything except delnum is garbage */
static int      delnum;
static stralloc messid = { 0 };
static stralloc sender = { 0 };
static stralloc qqeh = { 0 };
static stralloc envh = { 0 };
static stralloc recip = { 0 };
#ifdef ENABLE_VIRTUAL_PKG
static stralloc libfn = { 0 };
#endif

static void
err(char *s)
{
	char            ch;

	ch = delnum;
	substdio_put(&ssout, &ch, 1);
	ch = delnum >> 8;
	substdio_put(&ssout, &ch, 1);
	substdio_puts(&ssout, s);
	substdio_putflush(&ssout, "", 1);
}

static int
variables_set()
{
	char           *x, *y, *z, *c, *n;

	for (y = z = envh.s;envh.len && *z;z++) {
		if (*z == '\n') {
			*(n = z) = 0;
			for (x = y;*x;x++) {
				if (*x == ':') {
					c = x;
					*x++ = 0;
					for (;*x && *x == ' ';x++);
					if (!env_put2(y, x)) {
						*n = '\n';
						*c = ':';
						return (1);
					}
					*c = ':';
					break;
				}
			}
			*n = '\n';
			y = z + 1;
		}
	}
	if (!env_put2("MESSID", messid.s))
		return (1);
	return (0);
}

static int
variables_unset()
{
	char           *x, *y, *z;

	for (y = z = envh.s;envh.len && *z;z++) {
		if (*z == '\n') {
			*z = 0;
			for (x = y;*x;x++) {
				if (*x == ':') {
					*x = 0;
					if (!env_unset(y))
						return (1);
					break;
				}
			}
			y = z + 1;
		}
	}
	return (0);
}

static void
docmd()
{
	int             f, i, j, fdmess;
	int             pi[2];
	struct stat     st;

	if (flagabort) {
		err("Zqmail-spawn out of memory. (#4.3.0)\n");
		return;
	}
	if (delnum < 0) {
		err("ZInternal error: delnum negative. (#4.3.5)\n");
		return;
	}
	if (delnum >= auto_spawn) {
		err("ZInternal error: delnum too big. (#4.3.5)\n");
		return;
	}
	if (d[delnum].used) {
		err("ZInternal error: delnum in use. (#4.3.5)\n");
		return;
	}
	for (i = 0; i < messid.len; ++i) {
		if (messid.s[i]) {
			if (!i || (messid.s[i] != '/')) {
				if ((unsigned char) (messid.s[i] - '0') > 9) {
					err("DInternal error: messid has nonnumerics. (#5.3.5)\n");
					return;
				}
			}
		}
	}
	if (messid.len > 100) {
		err("DInternal error: messid too long. (#5.3.5)\n");
		return;
	}
	if (!messid.s[0]) {
		err("DInternal error: messid too short. (#5.3.5)\n");
		return;
	}
	if (!stralloc_copys(&d[delnum].output, "")) {
		err("Zqmail-spawn out of memory. (#4.3.0)\n");
		return;
	}
	if ((j = byte_rchr(recip.s, recip.len, '@')) >= recip.len) {
		err("DSorry, address must include host name. (#5.1.3)\n");
		return;
	}
	if ((fdmess = open_read(messid.s)) == -1) {
		err("Zqmail-spawn unable to open message. (#4.3.0)\n");
		return;
	}
	if (fstat(fdmess, &st) == -1) {
		close(fdmess);
		err("Zqmail-spawn unable to fstat message. (#4.3.0)\n");
		return;
	}
	if ((st.st_mode & S_IFMT) != S_IFREG) {
		close(fdmess);
		err("ZSorry, message has wrong type. (#4.3.5)\n");
		return;
	}
	if (st.st_uid != auto_uidq) { /*- aaack! qmailq has to be trusted!  */
		/*
		 * your security is already toast at this point. damage control... 
		 */
		close(fdmess);
		err("ZSorry, message has wrong owner. (#4.3.5)\n");
		return;
	}
	if (pipe(pi) == -1) {
		close(fdmess);
		err("Zqmail-spawn unable to create pipe. (#4.3.0)\n");
		return;
	}
	coe(pi[0]);
	if (envh.len && !stralloc_0(&envh)) {
		err("Zqmail-spawn out of memory. (#4.3.0)\n");
		return;
	}
	if (variables_set()) {
		err("Zqmail-spawn out of memory. (#4.3.0)\n");
		variables_unset();
		return;
	}
	f = SPAWN(fdmess, pi[1], st.st_size, sender.s, qqeh.s, recip.s, j);
	if (variables_unset()) {
		err("Zqmail-spawn out of memory. (#4.3.0)\n");
		return;
	}
	close(fdmess);
	if (f < 0) {
		close(pi[0]);
		close(pi[1]);
		if (f == -2)
			err("Zqmail-spawn temporary MySQL error. (#4.3.0)\n");
		else
		if (f == -3)
			err("Zqmail-spawn error loading indimail library. (#4.3.0)\n");
		else
			err("Zqmail-spawn unable to fork or out of mem. (#4.3.0)\n");
		return;
	}
	d[delnum].fdin = pi[0];
	d[delnum].fdout = pi[1];
	coe(pi[1]);
	d[delnum].pid = f;
	d[delnum].used = 1;
}

static char     cmdbuf[1024];

static void
getcmd()
{
	int             i;
	int             r;
	char            ch;

	if (!(r = read(0, cmdbuf, sizeof(cmdbuf)))) {
		flagreading = 0;
		return;
	}
	if (r == -1) {
		if (errno != error_intr)
			flagreading = 0;
		return;
	}
	for (i = 0; i < r; ++i) {
		ch = cmdbuf[i];
		switch (stage)
		{
		case 0:
			delnum = (unsigned int) (unsigned char) ch;
			stage = 1;
			break;
		case 1:
			delnum += (unsigned int) ((unsigned int) ch) << 8;
			messid.len = 0;
			stage = 2;
			break;
		case 2:
			if (!stralloc_append(&messid, &ch))
				flagabort = 1;
			if (ch)
				break;
			sender.len = 0;
			stage = 3;
			break;
		case 3:
			if (!stralloc_append(&sender, &ch))
				flagabort = 1;
			if (ch)
				break;
			qqeh.len = 0;
			stage = 4;
			break;
		case 4:
			if (!stralloc_append(&qqeh, &ch))
				flagabort = 1;
			if (ch)
				break;
			envh.len = 0;
			stage = 5;
			break;
		case 5:
			if (!stralloc_append(&envh, &ch))
				flagabort = 1;
			if (ch)
				break;
			recip.len = 0;
			stage = 6;
			break;
		case 6:
			if (!stralloc_append(&recip, &ch))
				flagabort = 1;
			if (ch)
				break;
			docmd();
			flagabort = 0;
			stage = 0;
			break;
		}
	}
}

static char     inbuf[128];

int
QSPAWN(int argc, char **argv)
{
	char            ch;
#ifdef ENABLE_VIRTUAL_PKG
	char           *ptr;
#endif
	int             i, r, nfds;
	fd_set          rfds;

	if (uidinit(1, 0) == -1 || auto_uidq == -1)
		_exit(111);
	if (!(queuedir = env_get("QUEUEDIR")))
		queuedir = "queue"; /*- single queue like qmail */
	if (chdir(queuedir) == -1)
		_exit(111);
	if (chdir("mess") == -1)
		_exit(111);
	if (!stralloc_copys(&messid, ""))
		_exit(111);
	if (!stralloc_copys(&sender, ""))
		_exit(111);
	if (!stralloc_copys(&qqeh, ""))
		_exit(111);
	if (!stralloc_copys(&envh, ""))
		_exit(111);
	if (!stralloc_copys(&recip, ""))
		_exit(111);
	if (!(d = (struct delivery *) alloc((auto_spawn + 10) * sizeof(struct delivery))))
		_exit(111);
	substdio_fdbuf(&ssout, okwrite, 1, outbuf, sizeof(outbuf));
	sig_pipeignore();
	sig_childcatch(sigchld);
	initialize_SPAWN(argc, argv);
	ch = auto_spawn;
	substdio_put(&ssout, &ch, 1);
	ch = auto_spawn >> 8;
	substdio_putflush(&ssout, &ch, 1);
	for (i = 0; i < auto_spawn; ++i) {
		d[i].used = 0;
		d[i].output.s = 0;
	}
#ifdef ENABLE_VIRTUAL_PKG
	if (!(ptr = env_get("VIRTUAL_PKG_LIB"))) {
		if (!controldir) {
			if (!(controldir = env_get("CONTROLDIR")))
				controldir = auto_control;
		}
		if (!libfn.len) {
			if (!stralloc_copys(&libfn, controldir))
				_exit(111);
			if (libfn.s[libfn.len - 1] != '/' && !stralloc_append(&libfn, "/"))
				_exit(111);
			if (!stralloc_catb(&libfn, "libindimail", 11) ||
					!stralloc_0(&libfn))
				_exit(111);
		}
		ptr = libfn.s;
	} else
		ptr = "VIRTUAL_PKG_LIB";
	if(!(phandle = loadLibrary(&phandle, ptr, &i, 0)) && i)
		_exit(111);
#endif
	for (;;) {
		if (!flagreading) {
			for (i = 0; i < auto_spawn; ++i)
				if (d[i].used)
					break;
			if (i >= auto_spawn)
				_exit(0);
		}
		sig_childunblock();
		FD_ZERO(&rfds);
		if (flagreading)
			FD_SET(0, &rfds);
		nfds = 1;
		for (i = 0; i < auto_spawn; ++i) {
			if (d[i].used) {
				FD_SET(d[i].fdin, &rfds);
				if (d[i].fdin >= nfds)
					nfds = d[i].fdin + 1;
			}
		}
		r = select(nfds, &rfds, (fd_set *) 0, (fd_set *) 0, (struct timeval *) 0);
		sig_childblock();
		if (r != -1) {
			if (flagreading && FD_ISSET(0, &rfds))
				getcmd();
			for (i = 0; i < auto_spawn; ++i) {
				if (d[i].used) {
					if (FD_ISSET(d[i].fdin, &rfds)) {
						if ((r = read(d[i].fdin, inbuf, 128)) == -1)
							continue;	/*- read error on a readable pipe? be serious */
						if (r == 0) {
							ch = i;
							substdio_put(&ssout, &ch, 1);
							ch = i >> 8;
							substdio_put(&ssout, &ch, 1);
							report_SPAWN(&ssout, d[i].wstat, d[i].output.s, d[i].output.len);
							substdio_put(&ssout, "", 1);
							substdio_flush(&ssout);
							close(d[i].fdin);
							d[i].used = 0;
							continue;
						}
						while (!stralloc_readyplus(&d[i].output, r))
							sleep(10);
						/*XXX*/ byte_copy(d[i].output.s + d[i].output.len, r, inbuf);
						d[i].output.len += r;
						if (truncreport_SPAWN > 100 && d[i].output.len > truncreport_SPAWN) {
							char           *truncmess = "\nError report too long, sorry.\n";
							d[i].output.len = truncreport_SPAWN - str_len(truncmess) - 3;
							stralloc_cats(&d[i].output, truncmess);
						}
					}
				}
			}
		}
	}
	/*- Not reached */
	return(0);
}

#ifdef MAIN
static void /*- for ident command */
getversion_spawn_c()
{
	static char    *x = "$Id: spawn.c,v 1.31 2021-06-27 10:39:26+05:30 Cprogrammer Exp mbhangui $";

	x++;
}

int
main(int argc, char **argv)
{
	return QSPAWN(argc, argv);
	getversion_spawn_c(); /*- suppress compiler warning of unused function */
}
#endif
