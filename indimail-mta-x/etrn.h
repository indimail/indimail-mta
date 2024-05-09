/*
 * $Log: etrn.h,v $
 * Revision 1.4  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.3  2023-12-03 12:22:48+05:30  Cprogrammer
 * moved hostname validation to valid_hname.c
 *
 * Revision 1.2  2004-06-18 22:58:36+05:30  Cprogrammer
 * added RCS log
 *
 */
#ifndef ETRN_H
#define ETRN_H

int             etrn_queue(const char *, const char *);

#endif
