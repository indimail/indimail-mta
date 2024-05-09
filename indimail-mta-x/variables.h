/*
 * $Log: variables.h,v $
 * Revision 1.9  2022-04-04 11:17:50+05:30  Cprogrammer
 * added use_fdatasync to turn on fdatasync() instead of fsync()
 *
 * Revision 1.8  2021-08-28 23:08:41+05:30  Cprogrammer
 * moved dtype enum delivery variable from variables.h to getDomainToken.h
 *
 * Revision 1.7  2021-05-30 00:18:42+05:30  Cprogrammer
 * added enum dtype for delivery type
 *
 * Revision 1.6  2020-09-15 21:10:46+05:30  Cprogrammer
 * added use_syncdir variable
 *
 * Revision 1.5  2018-05-30 23:26:52+05:30  Cprogrammer
 * moved noipv6 variable to variables.c
 *
 * Revision 1.4  2017-03-21 15:40:45+05:30  Cprogrammer
 * added certdir variable
 *
 * Revision 1.3  2004-06-18 23:02:28+05:30  Cprogrammer
 * added RCS log
 *
 */
#ifndef VARIABLES_H
#define VARIABLES_H

typedef const char c_char;
extern c_char  *queuedir;
extern c_char  *controldir;
extern c_char  *certdir;
extern int      use_fsync, use_fdatasync, use_syncdir;
extern int      noipv6;

#endif
