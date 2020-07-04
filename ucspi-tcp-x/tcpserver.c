/*
 * $Log: tcpserver.c,v $
 * Revision 1.64  2020-07-04 22:03:00+05:30  Cprogrammer
 * fixed global variables overshadowd by local variables
 *
 * Revision 1.63  2020-06-08 22:48:45+05:30  Cprogrammer
 * quench compiler warning
 *
 * Revision 1.62  2019-07-10 13:42:39+05:30  Cprogrammer
 * fixed wrong child exit status in logs
 *
 * Revision 1.61  2019-06-07 19:20:54+05:30  Cprogrammer
 * print MySQL load status
 *
 * Revision 1.60  2019-05-26 12:04:50+05:30  Cprogrammer
 * use /etc/indimail/control as controldir
 *
 * Revision 1.59  2019-04-22 21:57:59+05:30  Cprogrammer
 * use mysql only if use_sql is non-zero
 *
 * Revision 1.58  2019-04-21 10:25:07+05:30  Cprogrammer
 * load MySQL functions dynamically at run time
 *
 * Revision 1.57  2018-05-30 12:32:32+05:30  Cprogrammer
 * flagssl should not be used when TLS is not defined
 *
 * Revision 1.56  2018-05-30 12:30:33+05:30  Cprogrammer
 * replaced gethostbyname() with getaddrinfo()
 *
 * Revision 1.55  2017-04-05 04:08:39+05:30  Cprogrammer
 * execute tcpserver_plugin() after shedding root privilege
 *
 * Revision 1.54  2017-04-05 03:06:32+05:30  Cprogrammer
 * changed data type for uid, gid to uid_t, gid_t
 *
 * Revision 1.53  2016-06-21 13:33:06+05:30  Cprogrammer
 * use SSL_set_cipher_list as part of crypto-policy-compliance
 *
 * Revision 1.52  2016-05-16 21:21:09+05:30  Cprogrammer
 * call tcpserver_plugin with reload option on sighup
 *
 * Revision 1.51  2016-05-15 22:42:48+05:30  Cprogrammer
 * added tcpserver plugin
 *
 * Revision 1.50  2013-08-06 07:57:11+05:30  Cprogrammer
 * added socket_ip6optionskill for IPV6 socket
 *
 * Revision 1.49  2012-09-08 16:35:11+05:30  Cprogrammer
 * BUG - wrong variable used for ip address causing fnrules to fail in ip address matching
 *
 * Revision 1.48  2012-04-23 18:49:42+05:30  Cprogrammer
 * use ipv4 addresses if fakev4 detected for if -4 option is passed
 *
 * Revision 1.47  2012-04-19 21:11:31+05:30  Cprogrammer
 * undefine MYSQL_CONFIG for IPV6
 *
 * Revision 1.46  2010-04-16 13:13:13+05:30  Cprogrammer
 * fixed passing parameter for MYSQL_OPT_CONNECT_TIMEOUT
 *
 * Revision 1.45  2010-03-12 08:59:54+05:30  Cprogrammer
 * print connected IPs table on sigusr1
 *
 * Revision 1.44  2010-03-11 13:37:59+05:30  Cprogrammer
 * log maxperip
 *
 * Revision 1.43  2010-03-11 12:47:47+05:30  Cprogrammer
 * initialized maxperio with PerHostLimit
 *
 * Revision 1.42  2009-08-12 09:37:16+05:30  Cprogrammer
 * corrected display of usage
 *
 * Revision 1.41  2009-05-31 09:35:08+05:30  Cprogrammer
 * use CONTROL dir for certificats
 * added -s option for default certificate
 *
 * Revision 1.40  2009-05-29 15:54:48+05:30  Cprogrammer
 * unset env variables with - rule
 * set SSL_CIPHER
 * use ssl if -n option is provided
 *
 * Revision 1.39  2009-05-26 12:23:31+05:30  Cprogrammer
 * added setting of environment variable TCPPARANOID
 *
 * Revision 1.38  2009-05-05 14:58:10+05:30  Cprogrammer
 * close mysql connections
 *
 * Revision 1.37  2008-07-30 12:12:31+05:30  Cprogrammer
 * configure mysql automatically
 *
 * Revision 1.36  2008-07-29 23:01:48+05:30  Cprogrammer
 * mysql code made compile time configurable
 *
 * Revision 1.35  2008-07-25 16:50:23+05:30  Cprogrammer
 * fix for darwin
 *
 * Revision 1.34  2008-07-17 23:05:12+05:30  Cprogrammer
 * removed readwrite.h
 *
 * Revision 1.33  2008-06-30 16:11:14+05:30  Cprogrammer
 * removed license code
 *
 * Revision 1.32  2008-06-30 09:39:45+05:30  Cprogrammer
 * removed compilation warning
 *
 * Revision 1.31  2008-06-12 14:31:04+05:30  Cprogrammer
 * added mysql patch by Levent Serinol.
 *
 * Revision 1.30  2007-06-10 10:18:03+05:30  Cprogrammer
 * fixed ipv6 issue
 * added compile time license option
 *
 * Revision 1.29  2005-06-11 02:11:35+05:30  Cprogrammer
 * added IPV6 and SSL support
 *
 * Revision 1.28  2004-10-12 00:31:47+05:30  Cprogrammer
 * renamed remoteinfo.h to tcpremoteinfo.h
 *
 * Revision 1.27  2004-09-30 23:23:26+05:30  Cprogrammer
 * BUG - Fixed segmentation fault when concurrency was increased and sighup
 * was given
 *
 * Revision 1.26  2004-09-29 10:56:28+05:30  Cprogrammer
 * made maxperip code to work
 *
 * Revision 1.25  2004-09-24 10:05:03+05:30  Cprogrammer
 * added configurable max per ip limit
 *
 * Revision 1.24  2004-05-12 09:00:49+05:30  Cprogrammer
 * change in checklicense()
 * corrected compilation warning for fedora core
 *
 * Revision 1.23  2003-12-31 20:06:13+05:30  Cprogrammer
 * print receipt of sighup on stderr
 *
 * Revision 1.22  2003-12-30 00:33:45+05:30  Cprogrammer
 * made concurrency configurable.
 *
 * Revision 1.21  2003-12-25 23:45:33+05:30  Cprogrammer
 * BUG - wrongly used remoteip instead of remoteipstr
 * print IPs only if verbosity >= 2
 *
 * Revision 1.20  2003-10-17 21:08:31+05:30  Cprogrammer
 * added tcpserver: tag in print_ip()
 *
 * Revision 1.19  2003-10-11 09:17:17+05:30  Cprogrammer
 * print per host concurrency in log
 *
 * Revision 1.18  2003-09-23 15:50:15+05:30  Cprogrammer
 * added license code
 * numchildren did not get decremented when PerHostLimit was reached
 *
 * Revision 1.17  2002-08-19 19:58:02+05:30  Cprogrammer
 * removed mysql code
 *
 * Revision 1.16  2002-08-18 13:56:06+05:30  Cprogrammer
 * shifted snprintf for better efficiency
 *
 * Revision 1.15  2002-04-10 10:47:50+05:30  Cprogrammer
 * fifo path was wrong
 *
 * Revision 1.14  2002-04-09 13:53:31+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.13  2002-04-08 04:46:45+05:30  Cprogrammer
 * *** empty log message ***
 *
 */
#include <fcntl.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/ipc.h>
#ifdef TLS
#include <openssl/ssl.h>
#endif
#include "str.h"
#include "byte.h"
#include "fmt.h"
#include "scan.h"
#include "uint16.h"
#include "haveip6.h"
#include "ip4.h"
#ifdef IPV6
#include "ip6.h"
#include "uint32.h"
#endif
#include "fd.h"
#include "exit.h"
#include "env.h"
#include "prot.h"
#include "open.h"
#include "wait.h"
#include "stralloc.h"
#include "alloc.h"
#include "buffer.h"
#include "error.h"
#include "strerr.h"
#include <getopt.h>
#ifdef DARWIN
#define opteof -1
#else
#include "sgetopt.h"
#endif
#include "pathexec.h"
#include "socket.h"
#include "ndelay.h"
#include "tcpremoteinfo.h"
#include "rules.h"
#include "sig.h"
#include "dns.h"
#include "hasmysql.h"
#include "control.h"
#include "load_mysql.h"
#include "auto_home.h"

#ifndef	lint
static char     sccsid[] = "$Id: tcpserver.c,v 1.64 2020-07-04 22:03:00+05:30 Cprogrammer Exp mbhangui $";
#endif

#ifdef IPV6
int             forcev6 = 0;
uint32          netif = 0;
#endif
int             verbosity = 1;
int             flagkillopts = 1;
int             flagdelay = 1;
char           *banner = "";
int             flagremoteinfo = 1;
int             flagremotehost = 1;
int             flagparanoid = 0;
int             verbose;
unsigned long   timeout = 26, maxperip = 20, PerHostLimit = 20;
unsigned long   limit = 40;
unsigned long   alloc_count = 0;
unsigned long   numchildren = 0;
unsigned long   backlog = 20;
#ifdef TLS
int             flagssl = 0;
struct stralloc certfile = {0};
#endif

static stralloc tcpremoteinfo;
uint16          localport;
char            localportstr[FMT_ULONG];
#ifdef IPV6
char            localip[16];
char            localipstr[IP6_FMT];
char            localipstr6[IP6_FMT];
char            remoteip[16];
char            remoteipstr[IP6_FMT];
char            remoteipstr6[IP6_FMT];
#else
char            localip[4];
char            localipstr[IP4_FMT];
char            remoteip[4];
char            remoteipstr[IP4_FMT];
#endif
static stralloc localhostsa;
char           *localhost = 0;
uint16          remoteport;
char            remoteportstr[FMT_ULONG];
static stralloc remotehostsa;
char           *remotehost = 0;

char            strnum[FMT_ULONG];
char            strnum2[FMT_ULONG];

static stralloc tmp;
static stralloc fqdn;
static stralloc addresses;

int             flag1 = 0;
char            bspace[16];
buffer          b;
static stralloc limitFile;

uid_t           uid = 0;
gid_t           gid = 0;

struct iptable
{
	char            ipaddr[16];
	pid_t           pid;
};
typedef struct iptable IPTABLE;
IPTABLE      **IpTable;

void            add_ip(pid_t);
void            print_ip();
unsigned long   check_ip();
void            remove_ip(pid_t);
int             socket_ipoptionskill(int);
int             socket_ip6optionskill(int);
int             socket_tcpnodelay(int);
#ifdef TLS
void            translate(SSL*, int, int, unsigned int);
#endif
int             tcpserver_plugin(char **, int);
extern char   **environ;

/*---------------------------- child */

#define DROP  "tcpserver: warning: dropping connection, "

int             flagdeny = 0;
int             flagallownorules = 0;
char           *fnrules = 0;

void
drop_nomem(void)
{
	strerr_die2sys(111, DROP, "out of memory");
}

void
cats(char *s)
{
	if (!stralloc_cats(&tmp, s))
		drop_nomem();
}

void
append(char *ch)
{
	if (!stralloc_append(&tmp, ch))
		drop_nomem();
}

void
safecats(char *s)
{
	char            ch;
	int             i;

	for (i = 0; i < 100; ++i) {
		if (!(ch = s[i]))
			return;
		if (ch < 33)
			ch = '?';
		if (ch > 126)
			ch = '?';
		if (ch == '%')
			ch = '?'; /*- logger stupidity */
#ifndef IPV6
		if (ch == ':')
			ch = '?';
#endif
		append(&ch);
	}
	cats("..."); /*- Line longer than 100 bytes */
}

void
env(char *s, char *t)
{
	if (str_equal(s, "MAXPERIP"))
		scan_ulong(t, &maxperip);
	if (!pathexec_env(s, t))
		drop_nomem();
}

void
drop_rules(void)
{
	strerr_die4sys(111, DROP, "unable to read ", fnrules, ": ");
}

void
found(char *data, unsigned int datalen)
{
	unsigned int    next0;
	unsigned int    split;

	while ((next0 = byte_chr(data, datalen, 0)) < datalen) {
		switch (data[0])
		{
		case 'D':
			flagdeny = 1;
			break;
		case '+':
			split = str_chr(data + 1, '=');
			if (data[1 + split] == '=') {
				data[1 + split] = 0;
				env(data + 1, data + 1 + split + 1);
			}
			break;
		case '-':
			env(data + 1, (char *)0);
			break;
		}
		++next0;
		data += next0;
		datalen -= next0;
	}
}

void
doit(int t)
{
	int             j;
#ifdef IPV6
	int             fakev4 = 0;
	uint32          scope_id;
#endif

#ifdef IPV6
	if (!forcev6 && ip6_isv4mapped(remoteip))
		fakev4 = 1;
	if (fakev4)
		remoteipstr[ip4_fmt(remoteipstr, remoteip + 12)] = 0;
	else
	if (noipv6 && !forcev6)
		remoteipstr[ip4_fmt(remoteipstr, remoteip)] = 0;
	else
		remoteipstr[ip6_fmt(remoteipstr, remoteip)] = 0;
#else
	remoteipstr[ip4_fmt(remoteipstr, remoteip)] = 0;
#endif
	if (verbosity >= 2) {
		strnum[fmt_ulong(strnum, getpid())] = 0;
		strerr_warn4("tcpserver: pid ", strnum, " from ", remoteipstr, 0);
	}
	if (flagkillopts) {
		socket_ipoptionskill(t);
		socket_ip6optionskill(t);
	}
	if (!flagdelay)
		socket_tcpnodelay(t);
	if (*banner) {
		buffer_init(&b, write, t, bspace, sizeof bspace);
		if (buffer_putsflush(&b, banner) == -1)
			strerr_die2sys(111, DROP, "unable to print banner: ");
	}
#ifdef IPV6
	if (socket_local6(t, localip, &localport, &scope_id) == -1)
#else
	if (socket_local4(t, localip, &localport) == -1)
#endif
		strerr_die2sys(111, DROP, "unable to get local address: ");
#ifdef IPV6
	if (fakev4)
		localipstr[ip4_fmt(localipstr, localip + 12)] = 0;
	else
	if (noipv6 && !forcev6)
		localipstr[ip4_fmt(localipstr, localip)] = 0;
	else
		localipstr[ip6_fmt(localipstr, localip)] = 0;
#else
	localipstr[ip4_fmt(localipstr, localip)] = 0;
#endif
	remoteportstr[fmt_ulong(remoteportstr, remoteport)] = 0;
#ifdef IPV6
	if (!localhost && !dns_name6(&localhostsa, localip))
#else
	if (!localhost && !dns_name4(&localhostsa, localip))
#endif
	{
		if (localhostsa.len) {
			if (!stralloc_0(&localhostsa))
				drop_nomem();
			localhost = localhostsa.s;
		}
	}
#ifdef IPV6
	if (noipv6 && !forcev6)
		env("PROTO", "TCP");
	else
		env("PROTO", fakev4 ? "TCP" : "TCP6");
#else
	env("PROTO", "TCP");
#endif
	env("TCPLOCALIP", localipstr);
	env("TCPLOCALPORT", localportstr);
	env("TCPLOCALHOST", localhost);
#ifdef IPV6
	if (!noipv6) {
		localipstr6[ip6_fmt(localipstr6, localip)] = 0;
		env("TCP6LOCALIP", localipstr6);
		env("TCP6LOCALPORT", localportstr);
		env("TCP6LOCALHOST", localhost);
	}
	if (!fakev4 && scope_id)
		env("TCP6INTERFACE", (char *) socket_getifname(scope_id));
	if (flagremotehost && !dns_name6(&remotehostsa, remoteip))
#else
	if (flagremotehost && !dns_name4(&remotehostsa, remoteip))
#endif
	{
		if (remotehostsa.len) {
#ifdef IPV6
			if (flagparanoid && !dns_ip6(&tmp, &remotehostsa))
#else
			if (flagparanoid && !dns_ip4(&tmp, &remotehostsa))
#endif
			{
#ifdef IPV6
				for (j = 0; j + 16 <= tmp.len; j += 16)
#else
				for (j = 0; j + 4 <= tmp.len; j += 4)
#endif
				{
#ifdef IPV6
					if (byte_equal(remoteip, 16, tmp.s + j))
#else
					if (byte_equal(remoteip, 4, tmp.s + j))
#endif
					{
						flagparanoid = 0;
						break;
					}
				}
			}
			if (!flagparanoid) {
				if (!stralloc_0(&remotehostsa))
					drop_nomem();
				remotehost = remotehostsa.s;
			} else /*- host has reverse DNS but IP does not match remoteip */
				env("TCPPARANOID", remotehostsa.s);
		}  /*- host does not have reverse DNS */
	}
	env("TCPREMOTEIP", remoteipstr);
	env("TCPREMOTEPORT", remoteportstr);
	env("TCPREMOTEHOST", remotehost);
#ifdef IPV6
	if (!noipv6) {
		remoteipstr6[ip6_fmt(remoteipstr6, remoteip)] = 0;
		env("TCP6REMOTEIP", remoteipstr6);
		env("TCP6REMOTEPORT", remoteportstr);
		env("TCP6REMOTEHOST", remotehost);
	}
#endif
	if (flagremoteinfo) {
#ifdef IPV6
		if (remoteinfo6(&tcpremoteinfo, remoteip, remoteport, localip, localport, timeout, netif) == -1)
#else
		if (remoteinfo(&tcpremoteinfo, remoteip, remoteport, localip, localport, timeout) == -1)
#endif
			flagremoteinfo = 0;
		if (!stralloc_0(&tcpremoteinfo))
			drop_nomem();
	}
	env("TCPREMOTEINFO", flagremoteinfo ? tcpremoteinfo.s : 0);
#ifdef IPV6
	env("TCP6REMOTEINFO", flagremoteinfo ? tcpremoteinfo.s : 0);
#endif
	if (fnrules) {
		int             fdrules;

		if ((fdrules = open_read(fnrules)) == -1) {
			if (errno != error_noent)
				drop_rules();
			if (!flagallownorules)
				drop_rules();
		} else {
			char           *temp;
#ifdef IPV6
			if (fakev4 || (noipv6 && !forcev6))
				temp = remoteipstr;
			else
				temp = remoteipstr6;
#else
			temp = remoteipstr;
#endif
			if (rules(found, fdrules, temp, remotehost, flagremoteinfo ? tcpremoteinfo.s : 0) == -1)
				drop_rules();
			close(fdrules);
		}
	}
	if (verbosity >= 2) {
		strnum[fmt_ulong(strnum, getpid())] = 0;
		if (!stralloc_copys(&tmp, "tcpserver: "))
			drop_nomem();
		safecats(flagdeny ? "deny" : "ok");
		cats(" ");
		safecats(strnum);
		cats(" ");
		if (localhost)
			safecats(localhost);
		cats("[");
#ifdef IPV6
		safecats((fakev4 || (noipv6 && !forcev6)) ? localipstr : localipstr6);
#else
		safecats(localipstr);
#endif
		cats("]");
		safecats(localportstr);
		cats(" ");
		if (remotehost)
			safecats(remotehost);
		cats("[");
#ifdef IPV6
		safecats((fakev4 || (noipv6 && !forcev6)) ? remoteipstr : remoteipstr6);
#else
		safecats(remoteipstr);
#endif
		cats("]");
		if (flagremoteinfo)
			safecats(tcpremoteinfo.s);
		cats(":");
		safecats(remoteportstr);
		cats(":");
		cats("maxperip=");
		strnum[fmt_ulong(strnum, maxperip)] = 0;
		safecats(strnum);
		cats("\n");
		buffer_putflush(buffer_2, tmp.s, tmp.len);
	}
	if (flagdeny)
		_exit(100);
}

/*
 * ---------------------------- parent 
 */

#define FATAL "tcpserver: fatal: "

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

static char    *
my_strchr(char *str, char ch)
{
	int             i;

	i = str_chr(str, ch);
	if (!str[i])
		return ((char *) 0);
	return (str + i);
}

int
matchinet(char *ip, char *token)
{
	char            field1[8];
	char            field2[8];
	unsigned long   lnum, hnum, t;
	int             idx1, idx2, match;
	char           *ptr, *ptr1, *ptr2, *cptr;
#if defined(LIBC_HAS_IP6) && defined(IPV6)
	struct addrinfo hints = {0}, *addr_res = 0, *addr_res0 = 0;
	struct sockaddr     sa;
	struct sockaddr_in *in4 = (struct sockaddr_in *) &sa;
	struct sockaddr_in6 *in6 = (struct sockaddr_in6 *) &sa;
#else
	struct hostent *hp;
#endif

	/*- Exact match / match all */
	if (!str_diff(token, ip))
		return (1);
	/*
	 * If our token is a valid internet address then we don't need to
	 * check any further
	 * if (inet_aton(token, &inp))
	 */
	if (inet_addr(token) != INADDR_NONE)
		return (0);
	else
	for (match = idx1 = 0, ptr1 = token, ptr2 = ip; idx1 < 4 && match == idx1; idx1++) {
		/*- IP Address in control file */
		for (cptr = field1; *ptr1 && *ptr1 != '.'; *cptr++ = *ptr1++);
		*cptr = 0;
		ptr1++;

		/*
		 * IP Address of client 
		 */
		for (cptr = field2; *ptr2 && *ptr2 != '.'; *cptr++ = *ptr2++);
		*cptr = 0;
		ptr2++;
		/*-
		 * Network address wildcard match (i.e. "192.86.28.*")
		 */
		if (!str_diff(field1, "*")) {
			match++;
			continue;
		}
		/*
		 * Network address wildcard match (i.e.
		 * "192.86.2?.12?")
		 */
		for (; (ptr = my_strchr(field1, '?'));) {
			lnum = ptr - field1;
			*ptr = field2[lnum];
		}
		if (!str_diff(field1, field2)) {
			match++;
			continue;
		}
		/*
		 * Range match (i.e. "190-193.86.22.11") 
		 */
		if ((ptr = my_strchr(field1, '-'))) {
			*ptr = 0;
			ptr++;
			scan_ulong(field1, &lnum);
			if (!*ptr)
				hnum = 256;
			else
				scan_ulong(ptr, &hnum);
			scan_ulong(field2, &t);
			for (idx2 = lnum; idx2 <= hnum; idx2++) {
				if (idx2 == t) {
					match++;
					break;
				}
			}
			if (idx2 <= hnum && idx2 == t)
				continue;
		}
	} /*- for (match = idx1 = 0, ptr1 = token, ptr2 = ip; idx1 < 4 && match == idx1; idx1++) */
	if (match == 4)
		return (1);
	/*
	 * If our token is a host name then translate it and compare internet
	 * addresses
	 */
#if defined(LIBC_HAS_IP6) && defined(IPV6)
		static char     addrBuf[INET6_ADDRSTRLEN];
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_DGRAM;
		hints.ai_flags = AI_PASSIVE;    /* For wildcard IP address */
		hints.ai_protocol = 0;          /* Any protocol */
		hints.ai_canonname = 0;
		hints.ai_addr = 0;
		hints.ai_next = 0;
		if (getaddrinfo(token, 0, &hints, &addr_res0))
			return (-1);
		for (addr_res = addr_res0; addr_res; addr_res = addr_res->ai_next) {
			byte_copy((char *) &sa, addr_res->ai_addrlen, (char *) addr_res->ai_addr);
			if (sa.sa_family == AF_INET) {
				in4 = (struct sockaddr_in *) &sa;
				if (!inet_ntop(AF_INET, (void *) &in4->sin_addr, addrBuf, INET_ADDRSTRLEN))
					return (-1);
			} else
			if (sa.sa_family == AF_INET6) {
				in6 = (struct sockaddr_in6 *) &sa;
				if (!inet_ntop(AF_INET6, (void *) &in6->sin6_addr, addrBuf, INET_ADDRSTRLEN))
					return (-1);
			} else
				continue;
			if (!str_diff(ip, addrBuf))
				return (1);
		}
		freeaddrinfo(addr_res0);
#else
	if (!(hp = gethostbyname(token)))
		return (0);
	if (!str_diff(ip, inet_ntoa(*((struct in_addr *) hp->h_addr_list[0]))))
		return (1);
#endif
	return (0);
}

#if defined(HAS_MYSQL)
#include <mysql.h>
#include <mysqld_error.h>

struct stralloc dbserver = {0};
struct stralloc dbuser = {0};
struct stralloc dbpass = {0};
struct stralloc dbname = {0};
struct stralloc dbtable = {0};
MYSQL          *conn = (MYSQL *) 0;

void
create_table(MYSQL *mysql)
{
	static stralloc sql = { 0 };

	if (!stralloc_copys(&sql, "create table "))
		drop_nomem();
	if (!stralloc_cats(&sql, dbtable.s))
		drop_nomem();
	if (!stralloc_cats(&sql, " (port int(5) NOT NULL, timestamp timestamp not null, \
				iprule char(16) not null, \
				decision char(1) not null, env varchar(255), primary key (port))"))
		drop_nomem();
	if (!stralloc_0(&sql))
		drop_nomem();
	if (in_mysql_query(mysql, sql.s)) {
		sql.len--;
		if (!stralloc_cats(&sql, ": ")) {
			in_mysql_close(mysql);
			drop_nomem();
		}
		if (!stralloc_cats(&sql, (char *) in_mysql_error(mysql))) {
			in_mysql_close(mysql);
			drop_nomem();
		}
		in_mysql_close(mysql);
		if (!stralloc_0(&sql))
			drop_nomem();
		strerr_die2x(111, DROP, sql.s);
	}
	return;
}

void
connect_db(char *dbfile)
{
	char           *x = 0, *m_timeout;
	int             fd, i = 0;
	unsigned int    next, xlen, mysql_timeout;
	struct stat     st;
	static MYSQL    mysql;

	if (!(m_timeout = env_get("MYSQL_TIMEOUT")))
		m_timeout = "30";
	scan_ulong(m_timeout, (unsigned long *) &mysql_timeout);
	if ((fd = open_read(dbfile)) == -1)
		strerr_die4sys(111, FATAL, "unable to read ", dbfile, ": ");
	if (fstat(fd, &st) == -1) {
		close(fd);
		strerr_die4sys(111, FATAL, "unable to fstat ", dbfile, ": ");
	}
	if (st.st_size <= 0xffffffff) {
		x = mmap(0, st.st_size, PROT_READ, MAP_SHARED, fd, 0);
		xlen = st.st_size;
		while ((next = byte_chr(x, xlen, ':')) < xlen) {
			switch (i)
			{
			case 0:
				if (!stralloc_copyb(&dbserver, x, next)) {
					munmap(x, st.st_size);
					close(fd);
					drop_nomem();
				}
				if (!stralloc_0(&dbserver)) {
					munmap(x, st.st_size);
					close(fd);
					drop_nomem();
				}
				break;
			case 1:
				if (!stralloc_copyb(&dbuser, x, next)) {
					munmap(x, st.st_size);
					close(fd);
					drop_nomem();
				}
				if (!stralloc_0(&dbuser)) {
					munmap(x, st.st_size);
					close(fd);
					drop_nomem();
				}
				break;
			case 2:
				if (!stralloc_copyb(&dbpass, x, next)) {
					munmap(x, st.st_size);
					close(fd);
					drop_nomem();
				}
				if (!stralloc_0(&dbpass)) {
					munmap(x, st.st_size);
					close(fd);
					drop_nomem();
				}
				break;
			case 3:
				if (!stralloc_copyb(&dbname, x, next)) {
					munmap(x, st.st_size);
					close(fd);
					drop_nomem();
				}
				if (!stralloc_0(&dbname)) {
					munmap(x, st.st_size);
					close(fd);
					drop_nomem();
				}
				break;
			case 4:
				if (!stralloc_copyb(&dbtable, x, next)) {
					munmap(x, st.st_size);
					close(fd);
					drop_nomem();
				}
				if (!stralloc_0(&dbtable)) {
					munmap(x, st.st_size);
					close(fd);
					drop_nomem();
				}
				break;
			}
			i++;
			++next;
			x += next;
			xlen -= next;
		}
		if (i < 4) {
			munmap(x, st.st_size);
			close(fd);
			strerr_die2x(111, FATAL, "Invalid db.conf");
		}
		if (i == 4) {
			if (!*x || *x == '\n') {
				munmap(x, st.st_size);
				close(fd);
				strerr_die2x(111, FATAL, "Invalid db.conf");
			}
			if (!stralloc_copys(&dbtable, x)) {
				munmap(x, st.st_size);
				close(fd);
				drop_nomem();
			}
			if (*(dbtable.s + dbtable.len -1) == '\n')
				*(dbtable.s + dbtable.len -1) = 0;
			else
			if (!stralloc_0(&dbtable)) {
				munmap(x, st.st_size);
				close(fd);
				drop_nomem();
			}
		}
	} else {
		close(fd);
		strerr_die2x(111, FATAL, "File too large");
	}
	munmap(x, st.st_size);
	close(fd);
	if (!in_mysql_init(&mysql))
		drop_nomem();
	in_mysql_options(&mysql, MYSQL_OPT_CONNECT_TIMEOUT, (char *) &mysql_timeout);
	if (!in_mysql_real_connect(&mysql, dbserver.s, dbuser.s, dbpass.s, dbname.s, 0, NULL, 0))
		strerr_die3x(111, FATAL, "Unable to connect to MySQL Server: ", (char *) in_mysql_error(&mysql));
	conn = &mysql;
	return;
}

void
check_db(MYSQL *mysql)
{

	MYSQL_RES      *myres;
	MYSQL_ROW       row;
	static stralloc sql = { 0 };

	if (!stralloc_copys(&sql, "select decision, iprule, env from ")) {
		in_mysql_close(mysql);
		drop_nomem();
	}
	if (!stralloc_cats(&sql, dbtable.s)) {
		in_mysql_close(mysql);
		drop_nomem();
	}
	if (!stralloc_cats(&sql, " where port = ")) {
		in_mysql_close(mysql);
		drop_nomem();
	}
	if (!stralloc_cats(&sql, localportstr)) {
		in_mysql_close(mysql);
		drop_nomem();
	}
	if (!stralloc_0(&sql)) {
		in_mysql_close(mysql);
		drop_nomem();
	}
	if (in_mysql_query(mysql, sql.s)) {
		if (in_mysql_errno(mysql) == ER_NO_SUCH_TABLE) {
			create_table(mysql);
			if (!in_mysql_query(mysql, sql.s))
				goto done;
		}
		sql.len--;
		if (!stralloc_cats(&sql, ": ")) {
			in_mysql_close(mysql);
			drop_nomem();
		}
		if (!stralloc_cats(&sql, (char *) in_mysql_error(mysql))) {
			in_mysql_close(mysql);
			drop_nomem();
		}
		in_mysql_close(mysql);
		if (!stralloc_0(&sql))
			drop_nomem();
		strerr_die2x(111, DROP, sql.s);
	}
done:
	if (!(myres = in_mysql_store_result(mysql))) {
		sql.len--;
		if (!stralloc_cats(&sql, "mysql_store_result: ")) {
			in_mysql_close(mysql);
			drop_nomem();
		}
		if (!stralloc_cats(&sql, (char *) in_mysql_error(mysql))) {
			in_mysql_close(mysql);
			drop_nomem();
		}
		in_mysql_close(mysql);
		if (!stralloc_0(&sql))
			drop_nomem();
		strerr_die2x(111, DROP, sql.s);
	}
	for (;(row = in_mysql_fetch_row(myres));) {
		if (!str_diff(row[1], "*") || matchinet(remoteipstr, row[1])) {
			if (*row[0] == 'D') {
				strerr_warn4("tcpserver: MySQL: Port ", localportstr, ": IPrule ", row[1], 0);
				flagdeny = 1;
				in_mysql_free_result(myres);
				return;
			}
			if (row[2]) {
				char           *ptr1, *ptr2, *ptr3, *ptr4;
				unsigned int    split;

				for (ptr2 = ptr1 = row[2];*ptr1;ptr1++) {
					if (*ptr1 == ',') {
						/*
			 			* Allow ',' in environment variable if escaped
			 			* by '\' character
			 			*/
						if (ptr1 != row[2] && *(ptr1 - 1) == '\\') {
							/*- for (ptr3 = ptr1 - 1; *ptr3; *ptr3++ = *(ptr3 + 1)); -*/
							for (ptr3 = ptr1 - 1, ptr4 = ptr1; *ptr3; *ptr3++ = *ptr4++);
							continue;
						}
						*ptr1 = 0;
						split = str_chr(ptr2, '=');
						if (ptr2[split] == '=') {
							ptr2[split] = 0;
							env(ptr2, ptr2 + split + 1);
						}
						ptr2 = ptr1 + 1;
					}
				} /*- for */
				split = str_chr(ptr2, '=');
				if (ptr2[split] == '=') {
					ptr2[split] = 0;
					env(ptr2, ptr2 + split + 1);
				}
			} /*- if (row[2]) */
		}
	}
	in_mysql_free_result(myres);
}
#endif /*- #ifdef HAS_MYSQL */

void
usage(void)
{
	strerr_warn1(
"tcpserver: usage: tcpserver\n"
#ifdef IPV6
"[ -461UXpPhHrRoOdDqQv ]\n"
#else
"[ -1UXpPhHrRoOdDqQv ]\n"
#endif
"[ -c Maxlimit | -c concurrency_file]\n"
"[ -C PerHostlimit ]\n"
"[ -x rules.cdb ]\n"
"[ -B banner ]\n"
"[ -g gid ]\n"
"[ -u uid ]\n"
"[ -b backlog ]\n"
"[ -l localname ]\n"
"[ -t timeout ]\n"
#if defined(HAS_MYSQL)
"[ -m db.conf ]\n"
#endif
#ifdef TLS
"[ -s ]\n"
"[ -n certfile ]\n"
#endif
#ifdef IPV6
"[ -I interface ]\n"
#endif
"host port program", 0);
	_exit(100);
}

void
printstatus(void)
{
	if (verbosity < 2)
		return;
	strnum[fmt_ulong(strnum, numchildren)] = 0;
	strnum2[fmt_ulong(strnum2, limit)] = 0;
	strerr_warn5("tcpserver: status: ", strnum, "/", strnum2, use_sql ? " sql: 1" : " sql: 0", 0);
}

void
init_ip()
{
	int             i;
	IPTABLE       **iptable;

	if (!IpTable) {
		if (!(IpTable = (IPTABLE **) alloc(limit * sizeof(IPTABLE *))))
			drop_nomem();
		alloc_count = limit;
		for (i = 0, iptable = IpTable;i < limit;i++, iptable++) {
			if (!(*iptable = (IPTABLE *) alloc(sizeof(IPTABLE)))) {
				drop_nomem();
				return;
			}
			(*iptable)->ipaddr[0] = 0;
			(*iptable)->pid = -1;
		}
	}
	return;
}

void
add_ip(pid)
	pid_t           pid;
{
	int             i;
#ifdef IPV6
	int             fakev4 = 0;
#endif
	IPTABLE       **iptable;

	if (!IpTable)
		init_ip();
#ifdef IPV6
	if (!forcev6 && ip6_isv4mapped(remoteip))
		fakev4 = 1;
	if (fakev4)
		remoteipstr[ip4_fmt(remoteipstr, remoteip + 12)] = 0;
	else
	if (noipv6 && !forcev6)
		remoteipstr[ip4_fmt(remoteipstr, remoteip)] = 0;
	else
		remoteipstr[ip6_fmt(remoteipstr, remoteip)] = 0;
#else
	remoteipstr[ip4_fmt(remoteipstr, remoteip)] = 0;
#endif
	for (i = 0, iptable = IpTable; i < limit; iptable++, i++) {
		if ((*iptable)->pid == -1) {
			(*iptable)->pid = pid;
#ifdef IPV6
			byte_copy((*iptable)->ipaddr, 16, remoteip);
#else
			byte_copy((*iptable)->ipaddr, 16, remoteipstr);
			(*iptable)->ipaddr[15] = 0;
#endif
			break;
		}
	}
	if (i == limit)
		strerr_die2sys(111, DROP, "no ip slots: ");
}

unsigned long
check_ip()
{
	int             i, count;
	IPTABLE       **iptable;

	if (!(numchildren && numchildren <= limit))
		return (limit);
	if (!IpTable)
		init_ip();
#ifndef IPV6
	remoteipstr[ip4_fmt(remoteipstr, remoteip)] = 0;
#endif
	for (count = i = 0, iptable = IpTable; i < limit; iptable++, i++) {
		if ((*iptable)->pid == -1)
			continue;
#ifdef IPV6
		if (byte_equal(remoteip, 16, (*iptable)->ipaddr))
#else
		if (!str_diffn((*iptable)->ipaddr, remoteipstr, 16))
#endif
			count++;
	}
	return (count);
}

void
remove_ip(pid)
	pid_t           pid;
{
	int             i;
	IPTABLE       **iptable;

	if (!IpTable)
		init_ip();
	for (i = 0, iptable = IpTable; i < limit; iptable++, i++) {
		if ((*iptable)->pid == pid) {
			(*iptable)->pid = -1;
			break;
		}
	}
}

void
print_ip()
{
	int             i;
#ifdef IPV6
	int             fakev4 = 0;
#endif
	IPTABLE       **iptable;
	char            slotstr[FMT_ULONG], pid_str[FMT_ULONG];

	if (verbosity < 2)
		return;
	if (!IpTable)
		init_ip();
	for (i = 0, iptable = IpTable; i < limit; iptable++, i++) {
		if ((*iptable)->pid != -1) {
			slotstr[fmt_ulong(slotstr, i)] = 0;
			pid_str[fmt_ulong(pid_str, (*iptable)->pid)] = 0;
#ifdef IPV6
			if (!forcev6 && ip6_isv4mapped((*iptable)->ipaddr))
				fakev4 = 1;
			if (fakev4)
				remoteipstr[ip4_fmt(remoteipstr, (*iptable)->ipaddr + 12)] = 0;
			else
			if (noipv6 && !forcev6)
				remoteipstr[ip4_fmt(remoteipstr, (*iptable)->ipaddr)] = 0;
			else
				remoteipstr[ip6_fmt(remoteipstr, (*iptable)->ipaddr)] = 0;
#else
			remoteipstr[ip4_fmt(remoteipstr, (*iptable)->ipaddr)] = 0;
#endif
			strerr_warn6("tcpserver: ", slotstr, ": IP ", remoteipstr, ": PID ", pid_str, 0);
		}
	}
	return;
}

void
sigterm()
{
	_exit(0);
}

void
siguser1()
{
	print_ip();
}

void
sighangup()
{
	int             i, tmpLimit;
	IPTABLE       **iptable1, **iptable2, **tmpTable;

	if (!IpTable)
		init_ip();
	if (tcpserver_plugin(environ, 0))
		_exit(111);
	tmpLimit = limit;
	if (control_readint(&tmpLimit, limitFile.s) == -1)
		strerr_die4sys(111, FATAL, "unable to read ", limitFile.s, ": ");
	if (tmpLimit > alloc_count) {
		if (!(tmpTable = (IPTABLE **) alloc(tmpLimit * sizeof(IPTABLE *))))
			drop_nomem();
		for (i = 0, iptable1 = tmpTable, iptable2 = IpTable;i < tmpLimit;i++, iptable1++) {
			if (i < alloc_count) {
				*iptable1 = *iptable2++;
				continue;
			}
			if (!(*iptable1 = (IPTABLE *) alloc(sizeof(IPTABLE)))) {
				drop_nomem();
				return;
			}
			(*iptable1)->ipaddr[0] = 0;
			(*iptable1)->pid = -1;
		}
		alloc_free((char *) IpTable);
		IpTable = tmpTable;
		alloc_count = tmpLimit;
	}
	limit = tmpLimit;
	strnum[fmt_ulong(strnum, numchildren)] = 0;
	strnum2[fmt_ulong(strnum2, limit)] = 0;
	strerr_warn4("tcpserver: sighup: ", strnum, "/", strnum2, 0);
}

void
sigchld()
{
	int             i, wstat, pid;

	while ((pid = wait_nohang(&wstat)) > 0) {
		if (verbosity >= 2) {
			strnum[fmt_ulong(strnum, pid)] = 0;
			if (wait_crashed(wstat))
				strerr_warn3("tcpserver: end ", strnum, " status crashed", 0);
			else
			if ((i = wait_exitcode(wstat))) {
				strnum2[fmt_ulong(strnum2, i)] = 0;
				strerr_warn4("tcpserver: end ", strnum, " status ", strnum2, 0);
			}
		}
		if (numchildren) {
			--numchildren;
			remove_ip(pid);
		}
		printstatus();
	}
}

int
main(int argc, char **argv, char **envp)
{
	char           *x, *hostname;
#if defined(HAS_MYSQL)
	char           *dbfile = 0;
#endif
	struct servent *se;
	unsigned long   port, ipcount = -1;
	int             s, t, pid, opt, tmpLimit;
#ifdef IPV6
	int             fakev4 = 0;
#endif
#ifdef TLS
	BIO            *sbio;
	SSL            *ssl;
	SSL_CTX        *ctx;
	char           *controldir;
	int             pi2c[2], pi4c[2];
#endif
	struct stralloc options = {0};

	if ((x = env_get("MAXPERIP"))) { /*- '-C' option overrides this */
		scan_ulong(x, &PerHostLimit);
		maxperip = PerHostLimit;
	}
	if (!stralloc_copys(&options, "dDvqQhHrR1UXx:m:t:u:g:l:b:B:c:C:pPoO"))
		strerr_die2x(111, FATAL, "out of memory");
#ifdef IPV6
	if (!stralloc_cats(&options, "46I:"))
		strerr_die2x(111, FATAL, "out of memory");
#endif
#ifdef TLS
	if (!stralloc_cats(&options, "sn:"))
		strerr_die2x(111, FATAL, "out of memory");
#endif
	if (!stralloc_0(&options))
		strerr_die2x(111, FATAL, "out of memory");
#ifdef TLS
	ctx = NULL;
	if (!(controldir = env_get("CONTROLDIR")))
		controldir = "/etc/indimail/control";
	if (!stralloc_copys(&certfile, controldir))
		strerr_die2x(111, FATAL, "out of memory");
	else
	if (!stralloc_cats(&certfile, "/servercert.pem"))
		strerr_die2x(111, FATAL, "out of memory");
	else
	if (!stralloc_0(&certfile) )
		strerr_die2x(111, FATAL, "out of memory");
#endif
	while ((opt = getopt(argc, argv, options.s)) != opteof)
		switch (opt)
		{
		case 'c':
			for (x = optarg;*x;x++) {
				if (*x == '/' || *x == '.' || *x < '0' || *x > '9')
					break;
			}
			if (*x) {
				if (!stralloc_copys(&limitFile, optarg))
					strerr_die2x(111, FATAL, "out of memory");
				if (!stralloc_0(&limitFile))
					strerr_die2x(111, FATAL, "out of memory");
				tmpLimit = limit;
				if (control_readint(&tmpLimit, limitFile.s) == -1)
					strerr_die4sys(111, FATAL, "unable to read ", limitFile.s, ": ");
				limit = tmpLimit;
			} else
				scan_ulong(optarg, &limit);
			break;
		case 'C':
			scan_ulong(optarg, &PerHostLimit);
			break;
		case 'b':
			scan_ulong(optarg, &backlog);
			break;
		case 'X':
			flagallownorules = 1;
			break;
		case 'x':
			fnrules = optarg;
			break;
#if defined(HAS_MYSQL)
		case 'm':
			dbfile = optarg;
#endif
			break;
		case 'B':
			banner = optarg;
			break;
		case 'd':
			flagdelay = 1;
			break;
		case 'D':
			flagdelay = 0;
			break;
		case 'v':
			verbosity = 2;
			break;
		case 'q':
			verbosity = 0;
			break;
		case 'Q':
			verbosity = 1;
			break;
		case 'P':
			flagparanoid = 0;
			break;
		case 'p':
			flagparanoid = 1;
			break;
		case 'O':
			flagkillopts = 1;
			break;
		case 'o':
			flagkillopts = 0;
			break;
		case 'H':
			flagremotehost = 0;
			break;
		case 'h':
			flagremotehost = 1;
			break;
		case 'R':
			flagremoteinfo = 0;
			break;
		case 'r':
			flagremoteinfo = 1;
			break;
		case 't':
			scan_ulong(optarg, &timeout);
			break;
		case 'U':
			x = env_get("UID");
			if (x)
				scan_uint(x, &uid);
			x = env_get("GID");
			if (x)
				scan_uint(x, &gid);
			break;
		case 'u':
			scan_uint(optarg, &uid);
			break;
		case 'g':
			scan_uint(optarg, &gid);
			break;
		case '1':
			flag1 = 1;
			break;
		case 'l':
			localhost = optarg;
			break;
#ifdef TLS
		case 's':
			flagssl = 1;
			break;
		case 'n':
			flagssl = 1;
			if (!stralloc_copys(&certfile, optarg) || !stralloc_0(&certfile))
				strerr_die2x(111, FATAL, "out of memory");
			break;
#endif
#ifdef IPV6
		case '4':
			noipv6 = 1;
			break;
		case '6':
			forcev6 = 1;
			break;
		case 'I':
			netif = socket_getifidx(optarg);
			break;
#endif
		default:
			usage();
		}
	argc -= optind;
	argv += optind;
	if (!verbosity)
		buffer_2->fd = -1;
	if (!(hostname = *argv++))
		usage();
	if (str_equal(hostname, ""))
		hostname = "0";
	if (!(x = *argv++))
		usage();
	if (!x[scan_ulong(x, &port)])
		localport = port;
	else {
		if (!(se = getservbyname(x, "tcp")))
			strerr_die3x(111, FATAL, "unable to figure out port number for ", x);
#ifdef IPV6
		uint16_unpack_big((char*) &se->s_port, &localport);
#else
		localport = ntohs(se->s_port);
#endif
	}
	if (!*argv)
		usage();
#if defined(HAS_MYSQL)
	if (initMySQLlibrary(&x))
		strerr_die3x(111, FATAL, "couldn't load MySQL shared lib: ", x);
	if (use_sql && dbfile)
		connect_db(dbfile);
#endif
	sig_block(sig_child);
	sig_catch(sig_child, sigchld);
	sig_catch(sig_term, sigterm);
	sig_catch(sig_hangup, sighangup);
	sig_catch(sig_usr1, siguser1);
	sig_ignore(sig_pipe);
	if (str_equal(hostname, "0"))
		byte_zero(localip, sizeof(localip));
	else {
		if (!stralloc_copys(&tmp, hostname))
			strerr_die2x(111, FATAL, "out of memory");
#ifdef IPV6
		if (dns_ip6_qualify(&addresses, &fqdn, &tmp) == -1)
#else
		if (dns_ip4_qualify(&addresses, &fqdn, &tmp) == -1)
#endif
			strerr_die4sys(111, FATAL, "temporarily unable to figure out IP address for ", hostname, ": ");
#ifdef IPV6
		if (addresses.len < 16)
#else
		if (addresses.len < 4)
#endif
			strerr_die3x(111, FATAL, "no IP address for ", hostname);
#ifdef IPV6
		byte_copy(localip, 16, addresses.s);
		if (ip6_isv4mapped(localip))
			noipv6 = 1;
#else
		byte_copy(localip, 4, addresses.s);
#endif
	}
#ifdef TLS
	if (flagssl == 1) {
    	/* setup SSL context (load key and cert into ctx) */
		SSL_library_init();
		if (!(ctx = SSL_CTX_new(SSLv23_server_method())))
			strerr_die2x(111, FATAL, "unable to create SSL context");
		/* set prefered ciphers */
#ifdef CRYPTO_POLICY_NON_COMPLIANCE
		x = env_get("SSL_CIPHER");
		if (x && !SSL_CTX_set_cipher_list(ctx, x))
			strerr_die3x(111, FATAL, "unable to set cipher list:", x);
#endif
		if (SSL_CTX_use_RSAPrivateKey_file(ctx, certfile.s, SSL_FILETYPE_PEM) != 1)
			strerr_die2x(111, FATAL, "unable to load RSA private key");
		if (SSL_CTX_use_certificate_file(ctx, certfile.s, SSL_FILETYPE_PEM) != 1)
			strerr_die2x(111, FATAL, "unable to load certificate");
	}
#endif
#ifdef IPV6
	if ((s = socket_tcp6()) == -1)
#else
	if ((s = socket_tcp()) == -1)
#endif
		strerr_die2sys(111, FATAL, "unable to create socket: ");
#ifdef IPV6
	if (socket_bind6_reuse(s, localip, localport, netif) == -1)
#else
	if (socket_bind4_reuse(s, localip, localport) == -1)
#endif
		strerr_die2sys(111, FATAL, "unable to bind: ");
#ifdef IPV6
	if (socket_local6(s, localip, &localport, &netif) == -1)
#else
	if (socket_local4(s, localip, &localport) == -1)
#endif
		strerr_die2sys(111, FATAL, "unable to get local address: ");
	if (socket_listen(s, backlog) == -1)
		strerr_die2sys(111, FATAL, "unable to listen: ");
	ndelay_off(s);
	if (gid && prot_gid(gid) == -1)
		strerr_die2sys(111, FATAL, "unable to set gid: ");
	if (uid && prot_uid(uid) == -1)
		strerr_die2sys(111, FATAL, "unable to set uid: ");
	if (tcpserver_plugin(envp, 1))
		_exit(111);
	localportstr[fmt_ulong(localportstr, localport)] = 0;
	if (flag1) {
		buffer_init(&b, write, 1, bspace, sizeof bspace);
		buffer_puts(&b, localportstr);
		buffer_puts(&b, "\n");
		buffer_flush(&b);
	}
	close(0);
	close(1);
	printstatus();
	for (;;) {
		while (numchildren >= limit)
			sig_pause();
		sig_unblock(sig_child);
#ifdef IPV6
		t = socket_accept6(s, remoteip, &remoteport, &netif);
#else
		t = socket_accept4(s, remoteip, &remoteport);
#endif
		sig_block(sig_child);
		if (t == -1)
			continue;
		++numchildren;
		printstatus();
#ifdef IPV6
		if (!forcev6 && ip6_isv4mapped(remoteip))
			fakev4 = 1;
		if (fakev4)
			remoteipstr[ip4_fmt(remoteipstr, remoteip + 12)] = 0;
		else
		if (noipv6 && !forcev6)
			remoteipstr[ip4_fmt(remoteipstr, remoteip)] = 0;
		else
			remoteipstr[ip6_fmt(remoteipstr, remoteip)] = 0;
#else
		remoteipstr[ip4_fmt(remoteipstr, remoteip)] = 0;
#endif
		if (PerHostLimit && (ipcount = check_ip()) >= PerHostLimit) {
			strnum2[fmt_ulong(strnum2, ipcount)] = 0;
			strerr_warn4("tcpserver: end ", remoteipstr, " perIPlimit ", strnum2, 0);
			--numchildren;
			errno = error_acces;
			close(t);
			continue;
		}
		switch (pid = fork())
		{
		case 0:
			close(s);
#if defined(HAS_MYSQL)
			if (use_sql && conn)
				check_db(conn);
#endif
			doit(t); /*- MAXPERIP can only be checked after this */
			if ((fd_move(0, t) == -1) || (fd_copy(1, 0) == -1))
				strerr_die2sys(111, DROP, "unable to set up descriptors: ");
			sig_uncatch(sig_child);
			sig_unblock(sig_child);
			sig_uncatch(sig_term);
			sig_uncatch(sig_pipe);
			if (maxperip) {
				if (ipcount == -1)
					ipcount = check_ip();
				if (ipcount >= maxperip) {
					strnum2[fmt_ulong(strnum2, ipcount)] = 0;
					close(t);
					errno = error_acces;
					strerr_die6sys(111, DROP, "no ip slots for ", remoteipstr, ", perIPlimit ", strnum2, ": ");
				}
			}
#ifdef TLS
			if (flagssl == 1) {
				if (pipe(pi2c) != 0)
					strerr_die2sys(111, DROP, "unable to create pipe: ");
				if (pipe(pi4c) != 0)
					strerr_die2sys(111, DROP, "unable to create pipe: ");
				switch (fork())
				{
				case 0:
					SSL_CTX_free(ctx);
					close(0);
					close(1);
					close(pi2c[1]);
					close(pi4c[0]);
					if ((fd_move(0, pi2c[0]) == -1) || (fd_move(1, pi4c[1]) == -1))
						strerr_die2sys(111, DROP, "unable to set up descriptors: ");
					/*
					 * signals are allready set in the parent 
					 */
					pathexec(argv);
					strerr_die4sys(111, DROP, "unable to run ", *argv, ": ");
				case -1:
					strerr_die2sys(111, DROP, "unable to fork: ");
				default:
					ssl = SSL_new(ctx);
					SSL_CTX_free(ctx);
					if (!ssl)
						strerr_die2x(111, DROP, "unable to set up SSL session");
#ifdef CRYPTO_POLICY_NON_COMPLIANCE
					if (!(x = env_get("SSL_CIPHER")))
						x = "PROFILE:SYSTEM";
					if (!SSL_set_cipher_list(myssl, x))
						strerr_die3x(111, FATAL, "unable to set cipher list:", x);
#endif
					sbio = BIO_new_socket(0, BIO_NOCLOSE);
					if (!sbio) {
						SSL_free(ssl);
						strerr_die2x(111, DROP, "unable to set up BIO socket");
					}
					SSL_set_bio(ssl, sbio, sbio);
					close(pi2c[0]);
					close(pi4c[1]);
					translate(ssl, pi2c[1], pi4c[0], 3600);
					SSL_free(ssl);
					_exit(0);
				}
			}
#endif
			pathexec(argv);
			strerr_die4sys(111, DROP, "unable to run ", *argv, ": ");
		case -1:
			strerr_warn2(DROP, "unable to fork: ", &strerr_sys);
			--numchildren;
			printstatus();
		default:
			add_ip(pid);
		}
		close(t);
	}
}

#ifdef TLS
static int
allwrite(int fd, char *buf, int len)
{
	int             w;

	while (len) {
		if ((w = write(fd, buf, len)) == -1) {
			if (errno == error_intr)
				continue;
			return -1;	/*- note that some data may have been written */
		}
		if (w == 0);	/*- luser's fault */
		buf += w;
		len -= w;
	}
	return 0;
}

static int
allwritessl(SSL * ssl, char *buf, int len)
{
	int             w;

	while (len) {
		if ((w = SSL_write(ssl, buf, len)) == -1) {
			if (errno == error_intr)
				continue;
			return -1;	/*- note that some data may have been written */
		}
		if (w == 0);	/*- luser's fault */
		buf += w;
		len -= w;
	}
	return 0;
}

char            tbuf[2048];
void
translate(SSL * ssl, int clearout, int clearin, unsigned int iotimeout)
{
	struct taia     now;
	struct taia     deadline;
	iopause_fd      iop[2];
	int             flagexitasap;
	int             iopl;
	int             sslout, sslin;
	int             n, r;

	if ((sslin = SSL_get_fd(ssl)) == -1 || (sslout = SSL_get_fd(ssl)) == -1)
		strerr_die2x(111, DROP, "unable to set up SSL connection");
	flagexitasap = 0;
	if (SSL_accept(ssl) <= 0)
		strerr_die2x(111, DROP, "unable to accept SSL connection");
	while (!flagexitasap) {
		taia_now(&now);
		taia_uint(&deadline, iotimeout);
		taia_add(&deadline, &now, &deadline);

		/*- fill iopause struct */
		iopl = 2;
		iop[0].fd = sslin;
		iop[0].events = IOPAUSE_READ;
		iop[1].fd = clearin;
		iop[1].events = IOPAUSE_READ;

		/*- do iopause read */
		iopause(iop, iopl, &deadline, &now);
		if (iop[0].revents) {
			/*- data on sslin */
			n = SSL_read(ssl, tbuf, sizeof(tbuf));
			if (n < 0)
				strerr_die2sys(111, DROP, "unable to read from network: ");
			if (n == 0)
				flagexitasap = 1;
			r = allwrite(clearout, tbuf, n);
			if (r < 0)
				strerr_die2sys(111, DROP, "unable to write to client: ");
		}
		if (iop[1].revents) {
			/*- data on clearin */
			n = read(clearin, tbuf, sizeof(tbuf));
			if (n < 0)
				strerr_die2sys(111, DROP, "unable to read from client: ");
			if (n == 0)
				flagexitasap = 1;
			r = allwritessl(ssl, tbuf, n);
			if (r < 0)
				strerr_die2sys(111, DROP, "unable to write to network: ");
		}
		if (!iop[0].revents && !iop[1].revents)
			strerr_die2x(0, DROP, "timeout reached without input");
	}
}
#endif

void
getversion_tcpserver_c()
{
	if (write(1, sccsid, 0) == -1) ;
}
