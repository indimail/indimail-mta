/*
 * $Log: sgetopt.h,v $
 * Revision 1.3  2008-07-26 09:45:46+05:30  Cprogrammer
 * added getopt definition
 *
 * Revision 1.2  2005-05-13 23:46:54+05:30  Cprogrammer
 * code indentation
 *
 * Revision 1.1  2003-12-31 19:57:33+05:30  Cprogrammer
 * Initial revision
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

#ifndef SGETOPTNOSHORT
int             getopt(int, char **, char *);
#endif
#endif
