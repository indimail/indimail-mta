/*
 * $Log: set_environment.h,v $
 * Revision 1.3  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.2  2021-07-05 21:19:04+05:30  Cprogrammer
 * new argument root_rc to allow root to load $HOME/.defaultqueue
 *
 * Revision 1.1  2021-05-13 12:36:28+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifndef SET_ENVIRONMENT_H
#define SET_ENVIRONMENT_H

void            set_environment(const char *, const char *, int);

#endif
