#ifndef	tlsclient_h
#define	tlsclient_h

/*
** Copyright 2000-2001 Double Precision, Inc.
** See COPYING for distribution information.
*/


#ifdef  __cplusplus
extern "C" {
#endif

#include "config.h"
#include <sys/types.h>
#include <stdlib.h>

struct tls_subjitem {
	struct tls_subjitem *nextitem;
	const char *name;
	const char *value;
} ;

struct tls_subject {
	struct tls_subject *next;
	struct tls_subjitem *firstitem;
} ;

struct couriertls_info {
	char errmsg[128];
	char *x509info;
	size_t x509info_len;
	size_t x509info_size;

	struct tls_subject *first_subject;

	const char *cipher;
	const char *version;
	int bits;
} ;

void couriertls_init(struct couriertls_info *);
int couriertls_start(char **, struct couriertls_info *);

const char *couriertls_get_subject(struct couriertls_info *, const char *subject);
void couriertls_export_subject_environment(struct couriertls_info *);

void couriertls_destroy(struct couriertls_info *);

#ifdef  __cplusplus
}
#endif

#endif
