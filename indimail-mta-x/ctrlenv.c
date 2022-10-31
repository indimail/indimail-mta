/*
 * $Log: ctrlenv.c,v $
 * Revision 1.5  2022-10-31 09:09:26+05:30  Cprogrammer
 * look at last colon when parsing data from plain text files
 *
 * Revision 1.4  2022-10-30 22:15:39+05:30  Cprogrammer
 * added -c option to clear existing env variables
 * moved cdb_match() function to cdb_match.c
 *
 * Revision 1.3  2020-04-10 18:28:52+05:30  Cprogrammer
 * added feature to add multiple env variables
 *
 * Revision 1.2  2020-04-09 16:33:30+05:30  Cprogrammer
 * added search in MySQL db
 *
 * Revision 1.1  2020-04-08 16:02:19+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <unistd.h>
#include <ctype.h>
#include <sgetopt.h>
#include <strerr.h>
#include <case.h>
#include <str.h>
#include <stralloc.h>
#include <alloc.h>
#include <pathexec.h>
#include <uint32.h>
#include <alloc.h>
#include <cdb.h>
#include <byte.h>
#include <error.h>
#include <open.h>
#include <env.h>
#include "control.h"
#include "cdb_match.h"
#include "variables.h"
#include "auto_control.h"

#ifdef USE_SQL
#include "hasmysql.h"
#include "scan.h"
#ifdef HAS_MYSQL
#include <mysql.h>
#include <mysqld_error.h>
#include "indimail_stub.h"
#include "load_mysql.h"
#include "sqlmatch.h"
#endif
#endif

#define FATAL "ctrlenv: fatal: "

static int
parse_env(char *envStrings)
{
	char           *ptr1, *ptr2, *ptr3, *ptr4;

	for (ptr2 = ptr1 = envStrings;*ptr1;ptr1++) {
		if (*ptr1 == ',') {
			/*
			 * Allow ',' in environment variable if escaped
			 * by '\' character
			 */
			if (ptr1 != envStrings && *(ptr1 - 1) == '\\') {
				for (ptr3 = ptr1 - 1, ptr4 = ptr1; *ptr3; *ptr3++ = *ptr4++);
				continue;
			}
			*ptr1 = 0;
			/*- envar=, - Unset the environment variable */
			if (ptr1 != envStrings && *(ptr1 - 1) == '=') {
				*(ptr1 - 1) = 0;
				if (*ptr2 && !env_unset(ptr2))
					return (1);
			} else { /*- envar=val, - Set the environment variable */
				while (isspace(*ptr2))
					ptr2++;
				if (*ptr2 && !env_put(ptr2))
					return (1);
			}
			ptr2 = ptr1 + 1;
		}
	}
	/*- envar=, */
	if (ptr1 != envStrings && *(ptr1 - 1) == '=') {
		*(ptr1 - 1) = 0;
		if (*ptr2 && !env_unset(ptr2))
			return (1);
	} else { /*- envar=val, */
		while (isspace(*ptr2))
			ptr2++;
		if (*ptr2 && !env_put(ptr2))
			return (1);
	}
	return (0);
}

#ifdef HAS_MYSQL
int
match_in_db(MYSQL *conn, char *table_name, char *addr, char **envStr, char **errStr)
{

	MYSQL_RES      *res;
	MYSQL_ROW       row;
	int             num, m_error;
	static stralloc envStore = { 0 };
	static stralloc sql = { 0 };

	if (!conn)
		return (0);
	if (!stralloc_copys(&sql, "select value from ") ||
			!stralloc_cats(&sql, table_name) || !stralloc_cats(&sql, " where addr=\"") ||
			!stralloc_cats(&sql, addr) || !stralloc_cats(&sql, "\"")) {
		if (errStr)
			*errStr = error_str(errno);
		return (AM_MEMORY_ERR);
	}
again:
	if (!stralloc_0(&sql)) {
		if (errStr)
			*errStr = error_str(errno);
		return (AM_MEMORY_ERR);
	}
	if (in_mysql_query(conn, sql.s)) {
		if ((m_error = in_mysql_errno(conn)) == ER_NO_SUCH_TABLE)
			return (0);
		else
		if (m_error == ER_PARSE_ERROR) {
			if (!stralloc_copys(&sql, "select value from ") ||
					!stralloc_cats(&sql, table_name) || !stralloc_cats(&sql, " where addr='") ||
					!stralloc_cats(&sql, addr) || !stralloc_cats(&sql, "'")) {
				if (errStr)
					*errStr = error_str(errno);
				return (AM_MEMORY_ERR);
			}
			goto again;
		}
		sql.len--;
		if (!stralloc_cats(&sql, ": ") || !stralloc_cats(&sql, (char *) in_mysql_error(conn)) ||
				!stralloc_0(&sql)) {
			if (errStr)
				*errStr = error_str(errno);
			return (AM_MEMORY_ERR);
		}
		if (errStr)
			*errStr = sql.s;
		return (AM_MYSQL_ERR);
	}
	if (!(res = in_mysql_store_result(conn))) {
		sql.len--;
		if (!stralloc_cats(&sql, "mysql_store_result: ") ||
				!stralloc_cats(&sql, (char *) in_mysql_error(conn)) || !stralloc_0(&sql)) {
			if (errStr)
				*errStr = error_str(errno);
			return (AM_MEMORY_ERR);
		}
		return (AM_MYSQL_ERR);
	}
	num = in_mysql_num_rows(res);
	for (; (row = in_mysql_fetch_row(res));) {
		if (envStr) {
			if (!stralloc_copys(&envStore, row[0]) || !stralloc_0(&envStore)) {
				if (errStr)
					*errStr = error_str(errno);
				in_mysql_free_result(res);
				return (AM_MEMORY_ERR);
			}
			*envStr = envStore.s;
		}
	}
	in_mysql_free_result(res);
	return (num);
}

int
sql_match(char *fn, char *addr, int len, char **result)
{
	static stralloc controlfile = { 0 };
	int             cntrl_ok;
	MYSQL          *conn;
	char           *errStr = (char *) 0, *table_name = (char *) 0;

	if (!len || !*addr || !fn)
		return (0);
	if (!controldir) {
		if (!(controldir = env_get("CONTROLDIR")))
			controldir = auto_control;
	}
	if (!stralloc_copys(&controlfile, controldir) || !stralloc_cats(&controlfile, "/") ||
			!stralloc_cats(&controlfile, fn) ||
			!stralloc_0(&controlfile))
		strerr_die2x(111, FATAL, "out of memory");
	if ((cntrl_ok = initMySQLlibrary(&errStr)))
		return (0);
	else
	if (!use_sql)
		return (0);
	if ((cntrl_ok = connect_sqldb(controlfile.s, &conn, &table_name, &errStr)) < 0)
		return (cntrl_ok);
	if ((cntrl_ok = match_in_db(conn, table_name, addr, result, &errStr)) < 0)
		return (cntrl_ok);
	return (cntrl_ok ? 1 : 0);
}
#endif

int
main(int argc, char **argv)
{
	static stralloc ctrl = {0};
	char           *fn = (char *) 0, *ptr, *env_name = (char *) 0, *addr = (char *) 0, *result;
	int             i, j, opt, token_len, len, t;

	while ((opt = getopt(argc, argv, "cf:e:a:")) != opteof) {
		switch (opt)
		{
		case 'c':
			env_clear();
			break;
		case 'f':
			fn = optarg;
			break;
		case 'e':
			env_name = optarg;
			break;
		case 'a':
			addr = optarg;
			break;
		}
	}
	if (optind == argc || !fn || !addr)
		strerr_die1x(100, "usage: cntrlenv [-c] -f filename [-e env] -a address child");
	j = str_len(addr);
	i = str_end(fn, ".cdb");
	if (fn[i]) { /*- cdb extension */
		switch (cdb_match(fn, addr, j, &result))
		{
		case CDB_FOUND:
			if (env_name) {
				if (!pathexec_env(env_name, result))
					strerr_die2x(111, FATAL, "out of memory");
			} else
			if (parse_env(result))
				strerr_die2x(111, FATAL, "out of memory");
			pathexec(argv + optind);
			strerr_die4sys(111, FATAL, "unable to run ", argv[optind], ": ");
			break;
		case CDB_NOMATCH:
			pathexec(argv + optind);
			strerr_die4sys(111, FATAL, "unable to run ", argv[optind], ": ");
			break;
		case CDB_MEM_ERR:
			strerr_die2x(111, FATAL, "out of memory");
		case CDB_LSEEK_ERR:
			strerr_die3sys(111, FATAL, fn, ": unable to seek: ");
		case CDB_READ_ERR:
			strerr_die3sys(111, FATAL, fn, ": read error: ");
		case CDB_FILE_ERR:
			strerr_die4sys(111, FATAL, "error opening ", fn, ": ");
			break;
		}
	}
#ifdef HAS_MYSQL
	i = str_end(fn, ".sql");
	if (fn[i]) { /*- sql extension */
		if (sql_match(fn, addr, j, &result)) {
			if (env_name) {
				if (!pathexec_env(env_name, result))
					strerr_die2x(111, FATAL, "out of memory");
			} else
			if (parse_env(result))
				strerr_die2x(111, FATAL, "out of memory");
			pathexec(argv + optind);
		} else
			pathexec(argv + optind);
		strerr_die4sys(111, FATAL, "unable to run ", argv[optind], ": ");
	}
#endif
	if ((opt = control_readfile(&ctrl, fn, 0)) == -1) /*- plain text search */
		strerr_die3sys(111, FATAL, fn, ": ");
	for (len = 0, ptr = ctrl.s; len < ctrl.len; ) {
		i = str_chr(ptr, ':');
		if (ptr[i]) {
			t = str_len(addr);
			if ((ptr[0] == '=' && !case_diffb(addr, t > i - 1 ? t : i - 1, ptr + 1)) ||
					(ptr[0] == '+' && !case_diffb(addr, i - 1, ptr + 1)))
			{
				j = str_rchr(ptr, ':');
				ptr[j] = 0;
				if (env_name) {
					if (!pathexec_env(env_name, ptr + i + 1))
						strerr_die2x(111, FATAL, "out of memory");
				} else
				if (parse_env(ptr + i + 1))
					strerr_die2x(111, FATAL, "out of memory");
				pathexec(argv + optind);
				strerr_die4sys(111, FATAL, "unable to run ", argv[optind], ": ");
			}
		}
		len += ((token_len = str_len(ptr)) + 1);
		ptr += token_len + 1;
	}
	pathexec(argv + optind);
}

void
getversion_ctrlenv_c()
{
	static char    *x = "$Id: ctrlenv.c,v 1.5 2022-10-31 09:09:26+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
