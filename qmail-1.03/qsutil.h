/*
 * $Log: qsutil.h,v $
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

void            log1(char *);
void            log1_noflush(char *);
void            my_log2(char *, char *);
void            log2_noflush(char *, char *);
void            log3(char *, char *, char *);
void            log4(char *, char *, char *, char *);
void            log5(char *, char *, char *, char *, char *);
void            logsa(stralloc *);
void            logsa_noflush(stralloc *);
void            nomem(void);
void            pausedir(char *);
void            logsafe(char *);
void            logsafe_noflush(char *);

#endif
