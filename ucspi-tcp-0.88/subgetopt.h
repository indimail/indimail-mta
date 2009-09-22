/*
 * $Log: subgetopt.h,v $
 * Revision 1.2  2005-05-13 23:53:46+05:30  Cprogrammer
 * code indentation
 *
 * Revision 1.1  2003-12-31 19:57:33+05:30  Cprogrammer
 * Initial revision
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

int             subgetopt(int,char **,char *);
extern char    *subgetoptarg;
extern int      subgetoptind;
extern int      subgetoptpos;
extern int      subgetoptproblem;
extern char    *subgetoptprogname;
extern int      subgetoptdone;

#endif
