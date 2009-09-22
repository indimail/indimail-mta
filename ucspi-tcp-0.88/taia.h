/*
 * $Log: taia.h,v $
 * Revision 1.2  2005-05-13 23:53:50+05:30  Cprogrammer
 * code indentation
 *
 * Revision 1.1  2003-12-31 19:57:33+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifndef TAIA_H
#define TAIA_H

#include "tai.h"

struct taia
{
	struct tai      sec;
	unsigned long   nano;		/*- 0...999999999 */
	unsigned long   atto;		/*- 0...999999999 */
};

void            taia_tai(struct taia *, struct tai *);
void            taia_now(struct taia *);
double          taia_approx(struct taia *);
double          taia_frac(struct taia *);

void            taia_add(struct taia *, struct taia *, struct taia *);
void            taia_sub(struct taia *, struct taia *, struct taia *);
void            taia_half(struct taia *, struct taia *);
int             taia_less(struct taia *, struct taia *);

#define TAIA_PACK 16
void            taia_pack(char *, struct taia *);
void            taia_unpack(char *, struct taia *);

#define TAIA_FMTFRAC 19
unsigned int    taia_fmtfrac(char *, struct taia *);
void            taia_uint(struct taia *, unsigned int);

#endif
