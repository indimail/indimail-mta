/* $Id: bogofilter.h 2867 2003-08-23 18:03:56Z relson $ */
/*  constants and declarations for bogofilter */

#ifndef	BOGOFILTER_H
#define	BOGOFILTER_H

#define EVEN_ODDS	0.5		/* used for words we want to ignore */

#define DEVIATION(n)	fabs((n) - EVEN_ODDS)	/* deviation from average */

extern rc_t bogofilter(int argc, char **argv);

extern void print_stats(FILE *fp);

#endif	/* BOGOFILTER_H */
