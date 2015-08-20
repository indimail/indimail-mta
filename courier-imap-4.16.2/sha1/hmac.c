/*
** Copyright 2001-2005 Double Precision, Inc.
** See COPYING for distribution information.
*/
#define	SHA1_INTERNAL
#include	"sha1.h"
#include	"../libhmac/hmac.h"


static void alloc_context_sha1( void (*func)(void *, void *), void *arg)
{
struct	SHA1_CONTEXT c;

	(*func)((void *)&c, arg);
}

static void alloc_hash_sha1( void (*func)(unsigned char *, void *), void *arg)
{
unsigned char c[SHA1_DIGEST_SIZE];

	(*func)(c, arg);
}

struct hmac_hashinfo hmac_sha1 = {
	"sha1",
	SHA1_BLOCK_SIZE,
	SHA1_DIGEST_SIZE,
	sizeof(struct SHA1_CONTEXT),
	(void (*)(void *))sha1_context_init,
	(void (*)(void *, const void *, unsigned))sha1_context_hashstream,
	(void (*)(void *, unsigned long))sha1_context_endstream,
	(void (*)(void *, unsigned char *))sha1_context_digest,
	(void (*)(void *, const unsigned char *))sha1_context_restore,
        alloc_context_sha1,
	alloc_hash_sha1};

static void alloc_context_sha256( void (*func)(void *, void *), void *arg)
{
struct	SHA256_CONTEXT c;

	(*func)((void *)&c, arg);
}

static void alloc_hash_sha256( void (*func)(unsigned char *, void *), void *arg)
{
unsigned char c[SHA256_DIGEST_SIZE];

	(*func)(c, arg);
}

struct hmac_hashinfo hmac_sha256 = {
	"sha256",
	SHA256_BLOCK_SIZE,
	SHA256_DIGEST_SIZE,
	sizeof(struct SHA256_CONTEXT),
	(void (*)(void *))sha256_context_init,
	(void (*)(void *, const void *, unsigned))sha256_context_hashstream,
	(void (*)(void *, unsigned long))sha256_context_endstream,
	(void (*)(void *, unsigned char *))sha256_context_digest,
	(void (*)(void *, const unsigned char *))sha256_context_restore,
        alloc_context_sha256,
	alloc_hash_sha256};
