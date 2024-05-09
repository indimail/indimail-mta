/*
 * $Log: control.h,v $
 * Revision 1.9  2022-04-20 23:11:13+05:30  Cprogrammer
 * added control_writefile(), control_readint() functions
 *
 * Revision 1.8  2011-07-04 17:44:54+05:30  Cprogrammer
 * added control_readrandom()
 *
 * Revision 1.7  2004-10-21 22:44:56+05:30  Cprogrammer
 * prototype for control_nativefile changed to accomodate qmail-dk
 *
 * Revision 1.6  2004-10-11 13:51:45+05:30  Cprogrammer
 * added function prototypes
 *
 * Revision 1.5  2003-12-22 10:03:38+05:30  Cprogrammer
 * added prototype for striptrailingwhitespace()
 *
 * Revision 1.4  2003-10-23 01:18:39+05:30  Cprogrammer
 * fixed compilation warnings
 *
 * Revision 1.3  2003-10-18 18:32:06+05:30  Cprogrammer
 * removed SPAMTHROTTLE
 *
 * Revision 1.2  2002-09-30 22:54:15+05:30  Cprogrammer
 * added header spamdef.h
 * added sccsid
 *
 */
#ifndef CONTROL_H
#define CONTROL_H
#include "stralloc.h"

int             control_init(void);
int             control_readline(stralloc *, const char *);
int             control_rldef(stralloc *, const char *, int, const char *);
int             control_readint(int *, const char *);
int             control_writeint(int, const char *);
int             control_readulong(unsigned long *, const char *);
int             control_readnativefile(stralloc *, const char *, int);
int             control_readfile(stralloc *, const char *, int);
int             control_writefile(stralloc *, const char *);
int             control_readrandom(stralloc *, const char *);
int             control_readcmd(stralloc *, const char *);
void            striptrailingwhitespace(stralloc *);
#endif
