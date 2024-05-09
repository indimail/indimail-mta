/*
 * $Log: envrules.h,v $
 * Revision 1.8  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.7  2014-01-29 14:06:10+05:30  Cprogrammer
 * made domainqueue file configurable through env variable DOMAINQUEUE
 *
 * Revision 1.6  2013-11-21 15:40:24+05:30  Cprogrammer
 * added domainqueue functionality
 *
 * Revision 1.5  2010-07-23 13:58:35+05:30  Cprogrammer
 * added error definition constants
 *
 * Revision 1.4  2009-05-01 10:40:23+05:30  Cprogrammer
 * added errorstr argument to envrules()
 *
 * Revision 1.3  2008-06-12 08:38:25+05:30  Cprogrammer
 * added rulesfile argument
 *
 * Revision 1.2  2004-05-23 22:16:49+05:30  Cprogrammer
 * added envrules filename as argument
 *
 * Revision 1.1  2004-05-15 00:04:39+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifndef _ENVRULES_H
#define _ENVRULES_H
int             envrules(const char *, const char *, const char *, const char *err[]);
int             domainqueue(const char *, const char *, const char *, const char *err[]);

#ifndef AM_MEMORY_ERR
#define  AM_MEMORY_ERR -1
#endif
#ifndef AM_FILE_ERR
#define  AM_FILE_ERR   -2
#endif
#ifndef AM_LSEEK_ERR
#define  AM_LSEEK_ERR  -3
#endif
#ifndef AM_REGEX_ERR
#define  AM_REGEX_ERR  -4
#endif
#ifndef AM_CONFIG_ERR
#define  AM_CONFIG_ERR -5
#endif
#ifndef AM_MYSQL_ERR
#define  AM_MYSQL_ERR  -6
#endif

#endif
