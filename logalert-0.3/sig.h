/*
 * $Log: sig.h,v $
 * Revision 1.1  2013-05-15 00:14:57+05:30  Cprogrammer
 * Initial revision
 *
 */

#ifndef SIG_H
#define SIG_H
#include <stdio.h>
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#include <signal.h>

void            sig_handle_abort(int sig);
void            sig_handle_timer(uint timeout);
void            sig_init();

#endif
