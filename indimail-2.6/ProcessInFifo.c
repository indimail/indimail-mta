/*
 * $Log: ProcessInFifo.c,v $
 * Revision 2.51  2019-03-24 20:56:21+05:30  Cprogrammer
 * fixed typo
 *
 * Revision 2.50  2018-03-25 13:56:18+05:30  Cprogrammer
 * fixed ambiguous select statement in join statement with timestamp column in indimail, lastuath table
 *
 * Revision 2.49  2018-03-23 12:48:22+05:30  Cprogrammer
 * avoid connection to hostcntrl if host.cntrl is absent
 *
 * Revision 2.48  2018-01-08 10:46:52+05:30  Cprogrammer
 * create lastauth table if not existing
 *
 * Revision 2.47  2017-12-20 13:42:58+05:30  Cprogrammer
 * BUG - initialize pwStat
 *
 * Revision 2.46  2017-12-11 15:51:15+05:30  Cprogrammer
 * added function cache_active_pwd() to precache active login passwords in memory
 *
 * Revision 2.45  2017-11-21 14:41:16+05:30  Cprogrammer
 * use MAX_BTREE_COUNT to limit btree search nodes
 *
 * Revision 2.44  2017-11-21 01:12:27+05:30  Cprogrammer
 * use binary tree algorithm if USE_BTREE env variable is defined
 *
 * Revision 2.43  2017-11-20 23:23:22+05:30  Cprogrammer
 * binary search implementation for USER_QUERY, PWD_QUERY, HOST_QUERY, ALIAS_QUERY
 *
 * Revision 2.42  2017-03-27 08:53:35+05:30  Cprogrammer
 * added FIFODIR variable for location of infifo
 *
 * Revision 2.41  2017-03-13 14:05:45+05:30  Cprogrammer
 * replaced qmaildir with sysconfdir
 *
 * Revision 2.40  2016-05-25 09:05:20+05:30  Cprogrammer
 * use /usr/lib/indimail for plugins
 *
 * Revision 2.39  2016-05-17 17:09:39+05:30  Cprogrammer
 * use control directory set by configure
 *
 * Revision 2.38  2016-01-12 14:25:15+05:30  Cprogrammer
 * use AF_INET for get_local_ip()
 *
 * Revision 2.37  2015-08-21 10:47:09+05:30  Cprogrammer
 * replaced getEnvConfigInit with getEnvConfigLong
 *
 * Revision 2.36  2013-11-22 16:16:50+05:30  Cprogrammer
 * Display SigTerm message only if verbose or debug is enabled
 *
 * Revision 2.35  2013-08-03 17:18:49+05:30  Cprogrammer
 * close database on sighup so that we reload all domains
 *
 * Revision 2.34  2011-04-03 21:51:52+05:30  Cprogrammer
 * added enterprise support code
 *
 * Revision 2.33  2011-04-03 18:00:19+05:30  Cprogrammer
 * added instance number argument to ProcessInFifo()
 *
 * Revision 2.32  2010-06-07 18:33:18+05:30  Cprogrammer
 * pass additional connect_all argument to findmdahost()
 *
 * Revision 2.31  2010-04-11 22:22:25+05:30  Cprogrammer
 * replaced LPWD_QUERY with LIMIT_QUERY for domain limits
 *
 * Revision 2.30  2010-03-28 18:54:25+05:30  Cprogrammer
 * use 127.0.0.1 if get_local_ip() fails
 *
 * Revision 2.29  2010-02-26 10:52:11+05:30  Cprogrammer
 * use host.mysql if host.cntrl is not present
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
static char     sccsid[] = "$Id: ProcessInFifo.c,v 2.51 2019-03-24 20:56:21+05:30 Cprogrammer Exp mbhangui $";
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <errno.h>
#include <signal.h>
#include <time.h>
#include <sys/socket.h>
#include "indimail.h"
#include <mysqld_error.h>
#include "error_stack.h"
#include "in_bsearch.h"
#ifdef ENABLE_ENTERPRISE
#include <dlfcn.h>
#endif

int             user_query_count, relay_query_count, pwd_query_count, alias_query_count;
int             limit_query_count, dom_query_count, btree_count = 0, _debug;
time_t          start_time;
#ifdef CLUSTERED_SITE
int             host_query_count;
#endif
static void    *in_root = 0;

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

/*-
typedef struct
{
	char           *in_key;
	struct passwd   in_pw;
	char           *aliases;
	char           *mdahost;
	char           *domain;
	 *
	 *  0: User is fine
	 *  1: User is not present
	 *  2: User is Inactive
	 *  3: User is overquota
	 * -1: System Error
	 *
	int             in_userStatus;
} INENTRY;
*/

INENTRY        *
mk_in_entry(char *key)
{
	INENTRY        *in;

	if (!(in = (INENTRY *) malloc(sizeof (INENTRY))))
		return ((INENTRY *) 0);
	in->in_key = strdup(key);
	in->aliases = in->mdahost = in->domain = (char *) 0;
	in->in_userStatus = -1;
	in->pwStat = 0;
	in->in_pw.pw_name = (char *) 0;
	in->in_pw.pw_passwd = (char *) 0;
	in->in_pw.pw_gecos = (char *) 0;
	in->in_pw.pw_dir = (char *) 0;
	in->in_pw.pw_shell = (char *) 0;
	return in;
}

void
in_free_func(void *in_data)
{
	INENTRY        *m = in_data;
	if (!m)
		return;
	if (m->in_key) {
		free(m->in_key);
		free(m->aliases);
		free(m->mdahost);
		free(m->domain);
		free(m->in_pw.pw_name);
		free(m->in_pw.pw_passwd);
		free(m->in_pw.pw_gecos);
		free(m->in_pw.pw_dir);
		free(m->in_pw.pw_shell);
		m->in_key = 0;
	}
	free(in_data);
	return;
}

int
in_compare_func(const void *l, const void *r)
{
	return strcmp(*(char **) l, *(char **) r);
}

void
walk_entry(const void *in_data, VISIT x, int level)
{
	INENTRY        *m = *(INENTRY **) in_data;
	printf("<%d>Walk on node %s %s %s  \n", level,
		   x == preorder ? "preorder" : x == postorder ? "postorder" : x == endorder ? "endorder" : x == leaf ? "leaf" : "unknown",
		   m->in_key, m->in_pw.pw_passwd);
	return;
}

static int      pwdCache; /*- for sighup to figure out if caching was selected on startup */

int
cache_active_pwd(time_t tval)
{
	MYSQL_RES      *res;
	MYSQL_ROW       row;
	char            SqlBuf[SQL_BUF_SIZE], email[MAX_BUFF];
	int             use_btree, max_btree_count, err;
	static time_t   act_secs;
	char           *ptr;
	INENTRY        *in, *re, *retval;

	if (tval)
		act_secs = tval;
	pwdCache = 1;
	ptr = getenv("USE_BTREE");
	use_btree = ((ptr && *ptr == '1') ? 1 : 0);
	if (!use_btree)
		return (0);
	max_btree_count = ((ptr = getenv("MAX_BTREE_COUNT")) && *ptr ? atoi(ptr) : -1);
	if (in_root) {
		tdestroy(in_root, in_free_func);
		in_root = 0;
		btree_count = 0;
	}
	snprintf(SqlBuf, SQL_BUF_SIZE - 1,
		"SELECT "
		"    pw_name, pw_domain, pw_passwd, pw_uid, pw_gid, pw_gecos, pw_dir, pw_shell "
		"FROM"
		"    indimail"
		"        JOIN"
		"    lastauth ON pw_name = user AND pw_domain = domain "
		"WHERE"
		"    UNIX_timestamp(lastauth.timestamp) >= UNIX_timestamp() - %ld "
		"AND"
		"	service in (\"imap\", \"pop3\", \"wtbm\") "
		"GROUP BY pw_name, pw_domain "
		"ORDER BY lastauth.timestamp desc", act_secs);

	if ((err = vauth_open((char *) 0)) != 0)
		return (-1);
	if (mysql_query(&mysql[1], SqlBuf)) {
		if (mysql_errno(&mysql[1]) == ER_NO_SUCH_TABLE) {
			create_table(ON_LOCAL, "lastauth", LASTAUTH_TABLE_LAYOUT);
			vclose();
			return (0);
		}
		mysql_perror("cache_active_pwd-mysql_query: %s", SqlBuf);
		return (-1);
	}
	if (!(res = mysql_store_result(&mysql[1]))) {
		mysql_perror("cache_active_pwd-mysql_store_result");
		return (-1);
	}
	for(;(row = mysql_fetch_row(res));) {
		snprintf(email, sizeof(email) - 1, "%s@%s", row[0], row[1]);
		if (!(in = mk_in_entry(email))) {
			mysql_free_result(res);
			return (-1);
		}
		if (!(retval = tsearch (in, &in_root, in_compare_func))) {
			in_free_func(in);
			mysql_free_result(res);
			return (-1);
		} else {
			re = *(INENTRY **) retval;
			if (re != in) { /*- existing data, shouldn't happen */
				in_free_func(in);
				mysql_free_result(res);
				return (1);
			} else {/*- New entry in was added.  */
				in->in_pw.pw_name = strdup(row[0]);
				in->domain = strdup(row[1]);
				in->in_pw.pw_passwd = strdup(row[2]);
				in->in_pw.pw_uid = atoi(row[3]);
				in->in_pw.pw_gid = atoi(row[4]);
				in->in_pw.pw_gecos = strdup(row[5]);
				in->in_pw.pw_dir = strdup(row[6]);
				in->in_pw.pw_shell = strdup(row[7]);
				in->pwStat = 1;
				btree_count++;
				if (max_btree_count > 0 && btree_count >= max_btree_count)
					break;
			}
		}
	}
	mysql_free_result(res);
	vclose();
	return (0);
}

#ifdef ENABLE_ENTERPRISE
int
do_startup(int instNum)
{
	void           *handle;
	char           *plugindir, *plugin_symb, *start_plugin, *error;
	char            plugin[MAX_BUFF];
	int             (*func) (void);
	int             status;

	if (!(plugindir = getenv("PLUGINDIR")))
		plugindir = "plugins";
	if (strchr(plugindir, '/')) {
		fprintf(stderr, "alert: plugindir cannot have an absolute path\n");
		return (-1);
	}
	if (!(plugin_symb = getenv("START_PLUGIN_SYMB")))
		plugin_symb = "startup";
	if (!(start_plugin = getenv("START_PLUGIN")))
		start_plugin = "indimail-license.so";
	snprintf(plugin, MAX_BUFF - 1, "/usr/lib/indimail/%s/%s", plugindir, start_plugin);
	if (access(plugin, F_OK)) {
		fprintf(stderr, "InLookup[%d] plugin %s: %s\n", instNum, plugin, strerror(errno));
		return (2);
	}
	if (!(handle = dlopen(plugin, RTLD_LAZY|RTLD_GLOBAL))) {
		fprintf(stderr, "InLookup[%d] dlopen failed for %s: %s\n", instNum, plugin, dlerror());
		return (-1);
	}
	dlerror(); /*- man page told me to do this */
	func = dlsym(handle, plugin_symb);
	if ((error = dlerror())) {
		fprintf(stderr, "InLookup[%d] dlsym %s failed: %s\n", instNum, plugin_symb, error);
		_exit(111);
	}
	printf("InLookup[%d] Checking Plugin %s\n", instNum, start_plugin);
	fflush(stdout);
	if ((status = (*func) ()))
		fprintf(stderr, "InLookup[%d] function %s failed with status %d\n",
			instNum, plugin_symb, status);
	if (dlclose(handle)) {
		fprintf(stderr, "InLookup[%d] dlclose for %s failed: %s\n", instNum, plugin, error);
		return (-1);
	}
	return(status);
}
#endif

int
ProcessInFifo(int instNum)
{
	int             rfd, wfd, bytes, status, idx, pipe_size, readTimeout, writeTimeout, relative, use_btree;
	int             max_btree_count;
	INENTRY        *in, *re, *retval;
	struct passwd  *pw;
	FILE           *fp;
	char            InFifo[MAX_BUFF], pwbuf[MAX_BUFF], host_path[MAX_BUFF], tmpbuf[MAX_BUFF];
	char           *ptr, *fifoName, *sysconfdir, *infifo_dir, *controldir, *QueryBuf, *email, *myFifo,
				   *remoteip, *infifo, *local_ip, *cntrl_host, *real_domain;
	void            (*pstat) ();
	void           *(*search_func) (const void *key, void *const *rootp, int (*compar)(const void *, const void *));
	time_t          prev_time = 0l;
#ifdef ENABLE_DOMAIN_LIMITS
	struct vlimits  limits;
#endif
#ifdef ENABLE_ENTERPRISE
	mdir_t          count;
	long            ucount;
#endif

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
#ifdef ENABLE_ENTERPRISE
	for (;;) {
		if ((count = count_table("indimail")) == -1) {
			flush_stack();
			printf("InLookup[%d] PPID %d PID %d unable to get count\n",
				instNum, getppid(), getpid());
			fflush(stdout);
			sleep(5);
			continue;
		}
		vclose();
		getEnvConfigLong(&ucount, "MAXUSERS", 10000);
		if (count > ucount && (idx = do_startup(instNum))) {
			printf("enterprise version requires plugin\n");
			if (idx)
				printf("invalid plugin\n");
			fflush(stdout);
			sleep(5);
			return (-1);
		}
		printf("InLookup[%d] PPID %d PID %d with %"PRId64" users\n",
			instNum, getppid(), getpid(), count);
		fflush(stdout);
		break;
	}
#else
	printf("InLookup[%d] PPID %d PID %d\n",
		instNum, getppid(), getpid());
	fflush(stdout);
#endif
	getEnvConfigStr(&controldir, "CONTROLDIR", CONTROLDIR);
	getEnvConfigStr(&infifo_dir, "FIFODIR", INDIMAILDIR"/inquery");
	relative = *infifo_dir == '/' ? 0 : 1;
	getEnvConfigStr(&infifo, "INFIFO", INFIFO);
	if (*infifo == '/' || *infifo == '.')
		snprintf(InFifo, MAX_BUFF, "%s", infifo);
	else {
		if (relative)
			snprintf(InFifo, MAX_BUFF, INDIMAILDIR"%s/%s", infifo_dir, infifo);
		else {
			if (indimailuid == -1 || indimailgid == -1)
				GetIndiId(&indimailuid, &indimailgid);
			r_mkdir(infifo_dir, 0775, indimailuid, indimailgid);
			snprintf(InFifo, MAX_BUFF, "%s/%s", infifo_dir, infifo);
		}
	}
	getEnvConfigStr(&sysconfdir, "SYSCONFDIR", SYSCONFDIR);
	getTimeoutValues(&readTimeout, &writeTimeout, sysconfdir, controldir);
	/*- Open the Fifos */
	if (FifoCreate(InFifo) == -1) {
		fprintf(stderr, "InLookup: FifoCreate: %s: %s\n", InFifo, strerror(errno));
		return (-1);
	} else
	if ((rfd = open(InFifo, O_RDWR, 0)) == -1) {
		fprintf(stderr, "InLookup: open: %s: %s\n", InFifo, strerror(errno));
		return (-1);
	} else 
	if ((pipe_size = fpathconf(rfd, _PC_PIPE_BUF)) == -1) {
		fprintf(stderr, "InLookup: fpathconf: %s: %s\n", InFifo, strerror(errno));
		return (-1);
	} else
	if (!(QueryBuf = (char *) malloc(pipe_size * sizeof(char)))) {
		fprintf(stderr, "InLookup: malloc(%d bytes): %s: %s\n", pipe_size, InFifo, strerror(errno));
		return (-1);
	}
	user_query_count = relay_query_count = pwd_query_count = limit_query_count = alias_query_count = dom_query_count = 0;
#ifdef CLUSTERED_SITE
	host_query_count = 0;
#endif
	if (!(local_ip = get_local_ip(PF_INET))) {
		local_ip = "127.0.0.1";
		fprintf(stderr, "ProcessInFifo: get_local_ip failed. using localhost\n");
	}
	if ((pstat = signal(SIGPIPE, SIG_IGN)) == SIG_ERR) {
		fprintf(stderr, "InLookup: signal: %s: %s\n", InFifo, strerror(errno));
		return (-1);
	}
	ptr = getenv("USE_BTREE");
	use_btree = ((ptr && *ptr == '1') ? 1 : 0);
	max_btree_count = ((ptr = getenv("MAX_BTREE_COUNT")) && *ptr ? atoi(ptr) : -1);
	search_func = (void *) tsearch; /*- this adds a record if not found */
	if ((fifoName = strrchr(InFifo, '/')))
		fifoName++;
	else
		fifoName = InFifo;
	for (bytes = 0;getppid() != 1;) {
		if ((idx = read(rfd, (char *) &bytes, sizeof(int))) == -1) {
#ifdef ERESTART
			if (errno != EINTR && errno != ERESTART)
#else
			if (errno != EINTR)
#endif
				fprintf(stderr, "InLookup: read: %s\n", strerror(errno));
			continue;
		} else
		if (!idx) {
			close(rfd);
			if ((rfd = open(InFifo, O_RDWR, 0)) == -1) {
				fprintf(stderr, "InLookup: open: %s: %s\n", InFifo, strerror(errno));
				signal(SIGPIPE, pstat);
				return (-1);
			} else
				continue;
		} else
		if (bytes > pipe_size) {
			errno = EMSGSIZE;
			fprintf(stderr, "InLookup: bytes %d, pipe_size %d, %s\n", bytes, pipe_size, strerror(errno));
			continue;
		} else
		if ((idx = timeoutread(readTimeout, rfd, QueryBuf, bytes)) == -1) {
			fprintf(stderr, "InLookup: read-int: %s\n", strerror(errno));
			continue;
		} else
		if (!idx) {
			close(rfd);
			if ((rfd = open(InFifo, O_RDWR, 0)) == -1) {
				fprintf(stderr, "InLookup: open: %s: %s\n", InFifo, strerror(errno));
				signal(SIGPIPE, pstat);
				return (-1);
			} else
				continue;
		}
		if (verbose || _debug)
			prev_time = time(0);
#ifdef CLUSTERED_SITE
		if (relative)
			snprintf(host_path, MAX_BUFF, "%s/%s/host.cntrl", sysconfdir, controldir);
		else
			snprintf(host_path, MAX_BUFF, "%s/host.cntrl", controldir);
		if (access(host_path, F_OK))
			cntrl_host = 0;
		else {
			if (!(fp = fopen(host_path, "r")))
				cntrl_host = 0;
			else {
				if (!fgets(tmpbuf, MAX_BUFF - 2, fp)) {
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
		if (cntrl_host) {
			if (!isopen_cntrl && open_central_db(cntrl_host)) {
				fprintf(stderr, "InLookup: Unable to open central db\n");
				signal(SIGPIPE, pstat);
				return (-1);
			}
			if (mysql_ping(&mysql[0])) {
				fprintf(stderr, "mysql_ping: %s: Reconnecting to central db...\n", mysql_error(&mysql[0]));
				mysql_close(&mysql[0]);
				isopen_cntrl = 0;
				if (open_central_db(cntrl_host)) {
					fprintf(stderr, "InLookup: Unable to open central db\n");
					signal(SIGPIPE, pstat);
					return (-1);
				}
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
#ifdef ENABLE_DOMAIN_LIMITS
			case LIMIT_QUERY:
				limit_query_count++;
				break;
#endif
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
		if (verbose || _debug) {
			printf("%s->%s, Bytes %d, Query %d, User %s, RemoteIp %s\n", 
				fifoName, myFifo, bytes, *QueryBuf, email, *QueryBuf == 2 ? remoteip : "N/A");
			fflush(stdout);
		}
		if ((wfd = open(myFifo, O_RDWR, 0)) == -1) {
			fprintf(stderr, "InLookup: open-probably-timeout: %s: QueryType %s: %s\n", myFifo, query_type(*QueryBuf), strerror(errno));
			if (errno != ENOENT)
				unlink(myFifo);
			continue;
		} else
			unlink(myFifo); /*- make this invisible */
		switch(*QueryBuf)
		{
			case USER_QUERY:
				if (!use_btree || !(in = mk_in_entry(email)))
					status = UserInLookup(email);
				else
				if (!(retval = search_func (in, &in_root, in_compare_func))) {
					in_free_func(in);
					status = UserInLookup(email);
				} else {
					re = *(INENTRY **) retval;
					if (re != in) { /*- existing data */
						status = re->in_userStatus;
						if (status < 0) {
							status = UserInLookup(email);
							re->in_userStatus = status;
						} else
						if (verbose || _debug) {
							printf("%s->%s, cache hit\n", fifoName, myFifo);
							fflush(stdout);
						}
						in_free_func(in); /*- Prevents data leak: in was already present.  */
					} else {/*- New entry in was added.  */
						status = UserInLookup(email);
						in->in_userStatus = status;
						btree_count++;
						if (max_btree_count > 0 && btree_count >= max_btree_count)
							search_func = tfind;
					}
				}
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
				if (!use_btree || !(in = mk_in_entry(email)))
					ptr = findmdahost(email, 0);
				else
				if (!(retval = search_func (in, &in_root, in_compare_func))) {
					in_free_func(in);
					ptr = findmdahost(email, 0);
				} else {
					re = *(INENTRY **) retval;
					if (re != in) { /*- existing data */
						ptr = re->mdahost;
						if (!ptr) {
							if ((ptr = findmdahost(email, 0)))
								re->mdahost = strdup(ptr);
						} else
						if (verbose || _debug) {
							printf("%s->%s, cache hit\n", fifoName, myFifo);
							fflush(stdout);
						}
						in_free_func(in); /*- Prevents data leak: in was already present.  */
					} else {/*- New entry in was added.  */
						if ((ptr = findmdahost(email, 0)))
							in->mdahost = strdup(ptr);
						btree_count++;
						if (max_btree_count > 0 && btree_count >= max_btree_count)
							search_func = tfind;
					}
				}
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
				if (!use_btree || !(in = mk_in_entry(email)))
					ptr = AliasInLookup(email);
				else
				if (!(retval = search_func (in, &in_root, in_compare_func))) {
					in_free_func(in);
					ptr = AliasInLookup(email);
				} else {
					re = *(INENTRY **) retval;
					if (re != in) { /*- existing data */
						ptr = re->aliases;
						if (!ptr) {
							if ((ptr = AliasInLookup(email)))
								re->aliases = strdup(ptr);
						} else
						if (verbose || _debug) {
							printf("%s->%s, cache hit\n", fifoName, myFifo);
							fflush(stdout);
						}
						in_free_func(in); /*- Prevents data leak: in was already present.  */
					} else {/*- New entry in was added.  */
						if ((ptr = AliasInLookup(email)))
							in->aliases = strdup(ptr);
						btree_count++;
						if (max_btree_count > 0 && btree_count >= max_btree_count)
							search_func = tfind;
					}
				}
				if (ptr && *ptr) {
					if ((bytes = slen(ptr) + 1) > pipe_size)
						bytes = -1;
				} else {
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
				if (!(real_domain = strrchr(email, '@'))) {
					if (!(real_domain = getenv("DEFAULT_DOMAIN")))
						real_domain = DEFAULT_DOMAIN;
				} else
					real_domain++;
				if (!use_btree || !(in = mk_in_entry(email)))
					pw = PwdInLookup(email);
				else
				if (!(retval = search_func (in, &in_root, in_compare_func))) {
					in_free_func(in);
					pw = PwdInLookup(email);
				} else {
					re = *(INENTRY **) retval;
					if (re != in) { /*- existing data */
						if (!re->pwStat) { /*- incomplete existing data */
							if ((pw = PwdInLookup(email))) {
								re->in_pw.pw_uid = pw->pw_uid;
								re->in_pw.pw_gid = pw->pw_gid;
								re->in_pw.pw_name = strdup(pw->pw_name);
								re->domain = strdup(real_domain);
								re->in_pw.pw_passwd = strdup(pw->pw_passwd);
								re->in_pw.pw_gecos = strdup(pw->pw_gecos);
								re->in_pw.pw_dir = strdup(pw->pw_dir);
								re->in_pw.pw_shell = strdup(pw->pw_shell);
								re->pwStat = 1;
							}
						} else { /*- completed existing data */
							pw = &re->in_pw;
							real_domain = re->domain;
							if (verbose || _debug) {
								printf("%s->%s, cache hit\n", fifoName, myFifo);
								fflush(stdout);
							}
						}
						in_free_func(in); /*- Prevents data leak: in was already present.  */
					} else {/*- New entry in was added.  */
						if ((pw = PwdInLookup(email))) {
							in->in_pw.pw_uid = pw->pw_uid;
							in->in_pw.pw_gid = pw->pw_gid;
							in->in_pw.pw_name = strdup(pw->pw_name);
							in->domain = strdup(real_domain);
							in->in_pw.pw_passwd = strdup(pw->pw_passwd);
							in->in_pw.pw_gecos = strdup(pw->pw_gecos);
							in->in_pw.pw_dir = strdup(pw->pw_dir);
							in->in_pw.pw_shell = strdup(pw->pw_shell);
							in->pwStat = 1;
						} else
							in->pwStat = 0;
						btree_count++;
						if (max_btree_count > 0 && btree_count >= max_btree_count)
							search_func = tfind;
					}
				}
				if (pw) {
					snprintf(pwbuf, sizeof(pwbuf), "PWSTRUCT=%s@%s:%s:%d:%d:%s:%s:%s:%d", 
						pw->pw_name,
						real_domain,
						pw->pw_passwd,
						pw->pw_uid,
						pw->pw_gid,
						pw->pw_gecos,
						pw->pw_dir,
						pw->pw_shell, is_inactive);
					if ((bytes = (slen(pwbuf) + 1)) > pipe_size)
						bytes = -1;
				} else
				if (userNotFound) {
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
#ifdef ENABLE_DOMAIN_LIMITS
			case LIMIT_QUERY:
				if ((status = VlimitInLookup(email, &limits)) == -1)
					bytes = -1;
				else
				if (status) /*- user not found */
					bytes = 0;
				else {
					bytes = sizeof(struct vlimits);
					if (bytes > pipe_size)
						bytes = -1;
				}
				if (timeoutwrite(writeTimeout, wfd, (char *) &bytes, sizeof(int)) == -1)
					fprintf(stderr, "InLookup: write-VlimitInLookup: %s\n", strerror(errno));
				else
				if (bytes > 0 && timeoutwrite(writeTimeout, wfd, (char *) &limits, bytes) == -1)
					fprintf(stderr, "InLookup: write-VlimitInLookup: %s\n", strerror(errno));
				close(wfd);
				break;
#endif
			case DOMAIN_QUERY:
				if (!(real_domain = vget_real_domain(email)))
					real_domain = email;
				bytes = slen(real_domain) + 1;
				if (bytes > pipe_size)
					bytes = -1;
				if (timeoutwrite(writeTimeout, wfd, (char *) &bytes, sizeof(int)) == -1)
					fprintf(stderr, "InLookup: write-get_real_domain: %s\n", strerror(errno));
				else
				if (bytes > 0 && timeoutwrite(writeTimeout, wfd, real_domain, bytes) == -1)
					fprintf(stderr, "InLookup: write-get_real_domain: %s\n", strerror(errno));
				close(wfd);
				break;
		} /*- switch(*QueryBuf) */
		if (verbose || _debug) {
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
#ifdef ENABLE_DOMAIN_LIMITS
		case LIMIT_QUERY:
			return("'Domain Limits Query'");
#endif
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
	char           *infifo_dir, *infifo;

	getEnvConfigStr(&infifo, "INFIFO", INFIFO);
	if (*infifo == '/' || *infifo == '.')
		snprintf(inFifo, MAX_BUFF, "%s", infifo);
	else {
		getEnvConfigStr(&infifo_dir, "FIFODIR", INDIMAILDIR"/inquery");
		if (*infifo_dir == '/')
			snprintf(inFifo, MAX_BUFF, "%s/%s", infifo_dir, infifo);
		else
			snprintf(inFifo, MAX_BUFF, INDIMAILDIR"%s/%s", infifo_dir, infifo);
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
	printf("User Query %d, Relay Query %d, Password Query %d:%d, Alias Query %d, Host Query %d, Domain Query %d Cached Nodes %d\n", 
		user_query_count, relay_query_count, pwd_query_count, limit_query_count, alias_query_count, 
		host_query_count, dom_query_count, btree_count);
	total_count = user_query_count + relay_query_count + pwd_query_count + limit_query_count + alias_query_count + 
		host_query_count + dom_query_count;
#else
	printf("User Query %d, Relay Query %d, Password Query %d:%d, Alias Query %d, Domain Query %d Cached Nodes %d\n", 
		user_query_count, relay_query_count, pwd_query_count, limit_query_count, alias_query_count, dom_query_count, btree_count);
	total_count = user_query_count + relay_query_count + pwd_query_count + limit_query_count + alias_query_count + dom_query_count;
#endif
	printf("Start Time: %s", ctime(&start_time));
	printf("End   Time: %s", ctime(&cur_time));
	printf("Queries %ld, Total Time %ld secs, Query/Sec = %.02f\n", total_count, (cur_time - start_time), 
		(float) ((float) total_count/(cur_time - start_time)));
	twalk(in_root, walk_entry);
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

	signal(SIGHUP, (void(*)()) SIG_IGN);
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
	tdestroy(in_root, in_free_func);
	in_root = 0;
	btree_count = 0;
	if (pwdCache) {
		cache_active_pwd(0);
		printf("cached %d records\n", btree_count);
	}
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
	if (verbose || _debug) {
		printf("%d %s ARGH!! Committing suicide on SIGTERM\n", (int) getpid(), fifo_name);
		printf("%d INFIFO %s, Got SIGTERM\n", (int) getpid(), fifo_name);
	}
	cur_time = time(0);
#ifdef CLUSTERED_SITE
	printf("User Query %d, Relay Query %d, Password Query %d:%d, Alias Query %d, Host Query %d, Domain Query %d Cached Nodes %d\n", 
		user_query_count, relay_query_count, pwd_query_count, limit_query_count, alias_query_count, 
		host_query_count, dom_query_count, btree_count);
	total_count = user_query_count + relay_query_count + pwd_query_count + limit_query_count + alias_query_count + 
		host_query_count + dom_query_count;
#else
	printf("User Query %d, Relay Query %d, Password Query %d:%d, Alias Query %d, Domain Query %d Cached Nodes %d\n", 
		user_query_count, relay_query_count, pwd_query_count, limit_query_count, alias_query_count, dom_query_count, btree_count);
	total_count = user_query_count + relay_query_count + pwd_query_count + limit_query_count + alias_query_count + dom_query_count;
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
	if (sig == SIGTERM) {
		sig_block(sig);
		if (verbose || _debug) 
			printf("%d %s ARGH!! Committing suicide on SIGTERM\n", (int) getpid(), fifo_name);
	}
	if (sig != SIGTERM || verbose || _debug)
		printf("%d INFIFO %s, Got %s\n", (int) getpid(), fifo_name, sys_siglist[sig]);
	switch (sig)
	{
		case SIGUSR1:
			printf("%d %s Dumping Stats\n", (int) getpid(), fifo_name);
			/*- flow through */
		case SIGTERM:
			cur_time = time(0);
#ifdef CLUSTERED_SITE
			printf("User Query %d, Relay Query %d, Password Query %d:%d, Alias Query %d, Host Query %d, Domain Query %d Cached Nodes %d\n", 
				user_query_count, relay_query_count, pwd_query_count, limit_query_count, alias_query_count, 
				host_query_count, dom_query_count, btree_count);
			total_count = user_query_count + relay_query_count + pwd_query_count + limit_query_count + alias_query_count + 
				host_query_count + dom_query_count;
#else
			printf("User Query %d, Relay Query %d, Password Query %d:%d, Alias Query %d, Domain Query %d Cached Nodes %d\n", 
				user_query_count, relay_query_count, pwd_query_count, limit_query_count, alias_query_count,
				dom_query_count, btree_count);
			total_count = user_query_count + relay_query_count + pwd_query_count + limit_query_count +
				alias_query_count + dom_query_count;
#endif
			printf("Start Time: %s", ctime(&start_time));
			printf("End   Time: %s", ctime(&cur_time));
			printf("Queries %ld, Total Time %ld secs, Query/Sec = %.02f\n", total_count, (cur_time - start_time), 
				(float) ((float) total_count/(cur_time - start_time)));
			if (sig == SIGUSR1)
				twalk(in_root, walk_entry);
			if (sig == SIGTERM) {
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
			close_db();
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
			tdestroy(in_root, in_free_func);
			in_root = 0;
			btree_count = 0;
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
getTimeoutValues(int *readTimeout, int *writeTimeout, char *sysconfdir, char *controldir)
{
	char            TmpBuf[MAX_BUFF];
	FILE           *fp;

	if (*controldir == '/')
		snprintf(TmpBuf, MAX_BUFF, "%s/timeoutread", controldir);
	else
		snprintf(TmpBuf, MAX_BUFF, "%s/%s/timeoutread", sysconfdir, controldir);
	if ((fp = fopen(TmpBuf, "r"))) {
		if (fgets(TmpBuf, MAX_BUFF - 2, fp)) {
			if (sscanf(TmpBuf, "%d", readTimeout) != 1)
				*readTimeout = 4;
		}
		fclose(fp);
	} else
		*readTimeout = 4;
	if (*controldir == '/')
		snprintf(TmpBuf, MAX_BUFF, "%s/timeoutwrite", controldir);
	else
		snprintf(TmpBuf, MAX_BUFF, "%s/%s/timeoutwrite", sysconfdir, controldir);
	if ((fp = fopen(TmpBuf, "r"))) {
		if (fgets(TmpBuf, MAX_BUFF - 2, fp)) {
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
	printf("%s\n", sccsid_error_stackh);
}
