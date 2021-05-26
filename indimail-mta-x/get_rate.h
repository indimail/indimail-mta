/*
 * $Log: get_rate.h,v $
 * Revision 1.2  2021-05-26 07:35:18+05:30  Cprogrammer
 * added arguments to get email count, configured rate and current rate
 *
 * Revision 1.1  2021-05-23 06:35:05+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifndef _GET_RATE_H
#define _GET_RATE_H

#ifndef	lint
static char     sccsidgetrateh[] = "$Id: get_rate.h,v 1.2 2021-05-26 07:35:18+05:30 Cprogrammer Exp mbhangui $";
#endif

#define DELIMITER "\0"

int             get_rate(char *expression, double *rate);
int             is_rate_ok(char *file, char *rate_exp, unsigned long *e, double *c, double *r);

#endif
