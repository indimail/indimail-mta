/*
 * $Log: qmail-sql.c,v $
 * Revision 1.12  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.11  2023-02-14 09:16:35+05:30  Cprogrammer
 * renamed auto_uidv, auto_gidv to auto_uidi, auto_gidi
 *
 * Revision 1.10  2021-06-13 17:19:57+05:30  Cprogrammer
 * do chdir(controldir) instead of chdir(auto_sysconfdir)
 *
 * Revision 1.9  2021-02-27 20:59:05+05:30  Cprogrammer
 * changed error to warning for missing MySQL libs
 *
 * Revision 1.8  2020-04-09 16:00:18+05:30  Cprogrammer
 * collapsed stralloc calls
 *
 * Revision 1.7  2019-05-28 10:22:23+05:30  Cprogrammer
 * BUG - return value of insert_db wasn't checked
 *
 * Revision 1.6  2019-04-20 19:52:22+05:30  Cprogrammer
 * load MySQL shared library dynamically
 *
 * Revision 1.5  2018-02-11 21:20:48+05:30  Cprogrammer
 * use USE_SQL to compile sql support
 *
 * Revision 1.4  2018-01-09 11:54:26+05:30  Cprogrammer
 * link with MySQL without indimail
 *
 * Revision 1.3  2017-04-16 19:53:14+05:30  Cprogrammer
 * use auto_sysconfdir instead of auto_qmail
 *
 * Revision 1.2  2016-05-17 19:44:58+05:30  Cprogrammer
 * use auto_control, set by conf-control to set control directory
 *
 * Revision 1.1  2010-04-22 15:21:36+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <unistd.h>
#include "subfd.h"
#include "substdio.h"
#ifdef USE_SQL
#include "hasmysql.h"
#ifdef HAS_MYSQL
#include "auto_uids.h"
#include "auto_control.h"
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include "variables.h"
#include "env.h"
#include "error.h"
#include "fmt.h"
#include "str.h"
#include "sgetopt.h"
#include "stralloc.h"
#include "strerr.h"
#include "sqlmatch.h"
#include "load_mysql.h"
#include <mysql.h>
#include <mysqld_error.h>

#define FATAL "qmail-sql: fatal: "

void
out(const char *str)
{
	if (!str || !*str)
		return;
	if (substdio_puts(subfdout, str) == -1)
		strerr_die2sys(111, FATAL, "write: ");
	return;
}

void
flush()
{
	if (substdio_flush(subfdout) == -1)
		strerr_die2sys(111, FATAL, "write: ");
	return;
}

int
insert_db(MYSQL *conn, const char *fn, const char *table_name, int replace, const char *errStr[])
{

	int             num = 0, m_error;
	static stralloc sql = { 0 };

	if (!conn) {
		if (errStr)
			*errStr = "not connected to MySQL";
		return (0);
	}
	if (!stralloc_copys(&sql, "LOAD DATA LOW_PRIORITY LOCAL INFILE \"") ||
			!stralloc_cats(&sql, fn) ||
			!stralloc_cats(&sql, replace ? "\" REPLACE INTO TABLE " : "\" IGNORE INTO TABLE ") ||
			!stralloc_cats(&sql, table_name) || !stralloc_catb(&sql, " (email)", 8))
		strerr_die2x(111, FATAL, "out of memory");
again:
	if (!stralloc_0(&sql))
		strerr_die2x(111, FATAL, "out of memory");
	if (in_mysql_query(conn, sql.s)) {
		if ((m_error = in_mysql_errno(conn)) == ER_NO_SUCH_TABLE) {
			if (create_sqltable(conn, table_name, errStr))
				return (AM_MYSQL_ERR);
			if (in_mysql_query(conn, sql.s)) {
				sql.len--;
				if (!stralloc_cats(&sql, ": ") || !stralloc_cats(&sql, (char *) in_mysql_error(conn)) || !stralloc_0(&sql))
					strerr_die2x(111, FATAL, "out of memory");
				if (errStr)
					*errStr = sql.s;
				return (AM_MYSQL_ERR);
			}
		} else
		if (m_error == ER_PARSE_ERROR) {
			if (!stralloc_copys(&sql, "LOAD DATA LOW_PRIORITY LOCAL INFILE '") ||
					!stralloc_cats(&sql, fn) ||
					!stralloc_cats(&sql, replace ? "' REPLACE INTO TABLE " : "' IGNORE INTO TABLE ") ||
					!stralloc_cats(&sql, table_name) || !stralloc_catb(&sql, " (email)", 8))
				strerr_die2x(111, FATAL, "out of memory");
			goto again;
		} else {
			sql.len--;
			if (!stralloc_cats(&sql, ": ") || !stralloc_cats(&sql, (char *) in_mysql_error(conn)) ||
					!stralloc_0(&sql))
				strerr_die2x(111, FATAL, "out of memory");
			if (errStr)
				*errStr = sql.s;
			return (AM_MYSQL_ERR);
		}
	}
	if ((num = in_mysql_affected_rows(conn)) == -1) {
		sql.len--;
		if (!stralloc_cats(&sql, ": ") || !stralloc_cats(&sql, (char *) in_mysql_error(conn)) ||
				!stralloc_0(&sql))
			strerr_die2x(111, FATAL, "out of memory");
		if (errStr)
			*errStr = sql.s;
		return (AM_MYSQL_ERR);
	}
	return (num);
}

const char     *usage =
	"usage: qmail-sql [-Sr] [-s mysql_host -u user -p password -d database -t table_name] filename\n"
	"        -S (skip)\n"
	"        -r (replace table)";

int
main(int argc, char **argv)
{
	int             fd, opt, skip_load = 0, replace = 0;
	char           *tname;
	const char     *dbserver, *user, *pass, *dbname, *table_name, *errStr;
	stralloc        fn = {0}, str = {0};
	struct stat     statbuf;
	MYSQL          *conn;
	char            strnum[FMT_ULONG];

	dbserver = user = pass = dbname = table_name = 0;
	while ((opt = getopt(argc, argv, "s:u:p:d:t:Sr")) != opteof) {
		switch (opt) {
		case 's':
			dbserver = optarg;
			break;
		case 'u':
			user = optarg;
			break;
		case 'p':
			pass = optarg;
			break;
		case 'd':
			dbname = optarg;
			break;
		case 't':
			table_name = optarg;
			break;
		case 'S':
			skip_load = 1;
			break;
		case 'r':
			replace = 1;
			break;
		default:
			strerr_die1x(100, usage);
			break;
		}
	}
	if (dbserver && (!user || !pass || !dbname || !table_name))
		strerr_die1x(100, usage);
	if (optind + 1 != argc)
		strerr_die1x(100, usage);
	argc -= optind;
	argv += optind;
	if (!controldir) {
		if (!(controldir = env_get("CONTROLDIR")))
			controldir = auto_control;
	}
	if (chdir(controldir) == -1)
		strerr_die4sys(111, FATAL, "chdir: ", controldir, ": ");
	if (!stralloc_copys(&fn, *argv++) || !stralloc_0(&fn))
		strerr_die2x(111, FATAL, "out of memory");
	if (stat(fn.s, &statbuf))
		strerr_die4sys(111, FATAL, "stat: ", fn.s, ": ");
	--fn.len;
	if (!stralloc_cats(&fn, ".sql") || !stralloc_0(&fn))
		strerr_die2x(111, FATAL, "out of memory");
	if (stat(fn.s, &statbuf) && (!dbserver || !user || !pass || !dbname || !table_name))
		strerr_die1x(100, usage);
	if (dbserver && user && pass && dbname && table_name) {
		if ((fd = open(fn.s, O_CREAT|O_TRUNC|O_WRONLY, 0644)) == -1)
			strerr_die4sys(111, FATAL, "open: ", fn.s, ": ");
		if (!stralloc_copys(&str, dbserver) || !stralloc_catb(&str, ":", 1) ||
				!stralloc_cats(&str, user) || !stralloc_catb(&str, ":", 1) ||
				!stralloc_cats(&str, pass) || !stralloc_catb(&str, ":", 1) ||
				!stralloc_cats(&str, dbname) || !stralloc_catb(&str, ":", 1) ||
				!stralloc_cats(&str, table_name) || !stralloc_catb(&str, "\n", 1))
			strerr_die2x(111, FATAL, "out of memory");
		if (write(fd, str.s, str.len) == -1)
			strerr_die4sys(111, FATAL, "write: ", fn.s, ": ");
		if (fchown(fd, auto_uidi, auto_gidi))
			strerr_die4sys(111, FATAL, "fchown: ", fn.s, ": ");
		if (close(fd))
			strerr_die4sys(111, FATAL, "close: ", fn.s, ": ");
		out("created file ");
		out(fn.s);
		out("\n");
	}
	if (initMySQLlibrary(&errStr))
		strerr_die3x(111, FATAL, "initMySQLlibrary: couldn't load MySQL shared library: ", errStr);
	else
	if (!use_sql)
		strerr_die3x(111, FATAL, "initMySQLlibrary: couldn't load MySQL shared library: ", errStr);
	if (!skip_load) {
		if (connect_sqldb(fn.s, &conn, &tname, &errStr) < 0)
			strerr_die3x(111, FATAL, "MySQL connect: ", errStr);
		fn.len -= 5;
		if (!stralloc_0(&fn)) {
			in_mysql_close(conn);
			strerr_die2x(111, FATAL, "out of memory");
		}
		if ((opt = insert_db(conn, fn.s, tname, replace, &errStr)) < 0) {
			in_mysql_close(conn);
			strerr_die3x(111, FATAL, "insert_db: ", errStr);
		}
		in_mysql_close(conn);
		strnum[fmt_ulong(strnum, opt)] = 0;
		out(strnum);
		out(" rows affected\n");
		flush();
		return (opt);
	}
	flush();
	return (0);
}
#else
#warning "MySQL libs required for -DUSE_SQL"
int
main(int argc, char **argv)
{
	substdio_puts(subfderr, "This program was compiled with MySQL lib missing. It won't function\n");
	substdio_flush(subfderr);
	_exit(111);
}
#endif
#else
#warning "not compiled with -DUSE_SQL."
int
main(int argc, char **argv)
{
	substdio_puts(subfderr, "not compiled with -DUSE_SQL. Probably MySQL lib is missing\n");
	substdio_flush(subfderr);
	_exit(111);
}
#endif

void
getversion_qmail_sql_c()
{
	const char     *x = "$Id: qmail-sql.c,v 1.12 2024-05-09 22:03:17+05:30 mbhangui Exp mbhangui $";

	x++;
}
