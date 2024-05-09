/*
 * $Log: starttls.h,v $
 * Revision 1.3  2024-01-23 01:23:45+05:30  Cprogrammer
 * include buffer_defs.h for buffer size definitions
 *
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
#include "buffer_defs.h"

#ifndef	lint
static const char sccsidstarttlsh[] = "$Id: starttls.h,v 1.3 2024-01-23 01:23:45+05:30 Cprogrammer Exp mbhangui $";
#endif

void            die_control(const char *, const char *);
void            out(const char *);
void            die_nomem();
void            logerr(const char *);
void            logerrf(const char *);
void            flush();
int             do_dane_validation(const char *, int);
int             get_dane_records(const char *);
int             get_tlsa_rr(const char *, int, int);

extern stralloc sa;
extern stralloc save;
extern tlsarralloc ta;
extern ipalloc  ia;

#endif
