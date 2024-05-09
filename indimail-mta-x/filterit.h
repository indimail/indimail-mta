/*
 * $Id: filterit.h,v 1.2 2024-05-09 22:03:17+05:30 mbhangui Exp mbhangui $
 */
#ifndef _FILTERIT_H_
#define _FILTERIT_H_

#define FATAL "filterit: fatal: "
#define WARN  "filterit: warn: "

int             filterit_sub1(int, char **);
int             filterit_sub2(const char *);
#endif

/*
 * $Log: filterit.h,v $
 * Revision 1.2  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.1  2023-09-19 01:09:48+05:30  Cprogrammer
 * Initial revision
 *
 */
