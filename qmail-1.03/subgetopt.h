/*
 * $Log: subgetopt.h,v $
 * Revision 1.3  2004-10-11 15:04:55+05:30  Cprogrammer
 * added function prototypes
 *
 * Revision 1.2  2004-06-18 23:02:04+05:30  Cprogrammer
 * added RCS log
 *
 */
#ifndef SUBGETOPT_H
#define SUBGETOPT_H

#ifndef SUBGETOPTNOSHORT
#define sgopt subgetopt
#define sgoptarg subgetoptarg
#define sgoptind subgetoptind
#define sgoptpos subgetoptpos
#define sgoptproblem subgetoptproblem
#define sgoptprogname subgetoptprogname
#define sgoptdone subgetoptdone
#endif

#define SUBGETOPTDONE -1

char           *subgetoptarg;
int             subgetoptind;
int             subgetoptpos;
int             subgetoptproblem;
int             subgetoptdone;
char           *subgetoptprogname;
int             subgetopt(int, char **, char *);

#endif
