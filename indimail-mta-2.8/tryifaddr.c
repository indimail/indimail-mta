#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/if_link.h>

int
main()
{
    struct ifaddrs *ifaddr;
    getifaddrs(&ifaddr);
	_exit(0);
}

