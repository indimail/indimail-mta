/*
 * $Log: tcp-env.c,v $
 * Revision 1.14  2021-08-29 23:27:08+05:30  Cprogrammer
 * define functions as noreturn
 *
 * Revision 1.13  2021-07-07 08:53:42+05:30  Cprogrammer
 * removed setting of BOUNCEMAIL, WARNMAILx
 *
 * Revision 1.12  2020-11-24 13:48:38+05:30  Cprogrammer
 * removed exit.h
 *
 * Revision 1.11  2015-08-24 19:09:31+05:30  Cprogrammer
 * replace ip_fmt() with ip4_fmt()
 *
 * Revision 1.10  2005-08-23 17:39:24+05:30  Cprogrammer
 * gcc 4 compliance
 *
 * Revision 1.9  2005-06-18 11:32:20+05:30  Cprogrammer
 * ipv6 support
 *
 * Revision 1.8  2004-10-22 20:31:36+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.7  2004-10-09 23:39:24+05:30  Cprogrammer
 * removed compiler warnings
 *
 * Revision 1.6  2004-10-09 00:58:26+05:30  Cprogrammer
 * removed param.h
 *
 * Revision 1.5  2004-08-14 02:22:19+05:30  Cprogrammer
 * added SPF code
 *
 * Revision 1.4  2003-10-23 01:29:09+05:30  Cprogrammer
 * replaced strcpy with str_copy
 *
 */
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sig.h>
#include <stralloc.h>
#include <str.h>
#include <env.h>
#include <fmt.h>
#include <scan.h>
#include <subgetopt.h>
#include <byte.h>
#include <case.h>
#include <noreturn.h>

#include "socket.h"
#include "ip.h"
#include "hassalen.h"
#ifdef USE_SPF
#include "strsalloc.h"
#endif
#include "dns.h"
#include "remoteinfo.h"

static union sockunion salocal;
static union sockunion saremote;
static unsigned long   localport;
static unsigned long   remoteport;
static stralloc localname = { 0 };
static stralloc remotename = { 0 };

#ifdef IN6_IS_ADDR_V4MAPPED
void
mappedtov4(union sockunion *sa)
{
	struct sockaddr_in sin;
	struct sockaddr_in6 *sin6 = &sa->sa6;

	byte_zero((char *) &sin, sizeof(sin));
	if (sin6->sin6_family == AF_INET6 && IN6_IS_ADDR_V4MAPPED(&sin6->sin6_addr))
	{
		byte_copy((char *) &sin.sin_addr, sizeof(sin.sin_addr), (char *) sin6->sin6_addr.s6_addr + 12);
		sin.sin_port = sin6->sin6_port;
		sin.sin_family = AF_INET;
#ifdef HASSALEN
		sin.sin_len = sizeof(sin);
#endif
		byte_copy((char *) &sa->sa4, sizeof(sin), (char *) &sin);
	}
}
#else
#define mappedtov4(A)
#endif

no_return void
die()
{
	_exit(111);
}

int
main(int argc, char **argv)
{
	char           *proto;
	int             opt, flagremoteinfo, dummy;
	char            temp[IPFMT + FMT_ULONG];
	unsigned long   timeout;
#ifdef USE_SPF
	strsalloc       ssa = { 0 };
#endif

	sig_pipeignore();
	flagremoteinfo = 1;
	timeout = 30;
	while ((opt = sgopt(argc, argv, "rRt:")) != sgoptdone)
	{
		switch (opt)
		{
		case 'r':
			flagremoteinfo = 1;
			break;
		case 'R':
			flagremoteinfo = 0;
			break;
		case 't':
			scan_ulong(sgoptarg, &timeout);
			break;
		}
	}
	argv += sgoptind;
	argc -= sgoptind;
	if (argc < 1)
		die();
	if (!env_init())
		die();
	proto = env_get("PROTO");
#ifdef IPV6
	if (!proto || (str_diff(proto, "TCP") && str_diff(proto, "TCP6")))
#else
	if (!proto || str_diff(proto, "TCP"))
#endif
	{
		dummy = sizeof(salocal);
		if (getsockname(0, (struct sockaddr *) &salocal, (socklen_t *) &dummy) == -1)
			die();
		mappedtov4(&salocal);
		switch (salocal.sa.sa_family)
		{
		case AF_INET:
			if (!env_put("PROTO=TCP"))
				die();
			localport = ntohs(salocal.sa4.sin_port);
			temp[fmt_ulong(temp, localport)] = 0;
			if (!env_put2("TCPLOCALPORT", temp))
				die();
			temp[ip4_fmt(temp, (ip_addr *) &salocal.sa4.sin_addr)] = 0;
			if (!env_put2("TCPLOCALIP", temp))
				die();
#ifdef USE_SPF
			switch (dns_ptr(&ssa, (ip_addr *) &salocal.sa4.sin_addr))
#else
			switch (dns_ptr(&localname, (ip_addr *) &salocal.sa4.sin_addr))
#endif
			{
			case DNS_MEM:
				die();
			case DNS_SOFT:
				if (!stralloc_copys(&localname, "softdnserror"))
					die();
			case 0:
#ifdef USE_SPF
				if (!stralloc_copy(&localname, &ssa.sa[0]))
					die();
#endif
				if (!stralloc_0(&localname))
					die();
				case_lowers(localname.s);
				if (!env_put2("TCPLOCALHOST", localname.s))
					die();
				break;
			default:
				if (!env_unset("TCPLOCALHOST"))
					die();
			}
			break;
#ifdef IPV6
		case AF_INET6:
			if (!env_put("PROTO=TCP6"))
				die();
			localport = ntohs(salocal.sa6.sin6_port);
			temp[fmt_ulong(temp, localport)] = 0;
			if (!env_put2("TCPLOCALPORT", temp))
				die();
			if (!env_put2("TCP6LOCALPORT", temp))
				die();
			temp[ip6_fmt(temp, (ip6_addr *) &salocal.sa6.sin6_addr)] = 0;
			if (!env_put2("TCPLOCALIP", temp))
				die();
			if (!env_put2("TCP6LOCALIP", temp))
				die();
#ifdef USE_SPF
			switch (dns_ptr6(&ssa, (ip6_addr *) &salocal.sa6.sin6_addr))
#else
			switch (dns_ptr6(&localname, (ip6_addr *) &salocal.sa6.sin6_addr))
#endif
			{
			case DNS_MEM:
				die();
			case DNS_SOFT:
				if (!stralloc_copys(&localname, "softdnserror"))
					die();
			case 0:
#ifdef USE_SPF
				if (!stralloc_copy(&localname, &ssa.sa[0]))
					die();
#endif
				if (!stralloc_0(&localname))
					die();
				case_lowers(localname.s);
				if (!env_put2("TCPLOCALHOST", localname.s))
					die();
				break;
			default:
				if (!env_unset("TCPLOCALHOST"))
					die();
			}
			break;
#endif
		default:
			die();
		} /*- switch (salocal.sa.sa_family) */
		dummy = sizeof(saremote);
		if (getpeername(0, (struct sockaddr *) &saremote, (socklen_t *) &dummy) == -1)
			die();
		mappedtov4(&saremote);
		switch (saremote.sa.sa_family)
		{
		case AF_INET:
			remoteport = ntohs(saremote.sa4.sin_port);
			temp[fmt_ulong(temp, remoteport)] = 0;
			if (!env_put2("TCPREMOTEPORT", temp))
				die();
			temp[ip4_fmt(temp, (ip_addr *) &saremote.sa4.sin_addr)] = 0;
			if (!env_put2("TCPREMOTEIP", temp))
				die();
#ifdef USE_SPF
			switch (dns_ptr(&ssa, (ip_addr *) &saremote.sa4.sin_addr))
#else
			switch (dns_ptr(&remotename, (ip_addr *) &saremote.sa4.sin_addr))
#endif
			{
			case DNS_MEM:
				die();
			case DNS_SOFT:
				if (!stralloc_copys(&remotename, "softdnserror"))
					die();
			case 0:
#ifdef USE_SPF
				if (!stralloc_copy(&remotename, &ssa.sa[0]))
					die();
#endif
				if (!stralloc_0(&remotename))
					die();
				case_lowers(remotename.s);
				if (!env_put2("TCPREMOTEHOST", remotename.s))
					die();
				break;
			default:
				if (!env_unset("TCPREMOTEHOST"))
					die();
			}
			break;
#ifdef IPV6
		case AF_INET6:
			remoteport = ntohs(saremote.sa6.sin6_port);
			temp[fmt_ulong(temp, remoteport)] = 0;
			if (!env_put2("TCPREMOTEPORT", temp))
				die();
			if (!env_put2("TCP6REMOTEPORT", temp))
				die();
			temp[ip6_fmt(temp, (ip6_addr *) &saremote.sa6.sin6_addr)] = 0;
			if (!env_put2("TCPREMOTEIP", temp))
				die();
			if (!env_put2("TCP6REMOTEIP", temp))
				die();
#ifdef USE_SPF
			switch (dns_ptr6(&ssa, (ip6_addr *) &saremote.sa6.sin6_addr))
#else
			switch (dns_ptr6(&remotename, (ip6_addr *) &saremote.sa6.sin6_addr))
#endif
			{
			case DNS_MEM:
				die();
			case DNS_SOFT:
				if (!stralloc_copys(&remotename, "softdnserror"))
					die();
			case 0:
#ifdef USE_SPF
				if (!stralloc_copy(&remotename, &ssa.sa[0]))
					die();
#endif
				if (!stralloc_0(&remotename))
					die();
				case_lowers(remotename.s);
				if (!env_put2("TCPREMOTEHOST", remotename.s))
					die();
				if (!env_put2("TCP6REMOTEHOST", remotename.s))
					die();
				break;
			default:
				if (!env_unset("TCPREMOTEHOST"))
					die();
			}
			break;
#endif
		default:
			die();
		} /*- switch (saremote.sa.sa_family) */
		if (!env_unset("TCPREMOTEINFO"))
			die();
		if (flagremoteinfo)
		{
			char           *rinfo;

			rinfo = remoteinfo_get(&saremote, &salocal, (int) timeout);
			if (rinfo && !env_put2("TCPREMOTEINFO", rinfo))
				die();
		}
	}
	sig_pipedefault();
	execvp(*argv, argv);
	die();
	/*- Not reached */
	return (0);
}

void
getversion_tcp_env_c()
{
	const char     *x = "$Id: tcp-env.c,v 1.14 2021-08-29 23:27:08+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
