/*
 * $Log: spf.h,v $
 * Revision 1.4  2023-01-15 18:31:47+05:30  Cprogrammer
 * changed SPF_DEFEXP to use open-spf.org/Why
 *
 * Revision 1.3  2012-04-10 20:36:21+05:30  Cprogrammer
 * added remoteip argument (ipv4) to spfcheck()
 *
 * Revision 1.2  2004-10-11 14:06:32+05:30  Cprogrammer
 * added function prototypes
 *
 * Revision 1.1  2004-08-15 19:57:47+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifndef SPF_H
#define SPF_H

#define SPF_OK       0
#define SPF_NONE     1
#define SPF_UNKNOWN  2
#define SPF_NEUTRAL  3
#define SPF_SOFTFAIL 4
#define SPF_FAIL     5
#define SPF_ERROR    6
#define SPF_NOMEM    7

/*
 * "http://spf.pobox.com/why.html?sender=%{S}&ip=%{I}&receiver=%{xR}"
 */
#define SPF_DEFEXP   "See http://www.open-spf.org/Why" \
                     "?v=1&s=mfrom&id=%{s}&ip=%{i}&receiver=%{xR}"

int             spfcheck(char *);
int             spfexplanation(stralloc *);
int             spfinfo(stralloc *);

#endif
