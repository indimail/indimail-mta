/*
 * $Log: error_stack.h,v $
 * Revision 2.1  2008-08-07 13:12:45+05:30  Cprogrammer
 * header for error_stack
 *
 */
#ifndef ERROR_STACK_H
#define ERROR_STACK_H

#ifndef	lint
static char     sccsid_error_stackh[] = "$Id: error_stack.h,v 2.1 2008-08-07 13:12:45+05:30 Cprogrammer Stab mbhangui $";
#endif

#ifndef __P
#ifdef __STDC__
#define __P(args) args
#else
#define __P(args) ()
#endif
#endif

#ifdef HAVE_STDARG_H
char           *error_stack __P((FILE *, const char *, ...));
#else
char           *error_stack();
#endif
void            flush_stack();
void            discard_stack();
#endif
