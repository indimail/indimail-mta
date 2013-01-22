/*
 * $Log: hmac_md5.c,v $
 * Revision 2.3  2011-12-05 14:47:50+05:30  Cprogrammer
 * added version information for global.h
 *
 * Revision 2.2  2011-12-05 13:37:27+05:30  Cprogrammer
 * hmac_md5() function
 *
 */
#include "global.h"
#include "md5.h"

#ifndef	lint
static char     sccsid[] = "$Id: hmac_md5.c,v 2.3 2011-12-05 14:47:50+05:30 Cprogrammer Stab mbhangui $";
#endif
/*-
 * Function: hmac_md5
 */
void
hmac_md5(text, text_len, key, key_len, digest)
	unsigned char  *text;			 /*- pointer to data stream */
	int             text_len;		 /*- length of data stream */
	unsigned char  *key;			 /*- pointer to authentication key */
	int             key_len;		 /*- length of authentication key */
	unsigned char  *digest;			 /*- caller digest to be filled in */

{
	MD5_CTX         context;
	unsigned char   k_ipad[65];		 /*- inner padding - key XORd with ipad */
	unsigned char   k_opad[65];		 /*- outer padding - key XORd with opad */
	unsigned char   tk[16];
	int             i;

	/*- if key is longer than 64 bytes reset it to key=MD5(key) */
	if (key_len > 64)
	{

		MD5_CTX         tctx;

		MD5Init(&tctx);
		MD5Update(&tctx, key, key_len);
		MD5Final(tk, &tctx);

		key = tk;
		key_len = 16;
	}
	/*-
	 * the HMAC_MD5 transform looks like:
	 *
	 * MD5(K XOR opad, MD5(K XOR ipad, text))
	 *
	 * where K is an n byte key
	 * ipad is the byte 0x36 repeated 64 times
	 * opad is the byte 0x5c repeated 64 times
	 * and text is the data being protected
	 */

	/*- start out by storing key in pads */
	memset(k_ipad, 0, sizeof k_ipad);
	memset(k_opad, 0, sizeof k_opad);
	memcpy(k_ipad, key, key_len);
	memcpy(k_opad, key, key_len);

	/*- XOR key with ipad and opad values */
	for (i = 0; i < 64; i++)
	{
		k_ipad[i] ^= 0x36;
		k_opad[i] ^= 0x5c;
	}
	/*- perform inner MD5 */
	MD5Init(&context);						 /*- init context for 1st pass */
	MD5Update(&context, k_ipad, 64);		 /*- start with inner pad */
	MD5Update(&context, text, text_len);	 /*- then text of datagram */
	MD5Final(digest, &context);				 /*- finish up 1st pass */
	/*- perform outer MD5 */
	MD5Init(&context);						 /*- init context for 2nd pass */
	MD5Update(&context, k_opad, 64);		 /*- start with outer pad */
	MD5Update(&context, digest, 16);		 /*- then results of 1st hash */
	MD5Final(digest, &context);				 /*- finish up 2nd pass */
}

#include <stdio.h>
void
getversion_hmac_md5_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidglobalh);
	printf("%s\n", sccsidmd5h);
}
