/*
 * $Log: ip6.h,v $
 * Revision 1.3  2013-08-06 07:56:26+05:30  Cprogrammer
 * added V4MAPPREFIX
 *
 * Revision 1.2  2013-08-06 00:49:40+05:30  Cprogrammer
 * added ip6_expandaddr()
 *
 * Revision 1.1  2005-06-10 12:12:12+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifndef IP6_H
#define IP6_H

#include "byte.h"
#include "stralloc.h"

unsigned int    ip6_scan(char *, char *);
unsigned int    ip6_fmt(char *, char *);
unsigned int    scan_ip6_flat(char *src, char *);
unsigned int    fmt_ip6_flat(char *dest, char *);
int             ip6tobitstring(char *, stralloc *, unsigned int);
unsigned int    ip6_expandaddr(char *src, stralloc *destination);
unsigned int    ip6_compactaddr(char *s, char ip[16]);

/*
 * ip6 address syntax: (h = hex digit), no leading '0' required
 * 1. hhhh:hhhh:hhhh:hhhh:hhhh:hhhh:hhhh:hhhh
 * 2. any number of 0000 may be abbreviated as "::", but only once
 * flat ip6 address syntax:
 * hhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhh
 */

#define IP6_FMT 40

extern unsigned char V4mappedprefix[12]; /*- ={0,0,0,0,0,0,0,0,0,0,0xff,0xff}; */
extern unsigned char V6loopback[16]; /*- ={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1}; */
extern unsigned char V6any[16];	/*- ={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}; */

#define ip6_isv4mapped(ip) (byte_equal(ip,12,V4mappedprefix))

#define V4MAPPREFIX "::ffff:"

#endif
