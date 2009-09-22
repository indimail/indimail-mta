/*
 * $Log: tryip6.c,v $
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
	return 0;
#else
	return 1;
#endif
}
