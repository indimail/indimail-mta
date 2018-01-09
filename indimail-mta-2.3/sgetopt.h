/*
 * $Log: sgetopt.h,v $
 * Revision 1.3  2004-10-11 14:05:37+05:30  Cprogrammer
 * added function prototypes
 *
 * Revision 1.2  2004-06-18 23:01:48+05:30  Cprogrammer
 * added RCS log
 *
 */
#ifndef SGETOPT_H
#define SGETOPT_H

#ifndef SGETOPTNOSHORT
#define getopt sgetoptmine
#define optarg subgetoptarg
#define optind subgetoptind
#define optpos subgetoptpos
#define opterr sgetopterr
#define optproblem subgetoptproblem
#define optprogname sgetoptprogname
#define opteof subgetoptdone
#endif

#include "subgetopt.h"

extern int      sgetopterr;
extern char    *sgetoptprogname;

int             sgetoptmine(int, char **, char *);

#endif
