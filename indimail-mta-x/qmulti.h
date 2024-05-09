/*
 * $Log: qmulti.h,v $
 * Revision 1.2  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.1  2021-06-09 19:33:19+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifndef QMULTI_H
#define QMULTI_H

int             qmulti(const char *, int, char **);
int             discard_envelope();
int             rewrite_envelope(int);
int             getfreespace(const char *);

#endif
