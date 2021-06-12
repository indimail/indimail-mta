/*
 * $Log: pidopen.h,v $
 * Revision 1.1  2021-06-12 18:18:11+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifndef _PIDOPEN_H
#define _PIDOPEN_H
#include <datetime.h>

#ifndef	lint
static char     sccsidpidopenh[] = "$Id: pidopen.h,v 1.1 2021-06-12 18:18:11+05:30 Cprogrammer Exp mbhangui $";
#endif

extern char    *pidfn;
extern int      messfd;

int             pidopen(datetime_sec);

#endif
