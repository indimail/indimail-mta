/*
 * $Log: qsutil.h,v $
 * Revision 1.23  2023-01-15 12:38:42+05:30  Cprogrammer
 * use slog() function with varargs to replace all log functions
 *
 * Revision 1.22  2022-03-16 20:02:03+05:30  Cprogrammer
 * added log5_noflush() function
 *
 * Revision 1.21  2022-01-30 09:28:18+05:30  Cprogrammer
 * print program name in logs
 *
 * Revision 1.20  2021-10-22 14:00:33+05:30  Cprogrammer
 * added ident argument to loglock_open() for identification in logs
 *
 * Revision 1.19  2021-07-17 14:40:40+05:30  Cprogrammer
 * add fix_split function to generate file name for any split value
 *
 * Revision 1.18  2021-06-27 11:33:49+05:30  Cprogrammer
 * added loglock_open function
 *
 * Revision 1.17  2021-06-23 13:23:03+05:30  Cprogrammer
 * include sys/types.h for size_t
 *
 * Revision 1.16  2021-06-23 10:04:01+05:30  Cprogrammer
 * added log_stat function
 *
 * Revision 1.15  2021-06-05 12:53:00+05:30  Cprogrammer
 * added log4_noflush() function
 *
 * Revision 1.14  2021-06-04 09:22:09+05:30  Cprogrammer
 * added log15() function
 *
 * Revision 1.13  2021-05-30 00:14:28+05:30  Cprogrammer
 * added log11() function
 *
 * Revision 1.12  2021-05-08 12:25:15+05:30  Cprogrammer
 * include stralloc.h
 *
 * Revision 1.11  2016-03-31 17:02:49+05:30  Cprogrammer
 * added log3_noflush(), lockerr(), flush() functions
 *
 * Revision 1.10  2016-01-29 18:30:38+05:30  Cprogrammer
 * removed log11() and added log13()
 *
 * Revision 1.9  2014-03-07 19:15:11+05:30  Cprogrammer
 * added log9(), log11()
 *
 * Revision 1.8  2014-02-05 01:05:08+05:30  Cprogrammer
 * added prototype for log7()
 *
 * Revision 1.7  2013-09-23 22:13:25+05:30  Cprogrammer
 * added log functions which do not flush
 *
 * Revision 1.6  2009-05-03 22:46:56+05:30  Cprogrammer
 * added log5() function
 *
 * Revision 1.5  2004-12-20 22:58:09+05:30  Cprogrammer
 * changed log2() to my_log2() to avoid conflicts in fedora3
 *
 * Revision 1.4  2004-10-11 14:00:03+05:30  Cprogrammer
 * added function prototypes
 *
 * Revision 1.3  2004-06-18 23:01:30+05:30  Cprogrammer
 * added RCS log
 *
 */
#ifndef QSUTIL_H
#define QSUTIL_H
#include <stralloc.h>
#include <sys/types.h>
#include "varargs.h"

void            loglock_open(char *, int);
void            lockerr(void);
#ifdef HAVE_STDARG_H
void            slog(int, ...);
#else
void            slog();
#endif
void            logsa(stralloc *);
void            logsa_noflush(stralloc *);
void            log_stat(stralloc *, stralloc *, unsigned long, size_t);
void            nomem(char *);
void            pausedir(char *);
void            logsafe(char *, char *);
void            logsafe_noflush(char *, char *);
void            flush();
int             fix_split(char *s, char *path, char *client_split, unsigned long id);

extern int      loglock_fd;

#endif
