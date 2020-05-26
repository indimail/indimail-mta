/*
 * $Log: scan.h,v $
 * Revision 1.2  2005-05-13 23:46:45+05:30  Cprogrammer
 * code indentation
 *
 * Revision 1.1  2003-12-31 19:57:33+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifndef SCAN_H
#define SCAN_H

unsigned int    scan_uint(char *, unsigned int *);
unsigned int    scan_xint(char *, unsigned int *);
unsigned int    scan_nbbint(char *, unsigned int, unsigned int, unsigned int, unsigned int *);
unsigned int    scan_ushort(char *, unsigned short *);
unsigned int    scan_xshort(char *, unsigned short *);
unsigned int    scan_nbbshort(char *, unsigned int, unsigned int, unsigned int, unsigned short *);
unsigned int    scan_ulong(char *, unsigned long *);
unsigned int    scan_xlong(char *, unsigned long *);
unsigned int    scan_nbblong(char *, unsigned int, unsigned int, unsigned int, unsigned long *);

unsigned int    scan_plusminus(char *, int *);
unsigned int    scan_0x(char *, unsigned int *);

unsigned int    scan_whitenskip(char *, unsigned int);
unsigned int    scan_nonwhitenskip(char *, unsigned int);
unsigned int    scan_charsetnskip(char *, char *, unsigned int);
unsigned int    scan_noncharsetnskip(char *, char *, unsigned int);

unsigned int    scan_strncmp(char *, char *, unsigned int);
unsigned int    scan_memcmp(char *, char *, unsigned int);

unsigned int    scan_long(char *, long *);
unsigned int    scan_8long(char *, unsigned long *);

#endif
