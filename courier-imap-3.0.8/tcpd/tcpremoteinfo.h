#ifndef	tcpremoteinfo_h
#define	tcpremoteinfo_h

/*
** Copyright 1998 - 1999 Double Precision, Inc.
** See COPYING for distribution information.
*/

/*
** $Id: tcpremoteinfo.h,v 1.3 2000/05/21 20:27:57 mrsam Exp $
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
