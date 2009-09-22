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

static const char rcsid[]="$Id: rfc1035bindsource.c,v 1.1 2003/12/16 01:19:02 mrsam Exp $";

/*
**	Bind a socket to a local IP. This is used to control the source IP
**	address when making TCP connection.
*/

int	rfc1035_bindsource(int sockfd, const struct sockaddr *addr, int addrlen)
{
	return bind(sockfd, addr, addrlen);
}
