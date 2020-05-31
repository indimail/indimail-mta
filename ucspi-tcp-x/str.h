/*
 * $Log: str.h,v $
 * Revision 1.3  2016-02-08 18:30:51+05:30  Cprogrammer
 * added str_end() function
 *
 * Revision 1.2  2005-05-13 23:53:41+05:30  Cprogrammer
 * code indentation
 *
 * Revision 1.1  2003-12-31 19:57:33+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifndef STR_H
#define STR_H

unsigned int    str_copy(char *, char *);
int             str_diff(char *, char *);
int             str_diffn(char *, char *, unsigned int);
unsigned int    str_len(char *);
unsigned int    str_chr(char *, int);
unsigned int    str_rchr(char *, int);
int             str_start(char *, char *);
int             str_end(char *, char *);

#define str_equal(s,t) (!str_diff((s),(t)))

#endif
