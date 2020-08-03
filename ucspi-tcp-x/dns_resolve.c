/*
 * $Log: dns_resolve.c,v $
 * Revision 1.2  2005-06-10 09:12:27+05:30  Cprogrammer
 * added ipv6 support
 *
 * Revision 1.1  2003-12-31 19:46:55+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <iopause.h>
#include <taia.h>
#include <byte.h>
#ifdef IPV6
#include "ip6.h"
#endif
#include "dns.h"

struct dns_transmit dns_resolve_tx = { 0 };

int
dns_resolve(char *q, char qtype[2])
{
	struct taia     stamp;
	struct taia     deadline;
#ifdef IPV6
	char            servers[256];
#else
	char            servers[64];
#endif
	iopause_fd      x[1];
	int             r;

	if (dns_resolvconfip(servers) == -1)
		return -1;
#ifdef IPV6
	if (dns_transmit_start(&dns_resolve_tx, servers, 1, q, qtype, V6any) == -1)
#else
	if (dns_transmit_start(&dns_resolve_tx, servers, 1, q, qtype, (unsigned char *) "\0\0\0\0") == -1)
#endif
		return -1;
	for (;;) {
		taia_now(&stamp);
		taia_uint(&deadline, 120);
		taia_add(&deadline, &deadline, &stamp);
		dns_transmit_io(&dns_resolve_tx, x, &deadline);
		iopause(x, 1, &deadline, &stamp);
		if((r = dns_transmit_get(&dns_resolve_tx, x, &stamp)) == -1)
			return -1;
		else
		if (r == 1)
			return 0;
	}
}
