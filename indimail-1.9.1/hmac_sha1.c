/*
 * hmac_sha1 routine
 */
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

#include "typesx.h"
#include "sha1.h"

#define PAD 64
#define TK  20

#ifndef	lint
static char     sccsid[] = "$Id: hmac_sha1.c,v 2.2 2011-10-28 14:15:14+05:30 Cprogrammer Stab mbhangui $";
#endif

void
hmac_sha1(text, text_len, key, key_len, digest)
	u8             *text;		/* pointer to data stream */
	size_t          text_len;
	u8             *key;		/* pointer to authentication key */
	size_t          key_len;
	u8             *digest;		/* caller digest to be filled in */
{
	SHA1_CTX        ctx;
	u8              k_ipad[PAD + 1];	/* inner padding - key XORd with ipad */
	u8              k_opad[PAD + 1];	/* outer padding - key XORd with opad */
	u8              tk[TK];
	int             i;

	if (key_len > PAD) {
		SHA1_CTX        tctx;
		SHA1_Init(&tctx);
		SHA1_Update(&tctx, key, key_len);
		SHA1_Final(tk, &tctx);
		key = tk;
		key_len = TK;
	}

/*
 * start out by storing key in pads 
 */
	memset(k_ipad, 0, PAD);
	memcpy(k_ipad, key, key_len);
	memset(k_opad, 0, PAD);
	memcpy(k_opad, key, key_len);

/*
 * XOR key with ipad and opad values 
 */
	for (i = 0; i < PAD; i++) {
		k_ipad[i] ^= 0x36;
		k_opad[i] ^= 0x5c;
	}

/*
 * perform inner SHA1 
 */
	SHA1_Init(&ctx);			/* init ctx for 1st pass */
	SHA1_Update(&ctx, k_ipad, PAD);	/* start with inner pad */
	SHA1_Update(&ctx, text, text_len);	/* then text of datagram */
	SHA1_Final(digest, &ctx);	/* finish up 1st pass */

/*
 * perform outer SHA1 
 */
	SHA1_Init(&ctx);			/* init ctx for 2nd pass */
	SHA1_Update(&ctx, k_opad, PAD);	/* start with outer pad */
	SHA1_Update(&ctx, digest, TK);	/* then results of 1st hash */
	SHA1_Final(digest, &ctx);	/* finish up 2nd pass */
}

#include <stdio.h>
void
getversion_hmac_sha1_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidsha1h);
}
