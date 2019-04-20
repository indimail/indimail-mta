/*
 * $Log: load_mysql.c,v $
 * Revision 1.1  2019-04-20 19:48:07+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <unistd.h>
#include <mysql.h>
#include <mysqld_error.h>
#include "indimail_stub.h"
#include "error.h"
#include "stralloc.h"
#include "env.h"

void           *mysql_handle;
MYSQL          *(*in_mysql_init) (MYSQL *);
MYSQL          *(*in_mysql_real_connect) (MYSQL *, const char *, const char *, const char *, const char *, unsigned int, const char *, unsigned long);
const char     *(*in_mysql_error) (MYSQL *);
unsigned int    (*in_mysql_errno) (MYSQL *);
void            (*in_mysql_close) (MYSQL *);
int             (*in_mysql_options) (MYSQL *, enum mysql_option, const void *);
int             (*in_mysql_query) (MYSQL *, const char *);
MYSQL_RES      *(*in_mysql_store_result) (MYSQL *);
MYSQL_ROW       (*in_mysql_fetch_row) (MYSQL_RES *);
my_ulonglong    (*in_mysql_num_rows) (MYSQL_RES *);
my_ulonglong    (*in_mysql_affected_rows) (MYSQL *);
void            (*in_mysql_free_result) (MYSQL_RES *);
int             use_sql = 0;

int
initMySQLlibrary(char **errstr)
{
	void           *phandle = (void *) 0;
	int             i;

	if (!(phandle = loadLibrary(&phandle, "MYSQL_LIB", &i, errstr))) {
		use_sql = 0;
		if (!i)
			return (0);
		return (1);
	} else
		use_sql = 1;
	if (!(in_mysql_init = getlibObject("MYSQL_LIB", &phandle, "mysql_init", errstr)))
		return (1);
	if (!(in_mysql_real_connect = getlibObject("MYSQL_LIB", &phandle, "mysql_real_connect", errstr)))
		return (1);
	if (!(in_mysql_error = getlibObject("MYSQL_LIB", &phandle, "mysql_error", errstr)))
		return (1);
	if (!(in_mysql_close = getlibObject("MYSQL_LIB", &phandle, "mysql_close", errstr)))
		return (1);
	if (!(in_mysql_options = getlibObject("MYSQL_LIB", &phandle, "mysql_options", errstr)))
		return (1);
	if (!(in_mysql_query = getlibObject("MYSQL_LIB", &phandle, "mysql_query", errstr)))
		return (1);
	if (!(in_mysql_store_result = getlibObject("MYSQL_LIB", &phandle, "mysql_store_result", errstr)))
		return (1);
	if (!(in_mysql_free_result = getlibObject("MYSQL_LIB", &phandle, "mysql_free_result", errstr)))
		return (1);
	if (!(in_mysql_fetch_row = getlibObject("MYSQL_LIB", &phandle, "mysql_fetch_row", errstr)))
		return (1);
	return (0);
}

void
getversion_load_mysql_c()
{
	static char    *x = "$Id: load_mysql.c,v 1.1 2019-04-20 19:48:07+05:30 Cprogrammer Exp mbhangui $";
	if (x)
		x++;
}
