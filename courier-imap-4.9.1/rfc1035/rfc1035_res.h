#ifndef	rfc1035_res_h
#define	rfc1035_res_h

/*
** Copyright 1998 - 2002 Double Precision, Inc.
** See COPYING for distribution information.
*/

#include	"rfc1035.h"
#include	"random128/random128.h"
#include	"md5/md5.h"
#include	<sys/types.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>


#ifdef  __cplusplus
extern "C" {
#endif

#define	MAXNS	10
#define	DEFAULT_INITIAL_TIMEOUT	5
#define	DEFAULT_MAXIMUM_BACKOFF	3

		/* Resolver state */
struct rfc1035_res {

	RFC1035_ADDR nameservers[MAXNS];
	int rfc1035_nnameservers;

	char *rfc1035_defaultdomain;
	unsigned rfc1035_good_ns;
	unsigned rfc1035_timeout_initial;	/* Initial timeout */
	unsigned rfc1035_timeout_backoff;	/* Maximum exponential backoff */

	random128binbuf randseed;
	MD5_DIGEST	randbuf;
	unsigned randptr;
	} ;

extern struct rfc1035_res rfc1035_default_resolver;

#ifdef  __cplusplus
}
#endif

#endif
