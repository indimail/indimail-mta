/*
 * $Log: pathexec.h,v $
 * Revision 1.3  2010-06-08 22:00:08+05:30  Cprogrammer
 * pathexec() now returns the allocated environment variable
 *
 * Revision 1.2  2004-10-11 13:57:08+05:30  Cprogrammer
 * added function prototypes
 *
 * Revision 1.1  2004-06-18 22:59:20+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifndef PATHEXEC_H
#define PATHEXEC_H

void            pathexec_run(char *, char **, char **);
int             pathexec_env(char *, char *);
char          **pathexec(char **);

#endif
