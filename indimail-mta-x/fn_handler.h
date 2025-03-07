/*
 * $Id: fn_handler.h,v 1.3 2025-01-22 00:30:36+05:30 Cprogrammer Exp mbhangui $
 */
#ifndef FN_HANDLER_H
#define FN_HANDLER_H

int             fn_handler(void (*)(const char *), void (*)(void), int, const char *);

#endif
/*
 * $Log: fn_handler.h,v $
 * Revision 1.3  2025-01-22 00:30:36+05:30  Cprogrammer
 * Fixes for gcc14
 *
 * Revision 1.2  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.1  2018-04-25 21:37:00+05:30  Cprogrammer
 * Initial revision
 *
 */
