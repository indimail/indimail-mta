#ifndef	sha1_h
#define	sha1_h

/*
** Copyright 2001-2008 Double Precision, Inc.
** See COPYING for distribution information.
*/


#if	HAVE_CONFIG_H
#include	"sha1/config.h"
#endif

#if	HAVE_SYS_TYPES_H
#include	<sys/types.h>
#endif

#define	SHA1_DIGEST_SIZE	20
#define	SHA1_BLOCK_SIZE		64

#define SHA256_DIGEST_SIZE	32
#define SHA256_BLOCK_SIZE	64

#define SHA512_DIGEST_SIZE	64
#define SHA512_BLOCK_SIZE	128

typedef SHA1_WORD SHA256_WORD;

#ifdef	__cplusplus
extern "C" {
#endif

typedef	unsigned char SHA1_DIGEST[20];
typedef unsigned char SHA256_DIGEST[32];
typedef unsigned char SHA512_DIGEST[64];

#ifdef	SHA1_INTERNAL

struct SHA1_CONTEXT {

	SHA1_WORD	H[5];

	unsigned char blk[SHA1_BLOCK_SIZE];
	unsigned blk_ptr;
	} ;

struct SHA256_CONTEXT {

	SHA256_WORD	H[8];

	unsigned char blk[SHA256_BLOCK_SIZE];
	unsigned blk_ptr;
	} ;

struct SHA512_CONTEXT {

	SHA512_WORD	H[8];

	unsigned char blk[SHA512_BLOCK_SIZE];
	unsigned blk_ptr;
	} ;

void sha1_context_init(struct SHA1_CONTEXT *);
void sha1_context_hash(struct SHA1_CONTEXT *,
		       const unsigned char[SHA1_BLOCK_SIZE]);
void sha1_context_hashstream(struct SHA1_CONTEXT *, const void *, unsigned);
void sha1_context_endstream(struct SHA1_CONTEXT *, unsigned long);
void sha1_context_digest(struct SHA1_CONTEXT *, SHA1_DIGEST);
void sha1_context_restore(struct SHA1_CONTEXT *, const SHA1_DIGEST);

void sha256_context_init(struct SHA256_CONTEXT *);
void sha256_context_hash(struct SHA256_CONTEXT *,
			 const unsigned char[SHA256_BLOCK_SIZE]);
void sha256_context_hashstream(struct SHA256_CONTEXT *,
			       const void *, unsigned);
void sha256_context_endstream(struct SHA256_CONTEXT *, unsigned long);
void sha256_context_digest(struct SHA256_CONTEXT *, SHA256_DIGEST);
void sha256_context_restore(struct SHA256_CONTEXT *, const SHA256_DIGEST);

void sha512_context_init(struct SHA512_CONTEXT *);
void sha512_context_hash(struct SHA512_CONTEXT *,
			 const unsigned char[SHA512_BLOCK_SIZE]);
void sha512_context_hashstream(struct SHA512_CONTEXT *,
			       const void *, unsigned);
void sha512_context_endstream(struct SHA512_CONTEXT *, SHA512_WORD);
void sha512_context_digest(struct SHA512_CONTEXT *, SHA512_DIGEST);
void sha512_context_restore(struct SHA512_CONTEXT *, const SHA512_DIGEST);

#endif

void sha1_digest(const void *, unsigned, SHA1_DIGEST);
const char *sha1_hash(const char *);

typedef unsigned char SSHA_RAND[4];

const char *ssha_hash(const char *, SSHA_RAND);

void sha256_digest(const void *, unsigned, SHA256_DIGEST);

const char *sha256_hash(const char *);

void sha512_digest(const void *, unsigned, SHA512_DIGEST);

const char *sha512_hash(const char *);

#ifdef	__cplusplus
 } ;
#endif

#endif
