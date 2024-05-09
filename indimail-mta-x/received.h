/*
 * $Log: received.h,v $
 * Revision 1.6  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.5  2023-10-07 01:26:30+05:30  Cprogrammer
 * added parameter hide to hide IP, Host in received headers
 *
 * Revision 1.4  2022-10-22 13:08:38+05:30  Cprogrammer
 * added program identifier to Received header
 *
 * Revision 1.3  2004-10-11 14:01:09+05:30  Cprogrammer
 * added function prototypes
 *
 * Revision 1.2  2004-06-18 23:01:40+05:30  Cprogrammer
 * added RCS log
 *
 */
#ifndef RECEIVED_H
#define RECEIVED_H
#include "qmail.h"

void            received(struct qmail *, const char *, const char *, const char *, const char *, const char *, const char *, const char *, int);

#endif
