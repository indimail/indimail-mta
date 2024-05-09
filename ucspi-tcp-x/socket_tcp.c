/*
 * $Log: socket_tcp.c,v $
 * Revision 1.6  2024-05-09 22:55:54+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.5  2023-06-18 13:23:59+05:30  Cprogrammer
 * removed not needed include
 *
 * Revision 1.4  2020-08-03 17:27:16+05:30  Cprogrammer
 * use qmail library
 *
 * Revision 1.3  2007-06-10 10:17:25+05:30  Cprogrammer
 * fixed compilation warning
 *
 * Revision 1.2  2005-06-10 12:19:07+05:30  Cprogrammer
 * conditional ipv6 compilation
 *
 * Revision 1.1  2003-12-31 19:46:55+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <ndelay.h>
#include "socket.h"

int
socket_tcp(void)
{
	int             s;

	s = socket(AF_INET, SOCK_STREAM, 0);
	if (s == -1)
		return -1;
	if (ndelay_on(s) == -1) {
		close(s);
		return -1;
	}
	return s;
}
