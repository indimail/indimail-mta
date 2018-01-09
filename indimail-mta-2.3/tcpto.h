/*
 * $Log: tcpto.h,v $
 * Revision 1.6  2005-08-23 17:39:51+05:30  Cprogrammer
 * added penalty argument to tcpto() and interval argument to tcpto_err()
 *
 * Revision 1.5  2005-06-29 20:55:06+05:30  Cprogrammer
 * size of buffer changed to TCPTO_BUFSIZ
 *
 * Revision 1.4  2005-06-15 22:36:36+05:30  Cprogrammer
 * ipv6 support
 *
 * Revision 1.3  2004-10-11 14:15:15+05:30  Cprogrammer
 * added function prototypes
 *
 * Revision 1.2  2004-06-18 23:02:08+05:30  Cprogrammer
 * added RCS log
 *
 */
#ifndef TCPTO_H
#define TCPTO_H
#include "ip.h"
#include "ipalloc.h"

#define TCPTO_BUFSIZ 2048

int             tcpto(struct ip_mx *, int);
void            tcpto_err(struct ip_mx *, int, int);
void            tcpto_clean(void);

#endif
