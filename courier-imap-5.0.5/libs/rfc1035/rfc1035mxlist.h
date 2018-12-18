#ifndef	rfc1035_mx_h
#define	rfc1035_mx_h

/*
** Copyright 1998 - 1999 Double Precision, Inc.
** See COPYING for distribution information.
*/

#if	HAVE_CONFIG_H
#include "config.h"
#endif

#include	<sys/types.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>


#ifdef  __cplusplus
extern "C" {
#endif

#define	RFC1035_MX_OK		0	/* Ok, records follow */
#define	RFC1035_MX_SOFTERR	1	/* Soft DNS error */
#define	RFC1035_MX_HARDERR	2	/* Hard DNS error */
#define	RFC1035_MX_INTERNAL	3	/* Internal library error */
#define	RFC1035_MX_BADDNS	4	/* Bad DNS records */

struct rfc1035_mxlist {
	struct rfc1035_mxlist *next;
	int	protocol;
#if	RFC1035_IPV6
	struct sockaddr_storage address;
#else
	struct sockaddr address;
#endif
	int priority;	/* -1 for plain old A records */
	int ad;
	char *hostname;
	} ;

struct rfc1035_res;

int rfc1035_mxlist_create(struct rfc1035_res *,
	const char *, struct rfc1035_mxlist **);
void rfc1035_mxlist_free(struct rfc1035_mxlist *);

int rfc1035_mxlist_create_x(struct rfc1035_res *,
			    const char *, int,
			    struct rfc1035_mxlist **);
#define RFC1035_MX_AFALLBACK 1
#define RFC1035_MX_IGNORESOFTERR 2
#define RFC1035_MX_QUERYALL 4

#ifdef  __cplusplus
}
#endif

#endif
