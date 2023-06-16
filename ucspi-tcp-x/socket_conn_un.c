/*
 * $Log: $
 */
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <byte.h>
#include "socket.h"
#include "str.h"

int
socket_connect_un(int s, char *socket)
{
	struct sockaddr_un sa;

	byte_zero((char *) &sa, sizeof sa);
	sa.sun_family = AF_UNIX;
	str_copyb(sa.sun_path, socket, sizeof(sa.sun_path));
	return connect(s, (struct sockaddr *) &sa, sizeof sa);
}
