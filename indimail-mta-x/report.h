/*
 * $Log: report.h,v $
 * Revision 1.5  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.4  2022-01-30 09:38:19+05:30  Cprogrammer
 * define report() as no_return
 *
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
static const char sccsidreporth[] = "$Id: report.h,v 1.5 2024-05-09 22:03:17+05:30 mbhangui Exp mbhangui $";
#endif

no_return void  report(int, const char *, const char *, const char *, const char *, const char *, const char *);

#endif
