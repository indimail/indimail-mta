/*
 * $Log: do_rate.h,v $
 * Revision 1.1  2021-05-29 23:36:02+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifndef _GET_RATE_H
#define _GET_RATE_H

#ifndef	lint
static char     sccsidgetrateh[] = "$Id: do_rate.h,v 1.1 2021-05-29 23:36:02+05:30 Cprogrammer Exp mbhangui $";
#endif

#define DELIMITER "\0"

int             get_rate(char *expression, double *rate);
int             is_rate_ok(char *file, char *rate_exp, unsigned long *e, double *c, double *r);

#endif
