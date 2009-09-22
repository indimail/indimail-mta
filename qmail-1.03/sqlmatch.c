/*
 * $Log: sqlmatch.c,v $
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
#include "hasindimail.h"
#ifdef INDIMAIL
#include "stralloc.h"
#include "byte.h"
#include "error.h"
#include "env.h"
#include "open.h"
#include "variables.h"
#include "scan.h"
#include "qregex.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <mysql.h>
#include <mysqld_error.h>
#include <unistd.h>

struct stralloc dbserver = {0};
struct stralloc dbuser = {0};
struct stralloc dbpass = {0};
struct stralloc dbname = {0};
struct stralloc dbtable = {0};
MYSQL          *conn = (MYSQL *) 0;

static int
connect_sqldb(char *fn, char **error)
{
	char           *x, *mysql_timeout;
	int             fd, i = 0;
	unsigned int    next, xlen;
	struct stat     st;
	static MYSQL    mysql;

	if (conn)
		return (0);
	if (!(mysql_timeout = env_get("MYSQL_TIMEOUT")))
		mysql_timeout = "30";
	if ((fd = open_read(fn)) == -1)
	{
		if (error) 
			*error = error_str(errno);
		if (errno == error_noent)
			return 0;
		return (AM_FILE_ERR);
	}
	if (fstat(fd, &st) == -1)
	{
		if (error) 
			*error = error_str(errno);
		close(fd);
		return (AM_FILE_ERR);
	}
	if (st.st_size <= 0xffffffff)
	{
		x = mmap(0, st.st_size, PROT_READ, MAP_SHARED, fd, 0);
		xlen = st.st_size;
		while ((next = byte_chr(x, xlen, ':')) < xlen)
		{
			switch (i)
			{
			case 0:
				if (!stralloc_copyb(&dbserver, x, next))
				{
					if (error) 
						*error = error_str(errno);
					munmap(x, st.st_size);
					close(fd);
					return (AM_MEMORY_ERR);
				}
				if (!stralloc_0(&dbserver))
				{
					if (error) 
						*error = error_str(errno);
					munmap(x, st.st_size);
					close(fd);
					return (AM_MEMORY_ERR);
				}
				break;
			case 1:
				if (!stralloc_copyb(&dbuser, x, next))
				{
					if (error) 
						*error = error_str(errno);
					munmap(x, st.st_size);
					close(fd);
					return (AM_MEMORY_ERR);
				}
				if (!stralloc_0(&dbuser))
				{
					if (error) 
						*error = error_str(errno);
					munmap(x, st.st_size);
					close(fd);
					return (AM_MEMORY_ERR);
				}
				break;
			case 2:
				if (!stralloc_copyb(&dbpass, x, next))
				{
					if (error) 
						*error = error_str(errno);
					munmap(x, st.st_size);
					close(fd);
					return (AM_MEMORY_ERR);
				}
				if (!stralloc_0(&dbpass))
				{
					if (error) 
						*error = error_str(errno);
					munmap(x, st.st_size);
					close(fd);
					return (AM_MEMORY_ERR);
				}
				break;
			case 3:
				if (!stralloc_copyb(&dbname, x, next))
				{
					if (error) 
						*error = error_str(errno);
					munmap(x, st.st_size);
					close(fd);
					return (AM_MEMORY_ERR);
				}
				if (!stralloc_0(&dbname))
				{
					if (error) 
						*error = error_str(errno);
					munmap(x, st.st_size);
					close(fd);
					return (AM_MEMORY_ERR);
				}
				break;
			case 4:
				if (!stralloc_copyb(&dbtable, x, next))
				{
					if (error) 
						*error = error_str(errno);
					munmap(x, st.st_size);
					close(fd);
					return (AM_MEMORY_ERR);
				}
				if (!stralloc_0(&dbtable))
				{
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
		if (i < 4)
		{
			if (error)
				*error = "Invalid db.conf";
			return (AM_CONFIG_ERR);
		}
		if (i == 4)
		{
			if (!*x || *x == '\n')
			{
				if (error)
					*error = "Invalid db.conf";
				return (AM_CONFIG_ERR);
			}
			if (!stralloc_copys(&dbtable, x))
			{
				if (error) 
					*error = error_str(errno);
				munmap(x, st.st_size);
				close(fd);
				return (AM_MEMORY_ERR);
			}
			if (*(dbtable.s + dbtable.len -1) == '\n')
				*(dbtable.s + dbtable.len -1) = 0;
			else
			if (!stralloc_0(&dbtable))
			{
				if (error) 
					*error = error_str(errno);
				munmap(x, st.st_size);
				close(fd);
				return (AM_MEMORY_ERR);
			}
		}
	} else
	{
		if (error)
			*error = "file to large";
		close(fd);
		return (AM_FILE_ERR);
	}
	munmap(x, st.st_size);
	close(fd);
	if (!mysql_init(&mysql))
	{
		if (error)
			*error = "mysql_init: no memory";
		return (AM_MEMORY_ERR);
	}
	if (mysql_options(&mysql, MYSQL_OPT_CONNECT_TIMEOUT, mysql_timeout))
	{
		if (error)
			*error = "Invalid options in mysql config";
		return (AM_CONFIG_ERR);
	}
	if (!mysql_real_connect(&mysql, dbserver.s, dbuser.s, dbpass.s, dbname.s, 0, NULL, 0))
	{
		if (error)
			*error = (char *) mysql_error(&mysql);
		return (AM_MYSQL_ERR);
	}
	conn = &mysql;
	return (0);
}

void
sqlmatch_close_db(void)
{
	if (!conn)
		return;
	mysql_close(conn);
	conn = (MYSQL *) 0;
}

static int
create_sqltable(MYSQL *conn, char **error)
{
	static stralloc sql = { 0 };

	if (!stralloc_copys(&sql, "create table "))
	{
		if (error)
			*error = error_str(errno);
		return (AM_MEMORY_ERR);
	}
	if (!stralloc_cats(&sql, dbtable.s))
	{
		if (error)
			*error = error_str(errno);
		return (AM_MEMORY_ERR);
	}
	if (!stralloc_cats(&sql, " (email char(64) NOT NULL, timestamp timestamp NOT NULL, \
		primary key (email), index timestamp (timestamp))"))
	{
		if (error)
			*error = error_str(errno);
		return (AM_MEMORY_ERR);
	}
	if (!stralloc_0(&sql))
	{
		if (error)
			*error = error_str(errno);
		return (AM_MEMORY_ERR);
	}
	if (mysql_query(conn, sql.s))
	{
		sql.len--;
		if (!stralloc_cats(&sql, ": "))
		{
			if (error)
				*error = error_str(errno);
			return (AM_MEMORY_ERR);
		}
		if (!stralloc_cats(&sql, (char *) mysql_error(conn)))
		{
			if (error)
				*error = (char *) mysql_error(conn);
			return (AM_MEMORY_ERR);
		}
		if (!stralloc_0(&sql))
		{
			if (error)
				*error = error_str(errno);
			return (AM_MEMORY_ERR);
		}
		return (AM_MYSQL_ERR);
	}
	return (0);
}

int
check_db(char *addr, unsigned long *row_count, unsigned long *tmval, char *envStr, char **errStr)
{

	MYSQL_RES      *res;
	MYSQL_ROW       row;
	int             num;
	static stralloc envStore = { 0 };
	static stralloc sql = { 0 };

	if (!conn)
		return (0);
	if (!stralloc_copys(&sql, "select UNIX_TIMESTAMP(timestamp) from "))
	{
		if (errStr) 
			*errStr = error_str(errno);
		return (AM_MEMORY_ERR);
	}
	if (!stralloc_cats(&sql, dbtable.s))
	{
		if (errStr) 
			*errStr = error_str(errno);
		return (AM_MEMORY_ERR);
	}
	if (!stralloc_cats(&sql, " where email = '"))
	{
		if (errStr) 
			*errStr = error_str(errno);
		return (AM_MEMORY_ERR);
	}
	if (!stralloc_cats(&sql, addr))
	{
		if (errStr) 
			*errStr = error_str(errno);
		return (AM_MEMORY_ERR);
	}
	if (!stralloc_cats(&sql, "'"))
	{
		if (errStr) 
			*errStr = error_str(errno);
		return (AM_MEMORY_ERR);
	}
	if (!stralloc_0(&sql))
	{
		if (errStr) 
			*errStr = error_str(errno);
		return (AM_MEMORY_ERR);
	}
	if (mysql_query(conn, sql.s))
	{
		if (mysql_errno(conn) == ER_NO_SUCH_TABLE)
		{
			if (create_sqltable(conn, errStr))
				return (AM_MYSQL_ERR);
			return (0);
		}
		sql.len--;
		if (!stralloc_cats(&sql, ": "))
		{
			if (errStr) 
				*errStr = error_str(errno);
			return (AM_MEMORY_ERR);
		}
		if (!stralloc_cats(&sql, (char *) mysql_error(conn)))
		{
			if (errStr) 
				*errStr = (char *) mysql_error(conn);
			return (AM_MEMORY_ERR);
		}
		if (!stralloc_0(&sql))
		{
			if (errStr) 
				*errStr = error_str(errno);
			return (AM_MEMORY_ERR);
		}
		if (errStr)
			*errStr = sql.s;
		return (AM_MYSQL_ERR);
	}
	if (!(res = mysql_store_result(conn)))
	{
		sql.len--;
		if (!stralloc_cats(&sql, "mysql_store_result: "))
		{
			if (errStr) 
				*errStr = error_str(errno);
			return (AM_MEMORY_ERR);
		}
		if (!stralloc_cats(&sql, (char *) mysql_error(conn)))
		{
			if (errStr) 
				*errStr = (char *) mysql_error(conn);
			return (AM_MEMORY_ERR);
		}
		if (!stralloc_0(&sql))
		{
			if (errStr) 
				*errStr = error_str(errno);
			return (AM_MEMORY_ERR);
		}
		return (AM_MYSQL_ERR);
	}
	num = mysql_num_rows(res);
	if (row_count)
		*row_count = num;
	for (;(row = mysql_fetch_row(res));)
	{
		if (tmval)
			*tmval = scan_ulong(row[0], tmval);
		if (envStr)
		{
			if (!stralloc_copys(&envStore, row[1]))
			{
				if (errStr) 
					*errStr = error_str(errno);
				mysql_free_result(res);
				return (AM_MEMORY_ERR);
			}
		}
	}
	mysql_free_result(res);
	return (num);
}

int
sqlmatch(char *fn, char *addr, int len, char **errStr)
{
	static stralloc controlfile = {0};
	int             cntrl_ok;

	if (!len || !*addr || !fn)
		return (0);
	if (!controldir)
	{
		if(!(controldir = env_get("CONTROLDIR")))
			controldir = "control";
	}
	if (errStr)
		*errStr = 0;
	if (!stralloc_copys(&controlfile, controldir))
		return AM_MEMORY_ERR;
	if (!stralloc_cats(&controlfile, "/"))
		return AM_MEMORY_ERR;
	if (!stralloc_cats(&controlfile, fn))
		return AM_MEMORY_ERR;
	if (!stralloc_cats(&controlfile, ".sql"))
		return AM_MEMORY_ERR;
	if (!stralloc_0(&controlfile))
		return AM_MEMORY_ERR;
	if ((cntrl_ok = connect_sqldb(controlfile.s, errStr)) < 0)
		return (cntrl_ok);
	if ((cntrl_ok = check_db(addr, 0, 0, 0, errStr)) < 0)
		return (cntrl_ok);
	sqlmatch_close_db();
	return (cntrl_ok ? 1 : 0);
}
#else
#warning "not compiled with -DINDIMAIL"
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
#endif

void
getversion_sqlmatch_c()
{
	static char    *x = "$Id: sqlmatch.c,v 1.6 2009-09-07 15:33:31+05:30 Cprogrammer Stab mbhangui $";

#ifdef INDIMAIL
	x = sccsidh;
#endif
	x++;
}
