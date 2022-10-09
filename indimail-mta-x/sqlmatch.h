/*
 * $Log: sqlmatch.h,v $
 * Revision 1.7  2022-10-09 23:02:35+05:30  Cprogrammer
 * removed function check_db()
 *
 * Revision 1.6  2021-02-27 20:59:49+05:30  Cprogrammer
 * changed error to warning for missing MySQL libs
 *
 * Revision 1.5  2020-04-30 18:04:44+05:30  Cprogrammer
 * define function prototypes as extern
 *
 * Revision 1.4  2020-04-09 16:00:51+05:30  Cprogrammer
 * added proto for check_db()
 *
 * Revision 1.3  2018-01-09 12:36:42+05:30  Cprogrammer
 * removed #ifdef INDIMAIL
 *
 * Revision 1.2  2010-04-19 14:19:45+05:30  Cprogrammer
 * conditional compilation of MySQL code
 *
 * Revision 1.1  2010-04-18 17:01:50+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifndef _SQLMATCH_H_
#define _SQLMATCH_H_
#ifdef HAS_MYSQL
#include <mysql.h>
#endif

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

#ifdef HAS_MYSQL
extern int      create_sqltable(MYSQL *, char *, char **);
extern int      connect_sqldb(char *, MYSQL **, char **, char **);
#endif
extern int      sqlmatch(char *, char *, int, char **);
extern void     sqlmatch_close_db(void);
#endif
