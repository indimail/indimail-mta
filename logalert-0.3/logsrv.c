/*
 * $Log: logsrv.c,v $
 * Revision 1.6  2013-02-21 22:46:05+05:30  Cprogrammer
 * use 0 as IP address for localhost
 *
 * Revision 1.5  2013-02-11 23:03:10+05:30  Cprogrammer
 * added bytes read to statusfile
 *
 * Revision 1.4  2012-09-19 11:07:08+05:30  Cprogrammer
 * removed unused variable
 *
 * Revision 1.3  2012-09-18 17:08:47+05:30  Cprogrammer
 * removed syslog
 *
 * Revision 1.2  2012-09-18 14:55:24+05:30  Cprogrammer
 * removed stdio.h
 *
 * Revision 1.1  2012-09-18 13:23:48+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <getopt.h>
#include "common.h"
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#include <rpc/rpc.h>
#include <rpc/types.h>
#include <signal.h>
#include <errno.h>

/*-
program RPCLOG
{
	version CLOGVERS
	{
		int	SEND_MESSAGE( string ) = 1;
	} = 1;
} = 0x20000001;
-*/


#define RPCLOG ((u_long)0x20000001)
#define CLOGVERS ((u_long)1)
#define SEND_MESSAGE ((u_long)1)

#define MAXBUF    8192
#define PIDFILE   "/tmp/logsrv.pid"
#define PORT      "logsrv"
#define STATUSDIR PREFIX"/tmp/"

#ifndef	lint
static char     sccsid[] = "$Id: logsrv.c,v 1.6 2013-02-21 22:46:05+05:30 Cprogrammer Exp mbhangui $";
#endif

char           *loghost;
int             sighupflag, logsrvpid, rpcflag;
char            statusfile[MAXBUF];
CLIENT         *cl;

#ifdef __STDC__
int             main(int, char **);
int             server(int, int);
int             log_msg(char **);
int            *send_message_1(char **, CLIENT *);
#else
int             main();
int             server();
int             log_msg();
int            *send_message_1();
#endif
void            SigTerm();

char           *progname, *statusdir;

int
get_options(int argc, char **argv, int *foreground, int *silent)
{
	int             c, errflag = 0;

	while (!errflag && (c = getopt(argc, argv, "fsr:")) != -1) {
		switch (c)
		{
		case 's':
			*silent = 1;
			break;
		case 'f':
			*foreground = 1;
			break;
		case 'r':
			rpcflag = 1;
			loghost = optarg;
			break;
		default:
			errflag = 1;
			break;
		}
	}
	if (errflag)
		return (1);
	return (0);
}

void
usage(char *pname)
{
	filewrt(2, "Usage: %s [-s] [-f] [-r rpchost]\n", pname);
	return;
}

int
main(int argc, char **argv)
{
	FILE           *pidfp, *fp;
	char           *ptr;
	int             pid, bindfd, sfd, foreground = 0, silent = 0;
	struct sockaddr_in remotaddr;	/* for peer socket address */
	int             inaddrlen = sizeof(struct sockaddr_in);

	if (progname = strrchr(argv[0], '/'))
		progname++;
	else
		progname = argv[0];
	if (get_options(argc, argv, &foreground, &silent)) {
		usage(progname);
		return (1);
	}
	if (!(statusdir = getenv("STATUSDIR")))
		statusdir = STATUSDIR;
	if (!foreground) {
		if (pidfp = fopen(PIDFILE, "r")) {
			(void) fscanf(pidfp, "%d", &logsrvpid);
			(void) fclose(pidfp);
			if (logsrvpid && !kill(logsrvpid, 0)) {
				(void) filewrt(2, "%s is already running\n", progname);
				return (0);
			} else
			if (logsrvpid && errno == EACCES) {
				(void) filewrt(2, "%s is already running\n", progname);
				return (1);
			}
		}
		switch (logsrvpid = fork())
		{
		case -1:
			filewrt(2, "fork: %s\n", strerror(errno));
			return (1);
		case 0:
			(void) setsid();
			(void) close(0);
			(void) close(1);
			(void) close(2);
			logsrvpid = getpid();
			break;
		default:
			if (pidfp = fopen(PIDFILE, "w")) {
				(void) fprintf(pidfp, "%d", logsrvpid);
				(void) fclose(pidfp);
				return(0);
			} else {
				filewrt(2, "%s: %s\n", PIDFILE, strerror(errno));
				_exit(1);
			}
		}
	} else
		logsrvpid = getpid();
	if (!(ptr = getenv("PORT")))
		ptr = PORT;
	if ((bindfd = tcpbind("0", ptr, 5)) == -1) {
		(void) filewrt(2, "tcpsockbind failed for port %s\n", ptr);
		_exit (1);
	}
	signal(SIGTERM, SigTerm);
	for (;;) {
		if ((sfd = accept(bindfd, (struct sockaddr *) & remotaddr, &inaddrlen)) == -1) {
			/* Got signal. */
			switch (errno)
			{
			case EINTR:
				continue;
				/*
				 * have to add more cases here
				 */
			default:
				(void) filewrt(2, "accept: %s\n", strerror(errno));
				if (!foreground)
					(void) unlink(PIDFILE);
				_exit(1);
			}
		}
		switch (pid = fork())
		{
		case -1:
			(void) filewrt(2, "fork: %s\n", strerror(errno));
			(void) close(sfd);
		case 0:
			(void) close(bindfd);
			(void) server(sfd, silent);
			_exit(1);
		default:
			(void) close(sfd);
		}
	}	/* for (;;) */
}

int
write_bytes(int fd, umdir_t *Bytes)
{
	if (lseek(fd, sizeof(pid_t), SEEK_SET) == -1) {
		(void) filewrt(2, "lseek: %s\n", strerror(errno));
		return (-1);
	}
	if (write(fd, (char *) Bytes, sizeof(umdir_t)) == -1) {
		(void) filewrt(2, "write_bytes: %s\n", strerror(errno));
		return (-1);
	}
	return (0);
}


int
server(int sfd, int silent)
{
	int             fd, retval, count;
	umdir_t         bytes;
	ssize_t         n;
	pid_t           pid;
	FILE           *socketfp;
	char            buffer[MAXBUF], hostname[56];
	char           *ptr, *(parm[1]);

	if ((retval = sockread(sfd, hostname, sizeof(hostname) - 1)) == -1) {
		(void) filewrt(2, "read: %s\n", strerror(errno));
		write(sfd, "1", 1);
		return (1);
	} else
	if (!retval)
		return(1);
	hostname[retval] = 0;
	if (!(socketfp = fdopen(sfd, "r"))) {
		(void) filewrt(2, "fdopen: %s\n", strerror(errno));
		write(sfd, "1", 1);
		return (1);
	}
	write(sfd, "1", 1);
	(void) sprintf(statusfile, "%s/%s.status", statusdir, hostname);
	if (!access(statusfile, R_OK)) {
		if ((fd = open(statusfile, O_RDWR, 0644)) == -1) {
			(void) filewrt(2, "creat: %s %s\n", statusfile, strerror(errno));
			return(1);
		}
		if ((n = read(fd, (char *) &pid, sizeof(pid_t))) == -1) {
			(void) filewrt(2, "readpid: %s: %s\n", statusfile, strerror(errno));
			return(1);
		}
		if (n == sizeof(pid_t)) {
			if ((n = read(fd, (char *) &bytes, sizeof(umdir_t))) == -1) {
				(void) filewrt(2, "readbytes: %s: %s\n", statusfile, strerror(errno));
				return(1);
			}
		}
	} else {
		if ((fd = open(statusfile, O_CREAT|O_RDWR, 0644)) == -1) {
			(void) filewrt(2, "creat: %s %s\n", statusfile, strerror(errno));
			return(1);
		}
		bytes = 0;
	}
	if (lseek(fd, 0, SEEK_SET) == -1) {
		(void) filewrt(2, "lseek: %s\n", strerror(errno));
		return (-1);
	}
	pid = getpid();
	if (write(fd, (char *) &pid, sizeof(pid_t)) != sizeof(pid_t)) {
		(void) filewrt(2, "write: %s: %s\n", statusfile, strerror(errno));
		return(1);
	}
	filewrt(1, "Connection request from %s\n", hostname);
	(void) signal(SIGTERM, SigTerm);
	for (;; sleep(5)) {
		for (;;) {
			(void) fgets(buffer, MAXBUF - 1, socketfp);
			if (feof(socketfp)) {
				/*-
				if (!access(statusfile, F_OK))
					(void) unlink(statusfile);
				*/
				(void) filewrt(2, "client terminated on %s\n", hostname);
				shutdown(sfd, 0);
				return (1);
			}
			if (rpcflag) {
				*parm = buffer;
				if (log_msg(parm) == -1) {
					shutdown(sfd, 0);
					return(1);
				}
			} 
			if (!silent && filewrt(1, "%s", buffer) == -1) {
				(void) filewrt(2, "filewrt: out: %s\n", strerror(errno));
				shutdown(sfd, 0);
				return(1);
			}
			for (count = 0,ptr = buffer;*ptr;ptr++) {
				if (*ptr == ' ')
					count++;
				if (count == 2)
					break;
			}
			if (*ptr)
			{
				bytes += slen(ptr + 1);
				filewrt(2, "%s", ptr+1);
			}
			else
				bytes += slen(buffer);
			if (write_bytes(fd, &bytes) == -1) {
				(void) filewrt(2, "write_bytes: %s\n", strerror(errno));
				shutdown(sfd, 0);
				return(1);
			}
		} /* for (;;) */
	} /* for (;; sleep(5))*/
}

int
log_msg(char **str)
{
	int            *ret;
	static int      flag;

	if (!flag) {
		if (!(cl = clnt_create(loghost, RPCLOG, CLOGVERS, "tcp"))) {
			filewrt(2, "clnt_create: %s\n", clnt_spcreateerror(loghost));
			return (-1);
		}
		flag++;
	}
	if (!(ret = send_message_1(str, cl))) {
		clnt_perror(cl, "send_message_1");
		clnt_destroy(cl);
		flag = 0;
		return (-1);
	} else
		return (0);
}

int            *
send_message_1(char **argp, CLIENT *clnt)
{
	static int      res;
	struct timeval  TIMEOUT = {25, 0};

	(void) memset((char *) &res, 0, sizeof(res));
	if (clnt_call(clnt, SEND_MESSAGE, (xdrproc_t) xdr_wrapstring, (char *) argp,
			(xdrproc_t) xdr_int, (char *) &res, TIMEOUT) != RPC_SUCCESS) {
		filewrt(2, "clnt_call: %s\n", clnt_sperror(clnt, loghost));
		return (NULL);
	}
	return (&res);
}

void
SigTerm()
{
	if (logsrvpid == getpid()) {
		filewrt(2, "Sending SIGTERM to PROCESS GROUP %d\n", logsrvpid);
		(void) signal(SIGTERM, SIG_IGN);
		kill(-logsrvpid, SIGTERM);
	}
	/*-
	else
	if (!access(statusfile, F_OK))
		(void) unlink(statusfile);
	*/
	exit(0);
}
