#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

int
main()
{
#ifdef IPV6
	struct sockaddr_in6 sa;
	sa.sin6_family = PF_INET6;
	if (sa.sin6_family) ; /*- supress compiler warning */
	return 0;
#else
	return 1;
#endif
}
