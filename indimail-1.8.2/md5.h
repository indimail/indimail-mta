/*-
 * $Log: md5.h,v $
 * Revision 2.1  2002-05-10 23:18:17+05:30  Cprogrammer
 * New revision of md5 routines
 *
 * MD5.H - header file for MD5C.C
 * Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991. All
 * rights reserved.
 * 
 * License to copy and use this software is granted provided that it
 * is identified as the "RSA Data Security, Inc. MD5 Message-Digest
 * Algorithm" in all material mentioning or referencing this software
 * or this function.
 * 
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

/*-
 * GLOBAL.H - RSAREF types and constants 
 */
#include <string.h>
/*
 * Copyright (C) RSA Laboratories, a division of RSA Data Security,
 * Inc., created 1991. All rights reserved.
 */
#ifndef _GLOBAL_H_
#define _GLOBAL_H_ 1
/*-
 * PROTOTYPES should be set to one if and only if the compiler supports
 * function argument prototyping.
 * The following makes PROTOTYPES default to 1 if it has not already been
 * defined as 0 with C compiler flags.
 */
#ifndef PROTOTYPES
#define PROTOTYPES 1
#endif
/*
 * POINTER defines a generic pointer type 
 */
typedef unsigned char *POINTER;
/*
 * UINT2 defines a two byte word 
 */
typedef unsigned short int UINT2;
/*
 * UINT4 defines a four byte word 
 */
typedef unsigned long int UINT4;
#ifndef NULL_PTR
#define NULL_PTR ((POINTER)0)
#endif
#ifndef UNUSED_ARG
#define UNUSED_ARG(x) x = *(&x);
#endif
/*
 * PROTO_LIST is defined depending on how PROTOTYPES is defined above.
 * If using PROTOTYPES, then PROTO_LIST returns the list, otherwise it
 * returns an empty list.  
 */
#if PROTOTYPES
#define PROTO_LIST(list) list
#else
#define PROTO_LIST(list) ()
#endif
#endif /*- end _GLOBAL_H_ */

#ifndef _MD5_H_
#define _MD5_H_ 1

#ifndef	lint
static char     sccsidmd5h[] = "$Id: md5.h,v 2.1 2002-05-10 23:18:17+05:30 Cprogrammer Stab mbhangui $";
#endif

#ifdef __cplusplus
extern          "C"
{
#endif

	/*- MD5 context.  */
	typedef struct
	{
		UINT4           state[4];	/*- state (ABCD) */
		UINT4           count[2];	/*- number of bits, modulo 2^64 (lsb first) */
		unsigned char   buffer[64];	/*- input buffer */
	}
	MD5_CTX;
	void MD5Init    PROTO_LIST((MD5_CTX *));
	void MD5Update  PROTO_LIST((MD5_CTX *, unsigned char *, unsigned int));
	void MD5Final   PROTO_LIST((unsigned char[16], MD5_CTX *));

#ifdef __cplusplus
}
#endif
#endif
