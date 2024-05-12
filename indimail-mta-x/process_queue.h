/*
 * $Log: process_queue.h,v $
 * Revision 1.3  2024-05-12 00:20:03+05:30  mbhangui
 * fix function prototypes
 *
 * Revision 1.2  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.1  2021-06-28 17:08:05+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifndef _PROCESS_QUEUE_H
#define _PROCESS_QUEUE_H

void            process_queue(const char *, const char *, int (*)(int *, int *, int *, int *), int *w, int *x, int *y, int *z);

#endif
