/*
 * $Log: pathexec.h,v $
 * Revision 1.2  2005-05-13 23:46:06+05:30  Cprogrammer
 * code indentation
 *
 * Revision 1.1  2003-12-31 19:57:33+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifndef PATHEXEC_H
#define PATHEXEC_H

void            pathexec_run(char *, char **, char **);
int             pathexec_env(char *, char *);
void            pathexec(char **);

#endif
