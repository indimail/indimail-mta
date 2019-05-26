/*
 * $Log: load_mysql.c,v $
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
#include <unistd.h>
#include <dlfcn.h>
#include <mysql.h>
#include <mysqld_error.h>
#include "error.h"
#include "stralloc.h"
#include "env.h"
#include "control.h"

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

static char     memerr[] = "out of memory";
static char     ctlerr[] = "unable to read controls";
void           *mysql_handle;
int             use_sql = 0;
static stralloc errbuf = { 0 };
static stralloc mysql_libfn = { 0 };

void *
loadLibrary(void **handle, char *libenv, int *errflag, char **errstr)
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
	if (access(ptr, R_OK)) {
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
		if (errno == 2 && errflag) {
			*errflag = 0;
			if (errstr)
				*errstr = (char *) 0;
			return ((void *) 0);
		}
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
getlibObject(char *libenv, void **handle, char *plugin_symb, char **errstr)
{
	void           *i;
	char           *ptr;

	if (!*handle)
		*handle = loadLibrary(handle, libenv, 0, errstr);
	if (!*handle)
		return ((void *) 0);
	i = dlsym(*handle, plugin_symb);
	if (!stralloc_copyb(&errbuf, "getlibObject: ", 14) ||
			!stralloc_cats(&errbuf, plugin_symb) ||
			!stralloc_catb(&errbuf, ": ", 2))
	{
		if (errstr)
			*errstr = memerr;
	}
	if ((ptr = dlerror()) && !stralloc_cats(&errbuf, ptr)) {
		if (errstr)
			*errstr = memerr;
	}
	return (i);
}

int
initMySQLlibrary(char **errstr)
{
	void           *phandle = (void *) 0;
	char           *ptr;
	int             i;

	if (!(ptr = env_get("MYSQL_LIB"))
		ptr = "mysql_lib";
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
	if (!(in_mysql_free_result = getlibObject(ptr, &phandle, "mysql_free_result", errstr)))
		return (1);
	else
	if (!(in_mysql_fetch_row = getlibObject(ptr, &phandle, "mysql_fetch_row", errstr)))
		return (1);
	else
		use_sql = 1;
	return (0);
}
#endif

void
getversion_load_mysql_c()
{
	static char    *x = "$Id: load_mysql.c,v 1.4 2019-05-26 12:24:04+05:30 Cprogrammer Exp mbhangui $";
	if (x)
		x++;
}
