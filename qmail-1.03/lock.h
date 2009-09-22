/*
 * $Log: lock.h,v $
 * Revision 1.5  2004-10-11 13:54:59+05:30  Cprogrammer
 * added function prototypes
 *
 * Revision 1.4  2004-06-18 23:00:50+05:30  Cprogrammer
 * added RCS log
 *
 */
#ifndef LOCK_H
#define LOCK_H

int             lock_ex(int);
int             lock_un(int);
int             lock_exnb(int);
#endif
