/*
 * $Log: vauth_open.c,v $
 * Revision 2.24  2010-04-15 14:13:44+05:30  Cprogrammer
 * added flags argument to mysql_real_connect()
 *
 * Revision 2.23  2010-02-26 10:55:50+05:30  Cprogrammer
 * allow mysql host in host:user:password:socket/port format
 *
 * Revision 2.22  2010-02-24 09:12:04+05:30  Cprogrammer
 * allow MYSQL_SOCKET, MYSQL_VPORT variables to override indimail.cnf variables
 *
 * Revision 2.21  2010-02-19 16:17:01+05:30  Cprogrammer
 * indi_port was wrongly initialized to zero
 *
 * Revision 2.20  2010-02-19 13:12:56+05:30  Cprogrammer
 * set_mysql_options() needs to be called before each invocation of mysql_real_connect
 *
 * Revision 2.19  2010-01-06 09:40:14+05:30  Cprogrammer
 * host.mysql can now have host:user:passwd:socket/port format
 *
 * Revision 2.18  2009-02-18 21:34:27+05:30  Cprogrammer
 * check return value of fscanf
 *
 * Revision 2.17  2009-02-06 11:40:26+05:30  Cprogrammer
 * ignore return value of fscanf
 *
 * Revision 2.16  2009-01-27 21:47:42+05:30  Cprogrammer
 * updated documentation
 *
 * Revision 2.15  2009-01-22 11:57:14+05:30  Cprogrammer
 * changed MySQL config to indimail.cnf
 *
 * Revision 2.14  2008-11-13 20:25:34+05:30  Cprogrammer
 * added case where fopen returns error other than ENOENT
 *
 * Revision 2.13  2008-09-08 09:56:04+05:30  Cprogrammer
 * added MYSQL_SOCKET to define unix domain socket
 *
 * Revision 2.12  2008-05-28 16:39:43+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 2.11  2008-05-28 15:24:14+05:30  Cprogrammer
 * removed ldap module
 *
 * Revision 2.10  2006-01-23 21:50:06+05:30  Cprogrammer
 * 3rd arg to getEnvConfigStr() is a string
 *
 * Revision 2.9  2005-12-29 22:51:43+05:30  Cprogrammer
 * use set_mysql_options()
 * use getEnvConfigStr to set variables from environment variables
 *
 * Revision 2.8  2004-05-29 21:42:41+05:30  Cprogrammer
 * removed prototype for ldap_enable_cache
 *
 * Revision 2.7  2004-05-17 00:52:20+05:30  Cprogrammer
 * prototype for ldap_enable_cache()
 *
 * Revision 2.6  2003-08-24 16:16:21+05:30  Cprogrammer
 * use LDAP Version 3
 * enable client caching of ldap_search
 *
 * Revision 2.5  2002-12-06 01:56:03+05:30  Cprogrammer
 * connect to secondary ldap server if primary is down
 *
 * Revision 2.4  2002-08-25 22:35:29+05:30  Cprogrammer
 * made control dir configurable
 *
 * Revision 2.3  2002-07-11 18:22:56+05:30  Cprogrammer
 * set default ldap version
 *
 * Revision 2.2  2002-07-01 19:05:53+05:30  Cprogrammer
 * change to use environment variables for LDAP_HOST, LDAP_USER, LDAP_PASSWD, LDAP_PORt
 *
 * Revision 2.1  2002-06-26 03:20:48+05:30  Cprogrammer
 * added function ldapOpen()
 *
 * Revision 1.8  2002-04-06 22:38:02+05:30  Cprogrammer
 * do structure copy and reset affected_rows to 0
 *
 * Revision 1.7  2001-12-23 00:48:03+05:30  Cprogrammer
 * open connection if isopen_cntrl is 0
 *
 * Revision 1.6  2001-12-11 11:33:34+05:30  Cprogrammer
 * removed memcpy for relay_table
 *
 * Revision 1.5  2001-11-29 20:56:55+05:30  Cprogrammer
 * change for initiating only one connection if indimail, cntrl and relay hosts are same
 *
 * Revision 1.4  2001-11-24 12:21:01+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.3  2001-11-20 10:57:59+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.2  2001-11-14 19:27:07+05:30  Cprogrammer
 * distributed arch change
 *
 * Revision 1.1  2001-10-24 18:15:23+05:30  Cprogrammer
 * Initial revision
 *
 *
 * Function to open connection to database.
 * The variable is_open is used to check wether the connection is already open
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: vauth_open.c,v 2.24 2010-04-15 14:13:44+05:30 Cprogrammer Stab mbhangui $";
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

int
vauth_open(char *dbhost)
{
	char            SqlBuf[SQL_BUF_SIZE], host_path[MAX_BUFF];
	char           *ptr, *mysql_user = 0, *mysql_passwd = 0, *mysql_database = 0,
		           *mysql_socket = 0, *qmaildir, *controldir;
	int             mysqlport = -1, count;
	unsigned int    flags;
	FILE           *fp;

	if (is_open == 1)
		return (0);
	/*-
	 * 1. set mysql_host from dbhost if dbhost is not null.
	 * 2. Check Env Variable for MYSQL_HOST
	 * 3. If MYSQL_HOST is not defined check host.mysql in /var/indimail/control
	 * 4. If host.mysql not present then take the value of MYSQL_HOST 
	 *    defined in indimail.h
	 */
	if (dbhost && *dbhost)
		scopy(mysql_host, dbhost, MAX_BUFF);
	else
	if ((ptr = (char *) getenv("MYSQL_HOST")) != (char *) 0)
		scopy(mysql_host, ptr, MAX_BUFF);
	getEnvConfigStr(&qmaildir, "QMAILDIR", QMAILDIR);
	getEnvConfigStr(&controldir, "CONTROLDIR", "control");
	if (snprintf(host_path, MAX_BUFF, "%s/%s/host.mysql", qmaildir, controldir) == -1)
		host_path[MAX_BUFF - 1] = 0;
	if (!*mysql_host && !access(host_path, F_OK))
	{
		if (!(fp = fopen(host_path, "r")))
		{
			fprintf(stderr, "open: %s: %s\n", host_path, strerror(errno));
			return (1);
		} else
		{
			if (!fgets(mysql_host, MAX_BUFF - 2, fp))
			{
				fprintf(stderr, "fgets: %s\n", strerror(errno));
				fclose(fp);
				return (1);
			}
			if ((ptr = strrchr(mysql_host, '\n')))
				*ptr = 0;
			fclose(fp);
		}
	} else
	if (!*mysql_host)
		scopy(mysql_host, MYSQL_HOST, MAX_BUFF);
	mysql_init(&mysql[1]);
	atexit(vclose);
	for (count = 0,ptr = mysql_host;*ptr;ptr++)
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
					mysql_socket = ptr + 1;
				else
				if (*(ptr + 1))
					indi_port = ptr + 1;
			}
		}
	}
	if (!mysql_user)
		getEnvConfigStr(&mysql_user, "MYSQL_USER", MYSQL_USER);
	if (!mysql_passwd)
		getEnvConfigStr(&mysql_passwd, "MYSQL_PASSWD", MYSQL_PASSWD);
	if (!mysql_socket)
		mysql_socket = (char *) getenv("MYSQL_SOCKET");
	if (!indi_port && !(indi_port = (char *) getenv("MYSQL_VPORT")))
		indi_port = "0";
	getEnvConfigStr(&mysql_database, "MYSQL_DATABASE", MYSQL_DATABASE);
	getEnvConfigStr(&default_table, "MYSQL_TABLE", MYSQL_DEFAULT_TABLE);
	getEnvConfigStr(&inactive_table, "MYSQL_INACTIVE_TABLE", MYSQL_INACTIVE_TABLE);
	mysqlport = atoi(indi_port);
#ifdef CLUSTERED_SITE
	if (!isopen_cntrl || strncmp(cntrl_host, mysql_host, MAX_BUFF) || strncmp(cntrl_port, indi_port, MAX_BUFF))
	{
#endif
		if (set_mysql_options(&mysql[1], "indimail.cnf", "indimail", &flags))
		{
			fprintf(stderr, "mysql_options: Invalid options in MySQL options file\n");
			return(-1);
		}
		if (!(mysql_real_connect(&mysql[1], mysql_host, mysql_user, mysql_passwd,
			mysql_database, mysqlport, mysql_socket, flags)))
		{
			if (set_mysql_options(&mysql[1], "indimail.cnf", "indimail", &flags))
			{
				fprintf(stderr, "mysql_options: Invalid options in MySQL options file\n");
				return(-1);
			}
			if (!(mysql_real_connect(&mysql[1], mysql_host, mysql_user, mysql_passwd, NULL,
				mysqlport, mysql_socket, flags)))
			{
				mysql_perror("mysql_real_connect: %s", mysql_host);
				return (-1);
			}
			if (snprintf(SqlBuf, SQL_BUF_SIZE, "CREATE DATABASE %s", mysql_database) == -1)
				SqlBuf[SQL_BUF_SIZE - 1] = 0;
			if (mysql_query(&mysql[1], SqlBuf))
			{
				mysql_perror("mysql_query: %s", SqlBuf);
				return (-1);
			}
			if (mysql_select_db(&mysql[1], mysql_database))
			{
				mysql_perror("mysql_select_db: %s", mysql_database);
				return (-1);
			}
		}
		is_open = 1;
#ifdef CLUSTERED_SITE
	} else
	{
		mysql[1] = mysql[0];
		mysql[1].affected_rows= ~(my_ulonglong) 0;
		is_open = 2;
	}
#endif
	return (0);
}

void
getversion_vauth_open_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
