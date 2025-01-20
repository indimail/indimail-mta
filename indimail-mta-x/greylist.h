/*
 * $Log: greylist.h,v $
 * Revision 1.2  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.1  2009-08-22 19:00:29+05:30  Cprogrammer
 * Initial revision based on code by Richard Andrews
 *
 */
#ifndef GREY_H
#define GREY_H

#define MAXGREYDATASIZE 2000
#define DEFAULTGREYPORT 1999
#define DEFAULTGREYIP   "127.0.0.1"
#define GREYTIMEOUT     3

int             greylist(const char *, const char *, const char *, const char *, int, void (*)(void), void (*)(const char *));

#endif
