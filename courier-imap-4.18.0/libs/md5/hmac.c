/*
** Copyright 1998 - 1999 Double Precision, Inc.
** See COPYING for distribution information.
*/

#define	MD5_INTERNAL
#include	"md5.h"
#include	"../libhmac/hmac.h"

static void alloc_context( void (*func)(void *, void *), void *arg)
{
struct	MD5_CONTEXT c;

	(*func)((void *)&c, arg);
}

static void alloc_hash( void (*func)(unsigned char *, void *), void *arg)
{
unsigned char c[MD5_DIGEST_SIZE];

	(*func)(c, arg);
}

struct hmac_hashinfo hmac_md5 = {
	"md5",
	MD5_BLOCK_SIZE,
	MD5_DIGEST_SIZE,
	sizeof(struct MD5_CONTEXT),
	(void (*)(void *))md5_context_init,
	(void (*)(void *, const void *, unsigned))md5_context_hashstream,
	(void (*)(void *, unsigned long))md5_context_endstream,
	(void (*)(void *, unsigned char *))md5_context_digest,
	(void (*)(void *, const unsigned char *))md5_context_restore,
        alloc_context,
	alloc_hash};
