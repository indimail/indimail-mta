/*
 * $Log: getqueue.h,v $
 * Revision 1.1  2022-03-26 10:17:24+05:30  Cprogrammer
 * Initial revision
 *
 */

#ifndef _GETQUEUE_H
#define _GETQUEUE_H
#include "haslibrt.h"
#include <datetime.h>

#ifdef HASLIBRT
int             queueNo_from_shm(char *, datetime_sec);
#endif
int             queueNo_from_env(datetime_sec);

#endif
