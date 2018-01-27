#ifndef	tcpremoteinfo_h
#define	tcpremoteinfo_h

/*
** Copyright 1998 - 1999 Double Precision, Inc.
** See COPYING for distribution information.
*/

/*
*/

#include	<sys/types.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include	"rfc1035/rfc1035.h"

#ifdef	__cplusplus
extern "C" {
#endif

const char *tcpremoteinfo(const RFC1035_ADDR *, int,		/* Local */
	const RFC1035_ADDR *, int,				/* Remote */
	const char **);

#ifdef	__cplusplus
} ;
#endif

#endif
