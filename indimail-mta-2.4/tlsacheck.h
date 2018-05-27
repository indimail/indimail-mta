/*
 * $Log: tlsacheck.h,v $
 * Revision 1.3  2018-05-27 17:47:27+05:30  Cprogrammer
 * added option for qmail-remote to query/update records
 *
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

#define RECORD_NEW      0
#define RECORD_WHITE    1
#define RECORD_OK       2
#define RECORD_NOVRFY   3
#define RECORD_FAIL     4
#define RECORD_OLD      5

int             tlsacheck(char *, char *, int, char [], void (*)(), void (*)());

#endif
