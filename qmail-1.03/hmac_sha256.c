/*
 * $Log: hmac_sha256.c,v $
 * Revision 1.1  2011-12-10 15:21:52+05:30  Cprogrammer
 * Initial revision
 *
 */

#include <string.h>
#include <openssl/x509.h>
#include <openssl/hmac.h>

void
hmac_sha256(const unsigned char *text, /*- pointer to data stream        */
			size_t text_len,		   /*- length of data stream         */
			const unsigned char *key,  /*- pointer to authentication key */
			size_t key_len,			   /*- length of authentication key  */
			void *digest)
{
/*- caller digest to be filled in */
	unsigned char   k_ipad[65];	/*- inner padding - key XORd with ipad */
	unsigned char   k_opad[65];	/*- outer padding - key XORd with opad */
	unsigned char   tk[SHA256_DIGEST_LENGTH];
	unsigned char   tk2[SHA256_DIGEST_LENGTH];
	unsigned char   bufferIn[1024];
	unsigned char   bufferOut[1024];
	int             i;

	/*
 	 * if key is longer than 64 bytes reset it to key=sha256(key) 
 	 */
	if (key_len > 64) {
		SHA256(key, key_len, tk);
		key = tk;
		key_len = SHA256_DIGEST_LENGTH;
	}

	/*
 	 * the HMAC_SHA256 transform looks like:
 	 *
 	 * SHA256(K XOR opad, SHA256(K XOR ipad, text))
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
	for (i = 0; i < 64; i++) {
		k_ipad[i] ^= 0x36;
		k_opad[i] ^= 0x5c;
	}

	/*- perform inner SHA256 */
	memset(bufferIn, 0x00, 1024);
	memcpy(bufferIn, k_ipad, 64);
	memcpy(bufferIn + 64, text, text_len);

	SHA256(bufferIn, 64 + text_len, tk2);

	/*- perform outer SHA256 */
	memset(bufferOut, 0x00, 1024);
	memcpy(bufferOut, k_opad, 64);
	memcpy(bufferOut + 64, tk2, SHA256_DIGEST_LENGTH);

	SHA256(bufferOut, 64 + SHA256_DIGEST_LENGTH, digest);
}

void
getversion_hmac_sha256_c()
{
	static char    *x = "$Id: hmac_sha256.c,v 1.1 2011-12-10 15:21:52+05:30 Cprogrammer Stab mbhangui $";
	x++;
}
