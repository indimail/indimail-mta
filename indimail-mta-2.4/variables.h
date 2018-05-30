/*
 * $Log: variables.h,v $
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

extern char    *queuedir;
extern char    *controldir;
extern char    *certdir;
extern int      use_fsync;
extern int      noipv6;

#endif
