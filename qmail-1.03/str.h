/*
 * $Log: str.h,v $
 * Revision 1.5  2004-10-24 21:39:53+05:30  Cprogrammer
 * added prototype for str_chrn()
 *
 * Revision 1.4  2004-10-09 23:34:19+05:30  Cprogrammer
 * added function prototypes
 *
 * Revision 1.3  2004-08-14 02:19:12+05:30  Cprogrammer
 * added str_copyb()
 *
 * Revision 1.2  2004-06-18 23:02:40+05:30  Cprogrammer
 * added RCS log
 *
 */
#ifndef STR_H
#define STR_H

unsigned int    str_copy(char *, char *);
unsigned int    str_copyb(char *, char *, unsigned int);
int             str_diff(char *, char *);
int             str_diffn(char *, char *, unsigned int);
unsigned int    str_len(char *);
unsigned int    str_chr(char *, int);
unsigned int    str_rchr(char *, int);
int             str_start(char *, char *);
char           *str_chrn(char *, int, int);

#define str_equal(s,t) (!str_diff((s),(t)))

#endif
