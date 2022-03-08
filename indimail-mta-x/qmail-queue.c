/*
 * $Id: qmail-queue.c,v 1.81 2022-03-08 23:07:17+05:30 Cprogrammer Exp mbhangui $
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include "haslibrt.h"
#ifdef HASLIBRT
#include <mqueue.h>
#endif
#include <sig.h>
#include <case.h>
#include <byte.h>
#include <str.h>
#include <getln.h>
#include <open.h>
#include <seek.h>
#include <fmt.h>
#include <alloc.h>
#include <substdio.h>
#include <datetime.h>
#include <now.h>
#include <date822fmt.h>
#include <stralloc.h>
#include <env.h>
#include <error.h>
#include <scan.h>
#include <mess822.h>
#include <wait.h>
#include <makeargs.h>
#include <getEnvConfig.h>
#include <noreturn.h>
#include "control.h"
#include "variables.h"
#include "fmtqfn.h"
#include "triggerpull.h"
#include "auto_uids.h"
#include "matchregex.h"
#include "auto_split.h"
#include "auto_prefix.h"
#ifdef USE_FSYNC
#include "syncdir.h"
#endif
#ifdef HASLIBRT
#include "qscheduler.h"
#include "qmail.h"
#endif

#define DEATH 86400	/*- 24 hours; _must_ be below q-s's OSSIFIED (36 hours) */
#define ADDR  1003
#ifndef INET_ADDRSTRLEN
#define INET_ADDRSTRLEN 16
#endif

static char    *tcpremoteip;
static char    *received;
static char    *pidfn, *messfn, *todofn, *intdfn;
static char    *origin; /* "X-Originating-IP: 10.0.0.1\n" */
static char     inbuf[2048], outbuf[256], logbuf[2048];
static int      messfd, intdfd, match;
static int      flagmademess = 0;
static int      flagmadeintd = 0;
static int      flagquarantine = 0;
static int      flagblackhole = 0;
static int      flagarchive = 0;
static int      bigtodo = 0;
static struct substdio ssin, ssout, sslog;
static datetime_sec    starttime;
static struct datetime dt;
static struct stat     pidst;
static unsigned int    receivedlen;
static unsigned int    originlen;
static unsigned long   mypid, uid;
static unsigned long   messnum;
static stralloc extraqueue = { 0 };
static stralloc quarantine = { 0 };
static stralloc qqehextra = { 0 };
static stralloc line = { 0 };
static stralloc excl = { 0 };
static stralloc incl = { 0 };
static stralloc logh = { 0 };
#if defined(MAILARCHIVE)
static stralloc ar_rules = { 0 };
static stralloc arch_email = {0};
#endif
static stralloc envheaders = { 0 };

void
cleanup()
{
	if (flagmadeintd) {
		seek_trunc(intdfd, 0);
		unlink(intdfn);
	}
	if (flagmademess) {
		seek_trunc(messfd, 0);
		unlink(messfn);
	}
}

no_return void
die(int e)
{
	_exit(e);
}

no_return void
die_write()
{
	cleanup();
	die(53);
}

no_return void
die_read()
{
	cleanup();
	die(54);
}

no_return void
sigalrm()
{
	/*- thou shalt not clean up here */
	die(52);
}

no_return void
sigbug()
{
	die(81);
}

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

	byte_zero((char *) &sa, sizeof(struct sockaddr));
	for (tcpremoteip = (char *) 0, errno = i = 0; i < 3; i++) {
		errno = 0;
		if (!(status = getpeername(i, (struct sockaddr *) &sa, (socklen_t *) &dummy))) {
#ifdef IPV6
			if (((struct sockaddr *) &sa)->sa_family == AF_INET) {
				tcpremoteip = (char *) inet_ntop(AF_INET,
					(void *) &((struct sockaddr_in *) &sa)->sin_addr, addrBuf,
					INET_ADDRSTRLEN);
			} else
			if (((struct sockaddr *)&sa)->sa_family == AF_INET6) {
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

/*
 * Received headers created by qmail-queue
 * "Received: (indimail-mta queue 37166 by host q.r.s.t invoked by alias); 26 Sep 1995 04:46:54 -0000\n"
 * "Received: (indimail-mta queue 37166 by host q.r.s.t invoked from network w.x.y.z uid=123); 26 Sep 1995 04:46:54 -0000\n"
 * "Received: (indimail-mta queue 37166 by host q.r.s.t invoked for bounce); 26 Sep 1995 04:46:54 -0000\n"
 */

static unsigned int
receivedfmt(char *s)
{
	unsigned int    i;
	unsigned int    len;
	char            Hostname[128];
	char           *host_name;

	len = 0;
	i = fmt_str(s, "Received: indimail-mta queue ");
	len += i;
	if (s)
		s += i;
	i = fmt_ulong(s, mypid);
	len += i;
	if (s)
		s += i;

	i = fmt_str(s, " by host ");
	len += i;
	if (s)
		s += i;
	if (!(host_name = env_get("HOSTNAME"))) {
		if (gethostname(Hostname, sizeof(Hostname)) == -1)
			str_copy(Hostname, "unknown");
		host_name = Hostname;
	}
	i = fmt_str(s, host_name);
	len += i;
	if (s)
		s += i;

	i = fmt_str(s, "\n ");
	len += i;
	if (s)
		s += i;

	if (uid == auto_uida) {
		i = fmt_str(s, "(invoked by alias");
		len += i;
		if (s)
			s += i;
	} else
	if (uid == auto_uidd) {
		i = fmt_str(s, "(invoked from network ");
		len += i;
		if (s)
			s += i;
		if (!tcpremoteip)
			tcpremoteip = env_get("TCPREMOTEIP");
		if (!tcpremoteip)
			tcpgetremoteip();
		i = fmt_str(s, tcpremoteip);
		len += i;
		if (s)
			s += i;
	} else
	if (uid == auto_uids) {
		i = fmt_str(s, "(invoked for bounce");
		len += i;
		if (s)
			s += i;
	} else {
		i = fmt_str(s, "(invoked ");
		len += i;
		if (s)
			s += i;
		if (!tcpremoteip)
			tcpremoteip = env_get("TCPREMOTEIP");
		if (!tcpremoteip)
			tcpgetremoteip();
		if (tcpremoteip) {
			i = fmt_str(s, "from network ");
			len += i;
			if (s)
				s += i;
			i = fmt_str(s, tcpremoteip);
			len += i;
			if (s)
				s += i;
			i = fmt_str(s, ", ");
			len += i;
			if (s)
				s += i;
		}
		i = fmt_str(s, "by uid ");
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
	for (seq = 1; seq < 10; ++seq) {
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
 * Following added in addition to QHPSI
 * X-QHPSI:
 * VIRUSFORWARD:
 * X-Quarantine-ID:
 */
void
qhpsiprog(char *program)
{
	int             wstat, child, rejectvirus = 0, childrc = -1,
	                qhpsirc = 1, qhpsirn = 0;
	unsigned long   u;
	unsigned int    size, qhpsiminsize = 0, qhpsimaxsize = 0;
	char          **argv;
	char           *scancmd[3] = { 0, 0, 0 };
	char           *x;
	stralloc        plugin = { 0 }, qhpsibin = { 0 };
	struct stat     st;

	if (stat(messfn, &st) == -1) {
		cleanup();
		die(62);
	}
	size = (unsigned int) st.st_size;
	if ((x = env_get("QHPSIMINSIZE"))) {
		scan_ulong(x, &u);
		qhpsiminsize = (int) u;
	}
	if (qhpsiminsize && size < qhpsiminsize)
		return;
	if ((x = env_get("QHPSIMAXSIZE"))) {
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
		 * revoke all privileges and run with qmailq uid
		 */
		if (setregid(auto_gidq, auto_gidq) || setreuid(auto_uidq, auto_uidq))
			_exit(50);
		if (!str_diffn(program, "plugin:", 7)) {
			if (!stralloc_copys(&plugin, "plugin:") ||
					!stralloc_cats(&plugin, messfn) ||
					!stralloc_append(&plugin, " ") ||
					!stralloc_cats(&plugin, program + 7) ||
					!stralloc_0(&plugin) || !(argv = makeargs(plugin.s)))
				die(51);
		} else {
			if (!(argv = makeargs(program)))
				die(51);
			if (!argv[1]) {
				scancmd[0] = argv[0];
				scancmd[1] = messfn;
				argv = scancmd;
			} else
			for (u = 1; argv[u]; u++) {
				if (!str_diffn(argv[u], "%s", 2))
					argv[u] = messfn;
			}
		}
		if (!stralloc_copys(&qhpsibin, auto_prefix) ||
				!stralloc_catb(&qhpsibin, "/sbin/qhpsi", 11) ||
				!stralloc_0(&qhpsibin))
			die(51);
		execv(qhpsibin.s, argv);
		_exit(75);
	} /*- switch (child = fork()) */
	if (wait_pid(&wstat, child) == -1) {
		cleanup();
		die(122);
	}
	if (wait_crashed(wstat)) {
		cleanup();
		die(123);
	}
	childrc = wait_exitcode(wstat);
	if ((x = env_get("REJECTVIRUS"))) {
		scan_ulong(x, &u);
		rejectvirus = (int) u;
	}
	if ((x = env_get("QHPSIRC"))) {
		scan_ulong(x, &u);
		qhpsirc = (int) u;
	}
	if ((x = env_get("QHPSIRN"))) {
		scan_ulong(x, &u);
		qhpsirn = (int) u;
	}
	if (childrc == qhpsirc) { /*- Virus Infected */
		if (!stralloc_copys(&qqehextra, "X-QHPSI: Virus found\n")) {
			cleanup();
			die(51);
		}
		if ((x = env_get("VIRUSFORWARD"))) {
			if (!stralloc_copys(&quarantine, x)) {
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
	if (childrc == qhpsirn) { /*- Clean Mail */
		if (!stralloc_copys(&qqehextra, "X-QHPSI: clean\n")) {
			cleanup();
			die(51);
		}
	} else {
		cleanup();
		die(77); /*- QHPSI error in qmail.c */
	}
	return;
}
#endif
#if defined(MAILARCHIVE)
/*
 * sender based rules
 * ------------------
 * F:abc*@example.com:arch_%u@example.com
 * F:postmaster@*:arch_postmaster@%d
 * F:postmaster@*:arch_%e
 * F:abc@xyz.com:abc_13@example.com
 *
 * recipient based rules
 * ---------------------
 * T:spam_trap@example.com:spam_classifier@example.com
 * :abc@xyz.com:abc_13@example.com
 * *:abc@xyz.com:abc_13@example.com
 */
int
set_archive(char *eaddr)
{
	char           *rule_ptr, *dest, *ptr, *errStr = 0, *addr, *addr_ptr;
	int             len, at, type, found;
	static stralloc tmpe = {0};

	addr = eaddr + 1;
	type = *eaddr;
	for (rule_ptr = ar_rules.s, len = 0;len < ar_rules.len;len++) {
		if (((*rule_ptr == ':' || *rule_ptr == '*') && type == 'F')
			|| ((*rule_ptr == 'F' || *rule_ptr == 'T') && *rule_ptr != type)) {
			len += (str_len(rule_ptr) + 1);
			rule_ptr = ar_rules.s + len;
			continue; /*- type does not match */
		}
		for (addr_ptr = rule_ptr;*addr_ptr && *addr_ptr != ':';addr_ptr++);
		if (*addr_ptr != ':') { /*- invalid line in control file */
			len += (str_len(rule_ptr) + 1);
			rule_ptr = ar_rules.s + len;
			continue;
		}
		addr_ptr++; /*- address field in rule */
		for (dest = addr_ptr;*dest && *dest != ':';dest++);
		if (*dest != ':') { /*- invalid line in control file */
			len += (str_len(rule_ptr) + 1);
			rule_ptr = ar_rules.s + len;
			continue;
		}
		*dest++ = 0; /*- destination archival address */
		if (!*addr_ptr) /*- treat this as wildcard */
			addr_ptr = 0;
		else
		if (matchregex(addr, addr_ptr, &errStr))
			addr_ptr = 0;
		if (*dest && !addr_ptr) { /*- rule matched */
			*(dest - 1) = ':';
			if (!stralloc_copys(&tmpe, "T"))
				return (1);
			for (ptr = dest;*ptr;) {
				if (*ptr == '%' && *(ptr + 1)) {
					switch (*(ptr + 1))
					{
					case 'u':
						at = str_rchr(addr, '@');
						if (!stralloc_catb(&tmpe, addr, at))
							return (1);
						ptr++;
						break;
					case 'd':
						if (addr[at = str_rchr(addr, '@')] &&
							!stralloc_cats(&tmpe, addr + at + 1))
							return (1);
						ptr++;
						break;
					case 'e':
						if (!stralloc_cats(&tmpe, addr))
							return (1);
						ptr++;
						break;
					default:
						if (!stralloc_catb(&tmpe, ptr, 1))
							return (1);
						break;
					}
				} else
				if (!stralloc_catb(&tmpe, ptr, 1))
					return (1);
				ptr++;
			} /*-for (*ptr = dest;*ptr;) */
			if (!stralloc_0(&tmpe))
				return (1);
			/*- avoid duplicates */
			for (found = 0, ptr = arch_email.s;arch_email.len && *ptr;) {
				if (!str_diffn(ptr, tmpe.s, tmpe.len)) {
					found = 1;
					break;
				}
				ptr += (str_len(ptr) + 1);
			}
			if (!found && !stralloc_cat(&arch_email, &tmpe))
				return (1);
		} else
			*(dest - 1) = ':';
		len += (str_len(rule_ptr) + 1);
		rule_ptr = ar_rules.s + len;
	} /*- for (rule_ptr = ar_rule.s, len = 0;len < ar_rule.len;len++) */
	return (0);
}
#endif

#ifdef HASLIBRT
static char     errbuf[256];
static struct substdio sserr;

int
mq_todo(char *queue_ident, unsigned int priority)
{
	mqd_t           mqd;
	q_msg           qmsg;
	char           *ptr;
	int             errfd;

	if (!(ptr = env_get("ERROR_FD")))
		errfd = CUSTOM_ERR_FD;
	else
		scan_int(ptr, &errfd);
	substdio_fdbuf(&sserr, write, errfd, errbuf, sizeof(errbuf));
	/*- 
	 * send inode and split number for big todo
	 * send inode and conf_split for small todo
	 */
	if ((mqd = mq_open(queue_ident, O_WRONLY,  0600, NULL)) == (mqd_t) -1)
		return -1;
	qmsg.inum = messnum;
	qmsg.split = bigtodo ? messnum % conf_split : conf_split;
	for (;;) {
		if (mq_send(mqd, (char *) &qmsg, sizeof(q_msg), priority) == -1) {
			if (errno == error_intr)
				continue;
		} else
			break;
		return -1;
	}
	return (mq_close(mqd));
}
#endif

/*-
 * 1. setting env to any empty string turns off
 *    reading of control file f.
 * 2. If env is not set, control file f is read
 * 3. if flag is non-zero and env is set and non-empty,
 *    use value of env as the control filename
 *    If flag is zero and env is set and non-empty,
 *    use value of env as the value in s.
 */
void
read_control(stralloc *s, char *env, char *f, int flag)
{
	char           *ptr;

	ptr = env_get(env);
	if (!ptr) {
		if (control_readfile(s, f, 0) == -1)
			die(55);
	} else
	if (ptr && *ptr) {
		if (flag) {
			if (control_readfile(s, ptr, 0) == -1)
				die(55);
		} else {
			if (!stralloc_copys(s, ptr) || !stralloc_0(s))
				die(51);
			s->len--;
		}
	}
	return;
}

int
main()
{
	int             token_len, exclude = 0, include = 0, loghead = 0, line_brk_excl = 1,
					line_brk_incl = 1, in_header = 1, line_brk_log = 1, logfd = -1, field_len,
					itoken_len;
#ifdef USE_FSYNC
	int             fd;
#endif
	unsigned int    len, originipfield = 0;
	char            tmp[FMT_ULONG];
	char            ch;
	char           *ptr, *qqeh, *tmp_ptr;
	struct stat     st;
#ifdef HASLIBRT
	int             i;
#endif

	sig_blocknone();
	umask(033);
	if (uidinit(1, 0) == -1 || auto_uidq == -1 || auto_gidq == -1)
		die(67);
	if ((ptr = env_get("ORIGINIPFIELD")))
		scan_ulong(ptr, (unsigned long *) &originipfield);
	read_control(&extraqueue, "EXTRAQUEUE", "extraqueue", 0);
	read_control(&quarantine, "QUARANTINE", "quarantine", 0);
	/*-
	 * These control files will cause qmail-queue to 
	 * read one line at a time
	 */
	read_control(&excl, "REMOVEHEADERS", "removeheaders", 1);
	read_control(&incl, "ENVHEADERS", "envheaders", 1);
	read_control(&logh, "LOGHEADERS", "logheaders", 1);
#if defined(MAILARCHIVE)
	read_control(&ar_rules, "MAILARCHIVE", "mailarchive", 1);
	if (ar_rules.s && ar_rules.len)
		flagarchive = 1;
#endif
	if (!(queuedir = env_get("QUEUEDIR")))
		queuedir = "queue"; /*- single queue like qmail */
	if (chdir(queuedir) == -1)
		die(62);
#ifdef USE_FSYNC
	use_fsync = use_syncdir = 0;
	if ((ptr = env_get("USE_FSYNC")) && *ptr)
		use_fsync = 1;
	if ((ptr = env_get("USE_SYNCDIR")) && *ptr)
		use_syncdir = 1;
#endif
	getEnvConfigInt(&bigtodo, "BIGTODO", 0);
	getEnvConfigInt(&conf_split, "CONFSPLIT", auto_split);
	if (conf_split > auto_split)
		conf_split = auto_split;
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
	todofn = fnnum("todo/", bigtodo);
	intdfn = fnnum("intd/", bigtodo);
	if (link(pidfn, messfn) == -1) {
		unlink(pidfn);
		die(64);
	}
	if (unlink(pidfn) == -1) {
		unlink(messfn);
		die(63);
	}
	flagmademess = 1;
	substdio_fdbuf(&ssout, write, messfd, outbuf, sizeof(outbuf));
	/*- read the message body */
	substdio_fdbuf(&ssin, read, 0, inbuf, sizeof(inbuf));
	if (logh.len) {
		if ((ptr = env_get("LOGHEADERFD"))) {
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
		&& (env_get("RELAYCLIENT") || env_get("TCPREMOTEINFO"))) {
		origin_setup();
		if (substdio_bput(&ssout, origin, originlen) == -1)
			die_write();
	}

	if (!excl.len && !incl.len && !logh.len) {
		switch (substdio_copy(&ssout, &ssin))
		{
		case -2:
			die_read();
		case -3:
			die_write();
		}
	} else
	for (itoken_len = 0;;) { /*- Line Processing */
		if (getln(&ssin, &line, &match, '\n') == -1)
			die_read();
		if (!match && line.len == 0)
			break;
		if (in_header && !mess822_ok(&line))
			in_header = 0;
		if (!in_header) {
			if (substdio_put(&ssout, line.s, line.len))
				die_write();
			continue;
		}
		exclude = 0;
		include = 0;
		if (logfd > 0)
			loghead = 0;
		if (line.s[0] == ' ' || line.s[0] == '\t') { /*- RFC 822 LWSP char */
			if (!line_brk_excl) /*- Not a continuation line */
				exclude = 1;
			if (!line_brk_incl) {
				itoken_len += line.len;
				include = 1;
			}
			if (logfd > 0 && !line_brk_log)
				loghead = 1;
		} else {
			line_brk_excl = 1;
			line_brk_incl = 1;
			if (logfd > 0)
				line_brk_log = 1;
			for (len = 0, ptr = excl.s;len < excl.len;) {
				len += ((token_len = str_len(ptr)) + 1);
				if (!case_diffb(ptr, token_len, line.s)) {
					exclude = 1;
					line_brk_excl = 0; /*- Skip next line starting with LWSP */
					break;
				}
				ptr = excl.s + len;
			}
			for (len = 0, ptr = incl.s;len < incl.len;) {
				for (tmp_ptr = ptr;*tmp_ptr && *tmp_ptr != ':';tmp_ptr++);
				if (*tmp_ptr == ':') {
					*tmp_ptr = 0;
					scan_uint(tmp_ptr + 1, (unsigned int *) &field_len);
				} else
					field_len = -1;
				len += ((itoken_len = str_len(ptr)) + 1);
				if ((field_len == -1 || itoken_len <= field_len) && !case_diffb(ptr, itoken_len, line.s)) {
					include = 1;
					line_brk_incl = 0; /*- Include next line starting with LWSP */
					break;
				}
				ptr = incl.s + len;
			}
			for (len = 0, ptr = logh.s;logfd > 0 && len < logh.len;) {
				len += ((token_len = str_len(ptr)) + 1);
				if (!case_diffb(ptr, token_len, line.s)) {
					loghead = 1;
					line_brk_log = 0;
					break;
				}
				ptr = logh.s + len;
			}
		}
		if (include && (field_len == -1 || itoken_len <= field_len) && !stralloc_catb(&envheaders, line.s, line.len))
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
	/*- read the message envelope */
	substdio_fdbuf(&ssin, read, 1, inbuf, sizeof(inbuf));
	if (substdio_get(&ssin, &ch, 1) < 1)
		die_read();
	/*- Get the Sender */
	if (ch != 'F') {
		cleanup();
		die(79);
	}
	if (substdio_bput(&ssout, &ch, 1) == -1)
		die_write();
#if defined(MAILARCHIVE)
	if (flagarchive) {
		if (!stralloc_copys(&line, "") ||
				!stralloc_catb(&line, &ch, 1))
			die(51);
	}
#endif
	for (len = 0; len < ADDR; ++len) {
		if (substdio_get(&ssin, &ch, 1) < 1)
			die_read();
		if (substdio_put(&ssout, &ch, 1) == -1)
			die_write();
#if defined(MAILARCHIVE)
		if (flagarchive && !stralloc_catb(&line, &ch, 1))
			die(51);
#endif
		if (!ch)
			break;
	}
	if (len >= ADDR) {
		cleanup();
		die(11);
	}
	if (extraqueue.s && extraqueue.len) {
		for (len = 0, ptr = extraqueue.s;len < extraqueue.len;) {
			len += ((token_len = str_len(ptr)) + 1);
			if (substdio_bput(&ssout, "T", 1) == -1)
				die_write();
			if (substdio_bput(&ssout, ptr, token_len) == -1)
				die_write();
			if (substdio_bput(&ssout, "\0", 1) == -1)
				die_write();
			ptr = extraqueue.s + len;
		}
	}
	if (flagquarantine) {
		if (substdio_bput(&ssout, "T", 1) == -1)
			die_write();
		if (substdio_bput(&ssout, quarantine.s, quarantine.len) == -1)
			die_write();
		if (substdio_bput(&ssout, "\0", 1) == -1)
			die_write();
	}
	if (flagquarantine) {
		if (!stralloc_cats(&qqehextra, "X-Quarantine-ID: ") ||
				!stralloc_cat(&qqehextra, &quarantine)) {
			cleanup();
			die(51);
		}
	}
	/*- Get the recipients */
	if (flagblackhole) {
		for (;;) { /*- empty the pipe created by qmail_open() */
			if (substdio_get(&ssin, &ch, 1) < 1)
				die_read();
			if (!ch)
				break;
		}
		cleanup();
		die(0);
	} else /*- write all recipients */
	for (;;) {
		/*- get one recipient at a time */
		if (substdio_get(&ssin, &ch, 1) < 1)
			die_read();
		if (!ch)
			break;
#if defined(QHPSI)
		if (ch == 'Q') {
			qhpsi = 0;
			break;
		}
#endif
		if (ch != 'T') {
			cleanup();
			die(79);
		}
		if (flagquarantine) {
			if (!stralloc_append(&qqehextra, ",")) {
				cleanup();
				die(51);
			}
		} else
		if (substdio_bput(&ssout, &ch, 1) == -1)
			die_write();
#if defined(MAILARCHIVE)
		if (flagarchive && !stralloc_catb(&line, &ch, 1))
			die(51);
#endif
		for (len = 0; len < ADDR; ++len) {
			if (substdio_get(&ssin, &ch, 1) < 1)
				die_read();
			if (flagquarantine) {
				if (ch && !stralloc_append(&qqehextra, &ch)) {
					cleanup();
					die(51);
				}
			} else
			if (substdio_bput(&ssout, &ch, 1) == -1)
				die_write();
#if defined(MAILARCHIVE)
			if (flagarchive && !stralloc_catb(&line, &ch, 1))
				die(51);
#endif
			if (!ch) /* this completes one recipient */
				break;
		}
		if (len >= ADDR) {
			cleanup();
			die(11);
		}
	} /*- for (;;) */
#if defined(MAILARCHIVE)
	if (flagarchive) {
		/*
		 * cycle through all the sender/recipients addresses
		 * assign special address F<> for bounces
		 */
		if (!stralloc_0(&line))
			die(51);
		for (ptr = line.s;*ptr;) {
			if (set_archive(*(ptr + 1) ? ptr : "F<>"))
				die(51);
			ptr += str_len(ptr) + 1;
		}
	}
	if (arch_email.s && arch_email.len) {
		if (substdio_bput(&ssout, arch_email.s, arch_email.len) == -1)
			die_write();
	}
#endif
	if (flagquarantine && !stralloc_append(&qqehextra, "\n")) {
		cleanup();
		die(51);
	}
	if ((qqeh && *qqeh) || (qqehextra.s && qqehextra.len)) {
		if (substdio_bput(&ssout, "e", 1) == -1)
			die_write();
	}
	if (qqehextra.s && qqehextra.len) {
		if (substdio_bput(&ssout, qqehextra.s, qqehextra.len) == -1)
			die_write();
	}
	if (qqeh && *qqeh) {
		if (substdio_bputs(&ssout, qqeh) == -1)
			die_write();
		len = str_len(qqeh);
		if (qqeh[len - 1] != '\n' && substdio_bput(&ssout, "\n", 1) == -1)
			die_write();
	}
	if ((qqeh && *qqeh) || (qqehextra.s && qqehextra.len)) {
		if (substdio_bput(&ssout, "\0", 1) == -1)
			die_write();
	}
	if (envheaders.s && envheaders.len) {
		if (substdio_bput(&ssout, "h", 1) == -1 ||
				substdio_bput(&ssout, envheaders.s, envheaders.len) == -1 ||
				substdio_bput(&ssout, "\0", 1) == -1)
			die_write();
	}
	if (substdio_flush(&ssout) == -1)
		die_write();
#ifdef USE_FSYNC
	if (use_fsync > 0) {
		if (fsync(messfd) == -1 || fsync(intdfd) == -1)
			die_write();
	}
#endif
	if (link(intdfn, todofn) == -1) {
		cleanup();
		die(66);
	}
#ifdef USE_FSYNC
	if (use_syncdir > 0) {
		if ((fd = open(todofn, O_RDONLY)) == -1) {
			/*- 
			 * check if todofn has been picked up by qmail-todo
			 * and moved to local or remote folder. In such
			 * a case you will get error_noent
			 */
			if (errno != error_noent) {
				cleanup();
				die(69);
			} 
		} else
		if (fsync(fd) == -1 || close(fd) == -1) {
			cleanup();
			die(69);
		}
	}
#endif
#ifdef HASLIBRT
	if (!(ptr = env_get("DYNAMIC_QUEUE")))
		triggerpull();
	else {
		i = str_rchr(queuedir, '/');
		if (!queuedir[i])
			triggerpull();
		else
		if (mq_todo(queuedir + i, 10) == -1)
			triggerpull();
	}
#else
	triggerpull();
#endif
	die(0);
	/*- Not reached */
	return (0);
}

#ifndef lint
void
getversion_qmail_queue_c()
{
	static char    *x = "$Id: qmail-queue.c,v 1.81 2022-03-08 23:07:17+05:30 Cprogrammer Exp mbhangui $";

	x = sccsidmakeargsh;
	x++;
}
#endif
/*
 * $Log: qmail-queue.c,v $
 * Revision 1.81  2022-03-08 23:07:17+05:30  Cprogrammer
 * do not treat error_noent as an error
 *
 * Revision 1.80  2022-03-05 13:33:33+05:30  Cprogrammer
 * use auto_prefix/sbin for qhpsi path
 *
 * Revision 1.79  2022-02-11 11:52:01+05:30  Cprogrammer
 * new function read_control() for loading extraqueue, quarantine, removehaders, envheaders, logheaders control files.
 *
 * Revision 1.78  2022-01-30 08:43:56+05:30  Cprogrammer
 * added qscheduler, removed qmail-daemon
 * added haslibrt.h to configure dynamic queue
 * fix for FreeBSD
 * revert to trigger method if using message queue fails
 * fixed formatting of Received line
 * make USE_FSYNC, USE_SYNCDIR consistent across programs
 * allow configurable big/small todo/intd
 *
 * Revision 1.77  2021-09-11 23:21:45+05:30  Cprogrammer
 * updated Received headers
 *
 * Revision 1.76  2021-08-29 23:27:08+05:30  Cprogrammer
 * define functions as noreturn
 *
 * Revision 1.75  2021-07-15 12:43:03+05:30  Cprogrammer
 * uidnit new argument to disable/enable error on missing uids
 * moved conf_split to fmtqfn.c
 * removed auto_qmail.h
 *
 * Revision 1.74  2021-06-24 12:16:59+05:30  Cprogrammer
 * use uidinit function proto from auto_uids.h
 *
 * Revision 1.73  2021-06-15 12:16:01+05:30  Cprogrammer
 * removed chdir(auto_qmail)
 *
 * Revision 1.72  2021-05-29 23:50:28+05:30  Cprogrammer
 * replace str_chr with str_rchr to get domain correctly from email address
 *
 * Revision 1.71  2021-05-16 00:48:50+05:30  Cprogrammer
 * use configurable conf_split instead of auto_split variable
 *
 * Revision 1.70  2021-05-12 15:58:55+05:30  Cprogrammer
 * set conf_split from CONFSPLIT env variable
 *
 * Revision 1.69  2021-05-09 17:37:51+05:30  Cprogrammer
 * documented Received headers
 *
 * Revision 1.68  2021-05-08 12:22:00+05:30  Cprogrammer
 * use /var/indimail/queue if QUEUEDIR is not defined
 *
 * Revision 1.67  2020-12-07 16:10:12+05:30  Cprogrammer
 * use exit code 79 for envelope format error
 *
 * Revision 1.66  2020-11-24 13:47:17+05:30  Cprogrammer
 * removed exit.h
 *
 * Revision 1.65  2020-09-30 20:39:38+05:30  Cprogrammer
 * Darwin port for syncdir
 *
 * Revision 1.64  2020-09-15 21:11:08+05:30  Cprogrammer
 * changed default value of use_fsync as -1
 *
 * Revision 1.63  2020-04-01 16:14:45+05:30  Cprogrammer
 * added header for makeargs() function
 *
 * Revision 1.62  2018-01-09 11:49:26+05:30  Cprogrammer
 * use indimail-mta identifier in Received: headers
 *
 * Revision 1.61  2017-05-04 20:20:36+05:30  Cprogrammer
 * close passwd, group database
 *
 * Revision 1.60  2017-04-11 12:38:45+05:30  Cprogrammer
 * field_len was not initialized. Fixed envheader randomly not working
 *
 * Revision 1.59  2016-06-03 09:58:17+05:30  Cprogrammer
 * moved qhspi to sbin
 *
 * Revision 1.58  2016-01-29 11:50:48+05:30  Cprogrammer
 * append null only when copying extraqueue from env variable
 *
 * Revision 1.57  2016-01-29 11:31:10+05:30  Cprogrammer
 * fixed 'Z' getting appended to domain in extraqueue
 *
 * Revision 1.56  2013-05-30 11:10:32+05:30  Cprogrammer
 * extraqueue can have multiple lines as extra recipients
 *
 * Revision 1.55  2011-07-29 09:29:43+05:30  Cprogrammer
 * fixed gcc 4.6 warnings
 *
 * Revision 1.54  2010-07-22 13:48:53+05:30  Cprogrammer
 * terminate QQEH header with newline
 *
 * Revision 1.53  2010-06-29 11:15:33+05:30  Cprogrammer
 * fixed various bugs with rule based archival, eliminated duplication of emails
 *
 * Revision 1.52  2010-06-27 09:01:41+05:30  Cprogrammer
 * fixed recipient based rules for archival
 *
 * Revision 1.51  2010-06-18 23:52:36+05:30  Cprogrammer
 * added mailarchive functionality
 *
 * Revision 1.50  2010-04-07 19:30:13+05:30  Cprogrammer
 * use HOSTNAME environment variable to record host
 *
 * Revision 1.49  2010-03-25 10:16:11+05:30  Cprogrammer
 * added documentation
 *
 * Revision 1.48  2009-12-09 23:57:23+05:30  Cprogrammer
 * additional closeflag argument to uidinit
 *
 * Revision 1.47  2009-12-05 20:07:08+05:30  Cprogrammer
 * added prototype for makeargs
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
 * change exit code to "qq unable to read configuration (#4.3.0)"
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
