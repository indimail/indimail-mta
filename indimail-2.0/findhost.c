/*
 * $Log: findhost.c,v $
 * Revision 2.35  2016-05-17 15:40:14+05:30  Cprogrammer
 * use control directory set by configure
 *
 * Revision 2.34  2016-04-20 19:32:55+05:30  Cprogrammer
 * added comments for explaining code logic
 *
 * Revision 2.33  2010-08-09 18:28:12+05:30  Cprogrammer
 * use hostid instead of ip address
 *
 * Revision 2.32  2010-05-28 14:11:00+05:30  Cprogrammer
 * use QMTP as default
 *
 * Revision 2.31  2010-04-15 14:14:05+05:30  Cprogrammer
 * added flags argument to mysql_real_connect()
 *
 * Revision 2.30  2010-02-26 10:55:34+05:30  Cprogrammer
 * use host.mysql if host.cntrl is not present
 *
 * Revision 2.29  2010-02-24 09:10:55+05:30  Cprogrammer
 * use CNTRL_SOCKET, CNTRL_VPORT to override indimail.cnf variables
 *
 * Revision 2.28  2010-02-19 19:31:48+05:30  Cprogrammer
 * user,passwd,socket/port was getting lost
 *
 * Revision 2.27  2010-02-19 16:16:08+05:30  Cprogrammer
 * cntrl_port was getting wrongly initialized to zero
 *
 * Revision 2.26  2010-02-19 13:11:10+05:30  Cprogrammer
 * set_mysql_options() needs to be called before each invocation of mysql_real_connect()
 *
 * Revision 2.25  2010-01-06 09:39:36+05:30  Cprogrammer
 * host.cntrl, host.master can now have host:user:passwd:socket/port format
 *
 * Revision 2.24  2009-12-09 23:21:23+05:30  Cprogrammer
 * reconnect to MySQL if server gone away
 *
 * Revision 2.23  2009-09-23 20:54:04+05:30  Cprogrammer
 * register vclose_cntrl only once
 *
 * Revision 2.22  2009-02-18 21:25:21+05:30  Cprogrammer
 * check return value of fscanf
 *
 * Revision 2.21  2009-01-27 21:47:28+05:30  Cprogrammer
 * updated documention
 *
 * Revision 2.20  2009-01-22 11:56:01+05:30  Cprogrammer
 * changed MySQL config to indimail.cnf
 *
 * Revision 2.19  2008-12-18 11:54:07+05:30  Cprogrammer
 * changed PASSWD_CACHE to QUERY_CACHE
 *
 * Revision 2.18  2008-12-16 18:58:29+05:30  Cprogrammer
 * option to restrict findhost from looking up wildcard
 *
 * Revision 2.17  2008-11-13 20:24:24+05:30  Cprogrammer
 * added case where fopen returns error other than ENOENT
 *
 * Revision 2.16  2008-11-07 11:26:30+05:30  Cprogrammer
 * allow modification of returned static buffer
 *
 * Revision 2.15  2008-11-07 10:06:57+05:30  Cprogrammer
 * added cache for findhost request
 *
 * Revision 2.14  2008-09-08 09:34:52+05:30  Cprogrammer
 * removed mysql_escape
 * added CNTRL_SOCKET environment variable
 *
 * Revision 2.13  2008-05-28 16:35:23+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 2.12  2005-12-29 22:44:06+05:30  Cprogrammer
 * set MySQL options
 * use getEnvConfigStr to set variables from environment variables
 *
 * Revision 2.11  2005-01-28 15:36:44+05:30  Cprogrammer
 * set userNotFound to 0 before open_central_db() to prevent
 * incorrect bouncing due to MySQL errors
 *
 * Revision 2.10  2004-05-17 00:48:00+05:30  Cprogrammer
 * return a default entry if user not found (by adding special user *)
 *
 * Revision 2.9  2003-01-03 02:26:31+05:30  Cprogrammer
 * include syntax error as a permanent error
 * replace vcreate_cntrl_table with generic function create_table()
 *
 * Revision 2.8  2002-12-02 21:25:17+05:30  Cprogrammer
 * added #ifdef to prevent compilation problem with older mysql versions
 *
 * Revision 2.7  2002-12-02 01:46:28+05:30  Cprogrammer
 * return value of SqlServer() was not correctly used
 *
 * Revision 2.6  2002-11-04 12:30:32+05:30  Cprogrammer
 * added mysql_option() to allow LOAD DATA INFILE LOCAL
 *
 * Revision 2.5  2002-08-25 22:32:19+05:30  Cprogrammer
 * made control dir configurable
 *
 * Revision 2.4  2002-08-05 00:15:30+05:30  Cprogrammer
 * added mysql_escape for user
 *
 * Revision 2.3  2002-08-03 04:50:52+05:30  Cprogrammer
 * replaced mdaMysqlConnect() with SqlServer() and vauth_open()
 *
 * Revision 2.2  2002-07-27 10:46:45+05:30  Cprogrammer
 * replaced vauth_open() with mdaMysqlConnect()
 *
 * Revision 2.1  2002-04-10 23:55:55+05:30  Cprogrammer
 * bug - removed extra mysql_free_result()
 *
 * Revision 1.28  2002-04-06 22:36:52+05:30  Cprogrammer
 * do structure copy and reset affected_rows
 *
 * Revision 1.27  2002-03-29 22:08:32+05:30  Cprogrammer
 * map hostid from hostcntrl to host_table and return ip address
 *
 * Revision 1.26  2002-03-29 18:03:50+05:30  Cprogrammer
 * avoid memory leak - called mysql_free_result()
 *
 * Revision 1.25  2002-03-28 23:55:07+05:30  Cprogrammer
 * connect to mysql server instead of the mdahost
 *
 * Revision 1.24  2002-03-28 05:10:06+05:30  Cprogrammer
 * set userNotFound if wrong domain is specified
 *
 * Revision 1.23  2002-03-27 13:23:54+05:30  Cprogrammer
 * set userNotFound to 0 at start
 *
 * Revision 1.22  2002-02-23 20:23:17+05:30  Cprogrammer
 * corrected bug with ER_NO_SUCH_TABLE check
 *
 * Revision 1.21  2001-12-23 00:47:11+05:30  Cprogrammer
 * open new connection if is_open is 0
 *
 * Revision 1.20  2001-12-22 18:07:08+05:30  Cprogrammer
 * changed error string for open_master failure
 * create table only if it does not exist
 *
 * Revision 1.19  2001-12-21 02:20:43+05:30  Cprogrammer
 * create table when errno is ER_NO_SUCH_TABLE
 *
 * Revision 1.18  2001-12-21 00:34:07+05:30  Cprogrammer
 * use real_domain in case the domain passed is an alias domain
 *
 * Revision 1.17  2001-12-12 13:42:54+05:30  Cprogrammer
 * null argument to get_smtp_service_port passed to force use of get_local_ip inside get_smtp_service_port
 *
 * Revision 1.16  2001-12-11 11:31:12+05:30  Cprogrammer
 * change in open_central_db()
 *
 * Revision 1.15  2001-12-09 00:58:49+05:30  Cprogrammer
 * corrected size of SqlBuf
 *
 * Revision 1.14  2001-12-08 23:50:48+05:30  Cprogrammer
 * port obtained from get_smtp_service_port instead of the host column of hostcntrl table
 *
 * Revision 1.13  2001-12-08 12:33:42+05:30  Cprogrammer
 * take domain as DEFAULT_DOMAIN if email does not have domain component
 *
 * Revision 1.12  2001-12-02 20:19:57+05:30  Cprogrammer
 * definition of sccsid outside ifdef USE_MYSQL
 *
 * Revision 1.11  2001-11-30 00:12:51+05:30  Cprogrammer
 * used variable cntrl_table for hostcntrl table
 *
 * Revision 1.10  2001-11-29 20:54:56+05:30  Cprogrammer
 * change for initiating only one connection if cntrl_host is same as mysql_host
 *
 * Revision 1.9  2001-11-29 14:34:42+05:30  Cprogrammer
 * removed vclose_cntrl() from findhost()
 *
 * Revision 1.8  2001-11-29 00:31:55+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.7  2001-11-28 22:58:16+05:30  Cprogrammer
 * error statements reviewed and modified
 *
 * Revision 1.6  2001-11-24 12:18:56+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.5  2001-11-22 22:50:39+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.4  2001-11-20 10:54:37+05:30  Cprogrammer
 * added #ifdef for conditional compilation of distributed architecture
 *
 * Revision 1.3  2001-11-18 00:48:29+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.2  2001-11-14 19:22:52+05:30  Cprogrammer
 * added findhost() for distributed mail architecture
 *
 * Revision 1.1  2001-11-11 23:24:15+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: findhost.c,v 2.35 2016-05-17 15:40:14+05:30 Cprogrammer Exp mbhangui $";
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <mysqld_error.h>

#ifdef CLUSTERED_SITE

#ifdef QUERY_CACHE
static char     _cacheSwitch = 1;
#endif

/*
 * connect_primarydb
 * 0 - do not connect, and look for '*' if email does not exist
 * 1 - connect, and look for '*' if email does not exist
 * 2 - do not connect and do not look for '*'
 * 3 - connect and do not look for '*'
 */
char           *
findhost(char *email, int connect_primarydb)
{
	static char     _mailhost[MAX_BUFF], mailhost[MAX_BUFF], prevEmail[MAX_BUFF];
	char            user[MAX_BUFF], domain[MAX_BUFF], SqlBuf[SQL_BUF_SIZE], hostid[MAX_BUFF];
	char           *ptr, *real_domain, *ip_addr;
	static int      mlen;
	int             len, port, err, attempt = 0;
	MYSQL_RES      *res;
	MYSQL_ROW       row;

	if (!email || !*email)
	{
		userNotFound = 1;
		return ((char *) 0);
	}
	len = strlen(email);
#ifdef QUERY_CACHE
	if (_cacheSwitch && getenv("QUERY_CACHE") && *mailhost && !strncmp(email, prevEmail, len))
	{
		if (!mlen)
			mlen = strlen(mailhost) + 1;
		strncpy(_mailhost, mailhost, mlen);
		return (_mailhost);
	}
	if (!_cacheSwitch)
		_cacheSwitch = 1;
#endif
	mlen = 0;
	*mailhost = 0;
	*user = *domain = 0;
	real_domain = (char *) 0;
	userNotFound = 0;
	if (open_central_db(0)) /*- open connection to mysql (cntrl_host or assign opened connection to primary db */
		return ((char *) 0);
	scopy(user, email, MAX_BUFF);
	if ((ptr = strchr(user, '@')) != (char *) 0)
	{
		if (*(ptr + 1))
		{
			scopy(domain, ptr + 1, MAX_BUFF);
			if (!(real_domain = vget_real_domain(domain)))
				return ((char *) 0);
		} else
		{
			getEnvConfigStr(&ptr, "DEFAULT_DOMAIN", DEFAULT_DOMAIN);
			scopy(domain, ptr, MAX_BUFF);
			real_domain = domain;
		}
		*ptr = 0;
	} else
	{
		getEnvConfigStr(&ptr, "DEFAULT_DOMAIN", DEFAULT_DOMAIN);
		scopy(domain, ptr, MAX_BUFF);
		real_domain = domain;
	}
	if (!*real_domain)
		return ((char *) 0);
	/* get data from hostcntrl */
	snprintf(SqlBuf, SQL_BUF_SIZE, "select high_priority host from %s where  pw_name=\"%s\" and pw_domain=\"%s\"",
		cntrl_table, user, real_domain);
again:
	attempt++;
	if (mysql_query(&mysql[0], SqlBuf))
	{
		err = mysql_errno(&mysql[0]);
		(void) fprintf(stderr, "findhost: mysql_query: %s: %s\n", SqlBuf, mysql_error(&mysql[0]));
		if (err == ER_NO_SUCH_TABLE && create_table(ON_MASTER, cntrl_table, CNTRL_TABLE_LAYOUT))
			fprintf(stderr, "findhost: create_table %s: %s: %s", cntrl_table, SqlBuf, mysql_error(&mysql[0]));
		if (err == ER_NO_SUCH_TABLE || err == ER_SYNTAX_ERROR)
			userNotFound = 1;
		/*- reconnect to MySQL if server gone away */
		if (mysql_ping(&mysql[0]))
		{
			if (attempt == 1)
			{
				vclose_cntrl();
				goto again;
			}
		}
		return ((char *) 0);
	} else
		attempt = 1;
	if (!(res = mysql_store_result(&mysql[0])))
	{
		(void) fprintf(stderr, "findhost: mysql_store_result: %s\n", mysql_error(&mysql[0]));
		return ((char *) 0);
	}
	if (mysql_num_rows(res) == 0)
	{
		if (connect_primarydb > 1)
		{
			userNotFound = 1;
			mysql_free_result(res);
			return ((char *) 0);
		}
		/*- connect_primarydb == 2 or connect_primarydb == 3 */
		if (attempt == 1)
		{
			mysql_free_result(res);
			/*- look for default entry (pw_name = '*' */
			snprintf(SqlBuf, SQL_BUF_SIZE, 
				"select high_priority host from %s where  pw_name=\"*\" and pw_domain=\"%s\"",
				cntrl_table, real_domain);
			goto again;
		}
		userNotFound = 1;
		mysql_free_result(res);
		return ((char *) 0);
	}
	row = mysql_fetch_row(res);
	if (!(ip_addr = vauth_getipaddr(row[0])))
	{
		(void) fprintf(stderr, "findhost: vauth_getipaddr: %s\n", mysql_error(&mysql[0]));
		mysql_free_result(res);
		return ((char *) 0);
	}
	scopy(hostid, row[0], MAX_BUFF);
	mysql_free_result(res);
	if (connect_primarydb == 1 || connect_primarydb == 3)
	{
		if (!(ptr = SqlServer(ip_addr, real_domain)))
		{
			fprintf(stderr, "findhost: SqlServer: Unable to find SqlServer IP for mailstore %s, %s\n", ip_addr, real_domain);
			return ((char *) 0);
		}
		if (vauth_open(ptr)) /*- connect to primary db */
			return ((char *) 0);
	}
	if ((port = get_smtp_service_port(0, real_domain, hostid)) == -1)
	{
		(void) fprintf(stderr, "findhost: failed to get smtp port for %s %s\n", 
		   real_domain, hostid);
		return ((char *) 0);
	} else
	if (!port)
		port = PORT_QMTP;
	strncpy(prevEmail, email, len);
	snprintf(mailhost, MAX_BUFF, "%s:%s:%d", domain, ip_addr, port);
	return (mailhost);
}

int
open_central_db(char *dbhost)
{
	static char     host_path[MAX_BUFF], atexit_registered = 0;
	char            SqlBuf[SQL_BUF_SIZE];
	char           *ptr, *mysql_user = 0, *mysql_passwd = 0, *mysql_database = 0,
				   *cntrl_socket = 0, *qmaildir, *controldir;
	int             mysqlport = -1, count;
	unsigned int    flags;
	FILE           *fp;

	if (isopen_cntrl == 1)
		return (0);
	/*-
	 * 1. Check Env Variable for CNTRL_HOST
	 * 2. If CNTRL_HOST is not defined check host.cntrl in /var/indimail/control
	 * 3. If host.cntrl is not present check host.mysql in /var/indimail/control
	 * 4. If host.cntrl not present then take the value of CNTRL_HOST 
	 *    defined in indimail.h
	 */
	if (dbhost && *dbhost)
		scopy(cntrl_host, dbhost, MAX_BUFF);
	else
	if ((ptr = (char *) getenv("CNTRL_HOST")) != (char *) 0)
		scopy(cntrl_host, ptr, MAX_BUFF);
	getEnvConfigStr(&qmaildir, "QMAILDIR", QMAILDIR);
	getEnvConfigStr(&controldir, "CONTROLDIR", CONTROLDIR);
	if (*controldir == '/') {
		if (snprintf(host_path, MAX_BUFF, "%s/host.cntrl", controldir) == -1)
			host_path[MAX_BUFF - 1] = 0;
		if (access(host_path, F_OK) && snprintf(host_path, MAX_BUFF, "%s/host.mysql", controldir) == -1)
			host_path[MAX_BUFF - 1] = 0;
	} else {
		if (snprintf(host_path, MAX_BUFF, "%s/%s/host.cntrl", qmaildir, controldir) == -1)
			host_path[MAX_BUFF - 1] = 0;
		if (access(host_path, F_OK) && snprintf(host_path, MAX_BUFF, "%s/%s/host.mysql", qmaildir, controldir) == -1)
			host_path[MAX_BUFF - 1] = 0;
	}
	if (!*cntrl_host && !access(host_path, F_OK))
	{
		if (!(fp = fopen(host_path, "r")))
		{
			fprintf(stderr, "fopen: %s: %s\n", host_path, strerror(errno));
			return(-1);
		} else
		{
			if (!fgets(cntrl_host, MAX_BUFF - 2, fp))
			{
				fprintf(stderr, "fgets: %s\n", strerror(errno));
				fclose(fp);
				return(-1);
			}
			if ((ptr = strrchr(cntrl_host, '\n')))
				*ptr = 0;
			fclose(fp);
		}
	} else
	if (!*cntrl_host)
		scopy(cntrl_host, CNTRL_HOST, MAX_BUFF);
	mysql_init(&mysql[0]);
	if (!atexit_registered++)
		atexit(vclose_cntrl);
#ifdef HAVE_LOCAL_INFILE
	if (mysql_options(&mysql[0], MYSQL_OPT_LOCAL_INFILE, 0))
	{
		fprintf(stderr, "mysql_options: MYSQL_OPT_LOCAL_INFILE: unknown option\n");
		return(-1);
	}
#endif
	for (count = 0,ptr = cntrl_host;*ptr;ptr++)
	{
		if (*ptr == ':')
		{
			*ptr = 0;
			switch (count++)
			{
			case 0: /*- mysql user */
				if (*(ptr + 1))
					mysql_user = ptr + 1;
			case 1: /*- mysql passwd */
				if (*(ptr + 1))
					mysql_passwd = ptr + 1;
			case 2: /*- mysql socket/port */
				if (*(ptr + 1) == '/' || *(ptr + 1) == '.')
					cntrl_socket = ptr + 1;
				else
				if (*(ptr + 1))
					cntrl_port = ptr + 1;
			}
		}
	}
	if (!mysql_user)
		getEnvConfigStr(&mysql_user, "CNTRL_USER", CNTRL_USER);
	if (!mysql_passwd)
		getEnvConfigStr(&mysql_passwd, "CNTRL_PASSWD", CNTRL_PASSWD);
	if (!cntrl_socket)
		cntrl_socket = (char *) getenv("CNTRL_SOCKET");
	if (!cntrl_port && !(cntrl_port = (char *) getenv("CNTRL_VPORT")))
		cntrl_port = "0";
	getEnvConfigStr(&mysql_database, "CNTRL_DATABASE", CNTRL_DATABASE);
	getEnvConfigStr(&cntrl_table, "CNTRL_TABLE", CNTRL_DEFAULT_TABLE);
	mysqlport = atoi(cntrl_port);
	/*
	 * is_open == 0 &&  cntrl_host == mysql_host && cntrl_port == indi_port -> connect to cntrl_host -> connect cntrl_host
	 * is_open == 0 &&  cntrl_host != mysql_host && cntrl_port == indi_port -> connect to cntrl_host -> connect cntrl_host
	 * is_open == 0 &&  cntrl_host == mysql_host && cntrl_port != indi_port -> connect to cntrl_host -> connect cntrl_host
	 * is_open == 1 &&  cntrl_host == mysql_host && cntrl_port == indi_port -> connect to cntrl_host -> use mysql_host
	 * is_open == 1 &&  cntrl_host != mysql_host && cntrl_port == indi_port -> connect to cntrl_host -> connect cntrl_host
	 * is_open == 1 &&  cntrl_host == mysql_host && cntrl_port != indi_port -> connect to cntrl_host -> connect cntrl_host
	 */
	if (!is_open || strncmp(cntrl_host, mysql_host, MAX_BUFF) || strncmp(cntrl_port, indi_port, MAX_BUFF))
	{
		if (set_mysql_options(&mysql[0], "indimail.cnf", "indimail", &flags))
		{
			fprintf(stderr, "mysql_options: Invalid options in MySQL options file\n");
			return(-1);
		}
		if (!(mysql_real_connect(&mysql[0], cntrl_host, mysql_user, mysql_passwd, mysql_database, mysqlport, cntrl_socket, flags)))
		{
			if (set_mysql_options(&mysql[0], "indimail.cnf", "indimail", &flags))
			{
				fprintf(stderr, "mysql_options: Invalid options in MySQL options file\n");
				return(-1);
			}
			if (!(mysql_real_connect(&mysql[0], cntrl_host, mysql_user, mysql_passwd, NULL, mysqlport, cntrl_socket, flags)))
			{
				fprintf(stderr, "open_central_db: mysql_real_connect: %s: %s\n", cntrl_host, mysql_error(&mysql[0]));
				return (-1);
			}
			if (snprintf(SqlBuf, SQL_BUF_SIZE, "create database %s", mysql_database) == -1)
				SqlBuf[SQL_BUF_SIZE - 1] = 0;
			if (mysql_query(&mysql[0], SqlBuf))
			{
				fprintf(stderr, "mysql_query: %s: %s\n", SqlBuf, mysql_error(&mysql[0]));
				return (-1);
			}
			if (mysql_select_db(&mysql[0], mysql_database))
			{
				fprintf(stderr, "mysql_select_db: %s: %s\n", mysql_database, mysql_error(&mysql[0]));
				return (-1);
			}
		}
		isopen_cntrl = 1;
	} else
	{
		mysql[0] = mysql[1];
		mysql[0].affected_rows= ~(my_ulonglong) 0;
		isopen_cntrl = 2; /*- same connection as from host.mysql */
	}
	return (0);
}

void
vclose_cntrl()
{
	/*
	 * disconnection from the database 
	 */
	if (isopen_cntrl == 1)
	{
		isopen_cntrl = 0;
		mysql_close(&mysql[0]);
	}
}

#ifdef QUERY_CACHE
void
findhost_cache(char cache_switch)
{
	_cacheSwitch = cache_switch;
	return;
}
#endif
#else
char           *
findhost(char *email, int connect_primarydb)
{
	char           *ptr;
	char            tmpbuf[MAX_BUFF], domain[MAX_BUFF];
	static char     mailstore[MAX_BUFF];

	if (!email || !*email)
		return ((char *) 0);
	*tmpbuf = *domain = 0;
	scopy(tmpbuf, email, MAX_BUFF);
	if ((ptr = strchr(tmpbuf, '@')) != (char *) 0)
	{
		if (*(ptr + 1))
			scopy(domain, ptr + 1, MAX_BUFF);
		*ptr = 0;
	} else
		return ((char *) 0);
	if (!*domain)
		return ((char *) 0);
	snprintf(mailstore, MAX_BUFF, "%s:localhost:25", domain);
	if (connect_primarydb)
		vauth_open(0);
	return(mailstore);
}
#endif

void
getversion_findhost_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
