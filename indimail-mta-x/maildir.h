/*
 * $Log: maildir.h,v $
 * Revision 1.4  2021-08-29 23:27:08+05:30  Cprogrammer
 * define functions as noreturn
 *
 * Revision 1.3  2004-10-11 13:55:06+05:30  Cprogrammer
 * added function prototypes
 *
 * Revision 1.2  2004-06-18 23:00:55+05:30  Cprogrammer
 * added RCS log
 *
 */
#ifndef MAILDIR_H
#define MAILDIR_H
#include <strerr.h>
#include "prioq.h"

extern struct strerr maildir_chdir_err;
extern struct strerr maildir_scan_err;

int             maildir_chdir(void);
void            maildir_clean(stralloc *);
int             maildir_scan(prioq *, stralloc *, int, int);

#endif
