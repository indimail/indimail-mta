/*
 * $Log: strerr.h,v $
 * Revision 1.4  2004-10-21 21:50:27+05:30  Cprogrammer
 * added strerr_warn8,strerr_warn7,strerr_die8,strerr_die7,strerr_die8sys,strerr_die7sys,
 * strerr_die8x,strerr_die7x
 *
 * Revision 1.3  2004-10-11 14:09:04+05:30  Cprogrammer
 * added function prototypes
 *
 * Revision 1.2  2004-06-18 23:01:58+05:30  Cprogrammer
 * added RCS log
 *
 */
#ifndef STRERR_H
#define STRERR_H

struct strerr
{
	struct strerr  *who;
	char           *x;
	char           *y;
	char           *z;
} ;

extern struct strerr strerr_sys;

void            strerr_sysinit(void);
void            strerr_warn(char *, char *, char *, char *, char *, char *, char *, char *, struct strerr *);
void            strerr_die(int, char *, char *, char *, char *, char *, char *, char *, char *, struct strerr *);

#define STRERR(r,se,a) \
{ se.who = 0; se.x = a; se.y = 0; se.z = 0; return r; }

#define STRERR_SYS(r,se,a) \
{ se.who = &strerr_sys; se.x = a; se.y = 0; se.z = 0; return r; }
#define STRERR_SYS3(r,se,a,b,c) \
{ se.who = &strerr_sys; se.x = a; se.y = b; se.z = c; return r; }

#define strerr_warn8(x1,x2,x3,x4,x5,x6,x7,x8,se) \
strerr_warn((x1),(x2),(x3),(x4),(x5),(x6),(x7),(x8),(struct strerr *) (se))
#define strerr_warn7(x1,x2,x3,x4,x5,x6,x7,se) \
strerr_warn((x1),(x2),(x3),(x4),(x5),(x6),(x7),(char *) 0,(struct strerr *) (se))
#define strerr_warn6(x1,x2,x3,x4,x5,x6,se) \
strerr_warn((x1),(x2),(x3),(x4),(x5),(x6),(char *) 0,(char *) 0,(struct strerr *) (se))
#define strerr_warn5(x1,x2,x3,x4,x5,se) \
strerr_warn((x1),(x2),(x3),(x4),(x5),(char *) 0,(char *) 0, (char *) 0, (struct strerr *) (se))
#define strerr_warn4(x1,x2,x3,x4,se) \
strerr_warn((x1),(x2),(x3),(x4),(char *) 0,(char *) 0,(char *) 0, (char *) 0, (struct strerr *) (se))
#define strerr_warn3(x1,x2,x3,se) \
strerr_warn((x1),(x2),(x3),(char *) 0,(char *) 0,(char *) 0,(char *) 0, (char *) 0, (struct strerr *) (se))
#define strerr_warn2(x1,x2,se) \
strerr_warn((x1),(x2),(char *) 0,(char *) 0,(char *) 0,(char *) 0,(char *) 0, (char *) 0, (struct strerr *) (se))
#define strerr_warn1(x1,se) \
strerr_warn((x1),(char *) 0,(char *) 0,(char *) 0,(char *) 0,(char *) 0,(char *) 0, (char *) 0, (struct strerr *) (se))

#define strerr_die8(e,x1,x2,x3,x4,x5,x6,x7,x8,se) \
strerr_die((e),(x1),(x2),(x3),(x4),(x5),(x6),(x7), (x8), (struct strerr *) (se))
#define strerr_die7(e,x1,x2,x3,x4,x5,x6,x7,se) \
strerr_die((e),(x1),(x2),(x3),(x4),(x5),(x6),(x7), (char *) 0, (struct strerr *) (se))
#define strerr_die6(e,x1,x2,x3,x4,x5,x6,se) \
strerr_die((e),(x1),(x2),(x3),(x4),(x5),(x6),(char *) 0, (char *) 0, (struct strerr *) (se))
#define strerr_die5(e,x1,x2,x3,x4,x5,se) \
strerr_die((e),(x1),(x2),(x3),(x4),(x5),(char *) 0,(char *) 0, (char *) 0, (struct strerr *) (se))
#define strerr_die4(e,x1,x2,x3,x4,se) \
strerr_die((e),(x1),(x2),(x3),(x4),(char *) 0,(char *) 0,(char *) 0, (char *) 0, (struct strerr *) (se))
#define strerr_die3(e,x1,x2,x3,se) \
strerr_die((e),(x1),(x2),(x3),(char *) 0,(char *) 0,(char *) 0,(char *) 0, (char *) 0, (struct strerr *) (se))
#define strerr_die2(e,x1,x2,se) \
strerr_die((e),(x1),(x2),(char *) 0,(char *) 0,(char *) 0,(char *) 0,(char *) 0, (char *) 0, (struct strerr *) (se))
#define strerr_die1(e,x1,se) \
strerr_die((e),(x1),(char *) 0,(char *) 0,(char *) 0,(char *) 0,(char *) 0,(char *) 0, (char *) 0, (struct strerr *) (se))

#define strerr_die8sys(e,x1,x2,x3,x4,x5,x6,x7,x8) \
strerr_die((e),(x1),(x2),(x3),(x4),(x5),(x6),(x7), (x8), &strerr_sys)
#define strerr_die7sys(e,x1,x2,x3,x4,x5,x6,x7) \
strerr_die((e),(x1),(x2),(x3),(x4),(x5),(x6),(x7), (char *) 0, &strerr_sys)
#define strerr_die6sys(e,x1,x2,x3,x4,x5,x6) \
strerr_die((e),(x1),(x2),(x3),(x4),(x5),(x6),(char *) 0, (char *) 0, &strerr_sys)
#define strerr_die5sys(e,x1,x2,x3,x4,x5) \
strerr_die((e),(x1),(x2),(x3),(x4),(x5),(char *) 0,(char *) 0, (char *) 0, &strerr_sys)
#define strerr_die4sys(e,x1,x2,x3,x4) \
strerr_die((e),(x1),(x2),(x3),(x4),(char *) 0,(char *) 0,(char *) 0, (char *) 0, &strerr_sys)
#define strerr_die3sys(e,x1,x2,x3) \
strerr_die((e),(x1),(x2),(x3),(char *) 0,(char *) 0,(char *) 0,(char *) 0, (char *) 0, &strerr_sys)
#define strerr_die2sys(e,x1,x2) \
strerr_die((e),(x1),(x2),(char *) 0,(char *) 0,(char *) 0,(char *) 0,(char *) 0, (char *) 0, &strerr_sys)
#define strerr_die1sys(e,x1) \
strerr_die((e),(x1),(char *) 0,(char *) 0,(char *) 0,(char *) 0,(char *) 0,(char *) 0, (char *) 0, &strerr_sys)

#define strerr_die8x(e,x1,x2,x3,x4,x5,x6,x7,x8) \
strerr_die((e),(x1),(x2),(x3),(x4),(x5),(x6),(x7),(x8), (struct strerr *) 0)
#define strerr_die7x(e,x1,x2,x3,x4,x5,x6,x7) \
strerr_die((e),(x1),(x2),(x3),(x4),(x5),(x6),(x7), (char *) 0, (struct strerr *) 0)
#define strerr_die6x(e,x1,x2,x3,x4,x5,x6) \
strerr_die((e),(x1),(x2),(x3),(x4),(x5),(x6),(char *) 0, (char *) 0, (struct strerr *) 0)
#define strerr_die5x(e,x1,x2,x3,x4,x5) \
strerr_die((e),(x1),(x2),(x3),(x4),(x5),(char *) 0,(char *) 0, (char *) 0, (struct strerr *) 0)
#define strerr_die4x(e,x1,x2,x3,x4) \
strerr_die((e),(x1),(x2),(x3),(x4),(char *) 0,(char *) 0,(char *) 0, (char *) 0, (struct strerr *) 0)
#define strerr_die3x(e,x1,x2,x3) \
strerr_die((e),(x1),(x2),(x3),(char *) 0,(char *) 0,(char *) 0,(char *) 0, (char *) 0, (struct strerr *) 0)
#define strerr_die2x(e,x1,x2) \
strerr_die((e),(x1),(x2),(char *) 0,(char *) 0,(char *) 0,(char *) 0,(char *) 0, (char *) 0, (struct strerr *) 0)
#define strerr_die1x(e,x1) \
strerr_die((e),(x1),(char *) 0,(char *) 0,(char *) 0,(char *) 0,(char *) 0,(char *) 0, (char *) 0, (struct strerr *) 0)

#endif
