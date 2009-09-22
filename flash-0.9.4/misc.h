/*
 * $Log: misc.h,v $
 * Revision 1.2  2002-12-21 19:08:21+05:30  Manny
 * added function unsetenv()
 *
 * Revision 1.1  2002-12-16 01:55:34+05:30  Manny
 * Initial revision
 *
 */
#if !defined (_MISC_H)
#   define _MISC_H
#include "config.h"
#include<termios.h>

void            error(char *);
void           *xmalloc(size_t);
void           *xrealloc(void *, size_t);
int             mail_check(int);
int             tty_cbreak(int, struct termios *);
int             tty_reset(int, struct termios *);
#ifndef HAVE_SETENV
int             setenv(const char *, const char *, int);
#endif
#ifndef HAVE_UNSETENV
void            unsetenv(const char *);
#endif
char           *shacrypt(char *, char *);

#endif
