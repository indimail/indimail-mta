/*
 * $Log: tlsacheck.h,v $
 * Revision 1.2  2018-04-26 11:40:23+05:30  Cprogrammer
 * changed DANETIMEOUT to 10
 *
 * Revision 1.1  2018-04-26 01:31:31+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifndef TLSACHECK_H
#define TLSACHECK_H

#define MAXDANEDATASIZE 2000
#define DEFAULTDANEPORT 1998
#define DEFAULTDANEIP   "127.0.0.1"
#define DANETIMEOUT     10

int             tlsacheck(char *, char *, void (*)(), void (*)());

#endif
