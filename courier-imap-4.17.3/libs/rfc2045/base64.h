#ifndef base64_h
#define base64_h

#include "rfc2045/rfc2045_config.h"


/*
** Copyright 2002 Double Precision, Inc.  See COPYING for
** distribution information.
*/

#ifdef  __cplusplus
extern "C" {
#endif

/* This is an attempt to write a portable base64 decoder */

struct base64decode {

	char workbuf[256];
	int workbuflen;

	int (*decode_func)(const char *, int, void *);
	void *decode_func_arg;
} ;

void base64_decode_init(struct base64decode *,
			int (*)(const char *, int, void *),
			void *);
int base64_decode(struct base64decode *, const char *, int);
int base64_decode_end(struct base64decode *);

char *base64_decode_str(const char *);

#ifdef  __cplusplus
}
#endif

#endif
