/*
 * $Log: prot.h,v $
 * Revision 1.4  2017-04-05 03:10:30+05:30  Cprogrammer
 * changed datatype for uid, gid to uid_t, gid_t
 *
 * Revision 1.3  2007-06-10 10:15:37+05:30  Cprogrammer
 * fixed compilation warning
 *
 * Revision 1.2  2005-05-13 23:46:11+05:30  Cprogrammer
 * code indentation
 *
 * Revision 1.1  2003-12-31 19:57:33+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifndef PROT_H
#define PROT_H
#include <sys/types.h>

int             prot_gid(gid_t);
int             prot_uid(uid_t);

#endif
