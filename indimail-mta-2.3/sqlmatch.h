/*
 * $Log: sqlmatch.h,v $
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
#include <mysql.h>

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

int             create_sqltable(MYSQL *, char *, char **);
int             connect_sqldb(char *, MYSQL **, char **, char **);
#endif
int             sqlmatch(char *, char *, int, char **);
void            sqlmatch_close_db(void);
#endif
