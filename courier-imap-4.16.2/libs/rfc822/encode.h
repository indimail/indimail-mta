/*
*/
#ifndef	rfc822_encode_h
#define	rfc822_encode_h

/*
** Copyright 2004 Double Precision, Inc.
** See COPYING for distribution information.
*/

#if	HAVE_CONFIG_H
#include	"rfc822/config.h"
#endif
#include	<stdio.h>
#include	<sys/types.h>
#include	<stdlib.h>
#include	<time.h>

#ifdef  __cplusplus
extern "C" {
#endif

struct libmail_encode_info {
	char output_buffer[BUFSIZ];
	int output_buf_cnt;

	char input_buffer[57]; /* For base64 */
	int input_buf_cnt;

	int (*encoding_func)(struct libmail_encode_info *,
			     const char *, size_t);
	int (*callback_func)(const char *, size_t, void *);
	void *callback_arg;
};

const char *libmail_encode_autodetect_fp(FILE *, int, int *);
const char *libmail_encode_autodetect_fpoff(FILE *, int, off_t, off_t, int *);
const char *libmail_encode_autodetect_buf(const char *, int);

void libmail_encode_start(struct libmail_encode_info *info,
			  const char *transfer_encoding,
			  int (*callback_func)(const char *, size_t, void *),
			  void *callback_arg);

int libmail_encode(struct libmail_encode_info *info,
		   const char *ptr,
		   size_t cnt);

int libmail_encode_end(struct libmail_encode_info *info);

#ifdef  __cplusplus
}
#endif

#endif
