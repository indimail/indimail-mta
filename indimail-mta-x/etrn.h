/*
 * $Log: etrn.h,v $
 * Revision 1.2  2004-06-18 22:58:36+05:30  Cprogrammer
 * added RCS log
 *
 */
#ifndef ETRN_H
#define ETRN_H

#define VALID_HOSTNAME_LEN	255	/*- RFC 1035 */
#define VALID_LABEL_LEN		63	/*- RFC 1035 */

#define DONT_GRIPE		0
#define DO_GRIPE		1

int             valid_hostname(char *name);
int             etrn_queue(char *, char *);

#endif
