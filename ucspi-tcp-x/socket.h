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
int             socket_connect_un(int, char *);
int             socket_connected(int);
int             socket_bind4(int, char ip[4], uint16);
int             socket_bindun(int, char *);
int             socket_listen(int, int);
int             socket_accept4(int, char *, uint16 *);
int             socket_acceptun(int, struct sockaddr_un *, char *);
int             socket_recv4(int, char *, int, char *, uint16 *);
int             socket_send4(int, char *, int, char *, uint16);
int             socket_local4(int, char *, uint16 *);
int             socket_remote4(int, char *, uint16 *);
void            socket_tryreservein(int, int);
#ifdef LIBC_HAS_IP6
int             socket_tcp6(void);
int             socket_udp6(void);
int             socket_connect6(int, char ip[16], uint16, uint32);
int             socket_bind6(int, char ip[16], uint16, uint32);
int             socket_accept6(int, char ip[16], uint16 *, uint32 *);
int             socket_recv6(int, char *, unsigned int, char ip[16], uint16 *, uint32 *);
int             socket_send6(int, char *, unsigned int, char ip[16], uint16, uint32);
int             socket_local6(int, char ip[16], uint16 *, uint32 *);
int             socket_remote6(int, char ip[16], uint16 *, uint32 *);

/*- enable sending udp packets to the broadcast address */
int             socket_broadcast(int);
/*- join a multicast group on the given interface */
int             socket_mcjoin4(int, char *, char *);
int             socket_mcjoin6(int, char *, int);
/*- leave a multicast group on the given interface */
int             socket_mcleave4(int, char *);
int             socket_mcleave6(int, char *);
/*- set multicast TTL/hop count for outgoing packets */
int             socket_mcttl4(int, char);
int             socket_mcttl6(int, char);
/*- enable multicast loopback */
int             socket_mcloop4(int, char);
int             socket_mcloop6(int, char);

char           *socket_getifname(uint32);
uint32          socket_getifidx(char *);
extern int      noipv6;
#endif

#endif
