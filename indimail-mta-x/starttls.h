/*
 * $Log: starttls.h,v $
 * Revision 1.1  2021-05-26 11:05:26+05:30  Cprogrammer
 * Initial revision
 *
 */

#ifndef _STARTTLS_H
#define _STARTTLS_H

#ifndef	lint
static char     sccsidstarttlsh[] = "$Id: starttls.h,v 1.1 2021-05-26 11:05:26+05:30 Cprogrammer Exp mbhangui $";
#endif

void            die_control(char *, char *);
void            out(char *);
void            die_nomem();
void            logerr(char *);
void            logerrf(char *);
void            flush();
int             do_dane_validation(char *, int);
int             get_dane_records(char *);

#endif
