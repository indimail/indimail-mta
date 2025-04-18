/*
 * $Id: tcpserver.c,v 1.96 2025-01-21 23:53:43+05:30 Cprogrammer Exp mbhangui $
 */
#include <fcntl.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/ipc.h>
#ifdef TLS
#include <openssl/ssl.h>
#include <sys/stat.h>
#include <tls.h>
#include <ctype.h>
#include <openreadclose.h>
#include <getEnvConfig.h>
#include <timeoutread.h>
#include <timeoutwrite.h>
#endif
#include <str.h>
#include <byte.h>
#include <fmt.h>
#include <scan.h>
#include <fd.h>
#include <env.h>
#include <prot.h>
#include <open.h>
#include <wait.h>
#include <stralloc.h>
#include <alloc.h>
#include <substdio.h>
#include <subfd.h>
#include <error.h>
#include <strerr.h>
#include <sig.h>
#include <setuserid.h>
#include <noreturn.h>
#include <ndelay.h>
#include <iopause.h>
#include <isnum.h>
#ifdef DARWIN
#define opteof -1
#else
#include <sgetopt.h>
#endif
#include <uint16.h>
#include "haveip6.h"
#include "ip4.h"
#ifdef IPV6
#include "ip6.h"
#include <uint32.h>
#endif
#if defined(IPV6) && defined(FREEBSD)
#include <sys/select.h>
#endif
#include "upathexec.h"
#include "socket.h"
#include "tcpremoteinfo.h"
#include "rules.h"
#include "dns.h"
#ifdef MYSQL_CONFIG
#include "hasmysql.h"
#include "load_mysql.h"
#endif
#include "control.h"
#include "auto_home.h"

typedef const char c_char;
#ifdef IPV6
static int      forcev6;
static uint32   netif;
#endif
typedef unsigned long my_ulong;
static int      verbosity = 1;
static int      flagkillopts = 1;
static int      flagdelay = 1;
static c_char  *banner = "";
static int      flagremoteinfo = 1;
static int      flagremotehost = 1;
static int      flagparanoid;
static my_ulong itimeout = 26; /* timeoutinfo -t option */
static my_ulong ctimeout = 60; /* timeoutssl  -S option */
static my_ulong dtimeout = 300;/* timeoutdata -T option */
static my_ulong maxperip = 20, PerHostLimit = 20;
static my_ulong limit = 40;
static my_ulong alloc_count;
static my_ulong numchildren;
static my_ulong backlog = 20;
#ifdef TLS
static SSL     *ssl;
static int      flagssl;
struct stralloc certfile, cafile, crlfile;
struct stralloc saciphers;
#endif

static stralloc tcpremoteinfo;
static uint16   localport;
static char     localportstr[FMT_ULONG];
#ifdef IPV6
static char     localip[16];
static char     localipstr[IP6_FMT];
static char     localipstr6[IP6_FMT];
static char     remoteip[16];
static char     remoteipstr[IP6_FMT];
static char     remoteipstr6[IP6_FMT];
#else
static char     localip[4];
static char     localipstr[IP4_FMT];
static char     remoteip[4];
static char     remoteipstr[IP4_FMT];
#endif
struct sockaddr_un un;
static stralloc localhostsa;
static char    *localhost;
static uint16   remoteport;
static char     remoteportstr[FMT_ULONG];
static stralloc remotehostsa;
static char    *remotehost;

static char     strnum1[FMT_ULONG];
static char     strnum2[FMT_ULONG];

static stralloc tmp;
static stralloc fqdn;
static stralloc addresses;
#ifdef TLS
static stralloc tls_server_version, tls_client_version;
#endif

static int      flag1;
static char     bspace[16];
static substdio b;
static stralloc limitFile;

static uid_t    uid = -1;
static gid_t    gid = -1;
static char    *user = NULL;

struct iptable
{
	char            ipaddr[16];
	pid_t           pid;
};
typedef struct iptable IPTABLE;
static IPTABLE **IpTable;
static c_char      *af_unix, *pidfile;

void            add_ip(pid_t);
void            print_ip();
unsigned long   check_ip();
void            remove_ip(pid_t);
int             socket_ipoptionskill(int);
int             socket_ip6optionskill(int);
int             socket_tcpnodelay(int);
int             tcpserver_plugin(char **, int);
extern char   **environ;

/*---------------------------- child */

#define DROP  "tcpserver: warning: dropping connection, "

static int      flagdeny;
static int      flagallownorules;
static char    *fnrules;

no_return void
drop_nomem(void)
{
	strerr_die2sys(111, DROP, "out of memory");
}

void
cats(const char *s)
{
	if (!stralloc_cats(&tmp, s))
		drop_nomem();
}

void
append(const char *ch)
{
	if (!stralloc_append(&tmp, ch))
		drop_nomem();
}

void
safecats(const char *s)
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
env(const char *s, const char *t)
{
	if (str_equal(s, "MAXPERIP"))
		scan_ulong(t, &maxperip);
	if (!upathexec_env(s, t))
		drop_nomem();
}

no_return void
drop_rules(void)
{
	strerr_die4sys(111, DROP, "unable to read ", fnrules, ": ");
}

void
found(char *data, unsigned int datalen)
{
	unsigned int    next0, split;

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
doit_unix(int t, const char *hostname)
{
	str_copyb(remoteipstr, "0.0.0.0", 8);
	if (verbosity >= 2) {
		strnum1[fmt_ulong(strnum1, getpid())] = 0;
		strerr_warn4("tcpserver: pid ", strnum1, " from ", hostname, 0);
	}
	if (*banner) {
		substdio_fdbuf(&b, (ssize_t (*)(int,  char *, size_t)) write, t, bspace, sizeof bspace);
		if (substdio_puts(&b, banner) == -1)
			strerr_die2sys(111, DROP, "unable to print banner: ");
	}
	env("PROTO", "UNIX");
	env("UNIXLOCALPATH", hostname);
	if (verbosity >= 2) {
		if (!stralloc_copys(&tmp, "tcpserver: ok "))
			drop_nomem();
		strnum1[fmt_ulong(strnum1, getpid())] = 0;
		safecats(strnum1);
		cats(" ");
		cats("[");
		safecats(hostname);
		cats("]\n");
		substdio_putflush(subfderr, tmp.s, tmp.len);
	}
}

void
doit_tcp(int t, const char *hostname)
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
		strnum1[fmt_ulong(strnum1, getpid())] = 0;
		strerr_warn4("tcpserver: pid ", strnum1, " from ", remoteipstr, 0);
	}
	if (flagkillopts) {
		socket_ipoptionskill(t);
		socket_ip6optionskill(t);
	}
	if (!flagdelay)
		socket_tcpnodelay(t);
	if (*banner) {
		substdio_fdbuf(&b, (ssize_t (*)(int,  char *, size_t)) write, t, bspace, sizeof bspace);
		if (substdio_puts(&b, banner) == -1)
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
		if (remoteinfo6(&tcpremoteinfo, remoteip, remoteport, localip, localport, itimeout, netif) == -1)
#else
		if (remoteinfo(&tcpremoteinfo, remoteip, remoteport, localip, localport, itimeout) == -1)
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
		strnum1[fmt_ulong(strnum1, getpid())] = 0;
		if (!stralloc_copys(&tmp, "tcpserver: "))
			drop_nomem();
		safecats(flagdeny ? "deny" : "ok");
		cats(" ");
		safecats(strnum1);
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
		strnum1[fmt_ulong(strnum1, maxperip)] = 0;
		safecats(strnum1);
		cats("\n");
		substdio_putflush(subfderr, tmp.s, tmp.len);
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

static const char *
my_strchr(const char *str, char ch)
{
	int             i;

	i = str_chr(str, ch);
	if (!str[i])
		return ((char *) 0);
	return (str + i);
}

int
matchinet(const char *ip, const char *token)
{
	char            field1[8], field2[8];
	char           *ptr, *ptr1, *ptr2, *cptr;
	unsigned long   lnum, hnum, t;
	int             idx1, idx2, match;
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
	/*-
	 * If our token is a valid internet address then we don't need to
	 * check any further
	 * if (inet_aton(token, &inp))
	 */
	if (inet_addr(token) != INADDR_NONE)
		return (0);
	else
	for (match = idx1 = 0, ptr1 = (char *) token, ptr2 = (char *) ip; idx1 < 4 && match == idx1; idx1++) {
		/*- IP Address in control file */
		for (cptr = field1; *ptr1 && *ptr1 != '.'; *cptr++ = *ptr1++);
		*cptr = 0;
		ptr1++;

		/*- IP Address of client */
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
		/*-
		 * Network address wildcard match (i.e.
		 * "192.86.2?.12?")
		 */
		for (; (ptr = (char *) my_strchr(field1, '?'));) {
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
		if ((ptr = (char *) my_strchr(field1, '-'))) {
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
		hints.ai_socktype = SOCK_STREAM;
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
				if (!inet_ntop(AF_INET6, (void *) &in6->sin6_addr, addrBuf, INET6_ADDRSTRLEN))
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

struct stralloc dbserver;
struct stralloc dbuser;
struct stralloc dbpass;
struct stralloc dbname;
struct stralloc dbtable;
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
connect_db(const char *dbfile)
{
	char           *x = NULL;
	const char     *m_timeout;
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

no_return void
usage(void)
{
	strerr_warn1(
		 "usage: tcpserver\n"
#ifdef IPV6
#ifdef TLS
#ifdef SSL_OP_ALLOW_CLIENT_RENEGOTIATION
		 "[ -461UXpPhHrRoOdDqQvsNz ]\n"
#else
		 "[ -461UXpPhHrRoOdDqQvsz ]\n"
#endif
#else
		 "[ -461UXpPhHrRoOdDqQv ]\n"
#endif
#else
#ifdef TLS
		 "[ -1UXpPhHrRoOdDqQvsNz ]\n"
#else
		 "[ -1UXpPhHrRoOdDqQv ]\n"
#endif
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
		 "[ -T timeoutidle ]\n"
#if defined(HAS_MYSQL)
		 "[ -m db.conf ]\n"
#endif
#ifdef TLS
		 "[ -i certdir ]\n"
		 "[ -n certfile ]\n"
		 "[ -f cipherlist ]\n"
		 "[ -M TLS method ]\n"
		 "[ -a cafile ] \n"
		 "[ -L crlfile ] \n"
#endif
#ifdef IPV6
		 "[ -I interface ]\n"
#endif
		 "host port program", 0);
	_exit(100);
}

void
printstatus(const char *str, pid_t pid, pid_t childpid)
{
	int             i;
#ifdef IPV6
	int             fakev4 = 0;
#endif

	if (verbosity < 2)
		return;
	strnum1[fmt_ulong(strnum1, numchildren)] = 0;
	strnum2[fmt_ulong(strnum2, limit)] = 0;
	if (!stralloc_copys(&tmp, "tcpserver: "))
		drop_nomem();
	cats("status: ");
	cats(strnum1);
	append("/");
	cats(strnum2);
	append(" ");
	cats(str);
	if (flagssl && ssl) {
		strnum1[fmt_ulong(strnum1, pid)] = 0;
		strnum2[fmt_ulong(strnum2, childpid)] = 0;
		cats(", sslpid=");
		cats(strnum1);
		if (childpid != -1) {
			cats(", childpid=");
			cats(strnum2);
		}
	} else {
		strnum1[fmt_ulong(strnum1, pid)] = 0;
		cats(", pid=");
		cats(strnum1);
	}
	if (!str_diffn(str, "startup", 8)) {
		cats(flagssl ? ", TLS=enabled" : ", TLS=disabled");
#ifdef HAS_MYSQL
		cats(use_sql ? ", use sql=YES\n" : ", use sql=NO\n");
#else
		cats(", use sql=NO\n");
#endif
		substdio_putflush(subfderr, tmp.s, tmp.len);
		return;
	} else
	if (str_diffn(str, "shutdown", 9) && str_diffn(str, "sigchild", 9)) {
		cats(", IP ");
#ifdef IPV6
		if (!forcev6 && ip6_isv4mapped(remoteip))
			fakev4 = 1;
		safecats((fakev4 || (noipv6 && !forcev6)) ? remoteipstr : remoteipstr6);
#else
		safecats(remoteipstr);
#endif
	}
	if (flagssl && ssl) {
		cats(", TLS Server=");
		safecats(tls_server_version.s);
		cats(", Client=");
		safecats(tls_client_version.s);
		cats(", Ciphers=");
		safecats(SSL_CIPHER_get_name(SSL_get_current_cipher(ssl)));
		cats(", bits=");
		SSL_get_cipher_bits(ssl, &i);
		strnum1[fmt_ulong(strnum1, i)] = 0;
		safecats(strnum1);
	}
	append("\n");
	substdio_putflush(subfderr, tmp.s, tmp.len);
	return;
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
add_ip(pid_t pid)
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
remove_ip(pid_t pid)
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

no_return void
sigterm(int i)
{
	if (verbosity >= 2)
		printstatus("shutdown", getpid(), -1);
	if (af_unix)
		unlink(af_unix);
	_exit(0);
}

void
siguser1(int i)
{
	print_ip();
}

void
sighangup(int x)
{
	int             i, tmpLimit;
	IPTABLE       **iptable1, **iptable2, **tmpTable;
	char            tmp1[FMT_ULONG], tmp2[FMT_ULONG];

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
	tmp1[fmt_ulong(tmp1, numchildren)] = 0;
	tmp2[fmt_ulong(tmp2, limit)] = 0;
	strerr_warn4("tcpserver: sighup: ", tmp1, "/", tmp2, 0);
}

void
sigchld(int x)
{
	int             i, wstat, pid;
	char            tmp1[FMT_ULONG], tmp2[FMT_ULONG];

	while ((pid = wait_nohang(&wstat)) > 0) {
		if (verbosity >= 2) {
			tmp1[fmt_ulong(tmp1, pid)] = 0;
			if (wait_stopped(wstat) || wait_continued(wstat)) {
				i = wait_stopped(wstat) ? wait_stopsig(wstat) : SIGCONT;
				tmp2[fmt_ulong(tmp2, i)] = 0;
				strerr_warn4("tcpserver: end ", tmp1, wait_stopped(wstat) ? " stopped by signal " : " continued by signal ", tmp2, 0);
			} else
			if (wait_signaled(wstat)) {
				i = wait_termsig(wstat);
				tmp2[fmt_ulong(tmp2, i)] = 0;
				strerr_warn4("tcpserver: end ", tmp1, " killed by signal ", tmp2, 0);
			} else
			if ((i = wait_exitcode(wstat))) {
				tmp2[fmt_ulong(tmp2, i)] = 0;
				strerr_warn4("tcpserver: end ", tmp1, " status ", tmp2, 0);
			}
		}
		if (numchildren) {
			--numchildren;
			remove_ip(pid);
		}
		printstatus("sigchild", pid, -1);
	}
}

#ifdef TLS
void
write_provider_data(SSL *myssl, int writefd, int readfd)
{
	int             i;
	ssize_t         n;

	SSL_get_cipher_bits(myssl, &i);
	strnum1[n = fmt_ulong(strnum1, i)] = 0;
	if (!stralloc_copyb(&tmp, "TLS Server=", 11) ||
			!stralloc_cat(&tmp, &tls_server_version) ||
			!stralloc_catb(&tmp, ", Client=", 9) ||
			!stralloc_cat(&tmp, &tls_client_version) ||
			!stralloc_catb(&tmp, ", Ciphers=", 10) ||
			!stralloc_cats(&tmp, SSL_CIPHER_get_name(SSL_get_current_cipher(myssl))) ||
			!stralloc_catb(&tmp, ", bits=", 7) ||
			!stralloc_catb(&tmp, strnum1, n))
		strerr_die2x(111, FATAL, "out of memory");
	i = tmp.len;
	if (write(writefd, (char *) &i, sizeof(int)) == -1)
		strerr_die2sys(111, FATAL, "unable to write provider data length: ");
	if (write(writefd, tmp.s, tmp.len) == -1)
		strerr_die2sys(111, FATAL, "unable to write provider data: ");
	if ((n = timeoutread(5, readfd, (char *) &i, 1)) == -1)
		strerr_die2sys(111, FATAL, "unable to read provider ACK: ");
	return;
}

void
read_provider_data(stralloc *t, int readfd, int writefd)
{
	int             i;
	ssize_t         n;

	if ((n = timeoutread(dtimeout, readfd, (char *) &i, sizeof(int))) == -1)
		strerr_die2sys(111, FATAL, "unable to read provider data length: ");
	if (!stralloc_ready(t, t->len + i))
		strerr_die2x(111, FATAL, "out of memory");
	if ((n = timeoutread(dtimeout, readfd, t->s + t->len, i)) == -1)
		strerr_die2sys(111, FATAL, "unable to read provider data: ");
	else
	if (n)
		t->len += i;
	if ((n = timeoutwrite(dtimeout, writefd, (char *) &i, 1)) == -1)
		strerr_die2sys(111, FATAL, "unable to write provider ACK: ");
	return;
}
#endif

int
check_pid(const char *fn)
{
	int             i, j;

	if ((i = open_read(fn)) == -1) {
		if (af_unix && !access(af_unix, F_OK) && unlink(af_unix) == -1)
			strerr_die4sys(111, FATAL, "couldn't remove ", af_unix, ": ");
		close(i);
	} else {
		j = read(i, strnum1, FMT_ULONG);
		close(i);
		if (j < FMT_ULONG) {
			strnum1[j] = 0;
			scan_int(strnum1, &j);
			if (!kill(j, 0))
				return 1;
		}
	}
	if (af_unix && !access(af_unix, F_OK) && unlink(af_unix) == -1)
		strerr_die4sys(111, FATAL, "couldn't remove ", af_unix, ": ");
	if ((i = open_trunc(fn)) == -1)
		strerr_die4sys(111, FATAL, "couldn't open ", fn, " for write: ");
	j = fmt_ulong(strnum1, getpid());
	if (write(i, strnum1, j) != j)
		strerr_die4sys(111, FATAL, "couldn't open ", fn, " for write: ");
	close(i);
	return 0;
}

int
main(int argc, char **argv, char **envp)
{
	const char     *x, *hostname, *groups = NULL;
#if defined(HAS_MYSQL)
	const char     *dbfile = NULL;
#endif
	struct servent *se;
	unsigned long   port, ipcount = -1;
	int             sfd, asd, opt, tmpLimit;
	pid_t           pid;
#ifdef IPV6
	int             fakev4 = 0;
#ifdef FREEBSD
	int              s4, s6, r;
	fd_set           rfds;
#endif
#endif
#ifdef TLS
	SSL_CTX        *ctx = NULL;
	const char     *certdir = NULL, *cipherfile = NULL;
	char           *ciphers = NULL, *tls_method = NULL;
	int             pi2c[2], pi4c[2], method;
	int             provide_data = 0;
	pid_t           child_pid;
#ifdef SSL_OP_ALLOW_CLIENT_RENEGOTIATION
	int             client_renegotiation = 0;
#endif
	struct stat     st;
#endif
	struct stralloc options = {0};
	struct linger   cflinger;
	int             i = 1;

	if ((x = env_get("MAXPERIP"))) { /*- '-C' option overrides this */
		scan_ulong(x, &PerHostLimit);
		maxperip = PerHostLimit;
	}
	/*
	 * unused options
	 * 0, 2, 3, 5, 7, 8, 9, E, F, G, j, J, k, K, w, W, y, Y, Z
	 */
	if (!stralloc_copys(&options, "dDe:vqQhHrR1UXx:m:M:t:T:u:g:l:b:B:c:C:pPoO"))
		strerr_die2x(111, FATAL, "out of memory");
#ifdef IPV6
	if (!stralloc_cats(&options, "46I:"))
		strerr_die2x(111, FATAL, "out of memory");
#endif
#ifdef TLS
#ifdef SSL_OP_ALLOW_CLIENT_RENEGOTIATION
	if (!stralloc_cats(&options, "sNn:a:f:M:i:L:S:z"))
#else
	if (!stralloc_cats(&options, "sn:a:f:M:i:L:S:z"))
#endif
		strerr_die2x(111, FATAL, "out of memory");
#endif
	if (!stralloc_0(&options))
		strerr_die2x(111, FATAL, "out of memory");
	while ((opt = getopt(argc, argv, options.s)) != opteof) {
		switch (opt)
		{
		case 'e':
			pidfile = optarg;
			break;
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
			scan_ulong(optarg, &itimeout);
			break;
		case 'T':
			scan_ulong(optarg, &dtimeout);
			break;
		case 'U':
			if ((x = env_get("UID")))
				scan_uint(x, &uid);
			if ((x = env_get("GID")))
				scan_uint(x, &gid);
			break;
		case 'u':
			if (!isnum(optarg))
				user = optarg;
			else
				scan_uint(optarg, &uid);
			break;
		case 'g':
			groups = optarg;
			if (isnum(optarg))
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
#ifdef SSL_OP_ALLOW_CLIENT_RENEGOTIATION
		case 'N':
			client_renegotiation = 1;
			break;
#endif
		case 'n':
			flagssl = 1;
			if (*optarg && (!stralloc_copys(&certfile, optarg) || !stralloc_0(&certfile)))
				strerr_die2x(111, FATAL, "out of memory");
			break;
		case 'a':
			if (*optarg && (!stralloc_copys(&cafile, optarg) || !stralloc_0(&cafile)))
				strerr_die2x(111, FATAL, "out of memory");
			break;
		case 'L':
			if (*optarg && (!stralloc_copys(&crlfile, optarg) || !stralloc_0(&crlfile)))
				strerr_die2x(111, FATAL, "out of memory");
			break;
		case 'f':
			cipherfile = optarg;
			break;
		case 'M':
			tls_method = optarg;
			break;
		case 'i':
			certdir = optarg;
			break;
		case 'S':
			scan_ulong(optarg, &ctimeout);
			break;
		case 'z':
			provide_data = 1;
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
		} /*- switch (opt) */
	} /* while ((opt = getopt(argc, argv, options.s)) != opteof) */
	argc -= optind;
	argv += optind;
	if (!verbosity)
		subfderr->fd = -1;
	if (!(hostname = *argv++))
		usage();
	if (str_equal(hostname, ""))
		hostname = "0";
	if (*hostname == '/')
		af_unix = hostname;
	if (!af_unix) {
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
	}
	if (!*argv)
		usage();
	if (pidfile && check_pid(pidfile))
		strerr_die2x(111, FATAL, " already running");
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

	if (af_unix)
		goto do_socket;
	if (str_equal(hostname, "0") || str_equal(hostname, "::"))
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

do_socket:
	cflinger.l_onoff = 1;
	cflinger.l_linger = 60;
#ifdef TLS
	if (!certdir && !(certdir = env_get("CERTDIR")))
		certdir = "/etc/indimail/certs";
	set_certdir(certdir);
	if (flagssl && !certfile.len) {
		if (!(x = env_get("TLS_CERTFILE")))
			x = "servercert.pem";
		if (*x != '.' && *x != '/') {
			if (!stralloc_copys(&certfile, certdir) ||
					!stralloc_append(&certfile, "/"))
				strerr_die2x(111, FATAL, "out of memory");
		}
		if (!stralloc_cats(&certfile, x) ||
				!stralloc_0(&certfile))
			strerr_die2x(111, FATAL, "out of memory");
	}
	if (!cafile.len) {
		if (!(x = env_get("CLIENTCA")))
			x = "clientca.pem";
		if (*x != '.' && *x != '/') {
			if (!stralloc_copys(&cafile, certdir) ||
					!stralloc_append(&cafile, "/"))
				strerr_die2x(111, FATAL, "out of memory");
		}
		if (!stralloc_cats(&cafile, x) ||
				!stralloc_0(&cafile))
			strerr_die2x(111, FATAL, "out of memory");
		if (access(cafile.s, R_OK))
			cafile.len = 0;
	}
	if (!crlfile.len) {
		if (!(x = env_get("CLIENTCRL")))
			x = "clientcrl.pem";
		if (*x != '.' && *x != '/') {
			if (!stralloc_copys(&crlfile, certdir) ||
					!stralloc_append(&crlfile, "/"))
				strerr_die2x(111, FATAL, "out of memory");
		}
		if (!stralloc_cats(&crlfile, x) ||
				!stralloc_0(&crlfile))
			strerr_die2x(111, FATAL, "out of memory");
		if (access(crlfile.s, R_OK))
			crlfile.len = 0;
	}
	if (flagssl == 1) {
    	/* setup SSL context (load key and cert into ctx) */
		if (cipherfile) {
			if (lstat(cipherfile, &st) == -1)
				strerr_die4sys(111, FATAL, "lstat: ", cipherfile, ": ");
			if (openreadclose(cipherfile, &saciphers, st.st_size) == -1)
				strerr_die3sys(111, FATAL, cipherfile, ": ");
			if (saciphers.s[saciphers.len - 1] == '\n')
				saciphers.s[saciphers.len - 1] = 0;
			else
			if (!stralloc_0(&saciphers))
				strerr_die2x(111, FATAL, "out of memory");
			for (ciphers = saciphers.s; *ciphers; ciphers++)
				if (isspace(*ciphers)) {
					*ciphers = 0;
					break;
				}
			ciphers = saciphers.s;
		} else
		if (!ciphers) {
			method = get_tls_method(tls_method);
			ciphers = env_get(method < 7 ? "TLS_CIPHER_LIST" : "TLS_CIPHER_SUITE");
		}
		if (!(ctx = tls_init(tls_method, certfile.s,
				cafile.len ? cafile.s : NULL, crlfile.len ? crlfile.s : NULL,
				ciphers, server)))
			strerr_die2x(111, FATAL, "unable to initialize TLS");
#ifdef SSL_OP_ALLOW_CLIENT_RENEGOTIATION
		if (client_renegotiation)
			SSL_CTX_set_options(ctx, SSL_OP_ALLOW_CLIENT_RENEGOTIATION);
#endif
	}
#endif

	if (af_unix) {
		if ((sfd = socket_unix()) == -1)
			strerr_die4sys(111, FATAL, "unable to create socket ", hostname, ": ");
		if (socket_bindun(sfd, hostname) == -1)
			strerr_die4sys(111, FATAL, "unable to bind to ", hostname, ": ");
		if (socket_listen(sfd, backlog) == -1)
			strerr_die4sys(111, FATAL, "unable to listen on socket  ", hostname, ": ");
		ndelay_off(sfd);
	} else {
#ifdef IPV6
#ifdef FREEBSD
		if ((s6 = socket_tcp6()) == -1)
#else
		if ((sfd = socket_tcp6()) == -1)
#endif
#else
		if ((sfd = socket_tcp()) == -1)
#endif
			strerr_die2sys(111, FATAL, "unable to create socket: ");

#if defined(IPV6) && defined(FREEBSD)
		if (setsockopt(s6, SOL_SOCKET, SO_LINGER, (char *) &cflinger, sizeof (struct linger)) == -1)
#else
		if (setsockopt(sfd, SOL_SOCKET, SO_LINGER, (char *) &cflinger, sizeof (struct linger)) == -1)
#endif
			strerr_die4sys(111, FATAL, "unable to do setsockopt SO_LINGER ", hostname, ": ");

#if defined(IPV6) && defined(FREEBSD)
		if (setsockopt(s6, SOL_SOCKET, SO_REUSEADDR, (char *) &i, sizeof(i)) < 0)
#else
		if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, (char *) &i, sizeof(i)) < 0)
#endif
			strerr_die4sys(111, FATAL, "unable to do set SO_REUSEADDR ", hostname, ": ");

#ifdef IPV6
#ifdef FREEBSD
		if (socket_bind6(s6, localip, localport, netif) == -1)
#else
		if (socket_bind6(sfd, localip, localport, netif) == -1)
#endif
#else
		if (socket_bind4(sfd, localip, localport) == -1)
#endif
			strerr_die2sys(111, FATAL, "unable to bind: ");

#ifdef IPV6
#ifdef FREEBSD
		if (socket_local6(s6, localip, &localport, &netif) == -1)
#else
		if (socket_local6(sfd, localip, &localport, &netif) == -1)
#endif
#else
		if (socket_local4(sfd, localip, &localport) == -1)
#endif
			strerr_die2sys(111, FATAL, "unable to get local address: ");

#if defined(IPV6) && defined(FREEBSD)
		if (socket_listen(s6, backlog) == -1)
#else
		if (socket_listen(sfd, backlog) == -1)
#endif
			strerr_die2sys(111, FATAL, "unable to listen: ");
#if defined(IPV6) && defined(FREEBSD)
		ndelay_off(s6);
#else
		ndelay_off(sfd);
#endif
#if defined(IPV6) && defined(FREEBSD)
		/*
		 * By default, FreeBSD does not route IPv4 traffic to AF_INET6 sockets. The
		 * default behavior intentionally violates RFC2553 for security reasons.
		 * Listen to two sockets if you want to accept both IPv4 and IPv6 traffic.
		 * IPv4 traffic may be routed with certain per-socket/per-node configura-
		 * tion, however, it is not recommended to do so. Consult ip6(4) for
		 * details.
		 */
		if (!noipv6) {
			noipv6 = 1;
			if ((s4 = socket_tcp6()) == -1)
				strerr_die2sys(111, FATAL, "unable to create socket: ");
			i = 1;
			if (setsockopt(s4, SOL_SOCKET, SO_REUSEADDR, (char *) &i, sizeof(i)) < 0)
				strerr_die4sys(111, FATAL, "unable to do set SO_REUSEADDR ", hostname, ": ");
			if (socket_bind6(s4, localip, localport, netif) == -1)
				strerr_die2sys(111, FATAL, "unable to bind: ");
			if (socket_local6(s4, localip, &localport, &netif) == -1)
				strerr_die2sys(111, FATAL, "unable to get local address: ");
			if (socket_listen(s4, backlog) == -1)
				strerr_die2sys(111, FATAL, "unable to listen: ");
			ndelay_off(s4);
			noipv6 = 0;
		}
#endif /*- #if defined(IPV6) && defined(FREEBSD) */
	} /* if (not af_unix) */

	if (user) {
		if (setuserid(user, 1, groups) == -1)
			strerr_die2sys(111, FATAL, "unable to set user: ");
	} else {
		if (gid == -1) {
			if (groups && set_additional_groups(groups, 1) == -1)
				strerr_die2sys(111, FATAL, "unable to add additional groups: ");
		} else
		if (prot_gid(gid) == -1)
			strerr_die2sys(111, FATAL, "unable to set gid: ");
		if (uid != -1 && prot_uid(uid) == -1)
			strerr_die2sys(111, FATAL, "unable to set uid: ");
	}
	if (tcpserver_plugin(envp, 1))
		_exit(111);
	if (!af_unix)
		localportstr[fmt_ulong(localportstr, localport)] = 0;
	if (flag1 && !af_unix) {
		substdio_fdbuf(&b, (ssize_t (*)(int,  char *, size_t)) write, 1, bspace, sizeof bspace);
		substdio_puts(&b, localportstr);
		substdio_puts(&b, "\n");
		substdio_flush(&b);
	}
	close(0);
	close(1);
	printstatus("startup", getpid(), -1);
	for (;;) {
		while (numchildren >= limit)
			sig_pause();
		sig_unblock(sig_child);
		if (af_unix)
			asd = socket_acceptun(sfd, &un);
		else {
#if defined(IPV6) && defined(FREEBSD)
			while (1) {
				FD_ZERO(&rfds);
				FD_SET(s6, &rfds);
				FD_SET(s4, &rfds);
				if ((r = select(s4 + 1, &rfds, 0, 0, 0)) == -1) {
					if (errno == error_intr)
						continue;
					else
						strerr_die2sys(111, FATAL, "select: ");
				}
				if (r) {
					if (FD_ISSET(s6, &rfds))
						sfd = s6;
					else
					if (FD_ISSET(s4, &rfds))
						sfd = s4;
					break;
				}
			}
#endif
#ifdef IPV6
			asd = socket_accept6(sfd, remoteip, &remoteport, &netif);
#else
			asd = socket_accept4(sfd, remoteip, &remoteport);
#endif
		} /* if (!af_unix) */
		sig_block(sig_child);
		if (asd == -1)
			continue;
		++numchildren;
		if (!af_unix) {
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
		}
		if (PerHostLimit && (ipcount = check_ip()) >= PerHostLimit) {
			strnum2[fmt_ulong(strnum2, ipcount)] = 0;
			strerr_warn4("tcpserver: end ", remoteipstr, " perIPlimit ", strnum2, 0);
			--numchildren;
			errno = error_acces;
			close(asd);
			continue;
		}
		switch ((pid = fork()))
		{
		case -1:
			strerr_warn2(DROP, "unable to fork: ", &strerr_sys);
			--numchildren;
			printstatus("Failure", getpid(), -1);
		case 0:
			close(sfd);
#if defined(HAS_MYSQL)
			if (use_sql && conn)
				check_db(conn);
#endif
			(af_unix ? doit_unix : doit_tcp) (asd, hostname); /*- MAXPERIP can only be checked after this */
			if (fd_move(0, asd) == -1 || fd_copy(1, 0) == -1)
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
					errno = error_acces;
					strerr_die6sys(111, DROP, af_unix ? "no slots for " : "no ip slots for ",
							af_unix ? af_unix : remoteipstr, ", perIPlimit ", strnum2, ": ");
				}
			}
#ifdef TLS
			if (flagssl == 1) {
				if (pipe(pi2c) != 0)
					strerr_die2sys(111, DROP, "unable to create pipe: ");
				if (pipe(pi4c) != 0)
					strerr_die2sys(111, DROP, "unable to create pipe: ");
				switch ((child_pid = fork()))
				{
				case 0:
					SSL_CTX_free(ctx);
					close(0);
					close(1);
					if (!stralloc_copyb(&tmp, "tcpserver, ", 11))
						strerr_die2x(111, FATAL, "out of memory");
					if (provide_data)
						read_provider_data(&tmp, pi4c[0], pi2c[1]);
					if (!stralloc_0(&tmp))
						strerr_die2x(111, FATAL, "out of memory");
					if (!env_put2("TLS_PROVIDER", tmp.s))
						strerr_die2x(111, FATAL, "out of memory");
					close(pi2c[1]);
					close(pi4c[0]);
					if ((fd_move(0, pi2c[0]) == -1) || (fd_move(1, pi4c[1]) == -1))
						strerr_die2sys(111, DROP, "unable to set up descriptors: ");
					/*- signals are allready set in the parent */
					upathexec(argv);
					strerr_die4sys(111, DROP, "unable to run ", *argv, ": ");
				case -1:
					strerr_die2sys(111, DROP, "unable to fork: ");
				default:
					break;
				} /*- switch fork() */
				if (!(ssl = tls_session(ctx, 0))) {
					SSL_CTX_free(ctx);
					strerr_die2x(111, DROP, "unable to setup SSL session");
				}
				if (!stralloc_copys(&tls_server_version, SSL_get_version(ssl)) ||
						!stralloc_0(&tls_server_version))
					strerr_die2x(111, FATAL, "out of memory");
				tls_server_version.len--;
				SSL_CTX_free(ctx);
				ctx = NULL;
				if (tls_accept(ctimeout, 0, 1, ssl))
					strerr_die2x(111, DROP, "unable to accept SSL connection");
				if (!stralloc_copys(&tls_client_version, SSL_get_version(ssl)) ||
						!stralloc_0(&tls_client_version))
					strerr_die2x(111, FATAL, "out of memory");
				printstatus("encrypted session", getpid(), child_pid);
				tls_client_version.len--;
				if (provide_data)
					write_provider_data(ssl, pi4c[1], pi2c[0]);
				close(pi2c[0]);
				close(pi4c[1]);
				translate(0, 1, pi2c[1], pi4c[0], dtimeout);
				ssl_free();
				_exit(0);
			} else
				printstatus("un-encrypted session", getpid(), -1);
#else
			printstatus("un-encrypted session", getpid(), -1);
#endif
			upathexec(argv);
			strerr_die4sys(111, DROP, "unable to run ", *argv, ": ");
		default:
			add_ip(pid);
		} /*- switch (pid = fork()) */
		close(asd);
	} /*- for (;;) */
}

void
getversion_tcpserver_c()
{
	const char    *x = "$Id: tcpserver.c,v 1.96 2025-01-21 23:53:43+05:30 Cprogrammer Exp mbhangui $";

	x++;
}

/*
 * $Log: tcpserver.c,v $
 * Revision 1.96  2025-01-21 23:53:43+05:30  Cprogrammer
 * Fixes for gcc14 errors
 *
 * Revision 1.95  2024-10-05 22:56:00+05:30  Cprogrammer
 * use INET6_ADDRSTRLEN for AF_INET6
 *
 * Revision 1.94  2024-05-19 08:57:35+05:30  Cprogrammer
 * fixed typo
 *
 * Revision 1.93  2024-05-09 22:55:54+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.92  2023-11-26 18:28:05+05:30  Cprogrammer
 * fixed TCPLOCALPORT not getting set
 *
 * Revision 1.91  2023-09-27 19:37:36+05:30  Cprogrammer
 * skip IP display in printstatus for sigchild
 *
 * Revision 1.90  2023-09-14 21:28:10+05:30  Cprogrammer
 * display pid, ip in printstatus
 *
 * Revision 1.89  2023-08-20 15:17:17+05:30  Cprogrammer
 * use TLS_CIPHER_LIST, TLS_CIPHER_SUITE to set ciphers
 *
 * Revision 1.88  2023-06-18 13:24:32+05:30  Cprogrammer
 * handle UNIX sockets
 *
 * Revision 1.87  2023-04-08 23:12:44+05:30  Cprogrammer
 * fixed status not getting printed in logs
 *
 * Revision 1.86  2023-02-20 18:45:33+05:30  Cprogrammer
 * added ability to set additional group ids
 *
 * Revision 1.85  2023-02-13 22:39:06+05:30  Cprogrammer
 * allow both user and uid to be passed to -u option
 *
 * Revision 1.84  2023-02-13 20:20:19+05:30  Cprogrammer
 * added error message for tls_init failure
 *
 * Revision 1.83  2023-01-11 08:19:41+05:30  Cprogrammer
 * added -N option to allow client side renegotiation
 *
 * Revision 1.82  2023-01-03 20:43:31+05:30  Cprogrammer
 * replace internal TLS function with TLS functions from libqmail
 * added -z option to turn on setting of TLS_PROVIDER env variable
 * added -S option for connection timeout
 *
 * Revision 1.81  2022-12-27 07:45:30+05:30  Cprogrammer
 * set flagssl only if -n option is specified
 * set TLS_PROVIDER env variable
 * use TLS_CERTFILE for default servercert
 *
 * Revision 1.80  2022-12-24 22:15:11+05:30  Cprogrammer
 * added -i option to specify certdir
 * set RSA/DH parameters
 *
 * Revision 1.79  2022-12-23 16:16:04+05:30  Cprogrammer
 * use ssl_free() to shutdown ssl
 *
 * Revision 1.78  2022-12-23 10:36:13+05:30  Cprogrammer
 * added -M option to set TLS / SSL client/server method
 *
 * Revision 1.77  2022-12-22 22:19:31+05:30  Cprogrammer
 * added -f option to load tls ciphers from a file
 *
 * Revision 1.76  2021-08-30 12:47:59+05:30  Cprogrammer
 * define functions as noreturn
 *
 * Revision 1.75  2021-06-15 08:24:25+05:30  Cprogrammer
 * renamed pathexec.. functions to upathexec to avoid clash with libqmail
 *
 * Revision 1.74  2021-03-12 14:05:48+05:30  Cprogrammer
 * use typedef my_ulong instead of ulong
 *
 * Revision 1.73  2021-03-12 13:55:08+05:30  Cprogrammer
 * use MYSQL_CONFIG for conditional compilation of mysql code
 *
 * Revision 1.72  2021-03-09 08:30:40+05:30  Cprogrammer
 * change in translate() function.
 *
 * Revision 1.71  2021-03-07 08:30:07+05:30  Cprogrammer
 * use common tls functions from tls.c
 * added idle timeout parameter
 *
 * Revision 1.70  2020-11-27 17:34:22+05:30  Cprogrammer
 * added option to specify CA file
 *
 * Revision 1.69  2020-11-11 19:50:11+05:30  Cprogrammer
 * changed scope of global veriables limited to tcpserver.c
 *
 * Revision 1.68  2020-09-20 10:55:06+05:30  Cprogrammer
 * open ipv4 and ipv6 sockets on FreeBSD
 *
 * Revision 1.67  2020-09-19 17:35:36+05:30  Cprogrammer
 * treat IPV6 :: same as 0
 *
 * Revision 1.66  2020-09-16 20:50:32+05:30  Cprogrammer
 * fixed compiler warnings
 *
 * Revision 1.65  2020-08-03 17:28:15+05:30  Cprogrammer
 * replaced buffer with substdio
 *
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
 * print receipt of sighup on descriptor 2
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
