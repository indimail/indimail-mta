/* 
 * $Log: hmac_md5.h,v $
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

#endif
