/*
 * $Log: starttls.h,v $
 * Revision 1.2  2023-01-03 17:43:29+05:30  Cprogrammer
 * define variables used in qmail-daned, dnstlsarr as extern
 *
 * Revision 1.1  2021-05-26 11:05:26+05:30  Cprogrammer
 * Initial revision
 *
 */

#ifndef _STARTTLS_H
#define _STARTTLS_H
#include "ipalloc.h"
#include "tlsarralloc.h"

#ifndef	lint
static char     sccsidstarttlsh[] = "$Id: starttls.h,v 1.2 2023-01-03 17:43:29+05:30 Cprogrammer Exp mbhangui $";
#endif

void            die_control(char *, char *);
void            out(char *);
void            die_nomem();
void            logerr(char *);
void            logerrf(char *);
void            flush();
int             do_dane_validation(char *, int);
int             get_dane_records(char *);

extern stralloc sa;
extern stralloc save;
extern tlsarralloc ta;
extern ipalloc  ia;

#endif
