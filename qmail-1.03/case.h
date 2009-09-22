/*
 * $Log: case.h,v $
 * Revision 1.3  2004-10-11 13:49:14+05:30  Cprogrammer
 * added function prototypes
 *
 * Revision 1.2  2004-06-18 22:57:50+05:30  Cprogrammer
 * added RCS log
 *
 */
#ifndef CASE_H
#define CASE_H

void            case_lowers(char *);
void            case_lowerb(char *, unsigned int);
int             case_diffs(char *, char *);
int             case_diffb(char *, unsigned int, char *);
int             case_starts(char *, char *);
int             case_startb(char *, unsigned int, char *);

#define case_equals(s,t) (!case_diffs((s),(t)))

#endif
