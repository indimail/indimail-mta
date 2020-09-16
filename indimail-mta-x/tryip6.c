/*
 * $Log: tryip6.c,v $
 * Revision 1.3  2020-09-16 19:08:44+05:30  Cprogrammer
 * fix compiler warning for FreeBSD
 *
 * Revision 1.2  2011-07-29 09:30:15+05:30  Cprogrammer
 * fixed gcc 4.6 warnings
 *
 * Revision 1.1  2005-06-15 22:13:31+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

int
main()
{
#ifdef IPV6
	struct sockaddr_in6 sa;
	sa.sin6_family = PF_INET6;
	if (sa.sin6_family == PF_INET6)
		; /*- null statement to keep gcc happy */
	return 0;
#else
	return 1;
#endif
}
