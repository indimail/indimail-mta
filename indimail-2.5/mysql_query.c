/*
 * $Log: mysql_query.c,v $
 * Revision 2.8  2010-04-30 14:43:12+05:30  Cprogrammer
 * fixed illegal free of pointer returned by replacestr()
 *
 * Revision 2.7  2010-04-17 09:27:29+05:30  Cprogrammer
 * added fix for MySQL server not started with NO_BACKSLASH_ESCAPES
 *
 * Revision 2.6  2009-02-18 21:30:43+05:30  Cprogrammer
 * removed compiler warning
 *
 * Revision 2.5  2009-02-06 11:39:45+05:30  Cprogrammer
 * ignore return value of write
 *
 * Revision 2.4  2008-10-29 11:17:55+05:30  Cprogrammer
 * return the orignal escape settings
 *
 * Revision 2.3  2008-10-21 21:28:18+05:30  Cprogrammer
 * added disable_mysql_escape()
 *
 * Revision 2.2  2008-09-08 09:50:44+05:30  Cprogrammer
 * function to escape sql queries with mysql_real_escape_string
 *
 * Revision 2.1  2008-08-29 19:15:26+05:30  Cprogrammer
 * make query string safe
 *
 */
#include <stdlib.h>
#include <mysql.h>
#include <mysqld_error.h>
#include <unistd.h>
#include <errno.h>

#ifndef	lint
static char     sccsid[] = "$Id: mysql_query.c,v 2.8 2010-04-30 14:43:12+05:30 Cprogrammer Stab mbhangui $";
#endif

extern char    *replacestr(char *, char *, char *);

static int      _es_opt;

int
_mysql_Query(MYSQL *mysql, char *query_str)
{
	int             i, ret, len, end;
	char           *ptr, *rptr;

	if (_es_opt)
		return(mysql_query(mysql, query_str));
	for (i = 0,ptr = query_str;*ptr;ptr++,i++);
	len = ++i;
	i = (i * 2) + 1;
	errno = 0;
	if (!(ptr = (char *) malloc(sizeof(char) * i)))
		return (-1);
	end = mysql_real_escape_string(mysql, ptr, query_str, len);
	ptr[end] = 0;
	ptr[end - 1] = 0;
	ret = mysql_query(mysql, (const char *) ptr);
	if (mysql_errno(mysql) == ER_PARSE_ERROR) /*- NO_BACKSLASH_ESCAPES stupidity */
	{
		ptr[end - 2] = 0;
		if (!(rptr = replacestr(ptr, "\\\"", "'")))
		{
			i = errno;
			free(ptr);
			errno = i;
			return (-1);
		}
		ret = mysql_query(mysql, (const char *) rptr);
		if (rptr != ptr)
			free(rptr);
	}
	free(ptr);
	return (ret);
}

int
disable_mysql_escape(int opt)
{
	int             orig_opt;

	orig_opt = _es_opt;
	_es_opt = opt;
	return (orig_opt);
}

void
getversion_mysql_query_c()
{
	if (write(2, sccsid, 0) == -1) ;
}
