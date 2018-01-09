/*
 * $Log: udpopen.h,v $
 * Revision 1.1  2015-04-10 19:39:09+05:30  Cprogrammer
 * Initial revision
 *
 *
 */
#ifndef _UDPOPEN_H
#define _UDPOPEN_H

#ifndef __P
#ifdef __STDC__
#define __P(args) args
#else
#define __P(args) ()
#endif
#endif

int             udpopen     __P((char *, char *));

#endif

