/*
 * $Log: fmt.h,v $
 * Revision 1.3  2004-10-10 10:11:25+05:30  Cprogrammer
 * added function prototypes
 *
 * Revision 1.2  2004-06-18 22:58:52+05:30  Cprogrammer
 * added RCS log
 *
 */
#ifndef FMT_H
#define FMT_H

#define FMT_ULONG 40			/*- enough space to hold 2^128 - 1 in decimal, plus \0 */
#define FMT_LEN ((char *) 0)	/*- convenient abbreviation */

unsigned int    fmt_uint(char *, unsigned int);
unsigned int    fmt_uint0(char *, unsigned int, unsigned int);
unsigned int    fmt_xint(char *, unsigned int);
unsigned int    fmt_nbbint(char *, unsigned int, unsigned int, unsigned int, unsigned int);
unsigned int    fmt_ushort(char *, unsigned short);
unsigned int    fmt_xshort(char *, unsigned short);
unsigned int    fmt_nbbshort(char *, unsigned int, unsigned int, unsigned int, unsigned short);
unsigned int    fmt_ulong(char *, unsigned long);
unsigned int    fmt_xlong(char *, unsigned long);
unsigned int    fmt_nbblong(char *, unsigned int, unsigned int, unsigned int, unsigned long);

unsigned int    fmt_plusminus(char *, int);
unsigned int    fmt_minus(char *, int);
unsigned int    fmt_0x(char *, int);

unsigned int    fmt_str(char *, char *);
unsigned int    fmt_strn(char *, char *, unsigned int);

#endif
