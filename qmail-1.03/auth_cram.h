/* 
 * $Log: auth_cram.h,v $
 * Revision 1.2  2011-10-28 21:28:27+05:30  Cprogrammer
 * added hmac_sha1(), hmac_ripemd()
 *
 * Revision 1.1  2010-08-05 09:44:47+05:30  Cprogrammer
 * Initial revision
 *
 */

#ifndef HMAC_MD5_H
#define HMAC_MD5_H

/* 
 * pointer to data stream
 * length of data stream
 * pointer to authentication key
 * length of authentication key
 * caller digest to be filled in
 */
void            hmac_md5(unsigned char *text, int text_len, unsigned char *key, int key_len, unsigned char *digest);
void            hmac_sha1(unsigned char *text, int text_len, unsigned char *key, int key_len, unsigned char *digest);
void            hmac_ripemd(unsigned char *text, int text_len, unsigned char *key, int key_len, unsigned char *digest);

#endif
