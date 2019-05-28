/*
 * $Log: load_mysql.c,v $
 * Revision 1.6  2019-05-28 10:26:36+05:30  Cprogrammer
 * assign symbols mysql_errno, mysql_num_rows, mysql_affected_rows
 *
 * Revision 1.5  2019-05-27 20:28:32+05:30  Cprogrammer
 * use MYSQL_LIB env variable if defined
 *
 * Revision 1.4  2019-05-27 12:34:49+05:30  Cprogrammer
 * set libfn with full path of mysql_lib control file
 *
 * Revision 1.3  2019-05-26 12:30:13+05:30  Cprogrammer
 * use mysql_lib control file to dlopen libmysqlclient if MYSQL_LIB env variable not defined
 *
 * Revision 1.2  2019-05-26 11:37:26+05:30  Cprogrammer
 * set use_sql after all getlibObject()
 *
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
#include "auto_control.h"
#include "variables.h"

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

static stralloc libfn = { 0 };

int
initMySQLlibrary(char **errstr)
{
	static void    *phandle = (void *) 0;
	char           *ptr;
	int             i;

	if (phandle)
		return (0);
	if (!(ptr = env_get("MYSQL_LIB"))) {
		if (!controldir) {
			if (!(controldir = env_get("CONTROLDIR")))
				controldir = auto_control;
		}
		if (!libfn.len) {
			if (!stralloc_copys(&libfn, controldir))
				return(-1);
			if (libfn.s[libfn.len - 1] != '/' && !stralloc_append(&libfn, "/"))
				return(-1);
			if (!stralloc_catb(&libfn, "mysql_lib", 9) ||
					!stralloc_0(&libfn))
				return(-1);
		}
		ptr = libfn.s;
	} else
		ptr = "MYSQL_LIB";
	if (!(phandle = loadLibrary(&phandle, ptr, &i, errstr))) {
		use_sql = 0;
		if (!i)
			return (0);
		return (1);
	}
	if (!(in_mysql_init = getlibObject(ptr, &phandle, "mysql_init", errstr)))
		return (1);
	else
	if (!(in_mysql_real_connect = getlibObject(ptr, &phandle, "mysql_real_connect", errstr)))
		return (1);
	else
	if (!(in_mysql_error = getlibObject(ptr, &phandle, "mysql_error", errstr)))
		return (1);
	else
	if (!(in_mysql_errno = getlibObject(ptr, &phandle, "mysql_errno", errstr)))
		return (1);
	else
	if (!(in_mysql_close = getlibObject(ptr, &phandle, "mysql_close", errstr)))
		return (1);
	else
	if (!(in_mysql_options = getlibObject(ptr, &phandle, "mysql_options", errstr)))
		return (1);
	else
	if (!(in_mysql_query = getlibObject(ptr, &phandle, "mysql_query", errstr)))
		return (1);
	else
	if (!(in_mysql_store_result = getlibObject(ptr, &phandle, "mysql_store_result", errstr)))
		return (1);
	else
	if (!(in_mysql_fetch_row = getlibObject(ptr, &phandle, "mysql_fetch_row", errstr)))
		return (1);
	else
	if (!(in_mysql_num_rows = getlibObject(ptr, &phandle, "mysql_num_rows", errstr)))
		return (1);
	else
	if (!(in_mysql_affected_rows = getlibObject(ptr, &phandle, "mysql_affected_rows", errstr)))
		return (1);
	else
	if (!(in_mysql_free_result = getlibObject(ptr, &phandle, "mysql_free_result", errstr)))
		return (1);
	else
		use_sql = 1;
	return (0);
}

void
getversion_load_mysql_c()
{
	static char    *x = "$Id: load_mysql.c,v 1.6 2019-05-28 10:26:36+05:30 Cprogrammer Exp mbhangui $";
	if (x)
		x++;
}
