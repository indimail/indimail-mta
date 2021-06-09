/*
 * $Log: qmulti.h,v $
 * Revision 1.1  2021-06-09 19:33:19+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifndef QMULTI_H
#define QMULTI_H

#ifndef	lint
static char     sccsidqmultih[] = "$Id: qmulti.h,v 1.1 2021-06-09 19:33:19+05:30 Cprogrammer Exp mbhangui $";
#endif

int             qmulti(char *, int, char **);
int             discard_envelope();
int             rewrite_envelope(int);
int             getfreespace(char *);

#endif
