/*
 * $Log: report.h,v $
 * Revision 1.3  2021-05-30 00:17:09+05:30  Cprogrammer
 * moved dtype enum to variables.h
 *
 * Revision 1.2  2021-05-26 07:37:41+05:30  Cprogrammer
 * made delivery variable extern
 *
 * Revision 1.1  2021-05-23 06:35:03+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifndef _REPORT_H
#define _REPORT_H

#ifndef	lint
static char     sccsidreporth[] = "$Id: report.h,v 1.3 2021-05-30 00:17:09+05:30 Cprogrammer Exp mbhangui $";
#endif

void            report(int, char *, char *, char *, char *, char *, char *);

#endif
