/*
 * $Log: report.h,v $
 * Revision 1.1  2021-05-23 06:35:03+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifndef _REPORT_H
#define _REPORT_H

#ifndef	lint
static char     sccsidreporth[] = "$Id: report.h,v 1.1 2021-05-23 06:35:03+05:30 Cprogrammer Exp mbhangui $";
#endif

typedef enum {
	local,
	remote
} dtype;

void            report(int, char *, char *, char *, char *, char *, char *);

#endif
