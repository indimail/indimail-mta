/*
 * $Id: valid_hname.h,v 1.2 2024-05-12 00:20:03+05:30 mbhangui Exp mbhangui $
 */
#ifndef _VALID_HNAME_H
#define _VALID_HNAME_H

#define MAX_HNAME_LEN 255 /*- RFC 1035 */
#define MAX_LABEL_LEN 63  /*- RFC 1035 */

int             valid_hname(const char *name);

#endif

/*
 * $Log: valid_hname.h,v $
 * Revision 1.2  2024-05-12 00:20:03+05:30  mbhangui
 * fix function prototypes
 *
 * Revision 1.1  2023-12-03 12:17:24+05:30  Cprogrammer
 * Initial revision
 *
 */
