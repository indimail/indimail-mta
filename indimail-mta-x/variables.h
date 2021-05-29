/*
 * $Log: variables.h,v $
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

extern char    *queuedir;
extern char    *controldir;
extern char    *certdir;
extern int      use_fsync, use_syncdir;
extern int      noipv6;
typedef enum {
	local_delivery,
	remote_delivery,
	local_or_remote,
} dtype;

#endif
