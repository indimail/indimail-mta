/*-
 * Copyright (C) 2004 Ben Goodwin
 * This file is part of the nsvs package
 *
 * The nsvs package is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * The nsvs package is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with the nsvs package; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/*- A SIGNIFICANT PORTION OF THIS CODE IS BASED ON GNU NSCD */

/*
 * $Id: nsvsd.c,v 1.13 2006/09/16 04:08:13 cinergi Exp $ 
 */
#include "common.h"
#include <errno.h>
#include <pwd.h>
#include <grp.h>
#include <pthread.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <signal.h>
#include <stdarg.h>
#include <mysql.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits.h>
#include "nssd.h"

#define XMYSQL_FREE_RESULT(n)                                                \
do {                                                                         \
  if (n)                                                                     \
    {                                                                        \
      mysql_free_result (n);                                                 \
      (n) = '\0';                                                            \
    }                                                                        \
  } while (0)

int             debug_level = 0;
static int      sock;
static const char *conffile = _PATH_NSVSD_CONF;

/*- Each thread gets their own MySQL connection and result set */
typedef struct {
	int             connected;
	MYSQL           mysql;
	MYSQL_RES      *result;
} connections_t;
static connections_t connections[MAX_THREADS];
pthread_t       th[MAX_THREADS];

/*- This is the standard reply in case we haven't found the dataset.  */
static const response_header_t notfound = {
  version:INTERFACE_VERSION,
  count:0,
  response_size:0,
};

/*- Define query to run and how many fields to expect for each query type */
static const query_conf_t query_conf[] = {
	[GETPWBYNAME] = {NUM_PW_ELEMENTS, conf.getpwnam},
	[GETPWBYUID] = {NUM_PW_ELEMENTS, conf.getpwuid},
	[GETPW] = {NUM_PW_ELEMENTS, conf.getpwent},
	[GETGRBYNAME] = {NUM_GR_ELEMENTS - 1, conf.getgrnam},
	[GETGRBYGID] = {NUM_GR_ELEMENTS - 1, conf.getgrgid},
	[GETGR] = {NUM_GR_ELEMENTS - 1, conf.getgrent},
	[GETGRMEMSBYGID] = {1, conf.memsbygid},
	[GETGRGIDSBYMEM] = {1, conf.gidsbymem},
	[GETSPBYNAME] = {NUM_SP_ELEMENTS, conf.getspnam},
	[GETSP] = {NUM_SP_ELEMENTS, conf.getspent},
};

/*- Open up all MySQL connections */
static int
init_connection(int th_num)
{
	const unsigned int default_timeout = DEFAULT_TIMEOUT;
	const my_bool   reconnect = 1;

	if (connections[th_num].connected)
		return 1;
	nssd_log(LOG_DEBUG, "%s: Initializing MySQL connection #%d ...", __FUNCTION__, th_num);
	if (!mysql_init(&connections[th_num].mysql)) {
		nssd_log(LOG_ALERT, "%s: mysql_init: %s", __FUNCTION__, mysql_error(&connections[th_num].mysql));
		return 0;
	}
	if (mysql_options(&connections[th_num].mysql, MYSQL_OPT_CONNECT_TIMEOUT, (const char *) &default_timeout)) {
		nssd_log(LOG_ALERT, "%s: mysql_options: %s", __FUNCTION__, mysql_error(&connections[th_num].mysql));
		return 0;
	}
	if (mysql_options(&connections[th_num].mysql, MYSQL_READ_DEFAULT_GROUP, "nssd")) {
		nssd_log(LOG_ALERT, "%s: mysql_options: %s", __FUNCTION__, mysql_error(&connections[th_num].mysql));
		return 0;
	}
#if MYSQL_VERSION_ID >= 50013
	if (mysql_options(&connections[th_num].mysql, MYSQL_OPT_RECONNECT, (const char *) &reconnect)) {
		nssd_log(LOG_ALERT, "%s: mysql_options: %s", __FUNCTION__, mysql_error(&connections[th_num].mysql));
		return 0;
	}
#else
	connections[th_num].mysql.reconnect = (my_bool) 1;
#endif

	if (!mysql_real_connect
		(&connections[th_num].mysql, conf.host, conf.username[0] ? conf.username : NULL, conf.password[0] ? conf.password : NULL,
		 conf.database, conf.port, conf.socket[0] ? conf.socket : NULL, 0)) {
		nssd_log(LOG_ALERT, "%s: mysql_real_connect: %s", __FUNCTION__, mysql_error(&connections[th_num].mysql));
		return 0;
	}
	connections[th_num].result = NULL;
	connections[th_num].connected = 1;
	return 1;
}

/*- Run the appropriate query (based on TYPE and the struct above) */
static int
run_query(int32_t type, char *key, int th_num)
{
	char            query[MAX_QUERY_SIZE], ekey[MAX_USERNAME_SIZE * 2 + 1];
	char           *p;

	if (!init_connection(th_num))
		return 0;
	/*- If we have a key, then use snprintf () */
	if (key && *key) {
		mysql_real_escape_string(&connections[th_num].mysql, ekey, key, strlen(key));
		if ((p = strchr(ekey, '@'))) {
			*p = 0;
			p++;
			snprintf(query, sizeof (query), query_conf[type].query, ekey, p);
			*(p - 1) = '@';
		} else
		{
			p = (p = getenv("DEFAULT_DOMAIN")) ? p : DEFAULT_DOMAIN;
			snprintf(query, sizeof (query), query_conf[type].query, ekey, p);
		}
	} else /*- Otherwise just strncpy () */
		strncpy(query, query_conf[type].query, sizeof (query));
	nssd_log(LOG_DEBUG, "%s: query: %s", __FUNCTION__, query);
	/*- Run the query */
	if (mysql_query(&connections[th_num].mysql, query)) {
		nssd_log(LOG_ALERT, "%s: mysql_query: %s", __FUNCTION__, mysql_error(&connections[th_num].mysql));
		return 0;
	}
	return 1;
}

static int
get_response_size(int th_num)
{
	int32_t         response_size = 0;
	MYSQL_ROW       row;
	unsigned long  *lengths = NULL;
	int             i;
	unsigned int    num_fields;

	num_fields = mysql_num_fields(connections[th_num].result);
	while ((row = mysql_fetch_row(connections[th_num].result)) != NULL) {
		/*- This row size = size of response_data struct + size of all fields */
		lengths = mysql_fetch_lengths(connections[th_num].result);
		response_size += RDS;
		for (i = 0; i < num_fields; i++)
			response_size += lengths[i] + 1;
	}
	if (mysql_errno(&connections[th_num].mysql))
		nssd_log(LOG_ALERT, "%s: mysql_fetch_row: %s", __FUNCTION__, mysql_error(&connections[th_num].mysql));
	mysql_data_seek(connections[th_num].result, 0);
	return response_size;
}

static struct response_data *
build_response(int th_num, int32_t type, response_header_t * response_header, struct response_data *data)
{
	struct response_data *dp;
	unsigned int    num_fields;
	MYSQL_ROW       row;
	char           *sp;
	unsigned long  *lengths;
	int             i;
	int32_t         response_size;

	/*- Make sure we have all the fields we need */
	num_fields = mysql_num_fields(connections[th_num].result);
	if (num_fields != query_conf[type].num_fields) {
		nssd_log(LOG_ALERT, "%s: Expecting %d fields, got %d", __FUNCTION__, query_conf[type].num_fields, num_fields);
		return data;
	}
	/*
	 * The data we send looks like:
	 *   response_header (known size)
	 *   response_data (variable size)
	 *   ...
	 *   response_data (variable size)
	 */
	/*- Find out how much space we need to realloc */
	response_size = get_response_size(th_num);
	if (response_size == 0)
		return data;
	response_header->response_size += response_size;
	/*- Set up response header and realloc response data */
	if (data == NULL) {
		data = (struct response_data *) malloc(response_header->response_size);
		if (data == NULL)
			return data;
		memset(data, 0, response_header->response_size);
		dp = data;
	} else {
		data = (struct response_data *) realloc(data, response_header->response_size);
		dp = (struct response_data *) &(((char *) data)[response_header->response_size - response_size]);
	}
	while ((row = mysql_fetch_row(connections[th_num].result)) != NULL) {
		/*- Load string data starting at dp->strdata */
		sp = dp->strdata;
		lengths = mysql_fetch_lengths(connections[th_num].result);
		/*- This row size = size of response_data struct + size of all fields */
		dp->header.record_size = RDS;
		for (i = 0; i < num_fields; i++) {
			/*- Add in this field size */
			dp->header.record_size += lengths[i] + 1;
			/*- Copy in this field */
			memcpy(sp, row[i], lengths[i] + 1);
			/*- Move string pointer to next field */
			sp += lengths[i] + 1;
			/*- Record where copied-in data starts */
			if (i == 0)
				dp->header.offsets[i] = 0;
			else
				dp->header.offsets[i] = dp->header.offsets[i - 1] + lengths[i - 1] + 1;
		}
		/*- Move data pointer to next record */
		dp = (struct response_data *) &(((char *) dp)[dp->header.record_size]);
	}
	response_header->count += mysql_num_rows(connections[th_num].result);
	return data;
}

static struct response_data *
process_result(int th_num, int32_t type, response_header_t * response_header, struct response_data *data)
{
	connections[th_num].result = mysql_store_result(&connections[th_num].mysql);
	if (connections[th_num].result == NULL)
		nssd_log(LOG_ALERT, "%s: mysql_store_result: %s", __FUNCTION__, mysql_error(&connections[th_num].mysql));
	else
		data = build_response(th_num, type, response_header, data);
	return data;
}

static int
send_response(int fd, int th_num, response_header_t response_header, struct response_data *data)
{
	ssize_t         bytes;

	nssd_log(LOG_DEBUG, "%s: sending %d-byte response", __FUNCTION__, sizeof (response_header) + response_header.response_size);
	/*- Write the header ...  */
	bytes = write_wt(fd, &response_header, sizeof (response_header), WRITE_TIMEOUT);
	/*- ... and the data */
	bytes += write_wt(fd, &data->header, response_header.response_size, WRITE_TIMEOUT);
	if (bytes != sizeof (response_header) + response_header.response_size)
		return 0;
	return 1;
}

/*
 * This is the main loop.  It is replicated in different threads but the
 * `poll' call makes sure only one thread handles an incoming connection. 
 */

static void    *
main_loop(void *p)
{
	struct pollfd   conn;
	int             th_num = (int) p;
#ifdef HAVE_STRUCT_UCRED
	struct ucred    caller;
	socklen_t       optlen = sizeof (caller);
#endif

	/*- "thread" 0 handles shutting down; block signals and set async cancel */
	if (th_num > 0) {
		sigset_t        newset;
		sigemptyset(&newset);
		sigaddset(&newset, SIGINT);
		sigaddset(&newset, SIGQUIT);
		sigaddset(&newset, SIGTERM);
		pthread_sigmask(SIG_SETMASK, &newset, NULL);
		pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
	}
	conn.fd = sock;
	conn.events = POLLIN;
	while (1) {
		int             rt;
		rt = poll(&conn, 1, conf.timeout);
		if (rt == 0) {
			nssd_log(LOG_DEBUG, "%s: Thread %d waiting ...", __FUNCTION__, th_num);
			continue;
		}
		if (conn.revents & (POLLIN | POLLPRI | POLLOUT | POLLERR | POLLHUP | POLLNVAL)) {
			int             fd = accept(conn.fd, NULL, NULL);
			request_header_t req;
			char            buf[256];

			if (fd < 0) {
				strerror_r(errno, buf, sizeof(buf));
				nssd_log(LOG_ALERT, "%d: %s: accept: %s", th_num, __FUNCTION__, buf);
				continue;
			}
			/*- Let's not block and hang a thread on a read/write!  */
			if (fcntl(fd, F_SETFL, O_NONBLOCK) == -1) {
				strerror_r(errno, buf, sizeof(buf));
				nssd_log(LOG_ALERT, "%d: %s: fcntl: %s", th_num, __FUNCTION__, buf);
				close(fd);
				continue;
			}
			/*
			 * FIXME: If any of these checks result in closing the connection,
			 * the client will get a sigpipe, which will cause problems
			 */
#ifdef HAVE_STRUCT_UCRED
			/*- Detect calling uid */
			if (getsockopt(fd, SOL_SOCKET, SO_PEERCRED, &caller, &optlen) < 0) {
				strerror_r(errno, buf, sizeof(buf));
				nssd_log(LOG_ALERT, "%d: %s: getsockopt: %s", th_num, __FUNCTION__, buf);
				close(fd);
				continue;
			}
#endif
			/*- Read the request header */
			if (read_wt(fd, &req, sizeof (req), READ_TIMEOUT) != sizeof (req)) {
				nssd_log(LOG_DEBUG, "%d: %s: Short read fetching request header", th_num, __FUNCTION__);
				write_wt(fd, &notfound, sizeof (notfound), WRITE_TIMEOUT);
				close(fd);
				continue;
			}
			/*- Make sure we have matching interface versions */
			if (req.version != INTERFACE_VERSION) {
				nssd_log(LOG_ALERT, "%d: %s: Interface version %d !=  %d", th_num, __FUNCTION__, req.version, INTERFACE_VERSION);
				write_wt(fd, &notfound, sizeof (notfound), WRITE_TIMEOUT);
				close(fd);
				continue;
			}
			/*- req.type is used as an array index!  */
			if (req.type >= NUM_REQUEST_TYPES || req.type < 0) {
				nssd_log(LOG_ALERT, "%d: %s: Bad request type: %d", th_num, __FUNCTION__, req.type);
				write_wt(fd, &notfound, sizeof (notfound), WRITE_TIMEOUT);
				close(fd);
				continue;
			}
			/*- Don't let caller cause a DoS with a large key */
			if (req.key_len < 1 || req.key_len > 1024) {
				nssd_log(LOG_ALERT, "%d: %s: Invalid lookup key len: %d", th_num, __FUNCTION__, req.key_len);
				write_wt(fd, &notfound, sizeof (notfound), WRITE_TIMEOUT);
				close(fd);
				continue;
			}
			/*- END FIXME */
			else {
				char            keybuf[req.key_len];
				/*- Get the key */
				if (read_wt(fd, keybuf, req.key_len, READ_TIMEOUT) != req.key_len) {
					nssd_log(LOG_DEBUG, "%d: %s: Short read fetching key", th_num, __FUNCTION__);
					write_wt(fd, &notfound, sizeof (notfound), WRITE_TIMEOUT);
					close(fd);
					continue;
				}
#ifdef HAVE_STRUCT_UCRED
				/*
				 * Don't allow shadow queries to non-root users
				 * It is CRITICAL that this code occurs AFTER reading the key
				 * to avoid clientside sigpipe issues
				 */
				if ((req.type == GETSPBYNAME || req.type == GETSP) && caller.uid != 0) {
					nssd_log(LOG_DEBUG, "%d: %s: pid %d unauthorized for type %d", th_num, __FUNCTION__, caller.pid, req.type);
					write_wt(fd, &notfound, sizeof (notfound), WRITE_TIMEOUT);
					close(fd);
					continue;
				}
#endif
				/*- Checks passed; run query, then build & send response */
				if (run_query(req.type, keybuf, th_num)) {
					response_header_t response_header = { INTERFACE_VERSION, 0, 0 };
					struct response_data *data = NULL;

					do {
						data = process_result(th_num, req.type, &response_header, data);
						XMYSQL_FREE_RESULT(connections[th_num].result);
#if MYSQL_VERSION_ID >= 40107
					} while (!mysql_next_result(&connections[th_num].mysql));
#else
					} while (0);
#endif
					if (data == NULL)
						write_wt(fd, &notfound, sizeof (notfound), WRITE_TIMEOUT);
					else
						send_response(fd, th_num, response_header, data);
					XFREE(data);
				} else
					write_wt(fd, &notfound, sizeof (notfound), WRITE_TIMEOUT);
				close(fd);
			}
		}
	}
	return 0;
}

/*- Prevent the "dead store removal" problem present with stock memset() */
static void
_safe_memset(void *s, int c, size_t n)
{
	volatile char  *p = s;

	if (p) {
		while (n--)
			*p++ = c;
	}
}

/*- Kill off threads, close connections, clear memory, close socket, ...  */
static void
termination_handler(int signum)
{
	int             i;
	void           *result;

	nssd_log(LOG_DEBUG, "%s: Cancelling threads ...", __FUNCTION__);
	/*- "thread" 0 isn't really a thread, don't try to cancel it */
	for (i = 1; i < conf.threads; ++i) {
		char            buf[256];
		if (pthread_cancel(th[i]) != 0)
		{
			strerror_r(errno, buf, sizeof(buf));
			nssd_log(LOG_ERR, "%s: pthread_cancel: %s", __FUNCTION__, buf);
		}
		if (pthread_join(th[i], &result) != 0)
		{
			strerror_r(errno, buf, sizeof(buf));
			nssd_log(LOG_ERR, "%s: pthread_join: %s", __FUNCTION__, buf);
		}
		if (result == PTHREAD_CANCELED)
			nssd_log(LOG_DEBUG, "%s: Thread %d canceled.", __FUNCTION__, i);
		else
			nssd_log(LOG_ERR, "%s: Thread %d NOT canceled.", __FUNCTION__, i);
	}
	nssd_log(LOG_DEBUG, "%s: cleaning up ...", __FUNCTION__);
	close(sock);
	for (i = 0; i < conf.threads; ++i) {
		XMYSQL_FREE_RESULT(connections[i].result);
		if (connections[i].connected)
			mysql_close(&connections[i].mysql);
	}
	_safe_memset(connections, 0, sizeof (connections));
	_safe_memset(&conf, 0, sizeof (conf));
	closelog();
	exit(EXIT_SUCCESS);
}

static void
setup_socket(void)
{
	char           *socket_file;
	struct sockaddr_un sock_addr;
	char            buf[256];

	getEnvConfigStr(&socket_file, "NSSD_SOCKET", _PATH_NSVSD_SOCKET);
	nssd_log(LOG_DEBUG, "%s: Initializing socket ...", __FUNCTION__);
	if ((sock = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
		strerror_r(errno, buf, sizeof(buf));
		nssd_log(LOG_ALERT, "%s: socket: %s", __FUNCTION__, buf);
		exit(EXIT_FAILURE);
	}
	sock_addr.sun_family = AF_UNIX;
	strncpy(sock_addr.sun_path, socket_file, sizeof(sock_addr.sun_path));
	if (!access(socket_file, F_OK) && unlink(socket_file))
	{
		strerror_r(errno, buf, sizeof(buf));
		nssd_log(LOG_ALERT, "%s: unlink: %s", __FUNCTION__, buf);
		exit(EXIT_FAILURE);
	}
	if (bind(sock, (struct sockaddr *) &sock_addr, sizeof (sock_addr)) < 0) {
		strerror_r(errno, buf, sizeof(buf));
		nssd_log(LOG_ALERT, "%s: bind: %s", __FUNCTION__, buf);
		exit(EXIT_FAILURE);
	}
	if (chmod(socket_file, 0666)) {
		strerror_r(errno, buf, sizeof(buf));
		nssd_log(LOG_ALERT, "%s: chmod: %s", __FUNCTION__, buf);
		exit(EXIT_FAILURE);
	}
	if (listen(sock, SOMAXCONN) < 0) {
		strerror_r(errno, buf, sizeof(buf));
		nssd_log(LOG_ALERT, "%s: listen: %s", __FUNCTION__, buf);
		exit(EXIT_FAILURE);
	}
}

static void
spawn_threads(void)
{
	int             i;

	nssd_log(LOG_INFO, "%s: nssd starting up (%d threads) ...", __FUNCTION__, conf.threads);
	/*- Create conf.threads - 1 threads */
	for (i = 1; i < conf.threads; ++i)
		pthread_create(&th[i], NULL, main_loop, (void *) i);
	/*- And make this copy "thread" 0 */
	main_loop((void *) 0);
}

/*
 * Returns 1 if the process in pid file FILE is running, 0 if not.  
 */
static int
check_pid(const char *file)
{
	FILE           *fp;
	int             n;
	pid_t           pid;

	fp = fopen(file, "r");
	if (fp) {
		n = fscanf(fp, "%d", &pid);
		fclose(fp);
		if (n != 1 || kill(pid, 0) == 0)
			return 1;
		return(0);
	} else
	if (errno == 2)
		return 0;
	fprintf(stderr, "nssd: %s\n", strerror(errno));
	return (1);
}

/*
 * Write the current process id to the file FILE.
 * Returns 0 if successful, -1 if not.  
 */
static int
write_pid(const char *file)
{
	FILE           *fp;

	if (!(fp = fopen(file, "w")))
	{
		fprintf(stderr, "write_pid: %s: %s\n", file, strerror(errno));
		return -1;
	}
	fprintf(fp, "%d\n", getpid());
	if (fflush(fp) || ferror(fp))
	{
		fprintf(stderr, "fflush: %s: %s\n", file, strerror(errno));
		return -1;
	}
	fclose(fp);
	return 0;
}

static void
nssd_usage(void)
{
	printf("Usage: nssd [-f config] [ -d level] | [-V]\n");
	printf("Name Service via Sockets Daemon.\n");
  	printf("-f [config]   Read configuration data from CONFIG\n");
  	printf("-d [level]    Debug Level (can be one of below).\n");
	printf("              emerg   %d\n", LOG_EMERG);
	printf("              alert   %d\n", LOG_ALERT);
	printf("              crit    %d\n", LOG_CRIT);
	printf("              err     %d\n", LOG_ERR);
	printf("              warning %d\n", LOG_WARNING);
	printf("              notice  %d\n", LOG_NOTICE);
	printf("              info    %d\n", LOG_INFO);
	printf("              debug   %d\n", LOG_DEBUG);
  	printf("-V            Print program version\n");
}

static int
get_options(int argc, char **argv)
{
	int             c;

	while (1) {
		c = getopt(argc, argv, "Vd:f:");
		if (c == -1)
			break;
		switch (c) {
		case 'd':
			debug_level = str2priority(optarg);
			break;
		case 'V':
			printf("nssd Version %s\n", VERSION);
			exit(EXIT_SUCCESS);
			break;
		case 'f':
			conffile = optarg;
			break;
		default:
			return 0;
		}
	}
	return 1;
}

int
main(int argc, char **argv)
{
	if (!get_options(argc, argv)) {
		nssd_usage();
		exit(EXIT_FAILURE);
	}
	if (!load_config(conffile)) {
		printf("nssd: failed to load config\n");
		exit(EXIT_FAILURE);
	}
	if (check_pid(conf.pidfile)) {
		printf("nssd: already running\n");
		exit(EXIT_FAILURE);
	}
	if (write_pid(conf.pidfile))
		exit(EXIT_FAILURE);
	signal(SIGINT, termination_handler);
	signal(SIGQUIT, termination_handler);
	signal(SIGTERM, termination_handler);
	signal(SIGHUP, SIG_IGN);
	signal(SIGPIPE, SIG_IGN);
	memset(connections, 0, sizeof (connections));
	setup_socket();
	spawn_threads();
	return 0;
}
