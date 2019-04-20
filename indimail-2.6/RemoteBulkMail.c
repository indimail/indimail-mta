/*
 * $Log: RemoteBulkMail.c,v $
 * Revision 2.17  2018-10-30 19:03:11+05:30  Cprogrammer
 * skip MYSQL_READ_DEFAULT_FILE, MYSQL_READ_DEFAULT_GROUP if port or socket is provided
 * >> use TCP if port is provided and unix_socket is not defi
 *
 * Revision 2.16  2018-10-29 20:15:30+05:30  Cprogrammer
 * MariaDB bug fix for mysql_options(), mysql_real_connect() - MYSQL_READ_DEFAULT_FILE
 *
 * Revision 2.15  2018-09-11 10:47:35+05:30  Cprogrammer
 * fixed compiler warnings
 *
 * Revision 2.14  2018-03-31 20:29:29+05:30  Cprogrammer
 * display mysql_option() error index
 *
 * Revision 2.13  2018-03-27 10:42:12+05:30  Cprogrammer
 * set use_ssl if specified in BULK_HOST
 *
 * Revision 2.12  2018-03-21 11:12:52+05:30  Cprogrammer
 * added error_mysql_options_str() function to display the exact mysql_option() error
 *
 * Revision 2.11  2016-05-17 14:56:36+05:30  Cprogrammer
 * use control directory defined by configure
 *
 * Revision 2.10  2010-04-15 14:13:10+05:30  Cprogrammer
 * added flags argument to mysql_real_connect()
 *
 * Revision 2.9  2010-02-24 09:11:16+05:30  Cprogrammer
 * use BULK_SOCKET, BULK_VPORT env variable to override indimail.cnf variables
 *
 * Revision 2.8  2010-02-19 16:18:43+05:30  Cprogrammer
 * allow host:user:password:socket/port format for mysql host
 *
 * Revision 2.7  2008-09-08 09:51:25+05:30  Cprogrammer
 * removed mysql_escape
 * use BULK_SOCKET for unix domain socket name
 *
 * Revision 2.6  2008-05-28 16:37:37+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 2.5  2005-12-29 22:49:24+05:30  Cprogrammer
 * use getEnvConfigStr to set variables from environment variables
 *
 * Revision 2.4  2004-05-12 08:59:00+05:30  Cprogrammer
 * bulkmail table made remote
 *
 * Revision 2.3  2003-02-01 14:12:09+05:30  Cprogrammer
 * change for CopyEmailFile() change
 *
 * Revision 2.2  2002-10-27 21:29:22+05:30  Cprogrammer
 * used generic function create_table()
 *
 * Revision 2.1  2002-08-04 23:36:38+05:30  Cprogrammer
 * added mysql_escape for emailid
 *
 * Revision 1.4  2002-08-03 04:24:51+05:30  Cprogrammer
 * mysql string fix
 *
 * Revision 1.3  2002-03-03 16:14:47+05:30  Cprogrammer
 * create table bulmail if mysql_errno is ER_NO_SUCH_TABLE
 *
 * Revision 1.2  2002-03-03 11:49:18+05:30  Cprogrammer
 * Treat table bulmail non existence as "no bulk mails to be delivered"; hence return success
 *
 * Revision 1.1  2002-03-02 01:14:11+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: RemoteBulkMail.c,v 2.17 2018-10-30 19:03:11+05:30 Cprogrammer Exp mbhangui $";
#endif

#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <mysqld_error.h>

static MYSQL   *bulk_host_connect();

int
RemoteBulkMail(email, domain, homedir)
	const char     *email, *domain, *homedir;
{
	char            SqlBuf[SQL_BUF_SIZE], bulkdir[MAX_BUFF], TmpBuf[MAX_BUFF + 2];
	struct stat     statbuf;
	MYSQL_ROW       row;
	MYSQL_RES      *res;
	MYSQL          *mysqlptr;
	char           *ptr;
	int             status;

	if (vauth_open((char *) 0))
		return (1);
	if (!(mysqlptr = bulk_host_connect()))
		return (1);
	snprintf(SqlBuf, SQL_BUF_SIZE, "select high_priority filename from bulkmail where emailid=\"%s\"", email);
	if (mysql_query(mysqlptr, SqlBuf))
	{
		if (mysql_errno(mysqlptr) == ER_NO_SUCH_TABLE)
		{
			create_table(ON_LOCAL, "bulkmail", BULKMAIL_TABLE_LAYOUT);
			return (0);
		}
		mysql_perror("RemoteBulkMail: mysql_query: %s", SqlBuf);
		return (1);
	}
	if (!(res = mysql_store_result(mysqlptr)))
	{
		mysql_perror("RemoteBulkMail: mysql_store_result");
		return (1);
	}
	if (!mysql_num_rows(res))
	{
		mysql_free_result(res);
		return (0);
	}
	snprintf(bulkdir, MAX_BUFF, "%s/%s/%s", CONTROLDIR, domain, ((ptr = getenv("BULK_MAILDIR"))) ? ptr : BULK_MAILDIR);
	for (status = 0; (row = mysql_fetch_row(res));)
	{
		snprintf(TmpBuf, sizeof(TmpBuf) - 1, "%s/%s", bulkdir, row[0]);
		if (stat(TmpBuf, &statbuf))
			continue;
		if (CopyEmailFile(homedir, TmpBuf, email, 0, 0, 0, 0, 1, statbuf.st_size))
			status = 1;
	}
	mysql_free_result(res);
	snprintf(SqlBuf, SQL_BUF_SIZE, "delete low_priority from bulkmail where emailid=\"%s\"", email);
	if (mysql_query(mysqlptr, SqlBuf))
	{
		mysql_perror("RemoteBulkMail: mysql_query: %s", SqlBuf);
		return (1);
	}
	return (status);
}

static MYSQL   *
bulk_host_connect()
{
	char           *bulk_host, *bulk_user = 0, *bulk_passwd = 0, *bulk_database,
				   *bulk_socket = 0, *port = 0, *ptr;
	int             bulk_port, count, protocol;
	unsigned int    flags, use_ssl = 0;
	static MYSQL    bulkMySql;

	if ((bulk_host = (char *) getenv("BULK_HOST")) == (char *) 0)
		return (&mysql[1]);
	else
	{
		for (count = 0,ptr = bulk_host;*ptr;ptr++)
		{
			if (*ptr == ':')
			{
				*ptr = 0;
				switch (count++)
				{
				case 0: /*- mysql user */
					if (*(ptr + 1))
						bulk_user = ptr + 1;
					break;
				case 1: /*- mysql passwd */
					if (*(ptr + 1))
						bulk_passwd = ptr + 1;
					break;
				case 2: /*- mysql socket/port */
					if (*(ptr + 1) == '/' || *(ptr + 1) == '.')
						bulk_socket = ptr + 1;
					else
					if (*(ptr + 1))
						port = ptr + 1;
					break;
				case 3: /*- ssl/nossl */
					use_ssl = (strncmp(ptr + 1, "ssl", 3) ? 0 : 1);
					break;
				}
			}
		}
		if (!bulk_user)
			getEnvConfigStr(&bulk_user, "BULK_USER", MYSQL_USER);
		if (!bulk_passwd)
			getEnvConfigStr(&bulk_passwd, "BULK_PASSWD", MYSQL_PASSWD);
		getEnvConfigStr(&bulk_database, "BULK_DATABASE", MYSQL_DATABASE);
		if (!bulk_socket)
			bulk_socket = (char *) getenv("BULK_SOCKET");
		if (!port && !(port = (char *) getenv("BULK_VPORT")))
			port = "0";
		mysql_init(&bulkMySql);
		flags = use_ssl;
		/*- 
		 * mysql_options bug
		 * if MYSQL_READ_DEFAULT_FILE is used
		 * mysql_real_connect fails by connecting with a null unix domain socket
		 */
		bulk_port = atoi(port);
		if ((count = set_mysql_options(&mysql[1],
			bulk_port > 0 || bulk_socket ? 0 : "indimail.cnf",
			bulk_port > 0 || bulk_socket ? 0 : "indimail",
			&flags)))
		{
			fprintf(stderr, "mysql_options(%d): %s\n", count,
				(ptr = error_mysql_options_str(count)) ? ptr : "unknown error");
			return ((MYSQL *) 0);
		}
		/*
		 * MySQL/MariaDB is very stubborn.
		 * It uses Unix domain socket, even if port is set and the socket value is NULL
		 * Force it to use TCP when port is provided and unix_socket is NULL
		 */
		if (bulk_port > 0 && !bulk_socket) {
			protocol = MYSQL_PROTOCOL_TCP;
			if (int_mysql_options(&mysql[1], MYSQL_OPT_PROTOCOL, (char *) &protocol)) {
				fprintf(stderr, "mysql_options(MYSQL_OPT_PROTOCOL): %s\n",
					(ptr = error_mysql_options_str(count)) ? ptr : "unknown error");
				return ((MYSQL *) 0);
			}
		}
		if ((mysql_real_connect(&bulkMySql, bulk_host, bulk_user, bulk_passwd,
				bulk_database, bulk_port, bulk_socket, flags)))
			return (&bulkMySql);
		else
			return ((MYSQL *) 0);
	}
}

void
getversion_RemoteBulkMail_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}