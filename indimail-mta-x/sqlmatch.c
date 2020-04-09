/*
 * $Log: sqlmatch.c,v $
 * Revision 1.12  2020-04-09 16:03:16+05:30  Cprogrammer
 * removed not needed arguments to check_db()
 *
 * Revision 1.11  2019-04-20 19:53:13+05:30  Cprogrammer
 * load MySQL library dynamically
 *
 * Revision 1.10  2018-02-11 21:21:16+05:30  Cprogrammer
 * use USE_SQL to compile sql support
 *
 * Revision 1.9  2018-01-09 12:36:24+05:30  Cprogrammer
 * replaced #ifdef INDIMAIL with #ifdef HAS_MYSQL
 *
 * Revision 1.8  2016-05-17 19:44:58+05:30  Cprogrammer
 * use auto_control, set by conf-control to set control directory
 *
 * Revision 1.7  2010-04-19 10:19:44+05:30  Cprogrammer
 * fixed setting MYSQL_OPT_CONNECT_TIMEOUT
 * make query work for MySQL server without NO_BACKSLASH_ESCAPES
 * changes for qmail-sql
 *
 * Revision 1.6  2009-09-07 15:33:31+05:30  Cprogrammer
 * renamed create_table(), connect_db() to avoid clash with indimail
 *
 * Revision 1.5  2009-09-07 13:55:41+05:30  Cprogrammer
 * enable compilation without indimail
 *
 * Revision 1.4  2009-05-01 12:41:32+05:30  Cprogrammer
 * return error for create_table()
 *
 * Revision 1.3  2009-05-01 10:44:39+05:30  Cprogrammer
 * added errstr argument to sqlmatch()
 * use constants from qregex.h for returning errors
 *
 * Revision 1.2  2009-04-30 18:51:39+05:30  Cprogrammer
 * return no match if filename is null
 *
 * Revision 1.1  2009-04-30 15:34:07+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef USE_SQL
#include "hasmysql.h"
#ifdef HAS_MYSQL
#include "stralloc.h"
#include "byte.h"
#include "error.h"
#include "env.h"
#include "open.h"
#include "variables.h"
#include "scan.h"
#include "qregex.h"
#include "sqlmatch.h"
#include "auto_control.h"
#include "indimail_stub.h"
#include "load_mysql.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <mysql.h>
#include <mysqld_error.h>
#include <unistd.h>

stralloc        dbserver = { 0 };
stralloc        dbuser = { 0 };
stralloc        dbpass = { 0 };
stralloc        dbname = { 0 };
stralloc        dbtable = { 0 };

MYSQL          *db_mysql = (MYSQL *) 0;

int
connect_sqldb(char *fn, MYSQL **conn, char **table_name, char **error)
{
	char           *x, *m_timeout;
	int             fd, i = 0;
	unsigned int    next, xlen, mysql_timeout;
	struct stat     st;

	if (conn)
		*conn = (MYSQL *) 0;
	if (db_mysql) {
		if (conn)
			*conn = db_mysql;
		return (0);
	}
	if (!(m_timeout = env_get("MYSQL_TIMEOUT")))
		m_timeout = "30";
	scan_int(m_timeout, (int *) &mysql_timeout);
	if ((fd = open_read(fn)) == -1) {
		if (error)
			*error = error_str(errno);
		if (errno == error_noent)
			return 0;
		return (AM_FILE_ERR);
	}
	if (fstat(fd, &st) == -1) {
		if (error)
			*error = error_str(errno);
		close(fd);
		return (AM_FILE_ERR);
	}
	if (st.st_size <= 0xffffffff) {
		x = mmap(0, st.st_size, PROT_READ, MAP_SHARED, fd, 0);
		xlen = st.st_size;
		/*- 
		 * Format of controlfile.sql should be
		 * server:user:pass:dbname:tablename
		 */
		while ((next = byte_chr(x, xlen, ':')) < xlen) {
			switch (i) {
			case 0:
				if (!stralloc_copyb(&dbserver, x, next)) {
					if (error)
						*error = error_str(errno);
					munmap(x, st.st_size);
					close(fd);
					return (AM_MEMORY_ERR);
				}
				if (!stralloc_0(&dbserver)) {
					if (error)
						*error = error_str(errno);
					munmap(x, st.st_size);
					close(fd);
					return (AM_MEMORY_ERR);
				}
				break;
			case 1:
				if (!stralloc_copyb(&dbuser, x, next)) {
					if (error)
						*error = error_str(errno);
					munmap(x, st.st_size);
					close(fd);
					return (AM_MEMORY_ERR);
				}
				if (!stralloc_0(&dbuser)) {
					if (error)
						*error = error_str(errno);
					munmap(x, st.st_size);
					close(fd);
					return (AM_MEMORY_ERR);
				}
				break;
			case 2:
				if (!stralloc_copyb(&dbpass, x, next)) {
					if (error)
						*error = error_str(errno);
					munmap(x, st.st_size);
					close(fd);
					return (AM_MEMORY_ERR);
				}
				if (!stralloc_0(&dbpass)) {
					if (error)
						*error = error_str(errno);
					munmap(x, st.st_size);
					close(fd);
					return (AM_MEMORY_ERR);
				}
				break;
			case 3:
				if (!stralloc_copyb(&dbname, x, next)) {
					if (error)
						*error = error_str(errno);
					munmap(x, st.st_size);
					close(fd);
					return (AM_MEMORY_ERR);
				}
				if (!stralloc_0(&dbname)) {
					if (error)
						*error = error_str(errno);
					munmap(x, st.st_size);
					close(fd);
					return (AM_MEMORY_ERR);
				}
				break;
			case 4:
				if (!stralloc_copyb(&dbtable, x, next)) {
					if (error)
						*error = error_str(errno);
					munmap(x, st.st_size);
					close(fd);
					return (AM_MEMORY_ERR);
				}
				if (!stralloc_0(&dbtable)) {
					if (error)
						*error = error_str(errno);
					munmap(x, st.st_size);
					close(fd);
					return (AM_MEMORY_ERR);
				}
				break;
			}
			i++;
			++next;
			x += next;
			xlen -= next;
		}
		if (i < 4) {
			if (error)
				*error = "Invalid database config in control file";
			return (AM_CONFIG_ERR);
		}
		if (i == 4) {
			if (!*x || *x == '\n') {
				if (error)
					*error = "Invalid database config in control file";
				return (AM_CONFIG_ERR);
			}
			if (!stralloc_copys(&dbtable, x)) {
				if (error)
					*error = error_str(errno);
				munmap(x, st.st_size);
				close(fd);
				return (AM_MEMORY_ERR);
			}
			if (*(dbtable.s + dbtable.len - 1) == '\n')
				*(dbtable.s + dbtable.len - 1) = 0;
			else if (!stralloc_0(&dbtable)) {
				if (error)
					*error = error_str(errno);
				munmap(x, st.st_size);
				close(fd);
				return (AM_MEMORY_ERR);
			}
		}
	} else {
		if (error)
			*error = "file to large";
		close(fd);
		return (AM_FILE_ERR);
	}
	close(fd);
	munmap(x, st.st_size);
	if (!(db_mysql = in_mysql_init(0))) {
		if (error)
			*error = "mysql_init: no memory";
		return (AM_MEMORY_ERR);
	}
	if (in_mysql_options(db_mysql, MYSQL_READ_DEFAULT_FILE, "indimail.cnf")) {
		if (error)
			*error = "unable to set MYSQL_READ_DEFAULT_FILE";
		return (AM_CONFIG_ERR);
	}
	if (in_mysql_options(db_mysql, MYSQL_READ_DEFAULT_GROUP, "indimail")) {
		if (error)
			*error = "unable to set MYSQL_READ_DEFAULT_GROUP";
		return (AM_CONFIG_ERR);
	}
	if (in_mysql_options(db_mysql, MYSQL_OPT_CONNECT_TIMEOUT, (char *) &mysql_timeout)) {
		if (error)
			*error = "unable to set MYSQL_OPT_CONNECT_TIMEOUT";
		return (AM_CONFIG_ERR);
	}
	if (in_mysql_options(db_mysql, MYSQL_OPT_LOCAL_INFILE, 0)) {
		if (error)
			*error = "unable to set MYSQL_OPT_LOCAL_INFILE";
		return (AM_CONFIG_ERR);
	}
	if (!in_mysql_real_connect(db_mysql, dbserver.s, dbuser.s, dbpass.s, dbname.s, 0, NULL, 0)) {
		if (error)
			*error = (char *) in_mysql_error(db_mysql);
		return (AM_MYSQL_ERR);
	}
	if (conn)
		*conn = db_mysql;
	if (table_name)
		*table_name = dbtable.s;
	return (0);
}

int
create_sqltable(MYSQL *conn, char *table_name, char **error)
{
	static stralloc sql = { 0 };

	if (!stralloc_copys(&sql, "CREATE TABLE ") || !stralloc_cats(&sql, table_name) ||
			!stralloc_cats(&sql, " (email char(64) NOT NULL, timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP on update CURRENT_TIMESTAMP NOT NULL, PRIMARY KEY (email), INDEX timestamp (timestamp))") ||
			!stralloc_0(&sql)) {
		if (error)
			*error = error_str(errno);
		return (AM_MEMORY_ERR);
	}
	if (in_mysql_query(conn, sql.s)) {
		sql.len--;
		if (!stralloc_cats(&sql, ": ") || !stralloc_cats(&sql, (char *) in_mysql_error(conn)) ||
				!stralloc_0(&sql)) {
			if (error)
				*error = error_str(errno);
			return (AM_MEMORY_ERR);
		}
		return (AM_MYSQL_ERR);
	}
	return (0);
}

int
check_db(MYSQL *conn, char *addr, unsigned long *row_count, unsigned long *tmval, char **errStr)
{

	MYSQL_RES      *res;
	MYSQL_ROW       row;
	int             num, m_error;
	static stralloc sql = { 0 };

	if (!conn)
		return (0);
	if (!stralloc_copys(&sql, "select UNIX_TIMESTAMP(timestamp) from ") ||
			!stralloc_cats(&sql, dbtable.s) || !stralloc_cats(&sql, " where email=\"") ||
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
		if ((m_error = in_mysql_errno(conn)) == ER_NO_SUCH_TABLE) {
			if (create_sqltable(conn, dbtable.s, errStr))
				return (AM_MYSQL_ERR);
			return (0);
		} else
		if (m_error == ER_PARSE_ERROR) {
			if (!stralloc_copys(&sql, "select UNIX_TIMESTAMP(timestamp) from ") ||
					!stralloc_cats(&sql, dbtable.s) || !stralloc_cats(&sql, " where email='") ||
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
	if (row_count)
		*row_count = num;
	for (; (row = in_mysql_fetch_row(res));) {
		if (tmval)
			*tmval = scan_ulong(row[0], tmval);
	}
	in_mysql_free_result(res);
	return (num);
}

int
sqlmatch(char *fn, char *addr, int len, char **errStr)
{
	static stralloc controlfile = { 0 };
	int             cntrl_ok;
	MYSQL          *conn;

	if (!len || !*addr || !fn)
		return (0);
	if (!controldir) {
		if (!(controldir = env_get("CONTROLDIR")))
			controldir = auto_control;
	}
	if (errStr)
		*errStr = 0;
	if (!stralloc_copys(&controlfile, controldir) || !stralloc_cats(&controlfile, "/") ||
			!stralloc_cats(&controlfile, fn) || !stralloc_cats(&controlfile, ".sql") ||
			!stralloc_0(&controlfile))
		return AM_MEMORY_ERR;
	if ((cntrl_ok = initMySQLlibrary(errStr)))
		return (0);
	else
	if (!use_sql)
		return (0);
	if ((cntrl_ok = connect_sqldb(controlfile.s, &conn, 0, errStr)) < 0)
		return (cntrl_ok);
	if ((cntrl_ok = check_db(conn, addr, 0, 0, errStr)) < 0)
		return (cntrl_ok);
	return (cntrl_ok ? 1 : 0);
}

void
sqlmatch_close_db(void)
{
	if (!db_mysql)
		return;
	in_mysql_close(db_mysql);
	db_mysql = (MYSQL *) 0;
}
#else
#error "MySQL libs required for -DUSE_SQL"
#endif /*- #ifdef HAS_MYSQL*/
#else  /*- #ifdef USE_SQL */
#warning "not compiled with -DUSE_SQL"
int
sqlmatch(char *fn, char *addr, int len, char **errStr)
{
	return (0);
}

void
sqlmatch_close_db(void)
{
	return;
}
#endif /*- #ifdef USE_SQL */

void
getversion_sqlmatch_c()
{
	static char    *x = "$Id: sqlmatch.c,v 1.12 2020-04-09 16:03:16+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
