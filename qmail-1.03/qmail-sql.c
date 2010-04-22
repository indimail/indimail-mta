/*
 * $Log: qmail-sql.c,v $
 * Revision 1.1  2010-04-22 15:21:36+05:30  Cprogrammer
 * Initial revision
 *
 */

#include <unistd.h>
#include "hasindimail.h"
#include "subfd.h"
#include "substdio.h"
#ifdef INDIMAIL
#include "auto_uids.h"
#include "auto_qmail.h"
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
#include <mysql.h>
#include <mysqld_error.h>

#define FATAL "qmail-sql: fatal: "

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
	logerr(": ");
	if (s2)
	{
		logerr(s2);
		logerr(": ");
	}
	if (exit_val > 0)
		logerr(error_str(errno));
	logerrf("\n");
	_exit(exit_val > 0 ? exit_val : -exit_val);
}

void
die_nomem()
{
	substdio_putsflush(subfderr, "qmail-sql: fatal: out of memory\n");
	_exit(111);
}

void
die_control()
{
	substdio_putsflush(subfderr, "unable to read controls\n");
	_exit(111);
}

int
insert_db(MYSQL *conn, char *filename, char *table_name, int replace, char **errStr)
{

	int             num, m_error;
	static stralloc sql = { 0 };

	if (!conn)
	{
		if (errStr)
			*errStr = "not connected to MySQL";
		return (0);
	}
	if (!stralloc_copys(&sql, "LOAD DATA LOW_PRIORITY LOCAL INFILE \""))
		die_nomem();
	if (!stralloc_cats(&sql, filename))
		die_nomem();
	if (!stralloc_cats(&sql, replace ? "\"REPLACE INTO TABLE " : "\"IGNORE INTO TABLE "))
		die_nomem();
	if (!stralloc_cats(&sql, table_name))
		die_nomem();
	if (!stralloc_catb(&sql, " (email)", 8))
		die_nomem();
again:
	if (!stralloc_0(&sql))
		die_nomem();
	if (mysql_query(conn, sql.s))
	{
		if ((m_error = mysql_errno(conn)) == ER_NO_SUCH_TABLE)
		{
			if (create_sqltable(conn, table_name, errStr))
				return (AM_MYSQL_ERR);
			if (mysql_query(conn, sql.s))
			{
				sql.len--;
				if (!stralloc_cats(&sql, ": "))
					die_nomem();
				if (!stralloc_cats(&sql, (char *) mysql_error(conn)))
					die_nomem();
				if (!stralloc_0(&sql))
					die_nomem();
				if (errStr)
					*errStr = sql.s;
				return (AM_MYSQL_ERR);
			}
		} else
		if (m_error == ER_PARSE_ERROR)
		{
			if (!stralloc_copys(&sql, "LOAD DATA LOW_PRIORITY LOCAL INFILE '"))
				die_nomem();
			if (!stralloc_cats(&sql, filename))
				die_nomem();
			if (!stralloc_cats(&sql, "'IGNORE INTO TABLE "))
				die_nomem();
			if (!stralloc_cats(&sql, table_name))
				die_nomem();
			if (!stralloc_catb(&sql, " (email)", 8))
				die_nomem();
			goto again;
		} else
		{
			sql.len--;
			if (!stralloc_cats(&sql, ": "))
				die_nomem();
			if (!stralloc_cats(&sql, (char *) mysql_error(conn)))
				die_nomem();
			if (!stralloc_0(&sql))
				die_nomem();
			if (errStr)
				*errStr = sql.s;
			return (AM_MYSQL_ERR);
		}
	}
	if ((num = mysql_affected_rows(conn)) == -1)
	{
		sql.len--;
		if (!stralloc_cats(&sql, ": "))
			die_nomem();
		if (!stralloc_cats(&sql, (char *) mysql_error(conn)))
			die_nomem();
		if (!stralloc_0(&sql))
			die_nomem();
		if (errStr)
			*errStr = sql.s;
		return (AM_MYSQL_ERR);
	}
	return (num);
}

char           *usage =
	"usage: qmail-sql [-Sr] [-s mysql_host -u user -p password -d database -t table_name] filename\n"
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
	if (optind + 1 != argc)
		strerr_die1x(100, usage);
	argc -= optind;
	argv += optind;
	if (!controldir) {
		if (!(controldir = env_get("CONTROLDIR")))
			controldir = "control";
	}
	if (!stralloc_copys(&filename, controldir))
		die_nomem();
	if (!stralloc_catb(&filename, "/", 1))
		die_nomem();
	if (!stralloc_cats(&filename, *argv++))
		die_nomem();
	if (!stralloc_0(&filename))
		die_nomem();
	if (chdir(auto_qmail) == -1)
		die_control();
	if (stat(filename.s, &statbuf))
		my_error("stat", filename.s, 111);
	--filename.len;
	if (!stralloc_cats(&filename, ".sql"))
		die_nomem();
	if (!stralloc_0(&filename))
		die_nomem();
	if (stat(filename.s, &statbuf) && (!dbserver || !user || !pass || !dbname || !table_name))
		strerr_die1x(100, usage);
	if (!stralloc_0(&filename))
		die_nomem();
	if (dbserver && user && pass && dbname && table_name) {
		if ((fd = open(filename.s, O_CREAT|O_TRUNC|O_WRONLY, 0644)) == -1)
			my_error("open", filename.s, 111);
		if (fchown(fd, auto_uidv, auto_gidv))
			my_error("chown", filename.s, 111);
		if (!stralloc_copys(&str, dbserver))
			die_nomem();
		if (!stralloc_catb(&str, ":", 1))
			die_nomem();
		if (!stralloc_cats(&str, user))
			die_nomem();
		if (!stralloc_catb(&str, ":", 1))
			die_nomem();
		if (!stralloc_cats(&str, pass))
			die_nomem();
		if (!stralloc_catb(&str, ":", 1))
			die_nomem();
		if (!stralloc_cats(&str, dbname))
			die_nomem();
		if (!stralloc_catb(&str, ":", 1))
			die_nomem();
		if (!stralloc_cats(&str, table_name))
			die_nomem();
		if (!stralloc_catb(&str, "\n", 1))
			die_nomem();
		if (write(fd, str.s, str.len) == -1)
			my_error("write", filename.s, 111);
		if (close(fd))
			my_error("close", filename.s, 111);
		out("created file ");
		out(filename.s);
		out("\n");
	}
	if (!skip_load)
	{
		if (connect_sqldb(filename.s, &conn, &tname, &errStr) < 0)
			my_error("MySQL connect", errStr, 111);
		filename.len -= 6;
		if (!stralloc_0(&filename))
			die_nomem();
		opt = insert_db(conn, filename.s, tname, replace, &errStr);
		mysql_close(conn);
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
#warning "not compiled with -DINDIMAIL. Probably IndiMail is missing"
int
main(int argc, char **argv)
{
	substdio_puts(subfderr, "not compiled with -DINDIMAIL. Probably IndiMail is missing\n");
	substdio_flush(subfderr);
	_exit(111);
}
#endif

void
getversion_qmail_sql_c()
{
	static char    *x = "$Id: qmail-sql.c,v 1.1 2010-04-22 15:21:36+05:30 Cprogrammer Exp mbhangui $";

#ifdef INDIMAIL
	x = sccsidh;
#endif
	x++;
}
