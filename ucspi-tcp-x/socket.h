/*
 * $Log: socket.h,v $
 * Revision 1.5  2023-06-18 13:22:33+05:30  Cprogrammer
 * added unix domain socket functions
 *
 * Revision 1.4  2021-05-12 21:04:07+05:30  Cprogrammer
 * define arguments as array subscripts to fix gcc 11 warnings
 *
 * Revision 1.3  2005-06-10 09:13:16+05:30  Cprogrammer
 * added ipv6 support
 *
 * Revision 1.1  2003-12-31 19:57:33+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifndef SOCKET_H
#define SOCKET_H

#include "haveip6.h"
#include "uint16.h"
#ifdef LIBC_HAS_IP6
#include "uint32.h"
#endif
#include <sys/socket.h>
#include <sys/un.h>

int             socket_tcp(void);
int             socket_udp(void);
int             socket_unix(void);
int             socket_connect4(int, char ip[4], uint16);
int             socket_connect_un(int, const char *);
int             socket_connected(int);
int             socket_bind4(int, char ip[4], uint16);
int             socket_bindun(int, const char *);
int             socket_listen(int, int);
int             socket_accept4(int, char ip[4], uint16 *);
int             socket_acceptun(int, struct sockaddr_un *);
int             socket_local4(int, char ip[4], uint16 *);
int             socket_remote4(int, char ip[4], uint16 *);
void            socket_tryreservein(int, int);
#ifdef LIBC_HAS_IP6
int             socket_tcp6(void);
int             socket_udp6(void);
int             socket_connect6(int, char ip[16], uint16, uint32);
int             socket_bind6(int, char ip[16], uint16, uint32);
int             socket_accept6(int, char ip[16], uint16 *, uint32 *);
int             socket_local6(int, char ip[16], uint16 *, uint32 *);
int             socket_remote6(int, char ip[16], uint16 *, uint32 *);
char           *socket_getifname(uint32);
uint32          socket_getifidx(const char *);
extern int      noipv6;
#endif

#endif
