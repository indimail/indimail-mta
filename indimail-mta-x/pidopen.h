/*
 * $Log: pidopen.h,v $
 * Revision 1.3  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.2  2021-06-15 21:50:44+05:30  Cprogrammer
 * added tmpdir argument to pidfmt(), pidopen()
 *
 * Revision 1.1  2021-06-12 18:18:11+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifndef _PIDOPEN_H
#define _PIDOPEN_H
#include <datetime.h>

extern char    *pidfn;
extern int      messfd;

int             pidopen(datetime_sec, const char *);
unsigned int    pidfmt(char *, unsigned long, datetime_sec, const char *);

#endif
