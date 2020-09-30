/*
 * $Log: syncdir.h,v $
 * Revision 1.1  2020-09-30 20:38:43+05:30  Cprogrammer
 * Initial revision
 *
 */

#ifndef SYNCDIR_H
#define SYNCDIR_H
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
