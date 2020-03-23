/*
 * $Log: md5.h,v $
 * Revision 1.3  2011-12-07 18:23:18+05:30  Cprogrammer
 * use uint32 data type (fix for 64 bit system)
 *
 * Revision 1.2  2011-12-05 15:09:43+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.1  2010-08-05 09:45:04+05:30  Cprogrammer
 * Initial revision
 *
 */

/*
 * MD5.H - header file for MD5C.C
 *
 * Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991. All
 * rights reserved.
 *
 * License to copy and use this software is granted provided that it
 * is identified as the "RSA Data Security, Inc. MD5 Message-Digest
 * Algorithm" in all material mentioning or referencing this software
 * or this function.

 * License is also granted to make and use derivative works provided
 * that such works are identified as "derived from the RSA Data
 * Security, Inc. MD5 Message-Digest Algorithm" in all material
 * mentioning or referencing the derived work.  
 *                                                                  
 * RSA Data Security, Inc. makes no representations concerning either
 * the merchantability of this software or the suitability of this
 * software for any particular purpose. It is provided "as is"
 * without express or implied warranty of any kind.  
 *                                                                  
 * These notices must be retained in any copies of any part of this
 * documentation and/or software.  
 */

#ifndef _MD5_H_
#define _MD5_H_ 1
#include "uint32.h"

#ifndef	lint
static char     sccsidmd5h[] = "$Id: md5.h,v 1.3 2011-12-07 18:23:18+05:30 Cprogrammer Stab mbhangui $";
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* MD5 context. */
typedef struct {
  uint32 state[4];                                   /* state (ABCD) */
  uint32 count[2];        /* number of bits, modulo 2^64 (lsb first) */
  unsigned char buffer[64];                         /* input buffer */
} MD5_CTX;

void MD5Init PROTO_LIST ((MD5_CTX *));
void MD5Update PROTO_LIST
  ((MD5_CTX *, unsigned char *, unsigned int));
void MD5Final PROTO_LIST ((unsigned char [16], MD5_CTX *));

#ifdef __cplusplus
}
#endif

#endif
