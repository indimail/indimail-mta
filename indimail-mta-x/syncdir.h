/*
 * $Log: syncdir.h,v $
 * Revision 1.3  2022-04-04 11:17:15+05:30  Cprogrammer
 * added setting of fdatasync() instead of fsync()
 *
 * Revision 1.2  2021-05-16 23:02:52+05:30  Cprogrammer
 * define use_fsync, use_syncdir as extern
 *
 * Revision 1.1  2020-09-30 20:38:43+05:30  Cprogrammer
 * Initial revision
 *
 */

#ifndef SYNCDIR_H
#define SYNCDIR_H
extern int use_fsync, use_fdatasync, use_syncdir;
#ifdef DARWIN
int             qopen(char *, int, ...);
int             qlink(char *, char *); 
int             qunlink(char *); 
int             qrename(char *, char *); 
#define open(x, y) qopen(x, y)
#define link(x, y) qlink(x, y)
#define unlink(x) qunlink(x)
#define rename(x, y) qrename(x, y)
#endif
#endif
