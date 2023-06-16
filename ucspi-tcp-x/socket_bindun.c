/*
 * Log: $
 */
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include "str.h"
#include "byte.h"

int
socket_bindun(int s, char *socket_path)
{
	struct sockaddr_un localunaddr;

	byte_zero((char *) &localunaddr, sizeof(struct sockaddr_un));
    localunaddr.sun_family = AF_UNIX;
	str_copyb(localunaddr.sun_path, socket_path, str_len(socket_path));
	return bind(s, (struct sockaddr *) &localunaddr, sizeof(localunaddr));
}
