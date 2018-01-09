/*
 * $Log: global.h,v $
 * Revision 1.3  2011-12-07 18:22:09+05:30  Cprogrammer
 * use uint32 data type (fix for 64 bit system)
 *
 * Revision 1.2  2011-12-05 15:07:45+05:30  Cprogrammer
 * added RCS id
 * use 4 byte word for UINT4
 *
 * Revision 1.1  2010-08-05 09:44:07+05:30  Cprogrammer
 * Initial revision
 *
 */

/* GLOBAL.H - RSAREF types and constants */
#include <string.h>
/* 
 * Copyright (C) RSA Laboratories, a division of RSA Data Security,
 * Inc., created 1991. All rights reserved.
 */

#ifndef _GLOBAL_H_
#define _GLOBAL_H_ 1

#ifndef	lint
static char     sccsidglobalh[] = "$Id: global.h,v 1.3 2011-12-07 18:22:09+05:30 Cprogrammer Stab mbhangui $";
#endif

/* 
 * PROTOTYPES should be set to one if and only if the compiler supports
 * function argument prototyping.
 * The following makes PROTOTYPES default to 1 if it has not already been
 * defined as 0 with C compiler flags.
 */
#ifndef PROTOTYPES
#define PROTOTYPES 1
#endif

/* POINTER defines a generic pointer type */
typedef unsigned char *POINTER;

/* UINT2 defines a two byte word */
typedef unsigned short int UINT2;

#ifndef NULL_PTR
#define NULL_PTR ((POINTER)0)
#endif

#ifndef UNUSED_ARG
#define UNUSED_ARG(x) x = *(&x);
#endif

/* PROTO_LIST is defined depending on how PROTOTYPES is defined above.
   If using PROTOTYPES, then PROTO_LIST returns the list, otherwise it
     returns an empty list.  
 */
#if PROTOTYPES
#define PROTO_LIST(list) list
#else
#define PROTO_LIST(list) ()
#endif

#endif /* end _GLOBAL_H_ */
