/*
 * $Log: sqlmatch.h,v $
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
#endif

int             create_sqltable(MYSQL *, char *, char **);
int             connect_sqldb(char *, MYSQL **, char **, char **);
int             sqlmatch(char *, char *, int, char **);
void            sqlmatch_close_db(void);
#endif
