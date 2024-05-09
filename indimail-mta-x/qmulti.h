/*
 * $Log: qmulti.h,v $
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
