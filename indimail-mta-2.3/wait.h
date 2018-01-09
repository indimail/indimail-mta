/*
 * $Log: wait.h,v $
 * Revision 1.3  2004-10-11 14:16:12+05:30  Cprogrammer
 * added function prototypes
 *
 * Revision 1.2  2004-06-18 23:02:30+05:30  Cprogrammer
 * added RCS log
 *
 */
#ifndef WAIT_H
#define WAIT_H

int             wait_pid(int *, int);
int             wait_nohang(int *);

#define wait_crashed(w) ((w) & 127)
#define wait_exitcode(w) ((w) >> 8)
#define wait_stopsig(w) ((w) >> 8)
#define wait_stopped(w) (((w) & 127) == 127)

#endif
