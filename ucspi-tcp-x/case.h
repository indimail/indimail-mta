/*
 * $Log: case.h,v $
 * Revision 1.2  2005-05-13 23:43:34+05:30  Cprogrammer
 * code indentation
 *
 * Revision 1.1  2003-12-31 19:57:33+05:30  Cprogrammer
 * Initial revision
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
