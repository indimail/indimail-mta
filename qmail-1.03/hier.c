/*
 * $Log: hier.c,v $
 * Revision 1.127  2009-10-19 10:13:04+05:30  Cprogrammer
 * added qmail-daemon.8
 *
 * Revision 1.126  2009-09-08 12:33:23+05:30  Cprogrammer
 * compilation of control/domainkeys to be default
 *
 * Revision 1.125  2009-09-01 21:58:56+05:30  Cprogrammer
 * added batv tester
 *
 * Revision 1.124  2009-08-30 17:32:21+05:30  Cprogrammer
 * added greydaemon
 *
 * Revision 1.123  2009-08-29 20:47:15+05:30  Cprogrammer
 * removed greydaemon
 *
 * Revision 1.122  2009-08-29 15:25:52+05:30  Cprogrammer
 * added qmail-greyd
 *
 * Revision 1.121  2009-08-24 11:05:49+05:30  Cprogrammer
 * added README.greylist
 *
 * Revision 1.120  2009-08-23 21:10:42+05:30  Cprogrammer
 * added greydaemon
 *
 * Revision 1.119  2009-08-05 14:46:58+05:30  Cprogrammer
 * added qmail-poppass
 *
 * Revision 1.118  2009-06-25 12:39:08+05:30  Cprogrammer
 * renamed maildirmake to qmaildirmake to avoid conflict with courier-imap's maildirmake
 *
 * Revision 1.117  2009-06-17 14:14:18+05:30  Cprogrammer
 * added StartupParameters.plist and indimail.plist for MacOS startup
 *
 * Revision 1.116  2009-04-29 11:21:05+05:30  Cprogrammer
 * renamed qmail-spamdb as qmail-cdb
 *
 * Revision 1.115  2009-04-24 23:49:45+05:30  Cprogrammer
 * added qmail-qfilter
 *
 * Revision 1.114  2009-04-23 13:48:08+05:30  Cprogrammer
 * removed qmail-qfilter
 *
 * Revision 1.113  2009-04-22 15:23:39+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.112  2009-04-20 10:06:43+05:30  Cprogrammer
 * added dk-filter
 *
 * Revision 1.111  2009-04-19 13:39:39+05:30  Cprogrammer
 * removed echo program
 *
 * Revision 1.110  2009-04-10 20:24:17+05:30  Cprogrammer
 * added all README files
 *
 * Revision 1.109  2009-04-07 20:17:05+05:30  Cprogrammer
 * added domainkeys directory
 *
 * Revision 1.108  2009-04-06 16:38:51+05:30  Cprogrammer
 * removed function cd()
 *
 * Revision 1.107  2009-04-03 13:42:20+05:30  Cprogrammer
 * changed permission of qmail-dk, qmail-dkim to 555
 *
 * Revision 1.106  2009-04-02 20:35:22+05:30  Cprogrammer
 * added qmail-qfilter
 *
 * Revision 1.105  2009-04-02 14:56:09+05:30  Cprogrammer
 * added dk-filter
 *
 * Revision 1.104  2009-03-30 22:25:36+05:30  Cprogrammer
 * added rfc-4871.5
 *
 * Revision 1.103  2009-03-27 16:48:16+05:30  Cprogrammer
 * added qmail-dkim man page
 *
 * Revision 1.102  2009-03-19 10:09:39+05:30  Cprogrammer
 * added qmail-dkim
 *
 * Revision 1.101  2009-03-09 15:49:24+05:30  Cprogrammer
 * renamed FAQ to QMAILFAQ
 *
 * Revision 1.100  2009-02-21 15:05:31+05:30  Cprogrammer
 * added maildirmake
 *
 * Revision 1.99  2009-02-15 17:24:22+05:30  Cprogrammer
 * changed perm for upstart to 444
 *
 * Revision 1.98  2009-02-10 09:29:14+05:30  Cprogrammer
 * changed perm of domain direcotory to 775
 *
 * Revision 1.97  2009-02-08 10:08:25+05:30  Cprogrammer
 * changed owner of qmail-dk to root
 *
 * Revision 1.96  2009-02-01 00:00:23+05:30  Cprogrammer
 * added rpmattr
 *
 * Revision 1.95  2009-01-26 15:18:14+05:30  Cprogrammer
 * moved upstart to boot
 *
 * Revision 1.94  2008-09-16 08:25:09+05:30  Cprogrammer
 * added cdbmake package
 *
 * Revision 1.93  2008-07-27 15:26:39+05:30  Cprogrammer
 * renamed svscan.upstart to upstart
 *
 * Revision 1.92  2008-07-25 16:50:44+05:30  Cprogrammer
 * removed qbiff
 *
 * Revision 1.91  2008-06-25 20:38:04+05:30  Cprogrammer
 * added upstart
 *
 * Revision 1.90  2008-06-17 11:34:52+05:30  Cprogrammer
 * removed duplicate update_tmprsadh
 *
 * Revision 1.89  2008-06-06 14:32:12+05:30  Cprogrammer
 * added mbox2maildir
 *
 * Revision 1.88  2008-06-06 10:24:30+05:30  Cprogrammer
 * added rfc-1845 man page
 *
 * Revision 1.87  2008-06-05 10:57:07+05:30  Cprogrammer
 * moved to more secure permissions
 *
 * Revision 1.86  2008-06-01 15:31:28+05:30  Cprogrammer
 * added logselect
 *
 * Revision 1.85  2008-05-27 19:31:57+05:30  Cprogrammer
 * added instcheck binary to distribution
 *
 * Revision 1.84  2008-05-26 22:22:02+05:30  Cprogrammer
 * added commandline argument for configurable qmail home
 * moved documents to doc
 *
 * Revision 1.83  2008-05-22 19:46:34+05:30  Cprogrammer
 * added auto_uido, auto_gidq perms
 *
 * Revision 1.82  2008-02-05 15:28:48+05:30  Cprogrammer
 * added mlmatchup
 *
 * Revision 1.81  2007-12-20 12:44:25+05:30  Cprogrammer
 * removed compiler warning
 *
 * Revision 1.80  2005-07-04 18:04:48+05:30  Cprogrammer
 * bug fix for TCPTO_BUFSIZ
 *
 * Revision 1.79  2005-06-29 20:53:49+05:30  Cprogrammer
 * Size of tcpto changed to TCPTO_BUFSIZ
 *
 * Revision 1.78  2005-05-31 15:43:36+05:30  Cprogrammer
 * added update_tmprsadh
 *
 * Revision 1.77  2005-04-26 23:31:54+05:30  Cprogrammer
 * added plugins directory
 *
 * Revision 1.76  2005-04-24 22:30:55+05:30  Cprogrammer
 * added qhpsi executable
 *
 * Revision 1.75  2005-04-05 12:03:23+05:30  Cprogrammer
 * added relaytest
 *
 * Revision 1.74  2005-03-28 09:38:38+05:30  Cprogrammer
 * added qmail-internals
 *
 * Revision 1.73  2005-03-03 14:35:58+05:30  Cprogrammer
 * added stripmime
 *
 * Revision 1.72  2005-01-22 00:38:29+05:30  Cprogrammer
 * removed 822headers
 *
 * Revision 1.71  2004-10-29 00:09:27+05:30  Cprogrammer
 * added domain-keys man page
 *
 * Revision 1.70  2004-10-27 17:52:00+05:30  Cprogrammer
 * added rfc 3798
 *
 * Revision 1.69  2004-10-24 22:25:59+05:30  Cprogrammer
 * added recordio, argv0, addcr, delcr,fixcrio
 *
 * Revision 1.68  2004-10-22 20:25:44+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.67  2004-10-22 14:42:10+05:30  Cprogrammer
 * removed man pages for ucspi
 *
 * Revision 1.66  2004-10-20 20:03:07+05:30  Cprogrammer
 * install qmail-dk if DOMAIN_KEYS is defined
 *
 * Revision 1.65  2004-10-09 23:23:41+05:30  Cprogrammer
 * added daemontools
 *
 * Revision 1.64  2004-09-29 23:18:09+05:30  Cprogrammer
 * added dknewkey and dktest man pages
 *
 * Revision 1.63  2004-09-27 15:30:45+05:30  Cprogrammer
 * added substdio man pages
 *
 * Revision 1.62  2004-09-23 21:02:40+05:30  Cprogrammer
 * changed group permission of qscanq to qscand
 *
 * Revision 1.61  2004-09-21 23:46:07+05:30  Cprogrammer
 * added recipient extension
 *
 * Revision 1.60  2004-09-19 22:47:29+05:30  Cprogrammer
 * added qscanq virus scanner
 *
 * Revision 1.59  2004-08-28 01:05:58+05:30  Cprogrammer
 * added qmail-dk
 *
 * Revision 1.58  2004-08-25 13:24:09+05:30  Cprogrammer
 * added RFC 2476
 *
 * Revision 1.57  2004-08-15 20:22:47+05:30  Cprogrammer
 * added dktest, dknewkey spfquery dnstxt
 *
 * Revision 1.56  2004-07-30 12:52:26+05:30  Cprogrammer
 * added update_tmprsadh for TLS
 *
 * Revision 1.55  2004-07-27 22:57:56+05:30  Cprogrammer
 * added qlogtools
 *
 * Revision 1.54  2004-06-18 22:41:12+05:30  Cprogrammer
 * added qtools
 *
 * Revision 1.53  2004-06-16 01:20:42+05:30  Cprogrammer
 * added mess822
 *
 * Revision 1.52  2004-05-14 00:45:47+05:30  Cprogrammer
 * added serialmail
 *
 * Revision 1.51  2004-02-13 14:49:29+05:30  Cprogrammer
 * added rspamstat, rspamhist
 *
 * Revision 1.50  2004-01-08 00:30:30+05:30  Cprogrammer
 * added qmail-cat, rfc-2505, rfc-2635, spam stats scripts
 *
 * Revision 1.49  2004-01-05 14:03:10+05:30  Cprogrammer
 * added smtp-matchup
 *
 * Revision 1.48  2004-01-04 23:15:32+05:30  Cprogrammer
 * *** empty log message ***
 *
 */
#include "auto_qmail.h"
#include "auto_split.h"
#include "auto_uids.h"
#include "qmail-todo.h"
#include "fmt.h"
#include "fifo.h"
#include "tcpto.h"
#include "hasindimail.h"

void            d(char *, char *, int, int, int);
void            h(char *, int, int, int);
void            c(char *, char *, char *, int, int, int);
void            z(char *, char *, int, int, int, int);
void            p(char *, char *, int, int, int);

char            buf[100 + FMT_ULONG];
char           *auto_qmail_home = auto_qmail;

void
dsplit(base, uid, mode)
	char           *base; /*- must be under 100 bytes */
	int             uid;
	int             mode;
{
	char           *x;
	unsigned long   i;

	d(auto_qmail_home, base, uid, auto_gidq, mode);
	for (i = 0; i < auto_split; ++i)
	{
		x = buf;
		x += fmt_str(x, base);
		x += fmt_str(x, "/");
		x += fmt_ulong(x, i);
		*x = 0;
		d(auto_qmail_home, buf, uid, auto_gidq, mode);
	}
}

void
hier(inst_dir)
	char           *inst_dir;
{

	if (inst_dir && *inst_dir)
		auto_qmail_home = inst_dir;
	/* Directories */
	h(auto_qmail_home, auto_uido, auto_gidq, 0555);
	d(auto_qmail_home, "control", auto_uidv, auto_gidv, 0755);
	d(auto_qmail_home, "domains", auto_uido, auto_gidv, 0775);
	d(auto_qmail_home, "users", auto_uido, auto_gidq, 0555);
	d(auto_qmail_home, "bin", auto_uido, auto_gidq, 0555);
	d(auto_qmail_home, "boot", auto_uido, auto_gidq, 0555);
	d(auto_qmail_home, "doc", auto_uido, auto_gidq, 0555);
#ifdef INDIMAIL
	d(auto_qmail_home, "autoturn", auto_uidv, auto_gidq, 02755);
	d(auto_qmail_home, "control/inquery", auto_uidv, auto_gidv, 0775);
#endif
	d(auto_qmail_home, "control/domainkeys", auto_uidv, auto_gidv, 0755);
	d(auto_qmail_home, "plugins", auto_uido, auto_gidq, 0555);
	d(auto_qmail_home, "qscanq", auto_uidc, auto_gidc, 0750);
	d(auto_qmail_home, "qscanq/root", auto_uidc, auto_gidc, 0750);
	d(auto_qmail_home, "qscanq/root/scanq", auto_uidc, auto_gidc, 0750);
	d(auto_qmail_home, "man", auto_uido, auto_gidq, 0555);
	d(auto_qmail_home, "man/cat1", auto_uido, auto_gidq, 0555);
	d(auto_qmail_home, "man/cat5", auto_uido, auto_gidq, 0555);
	d(auto_qmail_home, "man/cat7", auto_uido, auto_gidq, 0555);
	d(auto_qmail_home, "man/cat8", auto_uido, auto_gidq, 0555);
	d(auto_qmail_home, "man/man1", auto_uido, auto_gidq, 0555);
	d(auto_qmail_home, "man/man3", auto_uido, auto_gidq, 0555);
	d(auto_qmail_home, "man/man5", auto_uido, auto_gidq, 0555);
	d(auto_qmail_home, "man/man7", auto_uido, auto_gidq, 0555);
	d(auto_qmail_home, "man/man8", auto_uido, auto_gidq, 0555);
	d(auto_qmail_home, "alias", auto_uida, auto_gidq, 02555);
#ifndef INDIMAIL
	d(auto_qmail_home, "queue", auto_uidq, auto_gidq, 0750);
	d(auto_qmail_home, "queue/pid", auto_uidq, auto_gidq, 0700);
	d(auto_qmail_home, "queue/intd", auto_uidq, auto_gidq, 0700);
	d(auto_qmail_home, "queue/todo", auto_uidq, auto_gidq, 0750);
	d(auto_qmail_home, "queue/bounce", auto_uids, auto_gidq, 0700);
	dsplit("queue/mess", auto_uidq, 0750);
	dsplit("queue/todo", auto_uidq, 0750);
	dsplit("queue/intd", auto_uidq, 0700);
	dsplit("queue/info", auto_uids, 0700);
	dsplit("queue/local", auto_uids, 0700);
	dsplit("queue/remote", auto_uids, 0700);
	d(auto_qmail_home, "queue/lock", auto_uidq, auto_gidq, 0750);
	z(auto_qmail_home, "queue/lock/tcpto", TCPTO_BUFSIZ, auto_uidr, auto_gidq, 0644);
	z(auto_qmail_home, "queue/lock/sendmutex", 0, auto_uids, auto_gidq, 0600);
	p(auto_qmail_home, "queue/lock/trigger", auto_uids, auto_gidq, 0622);
#endif /*- #ifndef INDIMAIL */

	/* Boot files */
	c(auto_qmail_home, "boot", "home", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "boot", "home+df", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "boot", "proc", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "boot", "proc+df", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "boot", "binm1", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "boot", "binm1+df", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "boot", "binm2", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "boot", "binm2+df", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "boot", "binm3", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "boot", "binm3+df", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "boot", "upstart", auto_uido, auto_gidq, 0444);
#ifdef DARWIN
	c(auto_qmail_home, "boot", "StartupParameters.plist", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "boot", "indimail.plist", auto_uido, auto_gidq, 0444);
#endif

	/* Binaries */
	c(auto_qmail_home, "bin", "qmail-lspawn", auto_uido, auto_gidq, 0500);
	c(auto_qmail_home, "bin", "qmail-start", auto_uido, auto_gidq, 0500);
	c(auto_qmail_home, "bin", "qmail-daemon", auto_uido, auto_gidq, 0500);
	c(auto_qmail_home, "bin", "qmail-getpw", auto_uido, auto_gidq, 0511);
	c(auto_qmail_home, "bin", "qmail-local", auto_uido, auto_gidq, 0511);
	c(auto_qmail_home, "bin", "qmail-remote", auto_uido, auto_gidq, 0511);
	c(auto_qmail_home, "bin", "qmail-rspawn", auto_uido, auto_gidq, 0511);
	c(auto_qmail_home, "bin", "qmail-clean", auto_uido, auto_gidq, 0511);
	c(auto_qmail_home, "bin", "qmail-send", auto_uido, auto_gidq, 0511);
#ifdef EXTERNAL_TODO
	c(auto_qmail_home, "bin", "qmail-todo", auto_uido,auto_gidq,0511);
#endif
	c(auto_qmail_home, "bin", "relaytest", auto_uido, auto_gidq, 0511);
	c(auto_qmail_home, "bin", "splogger", auto_uido, auto_gidq, 0511);
	c(auto_qmail_home, "bin", "qmail-newu", auto_uido, auto_gidq, 0500);
	c(auto_qmail_home, "bin", "qmail-newmrh", auto_uido, auto_gidq, 0500);
	c(auto_qmail_home, "bin", "qmail-recipients", auto_uido, auto_gidq, 0500);
	c(auto_qmail_home, "bin", "qmail-cdb", auto_uido, auto_gidq, 0500);
	c(auto_qmail_home, "bin", "qmail-pw2u", auto_uido, auto_gidq, 0511);
	c(auto_qmail_home, "bin", "qmail-inject", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "predate", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "datemail", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "mailsubj", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "qmail-showctl", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "qmail-qread", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "qmail-qstat", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "qmail-qmHandle", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "qmail-lint", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "qmail-multi", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "qmail-qfilter", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "spawn-filter", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "qmail-cat", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "qmail-poppass", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "qmail-lagcheck", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "qpq", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "qmail-queue-print", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "qmail-tcpto", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "qmail-tcpok", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "qmail-pop3d", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "qmail-popbull", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "qmail-popup", auto_uido, auto_gidq, 0511);
	c(auto_qmail_home, "bin", "qmail-qmqpc", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "qmail-qmqpd", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "qmail-qmtpd", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "qmail-smtpd", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "qmail-greyd", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "greydaemon", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "sendmail", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "tcp-env", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "qreceipt", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "qsmhook", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "forward", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "rrforward", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "preline", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "condredirect", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "bouncesaying", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "except", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "qmaildirmake", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "maildir2mbox", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "mbox2maildir", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "maildirwatch", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "maildirdeliver", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "qmail-rm", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "qmail-autoresponder", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "qail", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "elq", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "etrn", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "atrn", auto_uido, auto_gidq, 0555);
#if defined(HASDKIM) || defined(DOMAIN_KEYS)
	c(auto_qmail_home, "bin", "dk-filter", auto_uido, auto_gidq, 0555);
#endif
	c(auto_qmail_home, "bin", "envconf", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "qmail-nullqueue", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "config", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "config-fast", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "pinq", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "rpmattr", auto_uido, auto_gidq, 0555);
#ifdef TLS
	c(auto_qmail_home, "bin", "update_tmprsadh", auto_uido, auto_gidq, 0555);
#endif
	c(auto_qmail_home, "bin", "queue-fix", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "multirotate", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "svscanboot", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "qmail-sighup", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "qmail-sigalrm", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "qmail-sigterm", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "instcheck", auto_uido, auto_gidq, 0500);

	/* mess822 */
	c(auto_qmail_home, "bin", "ofmipd", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "ofmipname", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "iftocc", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "new-inject", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "822field", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "822header", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "822date", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "822received", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "822print", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "822body", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "822headerok", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "822bodyfilter", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "822headerfilter", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "822addr", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "822fields", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "checkaddr", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "checkdomain", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "filterto", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "condtomaildir", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "iftoccfrom", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "ifaddr", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "replier", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "replier-config", auto_uido, auto_gidq, 0555);

	c(auto_qmail_home, "bin", "dnscname", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "dnsptr", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "dnsip", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "dnsmxip", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "dnsfq", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "ipmeprint", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "idedit", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "qmailctl", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "b64encode", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "b64decode", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "plainencode", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "mbox2maildir.pl", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "qmail-scanner-queue.pl", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "stripmime.pl", auto_uido, auto_gidq, 0555);

	/* fastforward */
	c(auto_qmail_home, "bin", "envmigrate", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "dot-forward", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "fastforward", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "printforward", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "setforward", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "newaliases", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "printmaillist", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "setmaillist", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "newinclude", auto_uido, auto_gidq, 0555);

	/* supervise */
	c(auto_qmail_home, "bin", "envdir", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "envuidgid", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "fghack", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "multilog", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "pgrphack", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "readproctitle", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "setlock", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "setuidgid", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "softlimit", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "supervise", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "svc", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "svok", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "svscan", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "svstat", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "tai64n", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "tai64nlocal", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "tai64nunix", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "spipe", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "qfilelog", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "multipipe", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "teepipe", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "multitail", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "logselect", auto_uido, auto_gidq, 0555);

	/* serialmail */
	c(auto_qmail_home, "bin", "serialcmd", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "serialqmtp", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "serialsmtp", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "maildircmd", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "maildirqmtp", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "maildirsmtp", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "maildirserial", auto_uido, auto_gidq, 0555);

#ifdef USE_SPF
	c(auto_qmail_home, "bin", "spfquery", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "dnstxt", auto_uido, auto_gidq, 0555);
#endif
#ifdef BATV
	c(auto_qmail_home, "bin", "batv", auto_uido, auto_gidq, 0555);
#endif
#ifdef DOMAIN_KEYS
	c(auto_qmail_home, "bin", "dknewkey", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "dktest", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "qmail-dk", auto_uido, auto_gidq, 0555);
#endif
#ifdef HASDKIM
	c(auto_qmail_home, "bin", "qmail-dkim", auto_uido, auto_gidq, 0555);
#endif
	c(auto_qmail_home, "plugins", "generic.so", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "cleanq", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "qscanq-stdin", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "recordio", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "argv0", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "addcr", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "delcr", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "fixcrio", auto_uido, auto_gidq, 0555);

	/* Report Programs */
	c(auto_qmail_home, "bin", "matchup", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "mlmatchup", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "columnt", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "zoverall", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "zsendmail", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "xqp", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "xsender", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "xrecipient", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "ddist", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "deferrals", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "failures", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "successes", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "rhosts", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "recipients", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "rxdelay", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "senders", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "suids", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "zddist", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "zdeferrals", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "zfailures", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "zsuccesses", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "zrhosts", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "zrecipients", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "zrxdelay", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "zsenders", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "zsuids", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "multilog-matchup", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "smtp-matchup", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "rsmtp", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "rsmtpfailures", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "rsmtpsdomains", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "rsmtprdomains", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "rsmtpsenders", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "rsmtprecipients", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "zsmtp", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "zspam", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "rspamrdomain", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "rspamsdomain", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "rspamstat", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "rspamhist", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "cdbmake", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "cdbget", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "cdbdump", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "cdbstats", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "cdbtest", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "cdbmake-12", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "cdbmake-sv", auto_uido, auto_gidq, 0555);
	c(auto_qmail_home, "bin", "testzero", auto_uido, auto_gidq, 0555);

	/* Setuid Programs */
	c(auto_qmail_home, "bin", "qmail-queue", auto_uidq, auto_gidq, 06511);
	c(auto_qmail_home, "bin", "qhpsi", auto_uidc, auto_gidq, 06511);
	c(auto_qmail_home, "bin", "qscanq", auto_uidc, auto_gidc, 04511);
	c(auto_qmail_home, "bin", "run-cleanq", auto_uido, auto_gidc, 02511);

	/* Man Pages, Documents */
	c(auto_qmail_home, "doc", "QMAILFAQ", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "doc", "UPGRADE", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "doc", "SENDMAIL", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "doc", "INSTALL.qmail", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "doc", "INSTALL.alias", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "doc", "INSTALL.ctl", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "doc", "INSTALL.ids", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "doc", "INSTALL.maildir", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "doc", "INSTALL.mbox", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "doc", "INSTALL.vsm", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "doc", "TEST.deliver", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "doc", "TEST.receive", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "doc", "REMOVE.sendmail", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "doc", "REMOVE.binmail", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "doc", "PIC.local2alias", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "doc", "PIC.local2ext", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "doc", "PIC.local2local", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "doc", "PIC.local2rem", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "doc", "PIC.local2virt", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "doc", "PIC.nullclient", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "doc", "PIC.relaybad", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "doc", "PIC.relaygood", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "doc", "PIC.rem2local", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "doc", "FROMISP", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "doc", "TOISP", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "doc", "AUTOTURN", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "doc", "EXTTODO", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "doc", "INTERNALS", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "doc", "README.qmail", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "doc", "README.auth", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "doc", "README.clamav", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "doc", "README.greylist", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "doc", "README.logselect", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "doc", "README.moreipme", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "doc", "README.newline", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "doc", "README.qhpsi", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "doc", "README.qregex", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "doc", "README.queue-fix", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "doc", "README.recipients", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "doc", "README.remote-auth", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "doc", "README.spamcontrol", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "doc", "README.starttls", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "doc", "README.status", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "doc", "README.tls", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "doc", "README.wildmat", auto_uido, auto_gidq, 0444);

	c(auto_qmail_home, "man/man1", "argv0.1", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man1", "addcr.1", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man1", "delcr.1", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man1", "fixcrio.1", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man1", "recordio.1", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man1", "fastforward.1", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/cat1", "fastforward.0", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man1", "printforward.1", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/cat1", "printforward.0", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man1", "setforward.1", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/cat1", "setforward.0", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man1", "newaliases.1", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/cat1", "newaliases.0", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man1", "printmaillist.1", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/cat1", "printmaillist.0", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man1", "setmaillist.1", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/cat1", "setmaillist.0", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man1", "newinclude.1", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/cat1", "newinclude.0", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man1", "dot-forward.1", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/cat1", "dot-forward.0", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man7", "forgeries.7", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/cat7", "forgeries.0", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man7", "qmail-limits.7", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/cat7", "qmail-limits.0", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man7", "qmail.7", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/cat7", "qmail.0", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man1", "forward.1", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/cat1", "forward.0", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man1", "rrforward.1", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/cat1", "rrforward.0", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man1", "condredirect.1", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/cat1", "condredirect.0", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man1", "bouncesaying.1", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/cat1", "bouncesaying.0", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man1", "except.1", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/cat1", "except.0", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man1", "qmaildirmake.1", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/cat1", "qmaildirmake.0", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man1", "maildir2mbox.1", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/cat1", "maildir2mbox.0", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man1", "maildirwatch.1", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/cat1", "maildirwatch.0", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man1", "mailsubj.1", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/cat1", "mailsubj.0", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man1", "qreceipt.1", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/cat1", "qreceipt.0", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man1", "preline.1", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/cat1", "preline.0", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man1", "tcp-env.1", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/cat1", "tcp-env.0", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man1", "maildirdeliver.1", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/cat1", "maildirdeliver.0", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man1", "iftocc.1", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/cat1", "iftocc.0", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man1", "new-inject.1", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/cat1", "new-inject.0", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man1", "822field.1", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/cat1", "822field.0", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man1", "822header.1", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/cat1", "822header.0", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man1", "822date.1", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/cat1", "822date.0", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man1", "822received.1", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/cat1", "822received.0", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man1", "822print.1", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/cat1", "822print.0", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man1", "qmail-autoresponder.1", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/cat1", "qmail-autoresponder.0", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man1", "serialcmd.1", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man1", "serialcmd.0", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man1", "serialqmtp.1", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/cat1", "serialqmtp.0", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man1", "serialsmtp.1", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/cat1", "serialsmtp.0", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man1", "maildircmd.1", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/cat1", "maildircmd.0", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man1", "maildirqmtp.1", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/cat1", "maildirqmtp.0", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man1", "maildirsmtp.1", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/cat1", "maildirsmtp.0", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man1", "maildirserial.1", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/cat1", "maildirserial.0", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man1", "matchup.1", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/cat1", "matchup.0", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man1", "mlmatchup.1", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/cat1", "mlmatchup.0", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man1", "xqp.1", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/cat1", "xqp.0", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man1", "xsender.1", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/cat1", "xsender.0", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man1", "xrecipient.1", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/cat1", "xrecipient.0", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man1", "columnt.1", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/cat1", "columnt.0", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/cat1", "822body.0", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man1", "822body.1", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/cat1", "822headerok.0", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man1", "822headerok.1", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/cat1", "822bodyfilter.0", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man1", "822bodyfilter.1", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/cat1", "822headerfilter.0", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man1", "822headerfilter.1", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/cat1", "822addr.0", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man1", "822addr.1", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/cat1", "822fields.0", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man1", "822fields.1", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/cat1", "checkaddr.0", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man1", "checkaddr.1", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/cat1", "checkdomain.0", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man1", "checkdomain.1", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/cat1", "filterto.0", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man1", "filterto.1", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/cat1", "condtomaildir.0", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man1", "condtomaildir.1", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/cat1", "iftoccfrom.0", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man1", "iftoccfrom.1", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/cat1", "ifaddr.0", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man1", "ifaddr.1", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/cat1", "replier.0", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man1", "replier.1", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/cat1", "replier-config.0", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man1", "replier-config.1", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man1", "spipe.1", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man1", "qfilelog.1", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man1", "multipipe.1", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man1", "teepipe.1", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man1", "multitail.1", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man1", "tai2tai64n.1", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man1", "tai64n2tai.1", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man1", "qlogselect.1", auto_uido, auto_gidq, 0444);

	c(auto_qmail_home, "man/man3", "alloc.3", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man3", "case.3", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man3", "cdb.3", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man3", "coe.3", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man3", "datetime.3", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man3", "direntry.3", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man3", "env.3", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man3", "error.3", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man3", "error_str.3", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man3", "error_temp.3", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man3", "fd_copy.3", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man3", "fd_move.3", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man3", "fifo_make.3", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man3", "getln.3", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man3", "getln2.3", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man3", "now.3", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man3", "sgetopt.3", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man3", "stralloc.3", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man3", "byte.3", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man3", "substdio.3", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man3", "substdio_in.3", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man3", "substdio_out.3", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man3", "substdio_copy.3", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man3", "subgetopt.3", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man3", "wait.3", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man3", "caldate.3", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man3", "caldate_mjd.3", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man3", "caltime.3", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man3", "caltime_tai.3", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man3", "config.3", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man3", "leapsecs.3", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man3", "tai.3", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man3", "tai_now.3", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man3", "tai_pack.3", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man3", "rewritehost.3", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man3", "mess822.3", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man3", "mess822_addr.3", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man3", "mess822_date.3", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man3", "mess822_fold.3", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man3", "mess822_quote.3", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man3", "mess822_token.3", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man3", "mess822_when.3", auto_uido, auto_gidq, 0444);

	c(auto_qmail_home, "man/man5", "addresses.5", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/cat5", "addresses.0", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man5", "envelopes.5", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/cat5", "envelopes.0", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man5", "maildir.5", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/cat5", "maildir.0", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man5", "mbox.5", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/cat5", "mbox.0", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man5", "dot-qmail.5", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/cat5", "dot-qmail.0", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man5", "qmail-control.5", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/cat5", "qmail-control.0", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man5", "qmail-header.5", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/cat5", "qmail-header.0", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man5", "qmail-log.5", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/cat5", "qmail-log.0", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man5", "qmail-users.5", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/cat5", "qmail-users.0", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man5", "tcp-environ.5", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/cat5", "tcp-environ.0", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man5", "rewriting.5", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/cat5", "rewriting.0", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man5", "rfc-822.5", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man5", "rfc-1845.5", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man5", "rfc-2821.5", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man5", "rfc-1893.5", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man5", "rfc-1894.5", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man5", "rfc-1985.5", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man5", "rfc-1321.5", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man5", "rfc-2104.5", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man5", "rfc-2645.5", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man5", "rfc-2554.5", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man5", "rfc-2505.5", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man5", "rfc-2635.5", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man5", "rfc-2476.5", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man5", "rfc-3834.5", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man5", "rfc-3798.5", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man5", "rfc-4871.5", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man5", "rfc-4870.5", auto_uido, auto_gidq, 0444);

	c(auto_qmail_home, "man/man8", "qmail-rm.8", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/cat8", "qmail-rm.0", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man8", "ofmipd.8", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/cat8", "ofmipd.0", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man8", "ofmipname.8", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/cat8", "ofmipname.0", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man8", "qmail-local.8", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/cat8", "qmail-local.0", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man8", "qmail-lspawn.8", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/cat8", "qmail-lspawn.0", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man8", "qmail-getpw.8", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/cat8", "qmail-getpw.0", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man8", "qmail-remote.8", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/cat8", "qmail-remote.0", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man8", "qmail-rspawn.8", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/cat8", "qmail-rspawn.0", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man8", "qmail-clean.8", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/cat8", "qmail-clean.0", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man8", "qmail-send.8", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/cat8", "qmail-send.0", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man8", "qmail-todo.8", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/cat8", "qmail-todo.0", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man8", "qmail-daemon.8", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man8", "qmail-start.8", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/cat8", "qmail-start.0", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man8", "splogger.8", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/cat8", "splogger.0", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man8", "qmail-internals.8", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man8", "qmail-queue.8", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/cat8", "qmail-queue.0", auto_uido, auto_gidq, 0444);
#ifdef HASDKIM
	c(auto_qmail_home, "man/man8", "qmail-dkim.8", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man8", "dk-filter.8", auto_uido, auto_gidq, 0444);
#endif
#ifdef DOMAIN_KEYS
	c(auto_qmail_home, "man/man8", "qmail-dk.8", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/cat8", "qmail-dk.0", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man8", "dknewkey.8", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man8", "dktest.8", auto_uido, auto_gidq, 0444);
#endif
	c(auto_qmail_home, "man/man8", "qmail-multi.8", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/cat8", "qmail-multi.0", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man1", "qmail-qfilter.1", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/cat1", "qmail-qfilter.0", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man8", "spawn-filter.8", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/cat8", "spawn-filter.0", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man8", "qmail-inject.8", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/cat8", "qmail-inject.0", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man8", "qmail-showctl.8", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/cat8", "qmail-showctl.0", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man8", "qmail-newmrh.8", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/cat8", "qmail-newmrh.0", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man8", "qmail-recipients.8", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/cat8", "qmail-recipients.0", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man8", "qmail-cdb.8", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/cat8", "qmail-cdb.0", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man8", "qmail-newu.8", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/cat8", "qmail-newu.0", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man8", "qmail-pw2u.8", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/cat8", "qmail-pw2u.0", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man8", "qmail-qread.8", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/cat8", "qmail-qread.0", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man8", "qmail-qstat.8", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/cat8", "qmail-qstat.0", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man8", "qmail-tcpok.8", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/cat8", "qmail-tcpok.0", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man8", "qmail-tcpto.8", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/cat8", "qmail-tcpto.0", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man8", "qmail-pop3d.8", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/cat8", "qmail-pop3d.0", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man8", "qmail-popup.8", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/cat8", "qmail-popup.0", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man8", "qmail-qmqpc.8", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/cat8", "qmail-qmqpc.0", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man8", "qmail-qmqpd.8", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/cat8", "qmail-qmqpd.0", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man8", "qmail-qmtpd.8", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/cat8", "qmail-qmtpd.0", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man8", "qmail-smtpd.8", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/cat8", "qmail-smtpd.0", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man8", "qmail-greyd.8", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/cat8", "qmail-greyd.0", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man8", "greydaemon.8", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/cat8", "greydaemon.0", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man8", "qmail-poppass.8", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/cat8", "qmail-poppass.0", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man8", "qmail-command.8", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/cat8", "qmail-command.0", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man8", "envdir.8", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man8", "envuidgid.8", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man8", "fghack.8", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man8", "multilog.8", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man8", "setlock.8", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man8", "setuidgid.8", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man8", "softlimit.8", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man8", "supervise.8", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man8", "svc.8", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man8", "svok.8", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man8", "svscan.8", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man8", "svstat.8", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man8", "tai64n.8", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man8", "tai64nlocal.8", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man8", "tai64nunix.8", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man8", "pgrphack.8", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man8", "readproctitle.8", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man8", "svscanboot.8", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man8", "qscanq.8", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/cat8", "qscanq.0", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man8", "qscanq-stdin.8", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/cat8", "qscanq-stdin.0", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man8", "cleanq.8", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/cat8", "cleanq.0", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/man8", "run-cleanq.8", auto_uido, auto_gidq, 0444);
	c(auto_qmail_home, "man/cat8", "run-cleanq.0", auto_uido, auto_gidq, 0444);
}

void
getversion_install_big_c()
{
	static char    *x = "$Id: hier.c,v 1.127 2009-10-19 10:13:04+05:30 Cprogrammer Stab mbhangui $";

#ifdef INDIMAIL
	x = sccsidh;
#else
	x++;
#endif
}
