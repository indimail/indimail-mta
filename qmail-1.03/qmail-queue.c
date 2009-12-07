/*
 * $Log: qmail-queue.c,v $
 * Revision 1.47  2009-12-05 20:07:08+05:30  Cprogrammer
 * added prototype for MakeArgs
 *
 * Revision 1.46  2009-12-05 20:01:24+05:30  Cprogrammer
 * ansic conversion
 *
 * Revision 1.45  2009-12-05 11:23:47+05:30  Cprogrammer
 * added X-Originating-IP header
 *
 * Revision 1.44  2009-11-13 23:04:38+05:30  Cprogrammer
 * report QHPSI error using return value 77
 *
 * Revision 1.43  2009-09-08 13:32:57+05:30  Cprogrammer
 * compile QHPSI without INDIMAIL
 *
 * Revision 1.42  2009-08-13 20:22:49+05:30  Cprogrammer
 * BUG - fixed missing die_write() statement
 *
 * Revision 1.41  2009-08-13 19:08:21+05:30  Cprogrammer
 * use sockaddr_storage to get IPV6 address using getpeername()
 * logheaders code completed
 *
 * Revision 1.40  2008-07-14 20:57:16+05:30  Cprogrammer
 * fixed compilation warning
 *
 * Revision 1.39  2007-12-20 12:49:33+05:30  Cprogrammer
 * added ability to log headers
 *
 * Revision 1.38  2006-01-22 11:11:46+05:30  Cprogrammer
 * added control file envheaders to pass headers through queue
 * improved line processing for headers by using mess822_ok()
 *
 * Revision 1.37  2005-08-23 17:35:12+05:30  Cprogrammer
 * gcc 4 compliance
 *
 * Revision 1.36  2005-05-16 16:34:12+05:30  Cprogrammer
 * added missing cleanup
 *
 * Revision 1.35  2005-04-25 22:52:00+05:30  Cprogrammer
 * added plugin feature
 *
 * Revision 1.34  2005-04-05 22:45:41+05:30  Cprogrammer
 * new control file removeheaders to eliminate headers from message input stream
 *
 * Revision 1.33  2005-03-28 10:00:56+05:30  Cprogrammer
 * made bounce if virus mails default
 * added back case 0, for accepting infected mails
 *
 * Revision 1.32  2005-03-28 09:38:52+05:30  Cprogrammer
 * removed qqeh argument to qhpsiprog()
 * made default case to blackhole
 *
 * Revision 1.31  2005-03-25 00:18:51+05:30  Cprogrammer
 * added Quarantine feature
 * Added BSD style syncdir semantics
 * added X-Quarantine-ID, X-QHPSI headers
 * Added missing calls to cleanup()
 *
 * Revision 1.30  2005-02-18 17:57:51+05:30  Cprogrammer
 * Facility to specify name of scanned file by %s in SCANCMD env variable
 *
 * Revision 1.29  2004-12-20 22:56:54+05:30  Cprogrammer
 * corrected compilation problem with non-indimail environment
 *
 * Revision 1.28  2004-10-22 20:29:26+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.27  2004-10-22 15:37:45+05:30  Cprogrammer
 * removed readwrite.h
 *
 * Revision 1.26  2004-10-21 11:30:01+05:30  Cprogrammer
 * removed domainkeys code
 *
 * Revision 1.25  2004-09-29 23:47:50+05:30  Cprogrammer
 * added Qmail High Performance Scanner Interface
 *
 * Revision 1.24  2004-09-21 23:46:46+05:30  Cprogrammer
 * change exit code to "qq unable to read configuration (#4.3.0)
 *
 * Revision 1.23  2004-08-28 01:04:23+05:30  Cprogrammer
 * reformatted part of code
 *
 * Revision 1.22  2004-08-15 12:54:11+05:30  Cprogrammer
 * qqeh bug fix
 *
 * Revision 1.21  2004-08-13 00:15:22+05:30  Cprogrammer
 * domain keys implementation
 *
 * Revision 1.20  2004-07-27 22:58:52+05:30  Cprogrammer
 * fixed bug with qqeh
 *
 * Revision 1.19  2004-07-17 21:21:18+05:30  Cprogrammer
 * added qqeh code
 * added RCS log
 *
 * Revision 1.18  2004-01-14 23:40:26+05:30  Cprogrammer
 * delay fsync() - idea from mailing list
 *
 * Revision 1.17  2003-12-31 20:03:12+05:30  Cprogrammer
 * added use_fsync to turn on/off fsync
 *
 * Revision 1.16  2003-10-23 01:25:19+05:30  Cprogrammer
 * fixed compilation warnings
 *
 * Revision 1.15  2003-10-21 16:41:23+05:30  Cprogrammer
 * added comments
 *
 * Revision 1.14  2003-10-16 01:20:59+05:30  Cprogrammer
 * removed unused FILE pointer
 *
 * Revision 1.13  2003-10-11 00:11:33+05:30  Cprogrammer
 * comment removed
 *
 * Revision 1.12  2003-10-01 11:20:24+05:30  Cprogrammer
 * added environment variable EXTRAQUEUE for extra recipients to be
 * added before queueing mail
 *
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include "sig.h"
#include "case.h"
#include "str.h"
#include "getln.h"
#include "exit.h"
#include "open.h"
#include "seek.h"
#include "fmt.h"
#include "alloc.h"
#include "substdio.h"
#include "datetime.h"
#include "now.h"
#include "triggerpull.h"
#include "auto_qmail.h"
#include "auto_uids.h"
#include "date822fmt.h"
#include "fmtqfn.h"
#include "stralloc.h"
#include "control.h"
#include "env.h"
#include "variables.h"
#include "error.h"
#include "scan.h"
#include "mess822.h"
#include "wait.h"
#include "hasindimail.h"

#define DEATH 86400	/*- 24 hours; _must_ be below q-s's OSSIFIED (36 hours) */
#define ADDR  1003

int             uidinit();
char          **MakeArgs(char *);

char            inbuf[2048], outbuf[256], logbuf[2048];
char           *pidfn, *messfn, *todofn, *intdfn;
int             messfd, intdfd, match;
int             flagmademess = 0;
int             flagmadeintd = 0;
int             flagquarantine = 0;
int             flagblackhole = 0;
struct substdio ssin, ssout, sslog;
datetime_sec    starttime;
struct datetime dt;
struct stat     pidst;
unsigned long   mypid, uid;
unsigned long   messnum;
stralloc        extraqueue = { 0 };
stralloc        quarantine = { 0 };
stralloc        qqehextra = { 0 };
stralloc        line = { 0 };
stralloc        excl = { 0 };
stralloc        incl = { 0 };
stralloc        logh = { 0 };
stralloc        envheaders = { 0 };

void
cleanup()
{
	if (flagmadeintd)
	{
		seek_trunc(intdfd, 0);
		unlink(intdfn);
	}
	if (flagmademess)
	{
		seek_trunc(messfd, 0);
		unlink(messfn);
	}
}

void
die(e)
	int             e;
{
	_exit(e);
}

void
die_write()
{
	cleanup();
	die(53);
}

void
die_read()
{
	cleanup();
	die(54);
}

void
sigalrm()
{
	/*- thou shalt not clean up here */
	die(52);
}

void
sigbug()
{
	die(81);
}

#ifndef INET_ADDRSTRLEN
#define INET_ADDRSTRLEN 16
#endif

char           *tcpremoteip;
void
tcpgetremoteip()
{
#ifdef IPV6
	static char     addrBuf[INET6_ADDRSTRLEN];
	struct sockaddr_storage sa;
#else
	struct sockaddr sa;
	struct sockaddr_in *sin = (struct sockaddr_in *) (&sa);
#endif
	int             i, status, dummy = sizeof(sa);

	memset(&sa, 0, sizeof(struct sockaddr));
	for (tcpremoteip = (char *) 0, errno = i = 0; i < 3; i++)
	{
		errno = 0;
		if (!(status = getpeername(i, (struct sockaddr *) &sa, (socklen_t *) &dummy)))
		{
#ifdef IPV6
			if (((struct sockaddr *) &sa)->sa_family == AF_INET)
			{
				tcpremoteip = (char *) inet_ntop(AF_INET,
					(void *) &((struct sockaddr_in *) &sa)->sin_addr, addrBuf,
					INET_ADDRSTRLEN);
			} else
			if (((struct sockaddr *)&sa)->sa_family == AF_INET6)
			{
				tcpremoteip = (char *) inet_ntop(AF_INET6,
					(void *) &((struct sockaddr_in6 *) &sa)->sin6_addr, addrBuf,
					INET6_ADDRSTRLEN);
			} else
			if (((struct sockaddr *) &sa)->sa_family == AF_UNSPEC)
				tcpremoteip = "::1";
#else
			if (((struct sockaddr *) sin)->sa_family == AF_INET)
				tcpremoteip = (char *) inet_ntoa(sin->sin_addr);
			else
			if (((struct sockaddr *) sin)->sa_family == AF_UNSPEC)
				tcpremoteip = "127.0.0.1";
#endif
			else
				tcpremoteip = "?.?.?.?";
			break;
		}
		if (errno == error_ebadf)
			break;
	}
	return;
}

unsigned int    receivedlen;
unsigned int    originlen;
char           *received;
/* "Received: (qmail-queue invoked by alias); 26 Sep 1995 04:46:54 -0000\n" */
char           *origin; /* "X-Originating-IP: 10.0.0.1\n" */

static unsigned int
receivedfmt(char *s)
{
	unsigned int    i;
	unsigned int    len;
	len = 0;
#ifdef INDIMAIL
	i = fmt_str(s, "Received: (indimail ");
#else
	i = fmt_str(s, "Received: (qmail ");
#endif
	len += i;
	if (s)
		s += i;
	i = fmt_ulong(s, mypid);
	len += i;
	if (s)
		s += i;
	i = fmt_str(s, " invoked");
	len += i;
	if (s)
		s += i;
	if (uid == auto_uida)
	{
		i = fmt_str(s, " by alias");
		len += i;
		if (s)
			s += i;
	} else
	if (uid == auto_uidd)
	{
		i = fmt_str(s, " from network");
		len += i;
		if (s)
			s += i;
	} else
	if (uid == auto_uids)
	{
		i = fmt_str(s, " for bounce");
		len += i;
		if (s)
			s += i;
	} else
	{
		if (!tcpremoteip)
			tcpremoteip = env_get("TCPREMOTEIP");
		if (!tcpremoteip)
			tcpgetremoteip();
		if (tcpremoteip)
		{
			char            Hostname[128];

			i = fmt_str(s, " from ");
			len += i;
			if (s)
				s += i;
			i = fmt_str(s, tcpremoteip);
			len += i;
			if (s)
				s += i;
			i = fmt_str(s, " by host ");
			len += i;
			if (s)
				s += i;
			if (gethostname(Hostname, sizeof(Hostname)) == -1)
				str_copy(Hostname, "unknown");
			i = fmt_str(s, Hostname);
			len += i;
			if (s)
				s += i;
		}
		i = fmt_str(s, " by uid ");
		len += i;
		if (s)
			s += i;
		i = fmt_ulong(s, uid);
		len += i;
		if (s)
			s += i;
	}
	i = fmt_str(s, "); ");
	len += i;
	if (s)
		s += i;
	i = date822fmt(s, &dt);
	len += i;
	if (s)
		s += i;
	return len;
}

void
received_setup()
{
	receivedlen = receivedfmt((char *) 0);
	if (!(received = alloc(receivedlen + 1)))
		die(51);
	receivedfmt(received);
}

unsigned int
originfmt(char *s)
{
	unsigned int    i;
	unsigned int    len;

	len = 0;
	i = fmt_str(s, "X-Originating-IP: ");
	len += i;
	if (s)
		s += i;
	i = fmt_str(s, tcpremoteip);
	len += i;
	if (s)
		s += i;
	i = fmt_str(s, "\n");
	len += i;
	if (s)
		s += i;
	return len;
}

void
origin_setup()
{
	originlen = originfmt((char *) 0);
	if (!(origin = alloc(originlen + 1)))
		die(51);
	originfmt(origin);
}

unsigned int
pidfmt(char *s, unsigned long seq)
{
	unsigned int    i;
	unsigned int    len;

	len = 0;
	i = fmt_str(s, "pid/");
	len += i;
	if (s)
		s += i;
	i = fmt_ulong(s, mypid);
	len += i;
	if (s)
		s += i;
	i = fmt_str(s, ".");
	len += i;
	if (s)
		s += i;
	i = fmt_ulong(s, starttime);
	len += i;
	if (s)
		s += i;
	i = fmt_str(s, ".");
	len += i;
	if (s)
		s += i;
	i = fmt_ulong(s, seq);
	len += i;
	if (s)
		s += i;
	++len;
	if (s)
		*s++ = 0;

	return len;
}

char           *
fnnum(char *dirslash, int flagsplit)
{
	char           *s;

	if (!(s = alloc(fmtqfn((char *) 0, dirslash, messnum, flagsplit))))
		die(51);
	fmtqfn(s, dirslash, messnum, flagsplit);
	return s;
}

void
pidopen()
{
	unsigned int    len;
	unsigned long   seq;

	seq = 1;
	len = pidfmt((char *) 0, seq);
	if (!(pidfn = alloc(len)))
		die(51);
	for (seq = 1; seq < 10; ++seq)
	{
		if (pidfmt((char *) 0, seq) > len)
			die(81);			/*- paranoia */
		pidfmt(pidfn, seq);
		if ((messfd = open_excl(pidfn)) != -1)
			return;
	}
	die(63);
}

#if defined(QHPSI)
char           *qhpsi;

/*-
 * Following added additional to QHPSI
 * X-QHPSI:
 * VIRUSFORWARD:
 * X-Quarantine-ID:
 */
void
qhpsiprog(char *program)
{
	int             wstat, child, rejectvirus = 0;
	char          **argv;
	char           *scancmd[3] = { 0, 0, 0 };
	char           *x;
	unsigned long   u;
	int             childrc = -1;
	int             qhpsirc = 1, qhpsirn = 0;
	unsigned int    size;
	unsigned int    qhpsiminsize = 0;
	unsigned int    qhpsimaxsize = 0;
	stralloc        plugin = { 0 };
	struct stat     st;

	if (stat(messfn, &st) == -1)
	{
		cleanup();
		die(62);
	}
	size = (unsigned int) st.st_size;
	if ((x = env_get("QHPSIMINSIZE")))
	{
		scan_ulong(x, &u);
		qhpsiminsize = (int) u;
	}
	if (qhpsiminsize && size < qhpsiminsize)
		return;
	if ((x = env_get("QHPSIMAXSIZE")))
	{
		scan_ulong(x, &u);
		qhpsimaxsize = (int) u;
	}
	if (qhpsimaxsize && size > qhpsimaxsize)
		return;
	switch (child = fork())
	{
	case -1:
		cleanup();
		die(121);
	case 0:
		/* 
		 * execute the qhpsi executable for security reasons.
		 * revoke all privileges and run with indimail uid
		 */
		if (setregid(auto_gidq, auto_gidq) || setreuid(auto_uidq, auto_uidq))
			_exit(50);
		if (!str_diffn(program, "plugin:", 7))
		{
			if (!stralloc_copys(&plugin, "plugin:"))
				die(51);
			if (!stralloc_cats(&plugin, messfn))
				die(51);
			if (!stralloc_append(&plugin, " "))
				die(51);
			if (!stralloc_cats(&plugin, program + 7))
				die(51);
			if (!stralloc_0(&plugin))
				die(51);
			if (!(argv = MakeArgs(plugin.s)))
				die(51);
		} else
		{
			if (!(argv = MakeArgs(program)))
				die(51);
			if (!argv[1])
			{
				scancmd[0] = argv[0];
				scancmd[1] = messfn;
				argv = scancmd;
			} else
			for (u = 1; argv[u]; u++)
			{
				if (!str_diffn(argv[u], "%s", 2))
					argv[u] = messfn;
			}
		}
		if (!stralloc_copys(&line, auto_qmail))
			die(51);
		if (!stralloc_cats(&line, "/bin/qhpsi"))
			die(51);
		if (!stralloc_0(&line))
			die(51);
		execv(line.s, argv);
		_exit(75);
	} /*- switch (child = fork()) */
	if (wait_pid(&wstat, child) == -1)
	{
		cleanup();
		die(122);
	}
	if (wait_crashed(wstat))
	{
		cleanup();
		die(123);
	}
	childrc = wait_exitcode(wstat);
	if ((x = env_get("REJECTVIRUS")))
	{
		scan_ulong(x, &u);
		rejectvirus = (int) u;
	}
	if ((x = env_get("QHPSIRC")))
	{
		scan_ulong(x, &u);
		qhpsirc = (int) u;
	}
	if ((x = env_get("QHPSIRN")))
	{
		scan_ulong(x, &u);
		qhpsirn = (int) u;
	}
	if (childrc == qhpsirc) /*- Virus Infected */
	{
		if (!stralloc_copys(&qqehextra, "X-QHPSI: Virus found\n"))
		{
			cleanup();
			die(51);
		}
		if ((x = env_get("VIRUSFORWARD")))
		{
			if (!stralloc_copys(&quarantine, x))
			{
				cleanup();
				die(51);
			}
		}
		switch (rejectvirus)
		{
			case 0: /*- Accept infected mails */
				break;
			/*-
			 * Should the default be after case 2 ?
			 */
			default:/*- Bounce */
				cleanup();
				die(33); /*- message contains a virus */
				break;
			case 2: /*- Blackhole */
				flagblackhole = 1;
				break;
		}
	} else
	if (childrc == qhpsirn) /*- Clean Mail */
	{
		if (!stralloc_copys(&qqehextra, "X-QHPSI: clean\n"))
		{
			cleanup();
			die(51);
		}
	} else
	{
		cleanup();
		die(77); /*- QHPSI error in qmail.c */
	}
	return;
}
#endif

int
main()
{
	int             token_len, exclude = 0, include = 0, loghead = 0, line_brk_excl = 1, line_brk_incl = 1,
					in_header = 1, line_brk_log = 1, logfd = -1;
#ifdef USE_FSYNC
	int             fd;
#endif
	unsigned int    len, x, originipfield = 0;
	char            tmp[FMT_ULONG];
	char            ch;
	char           *ptr, *qqeh, *tmp_ptr;
	struct stat     st;

	sig_blocknone();
	umask(033);
	if (uidinit() == -1)
		die(67);
	if (chdir(auto_qmail) == -1)
		die(61);
	if (control_readint((int *) &originipfield, "originipfield") == -1)
		die(55);
	if ((ptr = env_get("ORIGINIPFIELD")))
	{
		scan_ulong(ptr, (unsigned long *) &x);
		originipfield = x;
	}
	if (!(ptr = env_get("EXTRAQUEUE")))
	{
		if (control_readline(&extraqueue, "extraqueue") == -1)
			die(55);
	} else
	if (!stralloc_copys(&extraqueue, ptr))
		die(51);
	if (!(ptr = env_get("QUARANTINE")))
	{
		if (control_readline(&quarantine, "quarantine") == -1)
			die(55);
	} else
	if (!stralloc_copys(&quarantine, ptr))
		die(51);
	if (control_readfile(&excl, (ptr = env_get("REMOVEHEADERS")) && *ptr ? ptr : "removeheaders", 0) == -1)
		die(55);
	if (control_readfile(&incl, (ptr = env_get("ENVHEADERS")) && *ptr ? ptr : "envheaders", 0) == -1)
		die(55);
	if (control_readfile(&logh, (ptr = env_get("LOGHEADERS")) && *ptr ? ptr : "logheaders", 0) == -1)
		die(55);
	if (!(queuedir = env_get("QUEUEDIR")))
#ifdef INDIMAIL
		queuedir = "queue1";
#else
		queuedir = "queue";
#endif
	if (chdir(queuedir) == -1)
		die(62);
#ifdef USE_FSYNC
	if (env_get("USE_FSYNC"))
		use_fsync = 1;
#endif
	qqeh = env_get("QQEH");
	mypid = getpid();
	uid = getuid();
	starttime = now();
	datetime_tai(&dt, starttime);
	received_setup();
	sig_pipeignore();
	sig_miscignore();
	sig_alarmcatch(sigalrm);
	sig_bugcatch(sigbug);
	alarm(DEATH);
	pidopen();
	if (fstat(messfd, &pidst) == -1)
		die(63);
	messnum = pidst.st_ino;
	messfn = fnnum("mess/", 1);
	todofn = fnnum("todo/", 1);
	intdfn = fnnum("intd/", 1);
	if (link(pidfn, messfn) == -1)
	{
		unlink(pidfn);
		die(64);
	}
	if (unlink(pidfn) == -1)
	{
		unlink(messfn);
		die(63);
	}
	flagmademess = 1;
	substdio_fdbuf(&ssout, write, messfd, outbuf, sizeof(outbuf));
	/*- read the message headers */
	substdio_fdbuf(&ssin, read, 0, inbuf, sizeof(inbuf));
	if (logh.len)
	{
		if ((ptr = env_get("LOGHEADERFD")))
		{
			scan_int(ptr, &logfd);
			if (!fstat(logfd, &st))
				substdio_fdbuf(&sslog, write, logfd, logbuf, sizeof(logbuf));
			else
				logfd = -1;
		}
	}
	if (substdio_bput(&ssout, received, receivedlen) == -1)
		die_write();
	if (originipfield && (tcpremoteip = env_get("TCPREMOTEIP"))
		&& (env_get("RELAYCLIENT") || env_get("TCPREMOTEINFO")))
	{
		origin_setup();
		if (substdio_bput(&ssout, origin, originlen) == -1)
			die_write();
	}

	if (!excl.len && !incl.len && !logh.len)
	{
		switch (substdio_copy(&ssout, &ssin))
		{
		case -2:
			die_read();
		case -3:
			die_write();
		}
	} else
	for (;;) /*- Line Processing */
	{
		if (getln(&ssin, &line, &match, '\n') == -1)
			die_read();
		if (!match && line.len == 0)
			break;
		if (in_header && !mess822_ok(&line))
			in_header = 0;
		if (!in_header)
		{
			if (substdio_put(&ssout, line.s, line.len))
				die_write();
			continue;
		}
		exclude = 0;
		include = 0;
		if (logfd > 0)
			loghead = 0;
		if (line.s[0] == ' ' || line.s[0] == '\t') /*- RFC 822 LWSP char */
		{
			if (!line_brk_excl) /*- Not a continuation line */
				exclude = 1;
			if (!line_brk_incl)
				include = 1;
			if (logfd > 0 && !line_brk_log)
				loghead = 1;
		} else
		{
			line_brk_excl = 1;
			line_brk_incl = 1;
			if (logfd > 0)
				line_brk_log = 1;
			for (len = 0, ptr = excl.s;len < excl.len;)
			{
				len += ((token_len = str_len(ptr)) + 1);
				if (!case_diffb(ptr, token_len, line.s))
				{
					exclude = 1;
					line_brk_excl = 0; /*- Skip next line starting with LWSP */
					break;
				}
				ptr = excl.s + len;
			}
			for (len = 0, ptr = incl.s;len < incl.len;)
			{
				for (tmp_ptr = ptr;*tmp_ptr && *tmp_ptr != ':';tmp_ptr++);
				if (*tmp_ptr == ':')
				{
					*tmp_ptr = 0;
					scan_uint(tmp_ptr + 1, &x);
				}
				len += ((token_len = str_len(ptr)) + 1);
				if (token_len <= x && !case_diffb(ptr, token_len, line.s))
				{
					include = 1;
					line_brk_incl = 0; /*- Include next line starting with LWSP */
					break;
				}
				ptr = incl.s + len;
			}
			for (len = 0, ptr = logh.s;logfd > 0 && len < logh.len;)
			{
				len += ((token_len = str_len(ptr)) + 1);
				if (!case_diffb(ptr, token_len, line.s))
				{
					loghead = 1;
					line_brk_log = 0;
					break;
				}
				ptr = logh.s + len;
			}
		}
		if (include && !stralloc_catb(&envheaders, line.s, line.len))
			die(51);
		if (!exclude && substdio_put(&ssout, line.s, line.len))
			die_write();
		if (logfd > 0 && loghead && substdio_put(&sslog, line.s, line.len))
			die_write();
		if (!match)
			break;
	} /*- for (;;) - Line Processing */
	if (substdio_flush(&ssout) == -1)
		die_write();
	if (logfd > 0 && substdio_flush(&sslog) == -1)
		die_write();
#if defined(QHPSI)
	if ((qhpsi = env_get("QHPSI")))
		qhpsiprog(qhpsi);
#endif
	if (quarantine.s && quarantine.len)
		flagquarantine = 1;
	if (flagblackhole && flagquarantine)
		flagblackhole = 0;
	/*- write the envelope */
	if ((intdfd = open_excl(intdfn)) == -1)
	{
		cleanup();
		die(65);
	}
	flagmadeintd = 1;
	substdio_fdbuf(&ssout, write, intdfd, outbuf, sizeof(outbuf));
	/*- read the message body */
	substdio_fdbuf(&ssin, read, 1, inbuf, sizeof(inbuf));
	if (substdio_bput(&ssout, "u", 1) == -1)
		die_write();
	if (substdio_bput(&ssout, tmp, fmt_ulong(tmp, uid)) == -1)
		die_write();
	if (substdio_bput(&ssout, "", 1) == -1)
		die_write();
	if (substdio_bput(&ssout, "p", 1) == -1)
		die_write();
	if (substdio_bput(&ssout, tmp, fmt_ulong(tmp, mypid)) == -1)
		die_write();
	if (substdio_bput(&ssout, "", 1) == -1)
		die_write();
	if (substdio_get(&ssin, &ch, 1) < 1)
		die_read();
	/*- Get the Sender */
	if (ch != 'F')
	{
		cleanup();
		die(91);
	}
	if (substdio_bput(&ssout, &ch, 1) == -1)
		die_write();
	for (len = 0; len < ADDR; ++len)
	{
		if (substdio_get(&ssin, &ch, 1) < 1)
			die_read();
		if (substdio_put(&ssout, &ch, 1) == -1)
			die_write();
		if (!ch)
			break;
	}
	if (len >= ADDR)
	{
		cleanup();
		die(11);
	}
	if (extraqueue.s && extraqueue.len)
	{
		if (substdio_bput(&ssout, "T", 1) == -1)
			die_write();
		if (substdio_bput(&ssout, extraqueue.s, extraqueue.len) == -1)
			die_write();
		if (substdio_bput(&ssout, "\0", 1) == -1)
			die_write();
	}
	if (flagquarantine)
	{
		if (substdio_bput(&ssout, "T", 1) == -1)
			die_write();
		if (substdio_bput(&ssout, quarantine.s, quarantine.len) == -1)
			die_write();
		if (substdio_bput(&ssout, "\0", 1) == -1)
			die_write();
	}
	if (flagquarantine)
	{
		if (!stralloc_cats(&qqehextra, "X-Quarantine-ID: "))
		{
			cleanup();
			die(51);
		}
		if (!stralloc_cat(&qqehextra, &quarantine))
		{
			cleanup();
			die(51);
		}
	}
	if (flagblackhole)
	{
		for (;;) /*- empty the pipe created by qmail_open() */
		{
			if (substdio_get(&ssin, &ch, 1) < 1)
				die_read();
			if (!ch)
				break;
		}
		cleanup();
		die(0);
	} else /*- Write all recipients */
	for (;;)
	{
		if (substdio_get(&ssin, &ch, 1) < 1)
			die_read();
		if (!ch)
			break;
#if defined(QHPSI)
		if (ch == 'Q')
		{
			qhpsi = 0;
			break;
		}
#endif
		if (ch != 'T')
		{
			cleanup();
			die(91);
		}
		if (flagquarantine)
		{
			if (!stralloc_append(&qqehextra, ","))
			{
				cleanup();
				die(51);
			}
		} else
		if (substdio_bput(&ssout, &ch, 1) == -1)
			die_write();
		for (len = 0; len < ADDR; ++len)
		{
			if (substdio_get(&ssin, &ch, 1) < 1)
				die_read();
			if (flagquarantine)
			{
				if (ch && !stralloc_append(&qqehextra, &ch))
				{
					cleanup();
					die(51);
				}
			} else
			if (substdio_bput(&ssout, &ch, 1) == -1)
				die_write();
			if (!ch)
				break;
		}
		if (len >= ADDR)
		{
			cleanup();
			die(11);
		}
	} /*- for (;;) */
	if (flagquarantine && !stralloc_append(&qqehextra, "\n"))
	{
		cleanup();
		die(51);
	}
	if ((qqeh && *qqeh) || (qqehextra.s && qqehextra.len))
	{
		if (substdio_bput(&ssout, "e", 1) == -1)
			die_write();
	}
	if (qqehextra.s && qqehextra.len)
	{
		if (substdio_bput(&ssout, qqehextra.s, qqehextra.len) == -1)
			die_write();
	}
	if (qqeh && *qqeh)
	{
		if (substdio_bputs(&ssout, qqeh) == -1)
			die_write();
	}
	if ((qqeh && *qqeh) || (qqehextra.s && qqehextra.len))
	{
		if (substdio_bput(&ssout, "\0", 1) == -1)
			die_write();
	}
	if (envheaders.s && envheaders.len)
	{
		if (substdio_bput(&ssout, "h", 1) == -1)
			die_write();
		if (substdio_bput(&ssout, envheaders.s, envheaders.len) == -1)
			die_write();
		if (substdio_bput(&ssout, "\0", 1) == -1)
			die_write();
	}
	if (substdio_flush(&ssout) == -1)
		die_write();
#ifdef USE_FSYNC
	if (use_fsync)
	{
		if (fsync(messfd) == -1)
			die_write();
		if (fsync(intdfd) == -1)
			die_write();
	}
#endif
	if (link(intdfn, todofn) == -1)
	{
		cleanup();
		die(66);
	}
#ifdef USE_FSYNC
	if (!env_get("USE_SYNCDIR") && use_fsync)
	{
		if ((fd = open(todofn, O_RDONLY)) < 0 || fsync(fd) < 0 || close(fd) < 0)
		{
			cleanup();
			die(66);
		}
	}
#endif
	triggerpull();
	die(0);
	/*- Not reached */
	return (0);
}

void
getversion_qmail_queue_c()
{
	static char    *x = "$Id: qmail-queue.c,v 1.47 2009-12-05 20:07:08+05:30 Cprogrammer Stab mbhangui $";

#ifdef INDIMAIL
	x = sccsidh;
#else
	x++;
#endif
}
