/*
*/
#ifndef	rfc822hdr_h
#define	rfc822hdr_h

/*
** Copyright 2001 Double Precision, Inc.
** See COPYING for distribution information.
*/


#if	HAVE_CONFIG_H
#include	"rfc822/config.h"
#endif
#include	<sys/types.h>
#include	<stdio.h>
#include	<string.h>
#include	<stdlib.h>

#ifdef  __cplusplus
extern "C" {
#endif

struct rfc822hdr {
	char *header;
	char *value;

	size_t hdrsize;
	size_t maxsize;
} ;

#define rfc822hdr_init(h,s) \
	do { memset((h), 0, sizeof(*h)); (h)->maxsize=(s); } while(0)

#define rfc822hdr_free(h) \
   do { if ((h)->header) free ((h)->header); } while (0)

int rfc822hdr_read(struct rfc822hdr *, FILE *, off_t *, off_t);
void rfc822hdr_fixname(struct rfc822hdr *);
void rfc822hdr_collapse(struct rfc822hdr *);

int rfc822hdr_namecmp(const char *a, const char *b);
int rfc822hdr_is_addr(const char *hdr);

#ifdef  __cplusplus
}
#endif

#endif
