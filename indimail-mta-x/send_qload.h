/*
 * $Log: send_qload.h,v $
 * Revision 1.2  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.1  2022-04-24 08:48:24+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifndef _SEND_QUEUE_H_
#define _SEND_QUEUE_H_
#include "haslibrt.h"

#ifdef HASLIBRT
int             send_qload(const char *, unsigned int, long, unsigned int);
#endif

#endif
