/*
 * $Log: dbload.c,v $
 * Revision 2.19  2016-05-17 17:09:39+05:30  Cprogrammer
 * use control directory set by configure
 *
 * Revision 2.18  2016-01-12 13:12:55+05:30  Cprogrammer
 * total was not correctly assigned if RelayHosts was already obtained
 *
 * Revision 2.17  2015-08-21 10:45:48+05:30  Cprogrammer
 * use integer for getEnvConfigInt
 *
 * Revision 2.16  2010-05-01 12:27:49+05:30  Cprogrammer
 * print error only if verbose is set
 *
 * Revision 2.15  2010-04-15 14:12:55+05:30  Cprogrammer
 * added flags argument to mysql_real_connect()
 *
 * Revision 2.14  2009-11-09 08:34:13+05:30  Cprogrammer
 * use set_mysql_options() to set mysql options before connecting to MDA MySQL db
 *
 * Revision 2.13  2009-03-13 20:13:03+05:30  Cprogrammer
 * added field last_error to dbinfo
 *
 * Revision 2.12  2008-12-19 11:52:38+05:30  Cprogrammer
 * return error if loadDbinfoTotal() returns 0
 *
 * Revision 2.11  2008-12-18 11:53:49+05:30  Cprogrammer
 * added debug statement
 *
 * Revision 2.10  2008-05-28 16:34:38+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 2.9  2007-12-21 21:24:14+05:30  Cprogrammer
 * initialize total
 *
 * Revision 2.8  2006-03-02 20:34:23+05:30  Cprogrammer
 * initialize failed attempts when reconnecting to MySQL
 *
 * Revision 2.7  2005-12-29 22:41:10+05:30  Cprogrammer
 * use getEnvConfigStr to set variables from environment variables
 *
 * Revision 2.6  2004-08-11 22:59:59+05:30  Cprogrammer
 * added more descriptive memory allocation errors
 * added comment in is_duplicate_connection()
 *
 * Revision 2.5  2003-11-03 00:48:38+05:30  Cprogrammer
 * use localhost for local ip addresses
 *
 * Revision 2.4  2003-01-08 09:58:33+05:30  Cprogrammer
 * close_db() changed to free RelayHosts and MdaMysql
 *
 * Revision 2.3  2002-08-25 22:48:51+05:30  Cprogrammer
 * made control dir configurable
 *
 * Revision 2.2  2002-05-11 15:23:55+05:30  Cprogrammer
 * misleading error text corrected
 *
 * Revision 2.1  2002-04-17 12:47:11+05:30  Cprogrammer
 * added function is_duplicate_conn()
 *
 * Revision 1.3  2002-04-10 04:42:54+05:30  Cprogrammer
 * Changed Error message for error in connection to mysql
 *
 * Revision 1.2  2002-04-09 20:27:06+05:30  Cprogrammer
 * set fd to -1 if connect fails
 *
 * Revision 1.1  2002-04-08 03:45:04+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: dbload.c,v 2.19 2016-05-17 17:09:39+05:30 Cprogrammer Exp mbhangui $";
#endif

#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

static MYSQL   *is_duplicate_conn(MYSQL **, DBINFO **);

int
OpenDatabases()
{
	static MYSQL    dummy; /*- some bug in mysql. Do mysql_init() to prevent it */
	MYSQL         **mysqlptr;
	int             count, idx;
	static int      total;
	DBINFO        **ptr;
	int             fd[3];
	extern int      loadDbinfoTotal();

	if (RelayHosts)
		total = loadDbinfoTotal();
	else
	if (!(RelayHosts = LoadDbInfo_TXT(&total)))
	{
		perror("OpenDatabases: LoadDbInfo_TXT");
		return(-1);
	}
	if (!MdaMysql)
	{
		if (!mysql_init(&dummy))
		{
			fprintf(stderr, "MYSQL Init Error:\n");
			return (1);
		}
		if (!total && !(total = loadDbinfoTotal()))
		{
			fprintf(stderr, "OpenDatabases: loadDbinfoTotal: unable to figure out totals\n");
			return (1);
		}
		if (!(MdaMysql = (MYSQL **) calloc(1, sizeof(MYSQL *) * (total))))
		{
			fprintf(stderr, "OpenDatabases: calloc: %d Bytes: %s", total * (int) sizeof(MYSQL *),
				strerror(errno));
			return (-1);
		}
		fd[0] = fd[1] = fd[2] = -1;
		for (count = idx = 0; idx < 3 && count < 2; idx++)
		{
			if ((count = fd[idx] = open("/dev/null", O_RDONLY)) > 2)
			{
				close(count);
				fd[idx] = -1;
				break;
			}
		}
		for (count = 1, mysqlptr = MdaMysql, ptr = RelayHosts; (*ptr); ptr++, mysqlptr++, count++)
		{
			if (!((*mysqlptr) = is_duplicate_conn(mysqlptr, ptr)))
			{
				if (!((*mysqlptr) = (MYSQL *) calloc(1, sizeof(MYSQL))))
				{
					fprintf(stderr, "OpenDatabases: calloc: %d Bytes: %s",
						(int) sizeof(MYSQL *), strerror(errno));
					(*ptr)->fd = -1;
					return (-1);
				}
				(*ptr)->failed_attempts = 0;
				(*ptr)->last_error = 0;
				if (connect_db(ptr, mysqlptr))
				{
					if (verbose)
						fprintf(stderr, "%d: %s Failed db %s@%s for user %s port %d\n",
							count, (*ptr)->domain, (*ptr)->database,
							(*ptr)->server, (*ptr)->user, (*ptr)->port);
					(*ptr)->fd = -1;
					continue;
				} else
					(*ptr)->fd = (*mysqlptr)->net.fd;
				if (verbose)
					printf("Connection %d Fd %d: %s Database %s Server %s User %s %d\n",
						count, (*ptr)->fd, (*ptr)->domain, (*ptr)->database, (*ptr)->server,
						(*ptr)->user, (*ptr)->port);
			} else
			{
				(*ptr)->fd = (*mysqlptr)->net.fd;
				if (verbose)
					printf("Connection %d Fd %d: %s Database %s Server %s User %s %d\n",
						count, (*ptr)->fd, (*ptr)->domain, (*ptr)->database, (*ptr)->server,
						(*ptr)->user, (*ptr)->port);
			}
		}
	} 
	return(0);
}

static MYSQL   *
is_duplicate_conn(MYSQL **mysqlptr, DBINFO **rhostsptr)
{
	DBINFO        **ptr;
	MYSQL         **mptr;

	if (rhostsptr == RelayHosts) /*- How can the first entry be duplicate */
		return((MYSQL *) 0);
	for (ptr = RelayHosts, mptr = MdaMysql;ptr != rhostsptr;ptr++, mptr++)
	{
		if (strncmp((*ptr)->server, (*rhostsptr)->server, DBINFO_BUFF))
		{
			/*
			 * if servers do not match but if both
			 * servers are local, then use
			 * first opened mysql connection
			 * NOTE: might be a problem if two
			 * different mysql server run on two different
			 * local interfaces.
			 */
			if (!islocalif((*ptr)->server) || !islocalif((*rhostsptr)->server))
				continue;
		}
		if (strncmp((*ptr)->database, (*rhostsptr)->database, DBINFO_BUFF))
			continue;
		else
		if (strncmp((*ptr)->user, (*rhostsptr)->user, DBINFO_BUFF))
			continue;
		else
		if (strncmp((*ptr)->password, (*rhostsptr)->password, DBINFO_BUFF))
			continue;
		else
		if ((*ptr)->port != (*rhostsptr)->port)
			continue;
		return((*mptr));
	}
	return((MYSQL *) 0);
}

int
connect_db(DBINFO **ptr, MYSQL **mysqlptr)
{
	char            mcdFile[MAX_BUFF];
	char           *qmaildir, *controldir, *mcdfile, *server;
	int             maxattempts, retry_interval;
	unsigned int    flags;

	if ((*ptr)->failed_attempts)
	{
		getEnvConfigInt(&maxattempts, "MAX_FAIL_ATTEMPTS", MAX_FAIL_ATTEMPTS);
		if ((*ptr)->failed_attempts > maxattempts) 
		{
			getEnvConfigInt(&retry_interval, "MYSQL_RETRY_INTERVAL", MYSQL_RETRY_INTERVAL);
			/*- Do not attempt reconnection to MySQL till this period is over */
			if ((time(0) - (*ptr)->last_attempted) < retry_interval)
			{
				fprintf(stderr,"%s@%s: Domain %s Port %d: Reached failed attempts [%d], retry connect after %ld secs [%s]\n",
					(*ptr)->database, (*ptr)->server, (*ptr)->domain, (*ptr)->port,
					(*ptr)->failed_attempts, retry_interval - time(0) + (*ptr)->last_attempted,
					(*ptr)->last_error ? (*ptr)->last_error : "?");
				return 1;
			}
		}
	}
	getEnvConfigStr(&controldir, "CONTROLDIR", CONTROLDIR);
	getEnvConfigStr(&mcdfile, "MCDFILE", MCDFILE);
	if (*mcdfile == '/' || *mcdfile == '.')
		snprintf(mcdFile, MAX_BUFF, "%s", mcdfile);
	else {
		if (*controldir == '/')
			snprintf(mcdFile, MAX_BUFF, "%s/%s", controldir, mcdfile);
		else {
			getEnvConfigStr(&qmaildir, "QMAILDIR", QMAILDIR);
			snprintf(mcdFile, MAX_BUFF, "%s/%s/%s", qmaildir, controldir, mcdfile);
		}
	}
	if (!mysql_init(*mysqlptr))
	{
		fprintf(stderr, "MYSQL Init Error: %s@%s\n", (*ptr)->database, (*ptr)->server);
		return (1);
	}
	if (set_mysql_options(*mysqlptr, "indimail.cnf", "indimail", &flags))
	{
		fprintf(stderr, "mysql_options: Invalid options in MySQL options file\n");
		return(-1);
	}
	if (islocalif((*ptr)->server) || !strncmp((*ptr)->server, "localhost", 10))
		server = "localhost";
	else
		server = (*ptr)->server;
	(*ptr)->last_attempted = time(0);
	if (!mysql_real_connect(*mysqlptr, server, (*ptr)->user,
			(*ptr)->password, (*ptr)->database, (*ptr)->port, NULL, flags))
	{
		char           *my_error;
		int             my_error_len;

		if ((my_error = (char *) mysql_error(*mysqlptr)))
		{
			my_error_len = strlen(my_error) + 1;
			if (!((*ptr)->last_error = (char *) realloc((*ptr)->last_error, my_error_len * sizeof(char))))
			{
				fprintf(stderr, "connect_db: realloc: %d Bytes: %s", my_error_len, strerror(errno));
				return (-1);
			}
			strncpy((*ptr)->last_error, my_error, my_error_len);
		}
		if (verbose)
			fprintf(stderr, "MYSQLconnect: %s@%s: Domain %s Port %d %s\n",
				(*ptr)->database, server, (*ptr)->domain, (*ptr)->port,
				mysql_error(*mysqlptr));
		(*ptr)->failed_attempts++;
		return (1);
	}
	(*ptr)->failed_attempts = 0;
	if ((*ptr)->last_error)
	{
		free((*ptr)->last_error);
		(*ptr)->last_error = 0;
	}
	return (0);
}

void
close_db()
{
	MYSQL         **mysqlptr, **mptr;
	MYSQL          *tptr; /*- for temp storage of address */
	DBINFO        **rhostsptr, **rptr;

	if (!RelayHosts || !MdaMysql)
		return;
	/*
	 * Find out entries for mysqlptr which are duplicate -
	 * assigned by is_duplicate_conn()
	 * to ensure free() and mysql_close() is done only once
	 */
	for (mysqlptr = MdaMysql, rhostsptr = RelayHosts;*rhostsptr;mysqlptr++, rhostsptr++)
	{
		if ((tptr = (*mysqlptr)))
		{
			if (verbose)
				printf("Closing connection Fd %d, Database %s, Server %s, User %s, Port %d\n",
						(*rhostsptr)->fd, (*rhostsptr)->database, (*rhostsptr)->server,
						(*rhostsptr)->user, (*rhostsptr)->port);
			mysql_close(*mysqlptr);
			free(*mysqlptr);
			for (rptr = RelayHosts, mptr = MdaMysql;*rptr;rptr++, mptr++)
			{
				if((*mptr) && (*mptr) == tptr)
					(*mptr) = (MYSQL *) 0;
			}
		}
		free(*rhostsptr);
	}
	free(MdaMysql);
	free(RelayHosts);
	RelayHosts = (DBINFO **) 0;
	MdaMysql = (MYSQL **) 0;
	return;
}

void
getversion_dbload_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
