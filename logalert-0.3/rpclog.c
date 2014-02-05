/*
 * $Log: rpclog.c,v $
 * Revision 1.3  2013-05-15 00:18:45+05:30  Cprogrammer
 * fixed warnings
 *
 * Revision 1.2  2012-09-19 11:07:26+05:30  Cprogrammer
 * rearranged functions
 *
 * Revision 1.1  2012-09-18 18:25:37+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <stdio.h>
#include <time.h>
#include <errno.h>
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#include <signal.h>
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#include <sys/ioctl.h>
#ifdef HAVE_RPC_RPC_H
#include <rpc/rpc.h>
#endif

#ifdef DEBUG
#define RPC_SVC_FG
#endif

#define _RPCSVC_CLOSEDOWN 900

#define RPCLOG       ((u_long)0x20000001)
#define LOGVERS      ((u_long)1)
#define SEND_MESSAGE ((u_long)1)

#ifndef	lint
static char     sccsid[] = "$Id: rpclog.c,v 1.3 2013-05-15 00:18:45+05:30 Cprogrammer Exp mbhangui $";
#endif

struct CONSOLE_MSG
{
	long            globnum;
	long            r_date;
	long            r_time;
	char            machine[21];
	long            machnum;
	char            type[21];
	char            text[256];
};

bool_t          pmap_unset(unsigned long, unsigned long);
static void     rpclog_1();
static int     *send_message_1();

static int      _rpcpmstart;	/*- Started by a port monitor ? */
static int      _rpcfdtype;		/*- Whether Stream or Datagram ? */
static int      _rpcsvcdirty;	/*- Still serving ? */


int
get_options(int argc, char **argv, int *foreground)
{
	int             c, errflag = 0;

	while (!errflag && (c = getopt(argc, argv, "f")) != -1)
	{
		switch (c)
		{
		case 'f':
			*foreground = 1;
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

static void
_msgout(char *msg)
{
#ifdef RPC_SVC_FG
	if (_rpcpmstart)
		(void) fprintf(stderr, "rpclog: %s\n", msg);
	else
		(void) fprintf(stderr, "rpclog: %s\n", msg); /*- could be syslog */
#else
	(void) fprintf(stderr, "rpclog: %s\n", msg);
#endif
}

static void
closedown()
{
	(void) signal(SIGALRM, closedown);
	if (_rpcsvcdirty == 0) {
		extern fd_set   svc_fdset;
		static int      size;
		int             i, openfd;

		if (_rpcfdtype == SOCK_DGRAM)
			exit(0);
		if (size == 0)
			size = getdtablesize();
		for (i = 0, openfd = 0; i < size && openfd < 2; i++)
			if (FD_ISSET(i, &svc_fdset))
				openfd++;
		if (openfd <= 1)
			exit(0);
	}
	(void) alarm(_RPCSVC_CLOSEDOWN);
}

static void
rpclog_1(rqstp, transp)
	struct svc_req *rqstp;
	register SVCXPRT *transp;
{
	union
	{
		char           *send_message_1_arg;
	}               argument;
	char           *result;
	bool_t(*xdr_argument) (), (*xdr_result) ();
	char           *(*local) ();

	_rpcsvcdirty = 1;
	switch (rqstp->rq_proc)
	{
	case NULLPROC:
		(void) svc_sendreply(transp, (xdrproc_t) xdr_void, (char *) NULL);
		_rpcsvcdirty = 0;
		return;

	case SEND_MESSAGE:
		xdr_argument = xdr_wrapstring;
		xdr_result = xdr_int;
		local = (char *(*) ()) send_message_1;
		break;

	default:
		svcerr_noproc(transp);
		_rpcsvcdirty = 0;
		return;
	}
	bzero((char *) &argument, sizeof(argument));
	if (!svc_getargs(transp, (xdrproc_t) xdr_argument, (caddr_t) &argument)) {
		svcerr_decode(transp);
		_rpcsvcdirty = 0;
		return;
	}
	result = (*local) (&argument, rqstp);
	if (result != NULL && !svc_sendreply(transp, (xdrproc_t) xdr_result, result))
		svcerr_systemerr(transp);
	if (!svc_freeargs(transp, (xdrproc_t) xdr_argument, (caddr_t) &argument)) {
		_msgout("unable to free arguments");
		exit(1);
	}
	_rpcsvcdirty = 0;
	return;
}

static int     *
send_message_1(msg)
	char          **msg;
{
	static int      retval;
	static char    *tmp;
	int             len;

	retval = 1;
	if(*msg) {
		len = strlen(*msg);
		if (!(tmp = (char *) realloc(tmp, len + 1))) {
			fprintf(stderr, "rpclog: realloc: %s/n", strerror(errno));
			retval = 0;
		} else {
			strncpy(tmp, *msg, len);
			tmp[len] = 0;
			if (write(1, tmp, len) == -1) {
				fprintf(stderr, "write: /dev/console: %s/n", strerror(errno));
				retval = 0;
			}
		}
	}
	return &retval;
}

int
main(int argc, char **argv)
{
	register SVCXPRT *transp = (SVCXPRT *) 0;
	struct sockaddr_in saddr;
	int             sock, fcount, proto, foreground, asize = sizeof(saddr);

	if (getsockname(0, (struct sockaddr *) & saddr, (socklen_t *) &asize) == 0) {
		int             ssize = sizeof(int);

		if (saddr.sin_family != AF_INET)
			exit(1);
		if (getsockopt(0, SOL_SOCKET, SO_TYPE, (char *) &_rpcfdtype, (socklen_t *) &ssize) == -1)
			exit(1);
		sock = 0;
		_rpcpmstart = 1;
		proto = 0;
	} else {
#ifndef RPC_SVC_FG
		int             i, pid;

		if (get_options(argc, argv, &foreground)) {
			fprintf(stderr, "Usage: rpclog [-s]\n");
			return (1);
		}
		if (!foreground) {
			if ((pid = fork()) < 0) {
				perror("cannot fork");
				exit(1);
			}
			if (pid)
				exit(0);
			setsid();
		}
		if (!foreground) {
			if ((fcount = (int) sysconf(_SC_OPEN_MAX)) == -1) {
				fprintf(stderr, "rpclog: sysconf: %s\n", strerror(errno));
				_exit(1);
			}
			for (i = 0; i < fcount; i++)
				(void) close(i);
			if ((i = open("/dev/console", O_WRONLY)) == -1) {
				fprintf(stderr, "rpclog: /dev/console: %s\n", strerror(errno));
				_exit(1);
			}
			if (dup2(i, 1) == -1 || dup2(i, 2) == -1) {
				fprintf(stderr, "rpclog: dup2: %s\n", strerror(errno));
				_exit(1);
			}
			if (!i)
				close(i);
		}
#endif
		sock = RPC_ANYSOCK;
		(void) pmap_unset(RPCLOG, LOGVERS);
	}

	if ((_rpcfdtype == 0) || (_rpcfdtype == SOCK_DGRAM)) {
		transp = (SVCXPRT *) svcudp_create(sock);
		if (transp == (SVCXPRT *) NULL) {
			_msgout("cannot create udp service.");
			exit(1);
		}
		if (!_rpcpmstart)
			proto = IPPROTO_UDP;
		if (!svc_register(transp, RPCLOG, LOGVERS, rpclog_1, proto)) {
			_msgout("unable to register (RPCLOG, LOGVERS, udp).");
			exit(1);
		}
	}
	if ((_rpcfdtype == 0) || (_rpcfdtype == SOCK_STREAM)) {
		transp = (SVCXPRT *) svctcp_create(sock, 0, 0);
		if (transp == (SVCXPRT *) NULL) {
			_msgout("cannot create tcp service.");
			exit(1);
		}
		if (!_rpcpmstart)
			proto = IPPROTO_TCP;
		if (!svc_register(transp, RPCLOG, LOGVERS, rpclog_1, proto)) {
			_msgout("unable to register (RPCLOG, LOGVERS, tcp).");
			exit(1);
		}
	}
	if (transp == (SVCXPRT *) NULL) {
		_msgout("could not create a handle");
		exit(1);
	}
	if (_rpcpmstart) {
		(void) signal(SIGALRM, closedown);
		(void) alarm(_RPCSVC_CLOSEDOWN);
	}
	svc_run();
	_msgout("svc_run returned");
	exit(1);
	/* NOTREACHED */
}

void
getversion_rpclog_c()
{
	printf("%s\n", sccsid);
}
