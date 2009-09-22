/*
 * $Log: error.h,v $
 * Revision 1.7  2008-08-02 14:20:24+05:30  Cprogrammer
 * added function estack()
 *
 * Revision 1.6  2004-10-11 13:53:20+05:30  Cprogrammer
 * added function prototypes
 *
 * Revision 1.5  2004-10-09 21:20:39+05:30  Cprogrammer
 * added error_nodevice
 *
 * Revision 1.4  2004-06-18 22:58:31+05:30  Cprogrammer
 * added RCS log
 *
 */
#ifndef ERROR_H
#define ERROR_H

#include <errno.h>

extern int      error_ebadf;
extern int      error_intr;
extern int      error_nomem;
extern int      error_noent;
extern int      error_txtbsy;
extern int      error_io;
extern int      error_exist;
extern int      error_timeout;
extern int      error_inprogress;
extern int      error_wouldblock;
extern int      error_again;
extern int      error_dquot;
extern int      error_pipe;
extern int      error_perm;
extern int      error_acces;
extern int      error_nodevice;
extern int      error_proto;
extern int      error_isdir;
extern int      error_connrefused;
extern int      error_hostdown;
extern int      error_netunreach;
extern int      error_hostunreach;

char           *error_str(int);
char           *estack(int, const char *);
int             error_temp(int);

#endif
