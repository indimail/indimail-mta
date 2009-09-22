/*
 * $Log: mystring.h,v $
 * Revision 1.3  2009-06-04 10:46:24+05:30  Cprogrammer
 * removed getline
 *
 * Revision 1.2  2002-12-21 19:08:37+05:30  Manny
 * corrected compilation warnings
 *
 * Revision 1.1  2002-12-16 01:55:36+05:30  Manny
 * Initial revision
 *
 */
#if !defined(__STRING_H)
#   define __STRING_H

#include <ctype.h>

#define CR '\n'
#define TAB '\t'

char           *Readline(FILE *);
char           *findline(char *, int *);
int             substr(const char *, char *, char);
int             strtokenize(char *, char, char **, int);
char           *stradp(char *, int);
void            setbasedirectory(char *b);

#define strwhite(X)	while (isspace((int) *(X))) (X)++

#endif
