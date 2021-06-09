/*
 * $Log: makeargs.h,v $
 * Revision 1.1  2020-04-01 16:07:53+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifndef MAKEARGS_H
#define MAKEARGS_H

#ifndef	lint
static char     sccsidmakeargsh[] = "$Id: $";
#endif

char          **makeargs(char *);
void            free_makeargs(char **);

#endif
