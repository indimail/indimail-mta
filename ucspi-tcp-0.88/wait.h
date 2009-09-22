/*
 * $Log: wait.h,v $
 * Revision 1.2  2005-05-13 23:54:08+05:30  Cprogrammer
 * code indentation
 *
 * Revision 1.1  2003-12-31 19:57:33+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifndef WAIT_H
#define WAIT_H

int             wait_pid();
int             wait_nohang();
int             wait_stop();
int             wait_stopnohang();

#define wait_crashed(w) ((w) & 127)
#define wait_exitcode(w) ((w) >> 8)
#define wait_stopsig(w) ((w) >> 8)
#define wait_stopped(w) (((w) & 127) == 127)

#endif
