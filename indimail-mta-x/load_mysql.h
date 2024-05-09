/*
 * $Log: load_mysql.h,v $
 * Revision 1.7  2021-06-16 13:59:40+05:30  Cprogrammer
 * use MYSQL_RES for non-libmariadb setup
 *
 * Revision 1.6  2020-05-19 21:27:16+05:30  Cprogrammer
 * fixed typedef for MYSQL_RES
 *
 * Revision 1.5  2020-05-11 10:55:57+05:30  Cprogrammer
 * use uppercasse for typedefs
 *
 * Revision 1.4  2020-04-30 18:04:06+05:30  Cprogrammer
 * define function prototypes as extern
 *
 * Revision 1.3  2019-06-07 20:02:39+05:30  Cprogrammer
 * fixed compilation warning for MYSQL_RES with mariadb
 *
 * Revision 1.2  2019-04-21 10:28:17+05:30  Cprogrammer
 * include mysql.h for MYSQL definition
 *
 * Revision 1.1  2019-04-20 19:48:09+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifndef LOAD_MYSQL_H
#define LOAD_MYSQL_H
#include <mysql.h>

typedef unsigned int i_uint;
#ifdef LIBMARIADB
typedef struct st_mysql_res MYRES;
#else
#define MYRES MYSQL_RES
#endif

extern int      use_sql;
extern MYSQL   *(*in_mysql_init) (MYSQL *);
extern MYSQL   *(*in_mysql_real_connect) (MYSQL *, const char *, const char *, const char *, const char *, unsigned int, const char *, unsigned long);
extern const char *(*in_mysql_error) (MYSQL *);
extern i_uint   (*in_mysql_errno) (MYSQL *mysql);
extern void     (*in_mysql_close) (MYSQL *);
extern int      (*in_mysql_options) (MYSQL *, enum mysql_option, const void *);
extern int      (*in_mysql_query) (MYSQL *, const char *);
extern MYRES   *(*in_mysql_store_result) (MYSQL *);
extern char   **(*in_mysql_fetch_row) (MYSQL_RES *);
extern my_ulonglong (*in_mysql_num_rows)(MYSQL_RES *);
extern my_ulonglong (*in_mysql_affected_rows) (MYSQL *);
extern void     (*in_mysql_free_result) (MYSQL_RES *);

extern int      initMySQLlibrary(const char *errstr[]);

#endif
