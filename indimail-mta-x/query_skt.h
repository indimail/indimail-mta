/*
 * $Id: query_skt.h,v 1.2 2025-01-22 00:30:36+05:30 Cprogrammer Exp mbhangui $
 */
#ifndef QUERY_SKT_H
#define QUERY_SKT_H

int             query_skt(int, char *, stralloc *, char *, int, int, void(*)(void), void (*)(const char *));

#endif
/*
 * $Log: query_skt.h,v $
 * Revision 1.2  2025-01-22 00:30:36+05:30  Cprogrammer
 * Fixes for gcc14
 *
 * Revision 1.1  2018-04-25 22:50:27+05:30  Cprogrammer
 * Initial revision
 *
 */
