/*
** Copyright 1998 - 2000 Double Precision, Inc.
** See COPYING for distribution information.
*/
#include	"config.h"
#include	"rfc1035.h"
#include	<sys/types.h>
#include	<sys/socket.h>
#include	<arpa/inet.h>
#include	<errno.h>


/*
**	Bind a socket to a local IP. This is used to control the source IP
**	address when making TCP connection.
*/

int	rfc1035_bindsource(int sockfd, const struct sockaddr *addr, int addrlen)
{
	return bind(sockfd, addr, addrlen);
}
