/*
 * $Log: load_mysql.c,v $
 * Revision 1.13  2024-05-09 22:55:54+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.12  2023-04-01 19:27:18+05:30  Cprogrammer
 * refactored getlibObject function
 *
 * Revision 1.11  2022-11-23 15:25:15+05:30  Cprogrammer
 * renamed mysql_lib to libmysql
 *
 * Revision 1.10  2021-03-12 14:16:17+05:30  Cprogrammer
 * conditional compilation of mysql code
 *
 * Revision 1.9  2020-08-03 17:24:48+05:30  Cprogrammer
 * use qmail library
 *
 * Revision 1.8  2019-06-07 19:20:10+05:30  Cprogrammer
 * return success and set use_sql=0 if libmysqlclient is missing
 *
 * Revision 1.7  2019-05-28 10:28:01+05:30  Cprogrammer
 * assign symbols mysql_errno, mysql_num_rows, mysql_affected_rows
 * assign error string to error buffer in getlibObject()
 *
 * Revision 1.6  2019-05-27 20:31:52+05:30  Cprogrammer
 * use MYSQL_LIB env variable if defined
 *
 * Revision 1.5  2019-05-27 12:41:51+05:30  Cprogrammer
 * set full path of mysql_lib control file
 *
 * Revision 1.4  2019-05-26 12:24:04+05:30  Cprogrammer
 * load mysql_lib control file if MYSQL_LIB env is not defined
 *
 * Revision 1.3  2019-05-26 12:03:43+05:30  Cprogrammer
 * load libmysqlclient using control file mysql_lib
 *
 * Revision 1.2  2019-04-22 21:51:00+05:30  Cprogrammer
 * fixed erroneous hardcoding
 *
 * Revision 1.1  2019-04-21 10:24:06+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef MYSQL_CONFIG
#include "hasmysql.h"
#ifdef HAS_MYSQL
#include <unistd.h>
#include <dlfcn.h>
#include <mysql.h>
#include <mysqld_error.h>
#include <error.h>
#include <stralloc.h>
#include <env.h>
#include "control.h"

typedef const char c_char;
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

static c_char   memerr[] = "out of memory";
static c_char   ctlerr[] = "unable to read controls";
int             use_sql = 0;
static stralloc errbuf = { 0 };
static stralloc mysql_libfn = { 0 };

void *
loadLibrary(void **handle, const char *libenv, int *errflag, const char *errstr[])
{
	char           *ptr;
	int             i;

	if (*libenv == '/') { /*- filename */
		if ((i = control_readline(&mysql_libfn, libenv)) == -1 || !i) {
			if (errflag)
				*errflag = errno;
			if (errstr)
				*errstr = (char *) 0;
			if (!stralloc_copys(&errbuf, ctlerr) ||
					!stralloc_catb(&errbuf, ": ", 2) ||
					!stralloc_copys(&errbuf, error_str(errno)) ||
					!stralloc_0(&errbuf)) {
				if (errstr)
					*errstr = memerr;
			} else
			if (errstr)
				*errstr = errbuf.s;
			return ((void *) 0);
		}
		if (!stralloc_0(&mysql_libfn)) {
			if (errstr)
				*errstr = memerr;
			return ((void *) 0);
		}
		ptr = mysql_libfn.s;
	} else
	if (!(ptr = env_get(libenv))) {
		if (errflag)
			*errflag = 0;
		if (errstr)
			*errstr = (char *) 0;
		return ((void *) 0);
	} else
	if (handle && *handle) {
		if (errflag)
			*errflag = 0;
		if (errstr)
			*errstr = (char *) 0;
		return (handle);
	}
	if (errflag)
		*errflag = -1;
	if (errstr)
		*errstr = (char *) 0;
	if (*libenv != '/' && access(ptr, R_OK)) {
		if (errflag)
			*errflag = errno;
		if (!stralloc_copys(&errbuf, error_str(errno))) {
			if (errstr)
				*errstr = memerr;
		} else
		if (!stralloc_0(&errbuf)) {
			if (errstr)
				*errstr = memerr;
		} else
		if (errstr)
			*errstr = errbuf.s;
		return ((void *) 0);
	}
#ifdef RTLD_DEEPBIND
	if (!(*handle = dlopen(ptr, RTLD_NOW|RTLD_LOCAL|RTLD_DEEPBIND|RTLD_NODELETE))) {
#else
	if (!(*handle = dlopen(ptr, RTLD_NOW|RTLD_LOCAL|RTLD_NODELETE))) {
#endif
		if (!stralloc_copys(&errbuf, dlerror())) {
			if (errstr)
				*errstr = memerr;
		} else
		if (!stralloc_0(&errbuf)) {
			if (errstr)
				*errstr = memerr;
		} else
		if (errstr)
			*errstr = errbuf.s;
		return ((void *) 0);
	}
	dlerror();
	if (errflag)
		*errflag = 0;
	return (*handle);
}

void
closeLibrary(void **handle)
{
	if (*handle) {
		dlclose(*handle);
		*handle = (void *) 0;
	}
	return;
}

void *
getlibObject(const char *libenv, void **handle, const char *plugin_symb, const char *errstr[])
{
	void           *i;
	char           *ptr;

	if (!*handle)
		*handle = loadLibrary(handle, libenv, 0, errstr);
	if (!*handle)
		return ((void *) 0);
	if (!(i = dlsym(*handle, plugin_symb)) && errstr) {
		if (!stralloc_copyb(&errbuf, "getlibObject: ", 14) ||
				!stralloc_cats(&errbuf, plugin_symb))
			*errstr = memerr;
		if ((ptr = dlerror())) {
			if (!stralloc_cats(&errbuf, ptr) ||
					!stralloc_catb(&errbuf, ": ", 2))
				*errstr = memerr;
		}
		if (!stralloc_0(&errbuf))
			*errstr = memerr;
		*errstr = errbuf.s;
	}
	return (i);
}

int
initMySQLlibrary(const char *errstr[])
{
	static void    *phandle = (void *) 0;
	const char     *ptr;
	int             i;

	if (phandle)
		return (0);
	if (!(ptr = env_get("MYSQL_LIB"))) {
		ptr = "/etc/indimail/control/libmysql";
		if (access(ptr, R_OK))
			return (0);
	} else {
		if (access(ptr, R_OK))
			return (0);
		ptr = "MYSQL_LIB";
	}
	if (!(phandle = loadLibrary(&phandle, ptr, &i, errstr))) {
		use_sql = 0;
		if (!i)
			return (0);
		return (1);
	} else
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
#endif
#endif

void
getversion_load_mysql_c()
{
	const char     *x = "$Id: load_mysql.c,v 1.13 2024-05-09 22:55:54+05:30 mbhangui Exp mbhangui $";
	if (x)
		x++;
}
