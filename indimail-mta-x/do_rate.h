/*
 * $Log: do_rate.h,v $
 * Revision 1.2  2021-06-05 12:45:57+05:30  Cprogrammer
 * return time_needed in seconds to reach configured rate in time_needed argument
 *
 * Revision 1.1  2021-05-29 23:36:02+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifndef _GET_RATE_H
#define _GET_RATE_H

#ifndef	lint
static char     sccsidgetrateh[] = "$Id: do_rate.h,v 1.2 2021-06-05 12:45:57+05:30 Cprogrammer Exp mbhangui $";
#endif

#define DELIMITER "\0"

int             get_rate(char *expression, double *rate);
int             is_rate_ok(char *file, char *rate_exp, unsigned long *e, double *c, double *r, datetime_sec *);

#endif
