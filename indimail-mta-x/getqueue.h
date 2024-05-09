/*
 * $Log: getqueue.h,v $
 * Revision 1.3  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.2  2022-03-30 21:08:56+05:30  Cprogrammer
 * removed time argument
 *
 * Revision 1.1  2022-03-26 10:17:24+05:30  Cprogrammer
 * Initial revision
 *
 */

#ifndef _GETQUEUE_H
#define _GETQUEUE_H
#include "haslibrt.h"

#ifdef HASLIBRT
int             queueNo_from_shm(const char *);
#endif
int             queueNo_from_env();

#endif
