/*
 * $Log: ProcessInFifo.c,v $
 * Revision 2.29  2010-02-26 10:52:11+05:30  Cprogrammer
 * host host.mysql if host.cntrl is not present
 *
 * Revision 2.28  2009-09-23 21:22:24+05:30  Cprogrammer
 * reconnect to hostcntrl database if connection is closed by server
 *
 * Revision 2.27  2008-11-21 15:08:52+05:30  Cprogrammer
 * fixed compilation for mac os
 *
 * Revision 2.26  2008-11-07 17:02:00+05:30  Cprogrammer
 * reset cache on SIGHUP
 * close db on SIGINT
 * generic signal handler
 *
 * Revision 2.25  2008-07-13 19:46:08+05:30  Cprogrammer
 * use ERESTART only if available
 *
 * Revision 2.24  2008-05-28 15:46:25+05:30  Cprogrammer
 * removed ldap code
 *
 * Revision 2.23  2005-12-29 22:47:14+05:30  Cprogrammer
 * use getEnvConfigStr to set variables from environment variables
 *
 * Revision 2.22  2004-02-18 14:24:21+05:30  Cprogrammer
 * added domain query
 *
 * Revision 2.21  2003-08-24 16:02:45+05:30  Cprogrammer
 * domain field added to LdapGetpw()
 *
 * Revision 2.20  2003-06-18 22:52:40+05:30  Cprogrammer
 * added comments
 *
 * Revision 2.19  2003-01-06 20:42:34+05:30  Cprogrammer
 * added facility for reconfiguring on SIGHUP
 *
 * Revision 2.18  2002-12-27 16:40:42+05:30  Cprogrammer
 * added code to free memory for a low memory system
 *
 * Revision 2.17  2002-12-13 14:36:11+05:30  Cprogrammer
 * added SIGUSR2 to toggle debugging
 * shortened debugging messages
 *
 * Revision 2.16  2002-12-12 01:26:04+05:30  Cprogrammer
 * debugging changes
 *
 * Revision 2.15  2002-12-11 10:28:30+05:30  Cprogrammer
 * added ERESTART check for errno
 *
 * Revision 2.14  2002-12-10 23:28:01+05:30  Cprogrammer
 * added debug option
 *
 * Revision 2.13  2002-12-03 03:03:53+05:30  Cprogrammer
 * Removed BSD style signal
 *
 * Revision 2.12  2002-11-22 01:15:42+05:30  Cprogrammer
 * corrected compilation problem with --enable-ldap-passwd=n
 *
 * Revision 2.11  2002-11-14 15:01:00+05:30  Cprogrammer
 * added SIGUSR1 to dump stats
 *
 * Revision 2.10  2002-11-03 00:30:25+05:30  Cprogrammer
 * use timeoutread() and timeoutwrite() functions instead of read() and write() to prevent deadlocks
 *
 * Revision 2.9  2002-10-06 00:00:32+05:30  Cprogrammer
 * removed signal statement which was causing signal to reset to default values
 *
 * Revision 2.8  2002-09-30 23:32:23+05:30  Cprogrammer
 * do not attempt connection to LDAP_HOST is not defined and control file host.ldap is absent
 *
 * Revision 2.7  2002-09-02 22:04:04+05:30  Cprogrammer
 * set signal for SIGPIPE to default before return
 *
 * Revision 2.6  2002-08-25 22:29:03+05:30  Cprogrammer
 * made control directory configurable
 *
 * Revision 2.5  2002-08-03 00:39:51+05:30  Cprogrammer
 * added LPWD_QUERY for local authentication only (for IMAP and POP3 daemons)
 *
 * Revision 2.4  2002-07-22 20:01:28+05:30  Cprogrammer
 * query result to be sent only if bytes > 0
 * added statistics
 * corrected problem with default infifo getting used
 * added LDAP_QUERY
 *
 * Revision 2.3  2002-07-04 00:31:28+05:30  Cprogrammer
 * set bytes for correct interpretation of userNotFound in inquery()
 *
 * Revision 2.2  2002-06-21 20:37:35+05:30  Cprogrammer
 * exit if parent dies
 *
 * Revision 2.1  2002-04-10 21:50:40+05:30  Cprogrammer
 * use static location for QueryBuf
 *
 * Revision 1.2  2002-04-10 15:41:48+05:30  Cprogrammer
 * safety check for too much memory allocation
 *
 * Revision 1.1  2002-04-10 01:12:09+05:30  Cprogrammer
 * Initial revision
 *
 */

#ifndef	lint
static char     sccsid[] = "$Id: ProcessInFifo.c,v 2.29 2010-02-26 10:52:11+05:30 Cprogrammer Exp mbhangui $";
#endif

#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include "indimail.h"

int             user_query_count, relay_query_count, pwd_query_count, alias_query_count;
int             lpwd_query_count, dom_query_count, _debug;
time_t          start_time;
#ifdef CLUSTERED_SITE
int             host_query_count;
#endif

#ifdef DARWIN
static void     sig_usr1();
static void     sig_usr2();
static void     sig_term();
static void     sig_hup();
static void     sig_int();
#else
static void     sig_hand(int, int, struct sigcontext *, char *);
#endif
static void     sig_block(int);
static char    *getFifo_name();
static void     getTimeoutValues(int *, int *, char *, char *);
static char    *query_type(int);

int
ProcessInFifo()
{
	int             rfd, wfd, bytes, status, idx, pipe_size, readTimeout, writeTimeout;
	MYSQL         **mysqlptr;
	struct passwd  *pw;
	FILE           *fp;
	char            InFifo[MAX_BUFF], username[MAX_BUFF], pwbuf[MAX_BUFF], domain[MAX_BUFF],
					host_path[MAX_BUFF], tmpbuf[MAX_BUFF];
	char           *ptr, *cptr, *qmaildir, *controldir, *QueryBuf, *email, *myFifo,
				   *remoteip, *infifo, *local_ip, *cntrl_host;
	char           *real_domain;
	void            (*pstat) ();
	time_t          prev_time = 0l;

	_debug = (getenv("DEBUG") ? 1 : 0);
	start_time = time(0);
#ifdef DARWIN
	signal(SIGTERM, (void(*)()) sig_term);
	signal(SIGUSR1, (void(*)()) sig_usr1);
	signal(SIGUSR2, (void(*)()) sig_usr2);
	signal(SIGHUP, (void(*)()) sig_hup);
	signal(SIGINT, (void(*)()) sig_int);
#else
	signal(SIGTERM, (void(*)()) sig_hand);
	signal(SIGUSR1, (void(*)()) sig_hand);
	signal(SIGUSR2, (void(*)()) sig_hand);
	signal(SIGHUP, (void(*)()) sig_hand);
	signal(SIGINT, (void(*)()) sig_hand);
#endif
	getEnvConfigStr(&qmaildir, "QMAILDIR", QMAILDIR);
	getEnvConfigStr(&controldir, "CONTROLDIR", "control");
	getEnvConfigStr(&infifo, "INFIFO", INFIFO);
	if (*infifo == '/' || *infifo == '.')
		snprintf(InFifo, MAX_BUFF, "%s", infifo);
	else
		snprintf(InFifo, MAX_BUFF, "%s/%s/inquery/%s", qmaildir, controldir, infifo);
	getTimeoutValues(&readTimeout, &writeTimeout, qmaildir, controldir);
	/*- Open the Fifos */
	if (FifoCreate(InFifo) == -1)
	{
		fprintf(stderr, "InLookup: FifoCreate: %s: %s\n", InFifo, strerror(errno));
		return (-1);
	} else
	if ((rfd = open(InFifo, O_RDWR, 0)) == -1)
	{
		fprintf(stderr, "InLookup: open: %s: %s\n", InFifo, strerror(errno));
		return (-1);
	} else 
	if ((pipe_size = fpathconf(rfd, _PC_PIPE_BUF)) == -1)
	{
		fprintf(stderr, "InLookup: fpathconf: %s: %s\n", InFifo, strerror(errno));
		return (-1);
	} else
	if (!(QueryBuf = (char *) malloc(pipe_size * sizeof(char))))
	{
		fprintf(stderr, "InLookup: malloc(%d bytes): %s: %s\n", pipe_size, InFifo, strerror(errno));
		return (-1);
	} else
	if ((pstat = signal(SIGPIPE, SIG_IGN)) == SIG_ERR)
	{
		fprintf(stderr, "InLookup: signal: %s: %s\n", InFifo, strerror(errno));
		return (-1);
	}
	user_query_count = relay_query_count = pwd_query_count = lpwd_query_count = alias_query_count = dom_query_count = 0;
#ifdef CLUSTERED_SITE
	host_query_count = 0;
#endif
	if (!(local_ip = get_local_ip()))
	{
		fprintf(stderr, "InLookup: Could not get local ip: %s\n", strerror(errno));
		signal(SIGPIPE, pstat);
		return(-1);
	}
	for (bytes = 0;getppid() != 1;)
	{
		if ((idx = read(rfd, (char *) &bytes, sizeof(int))) == -1)
		{
#ifdef ERESTART
			if (errno != EINTR && errno != ERESTART)
#else
			if (errno != EINTR)
#endif
				fprintf(stderr, "InLookup: read: %s\n", strerror(errno));
			continue;
		} else
		if (!idx)
		{
			close(rfd);
			if ((rfd = open(InFifo, O_RDWR, 0)) == -1)
			{
				fprintf(stderr, "InLookup: open: %s: %s\n", InFifo, strerror(errno));
				signal(SIGPIPE, pstat);
				return (-1);
			} else
				continue;
		} else
		if (bytes > pipe_size)
		{
			errno = EMSGSIZE;
			fprintf(stderr, "InLookup: bytes %d, pipe_size %d, %s\n", bytes, pipe_size, strerror(errno));
			continue;
		} else
		if ((idx = timeoutread(readTimeout, rfd, QueryBuf, bytes)) == -1)
		{
			fprintf(stderr, "InLookup: read-int: %s\n", strerror(errno));
			continue;
		} else
		if (!idx)
		{
			close(rfd);
			if ((rfd = open(InFifo, O_RDWR, 0)) == -1)
			{
				fprintf(stderr, "InLookup: open: %s: %s\n", InFifo, strerror(errno));
				signal(SIGPIPE, pstat);
				return (-1);
			} else
				continue;
		}
		if (verbose || _debug)
			prev_time = time(0);
#ifdef CLUSTERED_SITE
		snprintf(host_path, MAX_BUFF, "%s/%s/host.cntrl", qmaildir, controldir);
		if (access(host_path, F_OK))
			snprintf(host_path, MAX_BUFF, "%s/%s/host.mysql", qmaildir, controldir);
		if (access(host_path, F_OK))
			cntrl_host = 0;
		else
		{
			if (!(fp = fopen(host_path, "r")))
				cntrl_host = 0;
			else
			{
				if (!fgets(tmpbuf, MAX_BUFF - 2, fp))
				{
					fprintf(stderr, "fgets: %s\n", strerror(errno));
					fclose(fp);
					cntrl_host = 0;
					return(-1);
				}
				fclose(fp);
				if ((ptr = strrchr(tmpbuf, '\n')))
					*ptr = 0;
				cntrl_host = tmpbuf;
			}
		}
		if (!isopen_cntrl && open_central_db(cntrl_host))
		{
			fprintf(stderr, "InLookup: Unable to open central db\n");
			signal(SIGPIPE, pstat);
			return (-1);
		}
		if (mysql_ping(&mysql[0]))
		{
			fprintf(stderr, "mysql_ping: %s: Reconnecting to central db...\n", mysql_error(&mysql[0]));
			mysql_close(&mysql[0]);
			isopen_cntrl = 0;
			if (open_central_db(cntrl_host))
			{
				fprintf(stderr, "InLookup: Unable to open central db\n");
				signal(SIGPIPE, pstat);
				return (-1);
			}
		}
#endif
		switch(*QueryBuf)
		{
			case USER_QUERY:
				user_query_count++;
				break;
			case RELAY_QUERY:
				relay_query_count++;
				break;
			case PWD_QUERY:
				pwd_query_count++;
				break;
#ifdef CLUSTERED_SITE
			case HOST_QUERY:
				host_query_count++;
				break;
#endif
			case ALIAS_QUERY:
				alias_query_count++;
				break;
			case LPWD_QUERY:
				lpwd_query_count++;
				break;
			case DOMAIN_QUERY:
				dom_query_count++;
				break;
			default:
				continue;
		}
		email = QueryBuf + 2;
		for (ptr = email;*ptr;ptr++);
		ptr++;
		myFifo = ptr;
		for (;*ptr;ptr++);
		ptr++;
		remoteip = ptr;
		if (verbose || _debug)
		{
			if ((cptr = strrchr(InFifo, '/')))
				cptr++;
			else
				cptr = InFifo;
			printf("%s->%s, Bytes %d, Query %d, User %s, RemoteIp %s\n", 
				cptr, myFifo, bytes, *QueryBuf, email, *QueryBuf == 2 ? remoteip : "N/A");
			fflush(stdout);
		}
		if ((wfd = open(myFifo, O_RDWR, 0)) == -1)
		{
			fprintf(stderr, "InLookup: open-probably-timeout: %s: QueryType %s: %s\n", myFifo, query_type(*QueryBuf), strerror(errno));
			if (errno != ENOENT)
				unlink(myFifo);
			continue;
		} else
			unlink(myFifo); /*- make this invisible */
		switch(*QueryBuf)
		{
			case USER_QUERY:
				status = UserInLookup(email);
				if (timeoutwrite(writeTimeout, wfd, (char *) &status, sizeof(int)) == -1)
					fprintf(stderr, "InLookup: write-UserInLookup: %s\n", strerror(errno));
				close(wfd);
				break;
			case RELAY_QUERY:
				status = RelayInLookup(email, remoteip);
				if (timeoutwrite(writeTimeout, wfd, (char *) &status, sizeof(int)) == -1)
					fprintf(stderr, "InLookup: write-RelayInLookup: %s\n", strerror(errno));
				close(wfd);
				break;
#ifdef CLUSTERED_SITE
			case HOST_QUERY:
				ptr = findmdahost(email);
				if (ptr)
					bytes = slen(ptr) + 1;
				else
					bytes = (userNotFound ? 0 : -1);
				if (bytes > pipe_size)
					bytes = -1;
				if (timeoutwrite(writeTimeout, wfd, (char *) &bytes, sizeof(int)) == -1)
					fprintf(stderr, "InLookup: write-findmdahost: %s\n", strerror(errno));
				else
				if (bytes > 0 && timeoutwrite(writeTimeout, wfd, ptr, bytes) == -1)
					fprintf(stderr, "InLookup: write-findmdahost: %s\n", strerror(errno));
				close(wfd);
				break;
#endif
			case ALIAS_QUERY:
				ptr = AliasInLookup(email);
				if (ptr && *ptr)
				{
					if ((bytes = slen(ptr) + 1) > pipe_size)
						bytes = -1;
				}
				else
				{
					bytes = 1; /*- write Null Byte */
					ptr = "\0";
				}
				if (timeoutwrite(writeTimeout, wfd, (char *) &bytes, sizeof(int)) == -1)
					fprintf(stderr, "InLookup: write-AliasInLookup: %s\n", strerror(errno));
				else
				if (bytes > 0 && timeoutwrite(writeTimeout, wfd, ptr, bytes) == -1)
					fprintf(stderr, "InLookup: write-AliasInLookup: %s\n", strerror(errno));
				close(wfd);
#ifdef LOW_MEM
				AliasInLookup(0);
#endif
				break;
			case PWD_QUERY:
				/*- Connect to mysql after fetching ip host details for the user from hostcntrl.*/
				if ((pw = PwdInLookup(email)))
				{
					snprintf(pwbuf, sizeof(pwbuf), "PWSTRUCT=%s:%s:%d:%d:%s:%s:%s:%d", 
						email,
						pw->pw_passwd,
						pw->pw_uid,
						pw->pw_gid,
						pw->pw_gecos,
						pw->pw_dir,
						pw->pw_shell, is_inactive);
					if ((bytes = (slen(pwbuf) + 1)) > pipe_size)
						bytes = -1;
				} else
				if (userNotFound)
				{
					snprintf(pwbuf, sizeof(pwbuf), "PWSTRUCT=No such user %s", email);
					bytes = slen(pwbuf) + 1;
				} else
					bytes = 0;
				if (timeoutwrite(writeTimeout, wfd, (char *) &bytes, sizeof(int)) == -1)
					fprintf(stderr, "InLookup: write-PwdInLookup: %s\n", strerror(errno));
				else
				if (bytes > 0 && timeoutwrite(writeTimeout, wfd, pwbuf, bytes) == -1)
					fprintf(stderr, "InLookup: write-PwdInLookup: %s\n", strerror(errno));
				close(wfd);
				break;
			case LPWD_QUERY:
				/*- Connect directly to mysql without going to hostcntrl
				 * This is done by figuring out the mysql parameters from
				 * dbinfo structure for the local ip
				 */
				for (ptr = email, cptr = username;*ptr && *ptr != '@';*cptr++ = *ptr++);
				*cptr = 0;
				if (*(ptr++) == '@' && *ptr)
					scopy(domain, ptr, MAX_BUFF);
				else
				{
					getEnvConfigStr(&ptr, "DEFAULT_DOMAIN", DEFAULT_DOMAIN);
					scopy(domain, ptr, MAX_BUFF);
				}
				if (!(real_domain = vget_real_domain(domain)))
					real_domain = domain;
				if (!(mysqlptr = mdaMysqlConnect(local_ip, real_domain)))
				{
					fprintf(stderr, "InLookup: mdaMysqlConnect: %s %s: failure\n", local_ip, real_domain);
					bytes = -1;
				} else
				{
					vauth_init(1, *mysqlptr);
					if ((pw = vauth_getpw(username, domain)))
					{
						snprintf(pwbuf, sizeof(pwbuf), "PWSTRUCT=%s:%s:%d:%d:%s:%s:%s:%d", 
							email,
							pw->pw_passwd,
							pw->pw_uid,
							pw->pw_gid,
							pw->pw_gecos,
							pw->pw_dir,
							pw->pw_shell, is_inactive);
						if ((bytes = (slen(pwbuf) + 1)) > pipe_size)
							bytes = -1;
					} else
					if (userNotFound)
					{
						snprintf(pwbuf, sizeof(pwbuf), "PWSTRUCT=No such user %s", email);
						bytes = slen(pwbuf) + 1;
					} else
						bytes = 0;
					is_open = 0;
				}
				if (timeoutwrite(writeTimeout, wfd, (char *) &bytes, sizeof(int)) == -1)
					fprintf(stderr, "InLookup: write-mdaMysqlConnect: %s\n", strerror(errno));
				else
				if (bytes > 0 && timeoutwrite(writeTimeout, wfd, pwbuf, bytes) == -1)
					fprintf(stderr, "InLookup: write-mdaMysqlConnect: %s\n", strerror(errno));
				close(wfd);
				break;
			case DOMAIN_QUERY:
				if (!(real_domain = vget_real_domain(email)))
					real_domain = email;
				bytes = slen(real_domain) + 1;
				if (bytes > pipe_size)
					bytes = -1;
				if (timeoutwrite(writeTimeout, wfd, (char *) &bytes, sizeof(int)) == -1)
					fprintf(stderr, "InLookup: write-findmdahost: %s\n", strerror(errno));
				else
				if (bytes > 0 && timeoutwrite(writeTimeout, wfd, real_domain, bytes) == -1)
					fprintf(stderr, "InLookup: write-get_real_domain: %s\n", strerror(errno));
				close(wfd);
				break;
		} /*- switch(*QueryBuf) */
		if (verbose || _debug)
		{
			printf("%ld %s -> %s\n", time(0) - prev_time, query_type(*QueryBuf), myFifo);
			fflush(stdout);
		}
	} /*- for (QueryBuf = (char *) 0;;) */
	signal(SIGPIPE, pstat);
	return(1);
}

static char    *
query_type(int status)
{
	static char     tmpbuf[28];

	switch(status)
	{
		case USER_QUERY:
			return("'User Query'");
		case RELAY_QUERY:
			return("'Relay Query'");
		case PWD_QUERY:
			return("'Password Query'");
#ifdef CLUSTERED_SITE
		case HOST_QUERY:
			return("'Host Query'");
#endif
		case ALIAS_QUERY:
			return("'Alias Query'");
		case LPWD_QUERY:
			return("'Local Password Query'");
		case DOMAIN_QUERY:
			return("'Domain Query'");
		default:
			break;
	}
	snprintf(tmpbuf, sizeof(tmpbuf), "'Unknown %d'\n", status);
	return(tmpbuf);
}

static char *
getFifo_name()
{
	static char     inFifo[MAX_BUFF];
	char           *qmaildir, *controldir, *infifo;

	getEnvConfigStr(&infifo, "INFIFO", INFIFO);
	if (*infifo == '/' || *infifo == '.')
		snprintf(inFifo, MAX_BUFF, "%s", infifo);
	else
	{
		getEnvConfigStr(&qmaildir, "QMAILDIR", QMAILDIR);
		getEnvConfigStr(&controldir, "CONTROLDIR", "control");
		snprintf(inFifo, MAX_BUFF, "%s/%s/inquery/%s", qmaildir, controldir, infifo);
	}
	return(inFifo);
}

#ifdef DARWIN
static void
sig_usr1()
{
	char           *fifo_name;
	long            total_count;
	time_t          cur_time;

	fifo_name = getFifo_name();
	printf("%d INFIFO %s, Got SIGUSR1\n", (int) getpid(), fifo_name);
	printf("%d %s Dumping Stats\n", (int) getpid(), fifo_name);
	cur_time = time(0);
#ifdef CLUSTERED_SITE
	printf("User Query %d, Relay Query %d, Password Query %d:%d, Alias Query %d, Host Query %d, Domain Query %d\n", 
		user_query_count, relay_query_count, pwd_query_count, lpwd_query_count, alias_query_count, 
		host_query_count, dom_query_count);
	total_count = user_query_count + relay_query_count + pwd_query_count + lpwd_query_count + alias_query_count + 
		host_query_count + dom_query_count;
#else
	printf("User Query %d, Relay Query %d, Password Query %d:%d, Alias Query %d, Domain Query %d\n", 
		user_query_count, relay_query_count, pwd_query_count, lpwd_query_count, alias_query_count, dom_query_count);
	total_count = user_query_count + relay_query_count + pwd_query_count + lpwd_query_count + alias_query_count + dom_query_count;
#endif
	printf("Start Time: %s", ctime(&start_time));
	printf("End   Time: %s", ctime(&cur_time));
	printf("Queries %ld, Total Time %ld secs, Query/Sec = %.02f\n", total_count, (cur_time - start_time), 
		(float) ((float) total_count/(cur_time - start_time)));
	fflush(stdout);
	signal(SIGUSR1, (void(*)()) sig_usr1);
	errno = EINTR;
	return;
}

static void
sig_usr2()
{
	char           *fifo_name;

	fifo_name = getFifo_name();
	printf("%d INFIFO %s, Got SIGUSR2\n", (int) getpid(), fifo_name);
	printf("%d %s Resetting DEBUG flag to %d\n", (int) getpid(), fifo_name, _debug ? 0 : 1);
	_debug = (_debug ? 0 : 1);
	fflush(stdout);
	signal(SIGUSR2, (void(*)()) sig_usr2);
	errno = EINTR;
	return;
}

static void
sig_hup()
{
	char           *fifo_name;

	fifo_name = getFifo_name();
	printf("%d INFIFO %s, Got SIGHUP\n", (int) getpid(), fifo_name);
	printf("%d %s Reconfiguring\n", (int) getpid(), fifo_name);
#ifdef QUERY_CACHE
	findhost_cache(0);
	is_user_present_cache(0);
	vauth_getpw_cache(0);
	cntrl_clearaddflag_cache(0);
	cntrl_cleardelflag_cache(0);
	is_distributed_domain_cache(0);
	vauth_get_realdomain_cache(0);
	vget_assign_cache(0);
	vget_real_domain_cache(0);
#endif
	fflush(stdout);
	signal(SIGHUP, (void(*)()) sig_hup);
	errno = EINTR;
	return;
}

static void
sig_int()
{
	char           *fifo_name;

	fifo_name = getFifo_name();
	printf("%d INFIFO %s, Got SIGINT\n", (int) getpid(), fifo_name);
	printf("%d %s closing db\n", (int) getpid(), fifo_name);
	close_db();
	fflush(stdout);
	signal(SIGINT, (void(*)()) sig_int);
	errno = EINTR;
	return;
}

static void
sig_term()
{
	char           *fifo_name;
	long            total_count;
	time_t          cur_time;

	sig_block(SIGTERM);
	fifo_name = getFifo_name();
	printf("%d %s ARGH!! Committing suicide on SIGTERM\n", (int) getpid(), fifo_name);
	printf("%d INFIFO %s, Got SIGTERM\n", (int) getpid(), fifo_name);
	cur_time = time(0);
#ifdef CLUSTERED_SITE
	printf("User Query %d, Relay Query %d, Password Query %d:%d, Alias Query %d, Host Query %d, Domain Query %d\n", 
		user_query_count, relay_query_count, pwd_query_count, lpwd_query_count, alias_query_count, 
		host_query_count, dom_query_count);
	total_count = user_query_count + relay_query_count + pwd_query_count + lpwd_query_count + alias_query_count + 
		host_query_count + dom_query_count;
#else
	printf("User Query %d, Relay Query %d, Password Query %d:%d, Alias Query %d, Domain Query %d\n", 
		user_query_count, relay_query_count, pwd_query_count, lpwd_query_count, alias_query_count, dom_query_count);
	total_count = user_query_count + relay_query_count + pwd_query_count + lpwd_query_count + alias_query_count + dom_query_count;
#endif
	printf("Start Time: %s", ctime(&start_time));
	printf("End   Time: %s", ctime(&cur_time));
	printf("Queries %ld, Total Time %ld secs, Query/Sec = %.02f\n", total_count, (cur_time - start_time), 
		(float) ((float) total_count/(cur_time - start_time)));
	fflush(stdout);
	close_db();
	unlink(fifo_name);
	exit(0);
}
#else
static void
sig_hand(sig, code, scp, addr)
	int             sig, code;
	struct sigcontext *scp;
	char           *addr;
{
	char           *fifo_name;
	long            total_count;
	time_t          cur_time;

	fifo_name = getFifo_name();
	if (sig == SIGTERM)
	{
		sig_block(sig);
		printf("%d %s ARGH!! Committing suicide on SIGTERM\n", (int) getpid(), fifo_name);
	}
	printf("%d INFIFO %s, Got %s\n", (int) getpid(), fifo_name, sys_siglist[sig]);
	switch (sig)
	{
		case SIGUSR1:
			printf("%d %s Dumping Stats\n", (int) getpid(), fifo_name);
		case SIGTERM:
			cur_time = time(0);
#ifdef CLUSTERED_SITE
			printf("User Query %d, Relay Query %d, Password Query %d:%d, Alias Query %d, Host Query %d, Domain Query %d\n", 
				user_query_count, relay_query_count, pwd_query_count, lpwd_query_count, alias_query_count, 
				host_query_count, dom_query_count);
			total_count = user_query_count + relay_query_count + pwd_query_count + lpwd_query_count + alias_query_count + 
				host_query_count + dom_query_count;
#else
			printf("User Query %d, Relay Query %d, Password Query %d:%d, Alias Query %d, Domain Query %d\n", 
				user_query_count, relay_query_count, pwd_query_count, lpwd_query_count, alias_query_count, dom_query_count);
			total_count = user_query_count + relay_query_count + pwd_query_count + lpwd_query_count + alias_query_count + dom_query_count;
#endif
			printf("Start Time: %s", ctime(&start_time));
			printf("End   Time: %s", ctime(&cur_time));
			printf("Queries %ld, Total Time %ld secs, Query/Sec = %.02f\n", total_count, (cur_time - start_time), 
				(float) ((float) total_count/(cur_time - start_time)));
			if (sig == SIGTERM)
			{
				fflush(stdout);
				close_db();
				unlink(fifo_name);
				exit(0);
			}
			break;
		case SIGUSR2:
			printf("%d %s Resetting DEBUG flag to %d\n", (int) getpid(), fifo_name, _debug ? 0 : 1);
			_debug = (_debug ? 0 : 1);
		break;
		case SIGHUP:
			printf("%d %s Reconfiguring\n", (int) getpid(), fifo_name);
#ifdef QUERY_CACHE
			findhost_cache(0);
			is_user_present_cache(0);
			vauth_getpw_cache(0);
			cntrl_clearaddflag_cache(0);
			cntrl_cleardelflag_cache(0);
			is_distributed_domain_cache(0);
			vauth_get_realdomain_cache(0);
			vget_assign_cache(0);
			vget_real_domain_cache(0);
#endif
		break;
		case SIGINT:
			printf("%d %s closing db\n", (int) getpid(), fifo_name);
			close_db();
		break;
	} /*- switch (sig) */
	fflush(stdout);
	signal(sig, (void(*)()) sig_hand);
	errno = EINTR;
	return;
}
#endif /*- #ifdef DARWIN */

static void
sig_block(sig)
	int             sig;
{
#ifdef HAVE_SIGPROCMASK
	sigset_t        ss;
	sigemptyset(&ss);
	sigaddset(&ss, sig);
	sigprocmask(SIG_BLOCK, &ss, (sigset_t *) 0);
#else
	sigblock(1 << (sig - 1));
#endif
}

static void
getTimeoutValues(int *readTimeout, int *writeTimeout, char *qmaildir, char *controldir)
{
	char            TmpBuf[MAX_BUFF];
	FILE           *fp;

	snprintf(TmpBuf, MAX_BUFF, "%s/%s/timeoutread", qmaildir, controldir);
	if ((fp = fopen(TmpBuf, "r")))
	{
		if (fgets(TmpBuf, MAX_BUFF - 2, fp))
		{
			if (sscanf(TmpBuf, "%d", readTimeout) != 1)
				*readTimeout = 4;
		}
		fclose(fp);
	} else
		*readTimeout = 4;
	snprintf(TmpBuf, MAX_BUFF, "%s/%s/timeoutwrite", qmaildir, controldir);
	if ((fp = fopen(TmpBuf, "r")))
	{
		if (fgets(TmpBuf, MAX_BUFF - 2, fp))
		{
			if (sscanf(TmpBuf, "%d", writeTimeout) != 1)
				*writeTimeout = 4;
		}
		fclose(fp);
	} else
		*writeTimeout = 4;
	return;
}

void
getversion_ProcessInFifo_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
