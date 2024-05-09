/*
 * $Log: upathexec.h,v $
 * Revision 1.6  2024-05-09 22:55:54+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.5  2021-06-15 08:24:29+05:30  Cprogrammer
 * renamed pathexec.. functions to upathexec to avoid clash with libqmail
 *
 * Revision 1.4  2017-04-22 11:52:27+05:30  Cprogrammer
 * added pathexec_dl()
 *
 * Revision 1.3  2016-02-08 21:30:55+05:30  Cprogrammer
 * added function load_shared()
 *
 * Revision 1.2  2005-05-13 23:46:06+05:30  Cprogrammer
 * code indentation
 *
 * Revision 1.1  2003-12-31 19:57:33+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifndef PATHEXEC_H
#define PATHEXEC_H

int             upathexec_env(const char *, const char *);
void            upathexec_run(const char *, char **, char **);
void            upathexec(char **);
#ifdef LOAD_SHARED_OBJECTS
void            load_shared(const char *, char **, char **);
void            pathexec_dl(int, char **, char **, int (*) (int, char **, char **));
#endif

#endif
