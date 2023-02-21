/*
 * $Log: svcfns.h,v $
 * Revision 1.2  2004-10-11 14:09:52+05:30  Cprogrammer
 * code indented
 *
 * Revision 1.1  2004-08-15 19:57:51+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifndef SVC_FNS__H__
#define SVC_FNS__H__

#define FD_STDIN 0
#define FD_STDOUT 1
#define FD_STDERR 2

#include <sys/types.h>
void            exec_supervise(const char *dir, int fdin, int fdout);
pid_t           start_supervise(const char *dir, int fdin, int fdout);
int             stop_supervise(const char *dir, pid_t svcpid);

/*
 * Required external functions
 */
extern void     err(const char *msg);

#endif
