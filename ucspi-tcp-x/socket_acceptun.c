/*
 * $Log: socket_acceptun.c,v $
 * Revision 1.2  2024-05-09 22:55:54+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.1  2023-06-16 23:46:22+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <sys/socket.h>
#include <sys/un.h>
#include "byte.h"
#include "socket.h"

int
socket_acceptun(int s, struct sockaddr_un *un)
{
	unsigned int   dummy = sizeof(struct sockaddr_un);

	byte_zero((char *) un, sizeof(struct sockaddr_un));
	return accept(s, (struct sockaddr *) un, (socklen_t *) &dummy);
}
