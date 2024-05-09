/*
 * $Log: fmtqfn.h,v $
 * Revision 1.4  2021-06-27 10:44:51+05:30  Cprogrammer
 * moved conf_split variable to fmtqfn.c
 *
 * Revision 1.3  2004-10-11 13:53:56+05:30  Cprogrammer
 * added function prototypes
 *
 * Revision 1.2  2004-06-18 22:58:57+05:30  Cprogrammer
 * added RCS log
 *
 */
#ifndef FMTQFN_H
#define FMTQFN_H

#define FMTQFN 40	/*- maximum space needed, if len(dirslash) <= 10 */

unsigned int    fmtqfn(char *, const char *, unsigned long, int);

extern int      conf_split;

#endif
