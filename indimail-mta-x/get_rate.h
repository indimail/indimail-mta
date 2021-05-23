/*
 * $Log: get_rate.h,v $
 * Revision 1.1  2021-05-23 06:35:05+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifndef _GET_RATE_H
#define _GET_RATE_H

#ifndef	lint
static char     sccsidgetrateh[] = "$Id: get_rate.h,v 1.1 2021-05-23 06:35:05+05:30 Cprogrammer Exp mbhangui $";
#endif

int             get_rate(char *expression, double *rate);
int             is_rate_ok(char *rate_dir, char *file, char *rate_exp);

#endif
