/*
 * $Log: $
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

