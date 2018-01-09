/*
 * $Log: tcpopen.h,v $
 * Revision 1.1  2015-08-20 18:30:40+05:30  Cprogrammer
 * Initial revision
 *
 *
 */
#ifndef _TCPOPEN_H
#define _TCPOPEN_H

#ifndef __P
#ifdef __STDC__
#define __P(args) args
#else
#define __P(args) ()
#endif
#endif

int             tcpopen     __P((char *, char *, int));

#endif

