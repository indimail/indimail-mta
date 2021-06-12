/*
 * $Log: sql-database.c,v $
 * Revision 1.3  2021-06-12 19:57:29+05:30  Cprogrammer
 * removed #include "auto_qmail.h"
 *
 * Revision 1.2  2021-02-27 20:59:43+05:30  Cprogrammer
 * changed error to warning for missing MySQL libs
 *
 * Revision 1.1  2020-04-09 16:34:39+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <unistd.h>
#include "subfd.h"
#include "substdio.h"
#ifdef USE_SQL
#include "hasmysql.h"
#ifdef HAS_MYSQL
#include <ctype.h>
#include "auto_uids.h"
#include "auto_control.h"
#include "auto_sysconfdir.h"
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include "variables.h"
#include "env.h"
#include "open.h"
#include "error.h"
#include "fmt.h"
#include "str.h"
#include "sgetopt.h"
#include "stralloc.h"
#include "strerr.h"
#include "sqlmatch.h"
#include "getln.h"
#include "load_mysql.h"
#include <mysql.h>
#include <mysqld_error.h>

#define FATAL "sql-database: fatal: "

void
out(char *str)
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

void
logerr(char *s)
{
	if (substdio_puts(subfderr, s) == -1)
		_exit(111);
}

void
logerrf(char *s)
{
	if (substdio_puts(subfderr, s) == -1)
		_exit(111);
	if (substdio_flush(subfderr) == -1)
		_exit(111);
}

void
my_error(char *s1, char *s2, int exit_val)
{
	logerr(s1);
	if (s2) {
		logerr(": ");
		logerr(s2);
	}
	if (errno && exit_val > 0) {
		logerr(": ");
		logerr(error_str(errno));
	}
	logerrf("\n");
	if (exit_val)
		_exit(exit_val > 0 ? exit_val : -exit_val);
}

static int
create_db_table(MYSQL *conn, char *table_name, char **error)
{
	static stralloc sql = { 0 };

	if (!stralloc_copys(&sql, "CREATE TABLE ") || !stralloc_cats(&sql, table_name) ||
			!stralloc_cats(&sql, " (addr char(64) NOT NULL, value char(128) NOT NULL, timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP on update CURRENT_TIMESTAMP NOT NULL, PRIMARY KEY (addr), INDEX timestamp (timestamp))") ||
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
insert_db(MYSQL *conn, char *filename, char *table_name, int replace, char **errStr)
{

	int             i, num = 0, total = 0, m_error, match, fd;
	static stralloc line = {0}, sql = {0};
	char           *ptr;
	struct substdio ssin;
	char            inbuf[4096];

	if (!conn) {
		if (errStr)
			*errStr = "not connected to MySQL";
		return (0);
	}
	if ((fd = open_read(filename)) == -1) {
		strerr_warn4(FATAL, "open: ", filename, ": ", &strerr_sys);
		return (-1);
	}
	substdio_fdbuf(&ssin, read, fd, inbuf, sizeof(inbuf));
	for (;;) {
		if (getln(&ssin, &line, &match, '\n') == -1) {
			strerr_warn4(FATAL, "read: ", filename, ": ", &strerr_sys);
			close(fd);
			return (-1);
		}
		if (!match && line.len == 0)
			break;
		if (line.len && (line.s[0] == '.'))
			break;
		line.len--;
		line.s[line.len] = 0; /*- remove newline */
		match = str_chr(line.s, '#');
		if (line.s[match])
			line.s[match] = 0;
		for (ptr = line.s; *ptr && isspace((int) *ptr); ptr++);
		if (!*ptr)
			continue;
		i = str_chr(line.s, ':');
		if (!line.s[i])
			continue;
		if (!stralloc_copyb(&sql, replace ? "replace low_priority into " : "insert  low_priority into ", 26) || 
				!stralloc_cats(&sql, table_name) ||
				!stralloc_catb(&sql, " (addr, value) values (\"", 24) ||
				!stralloc_catb(&sql, line.s + 1, i - 1) ||
				!stralloc_catb(&sql, "\", \"", 4) ||
				!stralloc_catb(&sql, line.s + i + 1, line.len - i - 2) ||
				!stralloc_catb(&sql, "\")", 2) ||
				!stralloc_0(&sql))
			strerr_die2sys(111, FATAL, "out of memory");
again:
		if (in_mysql_query(conn, sql.s)) {
			if ((m_error = in_mysql_errno(conn)) == ER_NO_SUCH_TABLE) {
				if (create_db_table(conn, table_name, errStr))
					return (AM_MYSQL_ERR);
				if (in_mysql_query(conn, sql.s)) {
					sql.len--;
					if (!stralloc_cats(&sql, ": ") || !stralloc_cats(&sql, (char *) in_mysql_error(conn)) || !stralloc_0(&sql))
						strerr_die2sys(111, FATAL, "out of memory");
					if (errStr)
						*errStr = sql.s;
					return (AM_MYSQL_ERR);
				}
			} else
			if (m_error == ER_PARSE_ERROR) {
				if (!stralloc_copyb(&sql, replace ? "replace low_priority into " : "insert  low_priority into ", 26) || 
						!stralloc_cats(&sql, table_name) ||
						!stralloc_catb(&sql, " (addr, value) values ('", 24) ||
						!stralloc_catb(&sql, line.s + 1, i - i) ||
						!stralloc_catb(&sql, "', '", 4) ||
						!stralloc_catb(&sql, line.s + i + 1, line.len - i - 2) ||
						!stralloc_catb(&sql, "')", 2) ||
						!stralloc_0(&sql))
					strerr_die2sys(111, FATAL, "out of memory");
				goto again;
			} else {
				sql.len--;
				if (!stralloc_cats(&sql, ": ") || !stralloc_cats(&sql, (char *) in_mysql_error(conn)) ||
						!stralloc_0(&sql))
					strerr_die2sys(111, FATAL, "out of memory");
				if (errStr)
					*errStr = sql.s;
				my_error("mysql_query", sql.s, 0);
				continue;
			}
		}
		if ((num = in_mysql_affected_rows(conn)) == -1) {
			sql.len--;
			if (!stralloc_cats(&sql, ": ") || !stralloc_cats(&sql, (char *) in_mysql_error(conn)) ||
					!stralloc_0(&sql))
				strerr_die2sys(111, FATAL, "out of memory");
			if (errStr)
				*errStr = sql.s;
			return (AM_MYSQL_ERR);
		}
		total += num;
	} /*- for (;;) */
	close(fd);

	return (total);
}

char           *usage =
	"usage: sql-database [-Sr] [-s mysql_host -u user -p password -d database -t table_name] filename\n"
	"        -S (skip)\n"
	"        -r (replace table)";

int
main(int argc, char **argv)
{
	int             fd, opt, skip_load = 0, replace = 0;
	char           *dbserver, *user, *pass, *dbname, *table_name, *tname, *errStr;
	stralloc        filename = {0}, str = {0};
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
#if 0
	while (optind < argc) {
		fprintf(stderr, "%d %s\n", optind, argv[optind]);
		optind++;
	}
#endif
	if (optind + 1 != argc)
		strerr_die1x(100, usage);
	argc -= optind;
	argv += optind;
	if (!controldir) {
		if (!(controldir = env_get("CONTROLDIR")))
			controldir = auto_control;
	}
	if (!stralloc_copys(&filename, controldir) || !stralloc_catb(&filename, "/", 1) ||
			!stralloc_cats(&filename, *argv++) || !stralloc_0(&filename))
		strerr_die2sys(111, FATAL, "out of memory");
	if (chdir(auto_sysconfdir) == -1)
		strerr_die4sys(111, FATAL, "chdir: ", auto_sysconfdir, ": ");
	if (stat(filename.s, &statbuf))
		my_error("stat", filename.s, 111);
	--filename.len;
	if (!stralloc_cats(&filename, ".sql") || !stralloc_0(&filename))
		strerr_die2sys(111, FATAL, "out of memory");
	if (stat(filename.s, &statbuf) && (!dbserver || !user || !pass || !dbname || !table_name))
		strerr_die3sys(100, FATAL, filename.s, ": ");
	if (dbserver && user && pass && dbname && table_name) {
		if ((fd = open(filename.s, O_CREAT|O_TRUNC|O_WRONLY, 0644)) == -1)
			my_error("open", filename.s, 111);
		if (fchown(fd, auto_uidv, auto_gidv))
			my_error("chown", filename.s, 111);
		if (!stralloc_copys(&str, dbserver) || !stralloc_catb(&str, ":", 1) ||
				!stralloc_cats(&str, user) || !stralloc_catb(&str, ":", 1) ||
				!stralloc_cats(&str, pass) || !stralloc_catb(&str, ":", 1) ||
				!stralloc_cats(&str, dbname) || !stralloc_catb(&str, ":", 1) ||
				!stralloc_cats(&str, table_name) || !stralloc_catb(&str, "\n", 1))
			strerr_die2sys(111, FATAL, "out of memory");
		if (write(fd, str.s, str.len) == -1)
			my_error("write", filename.s, 111);
		if (close(fd))
			my_error("close", filename.s, 111);
		out("created file ");
		out(filename.s);
		out("\n");
	}
	if (initMySQLlibrary(&errStr))
		my_error("initMySQLlibrary: couldn't load MySQL shared library", errStr, 111);
	else
	if (!use_sql)
		my_error("initMySQLlibrary: couldn't load MySQL shared library", errStr, 111);
	if (!skip_load) {
		if (connect_sqldb(filename.s, &conn, &tname, &errStr) < 0)
			my_error("MySQL connect", errStr, 111);
		filename.len -= 5;
		if (!stralloc_0(&filename)) {
			in_mysql_close(conn);
			strerr_die2sys(111, FATAL, "out of memory");
		}
		if ((opt = insert_db(conn, filename.s, tname, replace, &errStr)) < 0) {
			in_mysql_close(conn);
			my_error("insert_db", errStr, 111);
			return (0);
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
getversion_sql_database_c()
{
	static char    *x = "$Id: sql-database.c,v 1.3 2021-06-12 19:57:29+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
