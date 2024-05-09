/*
 * $Log: delivery_rate.h,v $
 * Revision 1.5  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.4  2022-01-30 08:31:17+05:30  Cprogrammer
 * added additional argument in delivery_rate()
 *
 * Revision 1.3  2021-06-05 12:43:50+05:30  Cprogrammer
 * added do_ratelimit argument
 *
 * Revision 1.2  2021-06-04 09:23:48+05:30  Cprogrammer
 * added time_needed argument
 *
 * Revision 1.1  2021-06-01 01:48:07+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifndef _DELIVERY_RATE_H
#define _DELIVERY_RATE_H

#ifndef	lint
static const char sccsiddelivery_rateh[] = "$Id: delivery_rate.h,v 1.5 2024-05-09 22:03:17+05:30 mbhangui Exp mbhangui $";
#endif

#include <datetime.h>

int             delivery_rate(char *domain, unsigned long id, datetime_sec *, int *, const char *);

#endif

