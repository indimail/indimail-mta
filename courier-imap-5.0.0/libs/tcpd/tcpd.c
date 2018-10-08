/*
** Copyright 1998 - 2013 Double Precision, Inc.
** See COPYING for distribution information.
*/

#if	HAVE_CONFIG_H
#include	"config.h"
#endif

/*
** OK - the poop is that if we include socks.h after stdio.h, SOCKSfwrite
** does not get prototyped.
** If we include socks.h before stdio.h, gcc will complain about getc being
** redefined.  The easiest solution is to simply undef getc, because we
** don't use it here.
*/

#include	"soxwrap/soxwrap.h"
#include	<stdio.h>


#include	<sys/types.h>
#include	<sys/time.h>
#include	<arpa/inet.h>
#include	<pwd.h>
#include	<grp.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<stdio.h>
#include	<errno.h>
#include	<signal.h>
#if	HAVE_SYS_STAT_H
#include	<sys/stat.h>
#endif
#if	HAVE_FCNTL_H
#include	<fcntl.h>
#endif
#if	HAVE_SYS_IOCTL_H
#include	<sys/ioctl.h>
#endif
#if HAVE_UNISTD_H
#include	<unistd.h>
#endif
#include	"waitlib/waitlib.h"
#include	"rfc1035/rfc1035.h"
#include	"liblock/config.h"
#include	"liblock/liblock.h"
#include	"tcpremoteinfo.h"
#include	"numlib/numlib.h"
#include	"argparse.h"

#include	<netdb.h>


static const char *accessarg=0;
static const char *accesslocal=0;
static const char *denymsgarg=0;
static const char *listenarg=0;
static const char *ipaddrarg=0;
static const char *userarg=0;
static const char *grouparg=0;
static const char *maxprocsarg=0;
static const char *warnarg=0;
static const char *maxperiparg=0;
static const char *maxpercarg=0;
static const char *droparg=0;
static const char *nodnslookup=0;
static const char *noidentlookup=0;
static const char *stderrarg=0;
static const char *stderrloggerarg=0;
static const char *pidarg=0;
static const char *proxyarg=0;
static const char *restartarg=0;
static const char *stoparg=0;
static const char *stderrloggername=0;

static char *lockfilename;

static void setup_block(const char *);
static void setup_allow(const char *);

static struct args arginfo[]={
	{"access", &accessarg},
	{"accesslocal", &accesslocal},
	{"denymsg", &denymsgarg},
	{"drop", &droparg},
	{"address", &ipaddrarg},
	{"block", 0, setup_block},
	{"allow", 0, setup_allow},
	{"group", &grouparg},
	{"listen", &listenarg},
	{"maxperc", &maxpercarg},
	{"maxperip", &maxperiparg},
	{"maxprocs", &maxprocsarg},
	{"warn", &warnarg},
	{"nodnslookup", &nodnslookup},
	{"noidentlookup", &noidentlookup},
	{"pid", &pidarg},
	{"restart", &restartarg},
	{"stderr", &stderrarg},
	{"stderrlogger", &stderrloggerarg},
	{"stderrloggername", &stderrloggername},
	{"stop", &stoparg},
	{"user", &userarg},
	{"proxy", &proxyarg},
	{0}
	} ;

/* Ports we're listening on: */

static struct portinfo {
	struct portinfo *next;
	const char *ipaddr;	/* Specific IP addr, or 0 */
	const char *servname;	/* Service name/port */

	int fd1, fd2;		/* BSD may need both IPv4 and IPv6 sockets */
} *fdlist=0;
static int maxfd;

static int nprocs, maxperc, maxperip, nwarn;
static pid_t *pids;
static time_t last_alert=0, last_warn=0;
static RFC1035_ADDR *addrs;

static int sighup_received=0;

struct blocklist_s {
	struct blocklist_s *next;
	char *zone;             /* zone to lookup */
	char *display_zone;     /* zone for var_ZONE */
	char *var;
	struct in_addr ia;	/* 0, anything */
	char *msg;		/* NULL, query for TXT record */
	int allow;		/* This is an -allow entry */
	} *blocklist=0;

extern int openaccess(const char *);
extern void closeaccess();
extern char *chkaccess(const char *);

/*
** Process -block and -allow parameters
*/
static void setup_block_allow(const char *blockinfo, int isallow)
{
struct blocklist_s *newbl=(struct blocklist_s *)malloc(sizeof(*blocklist));
char	*p;
struct blocklist_s **blptr;
const char *ip;

	for (blptr= &blocklist; *blptr; blptr=&(*blptr)->next)
		;

	if (!newbl || (newbl->zone=malloc(strlen(blockinfo)+1)) == 0)
	{
		perror("malloc");
		exit(1);
	}

	*blptr=newbl;
	newbl->next=0;
	newbl->allow=isallow;

	strcpy(newbl->zone, blockinfo);

	newbl->var=0;
	newbl->msg=0;
	newbl->display_zone=0;
	newbl->ia.s_addr=INADDR_ANY;

	/* Look for var or IP address */

	for (p=newbl->zone; *p; ++p)
	{
		if (*p == '=')
		{
			*p++=0;
			newbl->display_zone=p;
			continue;
		}

		if (*p == '/')
			break;

		if (*p == ',')
		{
			*p++=0;
			newbl->var=p;
			break;
		}
	}

	if (newbl->display_zone == 0)
		newbl->display_zone = newbl->zone;

	ip=0;

	for (; *p; p++)
	{
		if (*p == ',')
			break;

		if (*p == '/')
		{
			*p++=0;
			ip=p;
			break;
		}
	}

	for (; *p; p++)
	{
		if (*p == ',')
		{
			*p++=0;
			newbl->msg=p;
			break;
		}
	}

	if (ip)
	{
		rfc1035_aton_ipv4(ip, &newbl->ia);
	}
}

static void setup_block(const char *blockinfo)
{
	setup_block_allow(blockinfo, 0);
}

static void setup_allow(const char *blockinfo)
{
	setup_block_allow(blockinfo, 1);
}

static int isid(const char *p)
{
	while (*p)
	{
		if (*p < '0' || *p > '9')	return (0);
		++p;
	}
	return (1);
}

static RETSIGTYPE sigexit(int n)
{
	kill( -getpid(), SIGTERM);
	_exit(0);

#if RETSIGTYPE != void
	return (0)
#endif
}

static RETSIGTYPE sighup(int n)
{
	sighup_received=1;

	signal(SIGHUP, sighup);

#if RETSIGTYPE != void
	return (0)
#endif
}

/*
** Initialize a single listening socket
*/

static struct portinfo *createport(const char *a, const char *s)
{
	struct portinfo *p=(struct portinfo *)malloc(sizeof(struct portinfo));

	if (!p)
	{
		perror("malloc");
		return (NULL);
	}

	p->next=fdlist;
	fdlist=p;
	p->ipaddr=a;
	p->servname=s;
	p->fd1=p->fd2= -1;
	return (p);
}

static int parseaddr(const char *p)
{
	char *buf=strdup(p);
	char *q, *a, *s;

	if (!buf)
	{
		perror("malloc");
		return (-1);
	}

	for (q=buf; (q=strtok(q, ",")) != NULL; q=0)
	{
		if ((s=strrchr(q, '.')) != 0)
		{
			*s++=0;
			a=q;
		}
		else
		{
			a=0;
			s=q;
		}

		if (createport(a, s) == NULL)
			return (-1);
	}

	if (ipaddrarg)
	{
		struct portinfo *p;

		for (p=fdlist; p; p=p->next)
		{
			if (p->ipaddr && strcmp(p->ipaddr, "0"))
				continue;
			p->ipaddr=ipaddrarg;
		}
	}

	return (0);
}

/*
** Create one socket, bound to a specific host/port
*/

static int mksocket(const char *ipaddrarg,	/* Host/IP address */
		    const char *servname,	/* Service/port */
		    int flags)

#define MKS_USEAFINET4 1
#define MKS_ERROK 2

{
	struct servent *servptr;
	int	port;
	int	fd;

	RFC1035_ADDR addr;
	RFC1035_NETADDR  netaddr;
	const struct sockaddr *sinaddr;
	int	sinaddrlen;

#if	RFC1035_IPV6
	struct sockaddr_in6	sin6;
#endif

	struct sockaddr_in	sin4;

	int	af;

	servptr=getservbyname(servname, "tcp");
	if (servptr)
		port=servptr->s_port;
	else
	{
		port=atoi(servname);
		if (port <= 0 || port > 65535)
		{
			fprintf(stderr, "Invalid port: %s\n", servname);
			return (-1);
		}
		port=htons(port);
	}

	/* Create an IPv6 or an IPv4 socket */

#if	RFC1035_IPV6
	if (flags & MKS_USEAFINET4)
	{
		fd=socket(PF_INET, SOCK_STREAM, 0);
		af=AF_INET;
	}
	else
#endif
		fd=rfc1035_mksocket(SOCK_STREAM, 0, &af);

	if (fd < 0)
	{
		perror("socket");
		return (-1);
	}

	/* Figure out what to bind based on what socket we created */

	if (ipaddrarg && strcmp(ipaddrarg, "0"))
	{
		if (rfc1035_aton(ipaddrarg, &addr) < 0)
		{
			fprintf(stderr,"Invalid IP address: %s\n", ipaddrarg);
			close(fd);
			return (-1);
		}

		if (rfc1035_mkaddress(af, &netaddr, &addr, port, &sinaddr,
				&sinaddrlen))
		{
			fprintf(stderr,"Unable to bind IP address: %s\n",
				ipaddrarg);
			close(fd);
			return (-1);
		}
	}
	else	/* Bind default address */
	{
#if	RFC1035_IPV6
		if (af == AF_INET6)
		{
			memset(&sin6, 0, sizeof(sin6));
			sin6.sin6_family=AF_INET6;
			sin6.sin6_addr=in6addr_any;
			sin6.sin6_port=port;
			sinaddr=(const struct sockaddr *)&sin6;
			sinaddrlen=sizeof(sin6);
		}
		else
#endif
			if (af == AF_INET)
			{
				sin4.sin_family=AF_INET;
				sin4.sin_addr.s_addr=INADDR_ANY;
				sin4.sin_port=port;
				sinaddr=(const struct sockaddr *)&sin4;
				sinaddrlen=sizeof(sin4);
			}
			else
			{
				errno=EAFNOSUPPORT;
				perror("socket");
				close(fd);
				return (-1);
			}
	}

	{
		int dummy=1;

		if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR,
		       (const char *)&dummy, sizeof(dummy)) < 0)
		{
			perror("setsockopt");
		}
	}

	if (fcntl(fd, F_SETFD, FD_CLOEXEC))
	{
		perror("fcntl");
		close(fd);
		return (-1);
	}

	if (fcntl(fd, F_SETFL, O_NONBLOCK))
	{
		perror("fcntl");
		close(fd);
		return (-1);
	}

	if (sox_bind(fd, (struct sockaddr *)sinaddr, sinaddrlen) < 0)
	{
		if (flags & MKS_ERROK)
		{
			close(fd);
			return (-2);
		}

		perror("bind");
		close(fd);
		return (-1);
	}

	if (sox_listen(fd,
#ifdef	SOMAXCONN
		       SOMAXCONN
#else
		       5
#endif
		       ))
	{
		if (flags && MKS_ERROK)
		{
			close(fd);
			return (-2);
		}
		perror("listen");
		close(fd);
		return (-1);
	}
	return (fd);
}

static int mksockets()
{
	struct portinfo *p;

	maxfd= -1;

	for (p=fdlist; p; p=p->next)
	{
		int fd;
		struct in_addr addr;
		int fd_flag=0;

		if (p->ipaddr && strcmp(p->ipaddr, "0"))
		{
			/* FreeBSD needs AF_INET binds for IPv4 addys */

			if (rfc1035_aton_ipv4(p->ipaddr, &addr) == 0)
			{
				fd_flag=MKS_USEAFINET4;
			}
		}

		fd=mksocket(p->ipaddr, p->servname, fd_flag);

		if (fd < 0)
			break;

		p->fd1=fd;

		if (fd > maxfd)
			maxfd=fd;

		/* BSD requires both an IPv6 and an IPv4 socket */

#if	RFC1035_IPV6
		if (p->ipaddr == 0 || strcmp(p->ipaddr, "0") == 0)
		{
			fd=mksocket(p->ipaddr, p->servname,
				    (MKS_USEAFINET4|MKS_ERROK));

			if (fd == -2)
				continue;	/* Ok if bind failed */
			if (fd < 0)
				break;

			if (fd > maxfd)
				maxfd=fd;
			p->fd2=fd;
		}
#endif

	}

	if (p)	/* Clean up after ourselves, after an error */
	{
		for (p=fdlist; p; p=p->next)
		{
			if (p->fd1 >= 0)
				close(p->fd1);
			if (p->fd2 >= 0)
				close(p->fd2);
		}
		return (-1);
	}

	return (0);
}

static int dup_and_check(int orig)
{
	int fd=sox_dup(orig);

	if (fd < 0)
	{
		perror("dup");
		exit(1);
	}
	return fd;
}

static int init(int argc, char **argv)
{
int	argn;

struct	group *gr;
int	i;
gid_t	gid=0;
const	char *servname;
int	forced=0;
int	lockfd=-1;

	argn=argparse(argc, argv, arginfo);

	if ((stoparg || restartarg) && pidarg == 0)
	{
		fprintf(stderr, "%s: -pid argument is required.\n", argv[0]);
		return (-1);
	}

	if (pidarg)
	{
		lockfilename=malloc(strlen(pidarg)+sizeof(".lock"));
		if (!lockfilename)
		{
			perror("malloc");
			return (-1);
		}
		strcat(strcpy(lockfilename, pidarg), ".lock");
	}

	if (stoparg)
	{
		ll_daemon_stop(lockfilename, pidarg);
		exit(0);
	}

	if (restartarg)
	{
		ll_daemon_restart(lockfilename, pidarg);
		exit(0);
	}

	if (argc - argn < 2)
	{
		fprintf(stderr, "Usage: %s [options] port prog arg1 arg2...\n",
			argv[0]);
		return (-1);
	}

	if (pidarg) { /* -start implied for backwards compatibility */
		lockfd=ll_daemon_start(lockfilename);
		if (lockfd < 0)
		{
			perror("ll_daemon_start");
			return (-1);
		}
	}

	servname=argv[argn++];

	if (parseaddr(servname))
	{
		close(lockfd);
		return (-1);
	}

	if (mksockets())
	{
		close(lockfd);
		return (-1);
	}

	signal(SIGINT, sigexit);
	signal(SIGHUP, sighup);
	signal(SIGTERM, sigexit);

#if 0
	{
	int	fd2;
	int	dummy;

		perror("bind");
		if (!forcebindarg || errno != EADDRINUSE)
		{
			sox_close(fd);
			return (-1);
		}

		/* Poke around */

		if ((fd2=rfc1035_mksocket(SOCK_STREAM, 0, &dummy)) < 0)
			/* Better get same socket as fd */
		{
			perror("socket");
			sox_close(fd);
			return (-1);
		}

		if (sox_connect(fd2, (struct sockaddr *)sinaddr,
			sinaddrlen) == 0)
		{
			sox_close(fd2);
			sox_close(fd);
			return (-1);
		}
		sox_close(fd2);
		savepid();
		sleep(60);
		forced=1;
	}
#endif

	if (pidarg)
		ll_daemon_started(pidarg, lockfd);

	if (grouparg)
	{
		if (isid(grouparg))
			gid=atoi(grouparg);
		else if ((gr=getgrnam(grouparg)) == 0)
		{
			fprintf(stderr, "Group not found: %s\n", grouparg);
			close(lockfd);
			return (-1);
		}
		else	gid=gr->gr_gid;

		libmail_changegroup(gid);
	}

	if (userarg)
	{
	uid_t	uid;

		if (isid(userarg))
		{
			uid=atoi(userarg);
			libmail_changeuidgid(uid, getgid());
		}
		else
		{
		gid_t	g=getgid(), *gp=0;

			if (grouparg)	gp= &g;
			libmail_changeusername(userarg, gp);
		}
	}

	if (pidarg && ll_daemon_resetio())
	{
		perror("ll_daemon_resetio");
		close(lockfd);
		return (-1);
	}

	if (stderrloggerarg)
	{
	pid_t	p;
	int	waitstat;
	int	pipefd[2];
	const char *progname=argv[argn];

		if (pipe(pipefd) < 0)
		{
			perror("pipe");
			return (-1);
		}

		signal(SIGCHLD, SIG_DFL);
		while ((p=fork()) == -1)
		{
			sleep(5);
		}

		if (p == 0)
		{
			signal(SIGHUP, SIG_IGN);
			sox_close(0);
			dup_and_check(pipefd[0]);
			sox_close(pipefd[0]);
			sox_close(pipefd[1]);
			sox_close(1);
			open("/dev/null", O_WRONLY);
			sox_close(2);
			dup_and_check(1);
			closeaccess();
			while ((p=fork()) == -1)
			{
				perror("fork");
				sleep(5);
			}
			if (p == 0)
			{
			const char *p=strrchr(progname, '/');

				if (p)	++p;
				else p=progname;

				if (stderrloggername && *stderrloggername)
					p=stderrloggername;

				execl(stderrloggerarg, stderrloggerarg,
						p, (char *)0);
				perror(stderrloggerarg);
				_exit(5);
			}
			_exit(0);
		}
		sox_close(2);
		dup_and_check(pipefd[1]);
		sox_close(pipefd[0]);
		sox_close(pipefd[1]);
		while (wait(&waitstat) != p)
			;
	}
	else if (stderrarg)
	{
	int	fd=open(stderrarg, O_WRONLY|O_APPEND|O_CREAT, 0660);

		if (!fd)
		{
			perror(stderrarg);
			return (-1);
		}
		sox_close(2);
		dup_and_check(fd);
		sox_close(fd);
	}

	nprocs=40;
	if (maxprocsarg)
	{
		nprocs=atoi(maxprocsarg);
		if (nprocs <= 0)
		{
			fprintf(stderr, "Invalid -maxprocsarg option.\n");
			return (-1);
		}
	}

	nwarn= nprocs - (nprocs / 10 + 1);

	if (warnarg)
	{
		int c=atoi(warnarg);

		if (c >= 0 && c <= nprocs)
			nwarn=c;
	}

	if ((pids=malloc(sizeof(*pids)*nprocs)) == 0)
	{
		perror("malloc");
		return (-1);
	}
	if ((addrs=malloc(sizeof(*addrs)*nprocs)) == 0)
	{
		free(pids);
		perror("malloc");
		return (-1);
	}


	for (i=0; i<nprocs; i++)
		pids[i]= -1;

	maxperc=nprocs;
	maxperip=4;

	if (maxpercarg)
	{
		maxperc=atoi(maxpercarg);
		if (maxperc <= 0)
		{
			fprintf(stderr, "Invalid -maxperc option.\n");
			free(pids);
			free(addrs);
			return (-1);
		}
	}
	if (maxperiparg)
	{
		maxperip=atoi(maxperiparg);
		if (maxperip <= 0)
		{
			fprintf(stderr, "Invalid -maxperip option.\n");
			free(pids);
			free(addrs);
			return (-1);
		}
	}
	if (forced)
	{
		fprintf(stderr, "couriertcpd: ready.\n");
		fflush(stderr);
	}
	return (argn);
}

static void run(int, const RFC1035_ADDR *, int, const char *, char **);

static void doreap(pid_t p, int wait_stat)
{
int	n;

	for (n=0; n<nprocs; n++)
		if (p == pids[n])
		{
			pids[n]= -1;
			break;
		}
}

static RETSIGTYPE childsig(int signum)
{
	signum=signum;
	wait_reap(doreap, childsig);
#if RETSIGTYPE != void
	return (0);
#endif
}

static int doallowaccess(char *, int);

#if	RFC1035_IPV6

static int allowaccess(const RFC1035_ADDR *sin, int port)
{
char	buf[RFC1035_MAXNAMESIZE+1+6];
char	*q;
int	i;

	if (IN6_IS_ADDR_V4MAPPED(sin))
	{
	const char *p=inet_ntop(AF_INET6, sin, buf, sizeof(buf)-6);

		if (p && (q=strrchr(buf, ':')) != 0)
			return (doallowaccess(q+1, port));
		return (1);
	}
	q=buf;
	for (i=0; i<sizeof(*sin); i += 2)
	{
#define	B(i) ((unsigned long)((unsigned char *)sin)[i])
	unsigned long n=(B(i) << 8) | B(i+1);
#undef	B
		sprintf(q, ":%04lx", n);
		q += 5;
	}
	*q=0;
	return (doallowaccess(buf, port));
}

#else
static int allowaccess(const RFC1035_ADDR *sin, int port)
{
char	buf[RFC1035_NTOABUFSIZE+6];

	rfc1035_ntoa(sin, buf);
	return (doallowaccess(buf, port));
}
#endif

static int doallowaccess(char *buf, int port)
{
char	*accessptr;
char *p, *q, *r;
int	quote=0;
int	l;

	if (accessarg == 0)	return (1);

	if (port) snprintf(buf+strlen(buf), 7, ".%d", ntohs(port));
	while ((accessptr= *buf ? chkaccess(buf):0) == 0)
	{
		if ((accessptr=strrchr(buf, '.')) == 0
#if RFC1035_IPV6
			&& (accessptr=strrchr(buf, ':')) == 0
#endif
			)
		{
			if (port)
			{
				snprintf(buf, 8, "*.%d", ntohs(port));
				if ((accessptr=chkaccess(buf)) != 0)
					break;
			}
			if ((accessptr=chkaccess("*")) != 0)
				break;
			return (1);
		}
		*accessptr=0;
	}

	if (strncmp(accessptr, "deny", 4) == 0)
	{
		free(accessptr);
		return (0);
	}

	p=accessptr;
	if (strncmp(accessptr, "allow", 5) == 0)
	{
		p += 5;
		if (*p == ',')	++p;
	}

	while ( p && *p )
	{
		q=p;
		r=q;
		while (*p)
		{
			if (*p == ',' && !quote)
			{
				*p++=0;
				break;
			}
			if (!quote && (*p == '"' || *p == '\''))
			{
				quote=*p;
				p++;
				continue;
			}
			if (quote && *p == quote)
			{
				quote=0;
				p++;
				continue;
			}
			*r++=*p++;
		}
		*r=0;
		if (strchr(q, '=') == 0)
		{
		char	*r=malloc(strlen(q)+2);

			if (!r)
			{
				perror("malloc");
				return (0);
			}
			q=strcat(strcpy(r, q), "=");
		}

		while (*q && isspace((int)(unsigned char)*q))	++q;
		while ((l=strlen(q)) > 0
		       && isspace((int)(unsigned char)q[l-1]))
			q[--l]=0;
		putenv(q);
	}
	return (1);
}

/* Wait until we have at least one available slot left */

static int getfreeslot(int *pidptr)
{
	int n;

	for (;;)
	{
		wait_block();

		for (n=0; n<nprocs; n++)
		{
			if (pids[*pidptr] == (pid_t)-1)	break;
			if (++*pidptr >= nprocs)	*pidptr=0;
		}
		if (pids[*pidptr] != (pid_t)-1)
		{
			wait_forchild(doreap, childsig);
			continue;
		}
		break;
	}
	wait_clear(childsig);
	return (*pidptr);
}

static void accepted(int, int, RFC1035_NETADDR *, int, const char *, char **);

static int doit(int argn, int argc, char **argv)
{
	char	**ptrs;
	int	pidptr;
	struct portinfo *pi;
	fd_set fdr, fdrcopy;
	int	dummy;

	ptrs=(char **)malloc((argc-argn+1) * sizeof(char *));
	if (!ptrs)
	{
		perror("malloc");
		return (-1);
	}
	for (dummy=0; dummy<argc-argn; dummy++)
	{
		ptrs[dummy]=argv[argn+dummy];
	}
	ptrs[dummy]=0;

	if (listenarg)
	{
		dummy=atoi(listenarg);
		if (dummy <= 0)
		{
			fprintf(stderr, "Invalid -listen option.\n");
			exit(1);
		}
	}

	FD_ZERO(&fdrcopy);
	for (pi=fdlist; pi; pi=pi->next)
	{
		if (pi->fd1 >= 0)
			FD_SET(pi->fd1, &fdrcopy);

		if (pi->fd2 >= 0)
			FD_SET(pi->fd2, &fdrcopy);
	}

	pidptr=0;

	signal(SIGCHLD, childsig);

#if	HAVE_SETPGRP
#if	SETPGRP_VOID
	setpgrp();
#else
	setpgrp(0, 0);
#endif
#else
#if	HAVE_SETPGID
	setpgid(0, 0);
#endif
#endif
#ifdef  TIOCNOTTY

	{
	int fd=open("/dev/tty", O_RDWR);

		if (fd >= 0)
		{
			ioctl(fd, TIOCNOTTY, 0);
			close(fd);
		}
	}
#endif

	signal(SIGPIPE, SIG_IGN);
	for (;;)
	{
		int n;
		int sockfd;
		RFC1035_NETADDR	sin;
		socklen_t	sinl;

		fdr=fdrcopy;

		if (select(maxfd+1, &fdr, NULL, NULL, NULL) <= 0)
		{
			if (errno != EINTR)
				perror("accept");
			continue;
		}

		for (pi=fdlist; pi; pi=pi->next)
		{
			if (pi->fd1 >= 0 && FD_ISSET(pi->fd1, &fdr) &&
			    ((n=getfreeslot(&pidptr)),
			     (sinl = sizeof(sin)),
			     (sockfd=sox_accept(pi->fd1,
						(struct sockaddr *)&sin,
						&sinl))) >= 0)
			{
				accepted(n, sockfd, &sin, sinl,
					 argv[argn], ptrs);
			}

			if (pi->fd2 >= 0 && FD_ISSET(pi->fd2, &fdr) &&
			    ((n=getfreeslot(&pidptr)),
			     (sinl = sizeof(sin)),
			     (sockfd=sox_accept(pi->fd2,
						(struct sockaddr *)&sin,
						&sinl))) >= 0)
			{
				accepted(n, sockfd, &sin, sinl,
					 argv[argn], ptrs);
			}
		}
	}
}

static void denied(int sockfd)
{
	if (denymsgarg) {
		if (write(sockfd, denymsgarg, strlen(denymsgarg)) < 0 ||
		    write(sockfd, "\n", 1) < 0)
		{
			sox_close(sockfd);
			_exit(1);
		}
	}
	sox_close(sockfd);
	_exit(0);
}

static void accepted(int n, int sockfd, RFC1035_NETADDR *sin, int sinl,
		     const char *prog,
		     char **args)
{
	RFC1035_ADDR addr;
	int	addrport;
#ifdef	SO_LINGER
	int	dummy;
	struct	linger l;
#endif
	pid_t	p;
	int cnt;

	if (rfc1035_sockaddrip(sin, sinl, &addr)
	    || rfc1035_sockaddrport(sin, sinl, &addrport))
	{
		sox_close(sockfd);
		return;
	}

	/* Turn off the CLOEXEC and NONBLOCK bits */

	if (fcntl(sockfd, F_SETFD, 0))
	{
		perror("fcntl");
		sox_close(sockfd);
		return;
	}

	if (fcntl(sockfd, F_SETFL, 0))
	{
		perror("fcntl");
		sox_close(sockfd);
		return;
	}

	if (sighup_received)
	{
		sighup_received=0;
		if (accessarg)
		{
			closeaccess();
			if (openaccess(accessarg))
				perror(accessarg);
		}
	}

#ifdef	SO_KEEPALIVE
	dummy=1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE,
		       (const char *)&dummy, sizeof(dummy)) < 0)
	{
		perror("setsockopt");
	}
#endif

#ifdef	SO_LINGER
	l.l_onoff=0;
	l.l_linger=0;

	if (setsockopt(sockfd, SOL_SOCKET, SO_LINGER,
		       (const char *)&l, sizeof(l)) < 0)
	{
		perror("setsockopt");
	}
#endif
	wait_block();
	if ((p=fork()) == -1)
	{
		perror("fork");
		sox_close(sockfd);
		return;
	}

	if (p == 0)
	{
		wait_restore(childsig);

		if (accesslocal) /* Lookup local interface address too? */
		{
		RFC1035_NETADDR lsin;
		RFC1035_ADDR laddr;
		int	lport;
		socklen_t	i=sizeof(lsin);

			if (sox_getsockname(sockfd, (struct sockaddr *)&lsin, &i) == 0 &&
				rfc1035_sockaddrip(&lsin, i, &laddr) == 0 &&
				rfc1035_sockaddrport(&lsin, i, &lport) == 0 &&
				allowaccess(&laddr,lport) == 0)
			{
				sox_close(sockfd);
				_exit(0);
			}
		}

		if (allowaccess(&addr,0) == 0)
		{
			denied(sockfd);
		}

		run(sockfd, &addr, addrport, prog, args);
	}
	pids[n]=p;

	memcpy(addrs+n, &addr, sizeof(addr));
	sox_close(sockfd);
	wait_clear(childsig);

	for (cnt=n=0; n<nprocs; n++)
		if (pids[n] != (pid_t)-1)
			++cnt;

	if (cnt == nprocs)
	{
		time_t t;

		time(&t);
		if (last_alert == 0 || last_alert > t || last_alert < t - 60)
		{
			last_alert=t;
			fprintf(stderr,
				"ALERT: %d maximum active connections.\n",
				nprocs);
		}
	}
	else if (cnt >= nwarn)
	{
		time_t t;

		time(&t);
		if (last_warn == 0 || last_warn > t || last_warn < t - 60)
		{
			last_warn=t;
			fprintf(stderr, "WARN: %d active connections.\n",
				cnt);
		}
	}
}

static void mysetenv(const char *name, const char *val)
{
char	*p=malloc(strlen(name)+strlen(val)+2);

	if (!p)
	{
		perror("malloc");
		_exit(1);
	}
	putenv(strcat(strcat(strcpy(p, name), "="), val));
}

/*
** Convert IP address to host name.  Make sure the IP address resolves
** backwards and forwards.
*/

static void ip2host(const RFC1035_ADDR *addr, const char *env)
{
const char *remotehost="softdnserr";
char	buf[RFC1035_MAXNAMESIZE+1];

#if TCPDUSERFC1035
struct rfc1035_res res;
#endif

	if (nodnslookup)	return;

	rfc1035_ntoa(addr, buf);

#if TCPDUSERFC1035

	rfc1035_init_resolv(&res);

	if (rfc1035_ptr(&res, addr, buf) != 0)
	{
		if (errno == ENOENT)
			remotehost=0;
	}
	else
	{
		RFC1035_ADDR *ias;
		unsigned nias, n;

		if (rfc1035_a(&res, buf, &ias, &nias) != 0)
		{
			if (errno == ENOENT)
				remotehost=0;
		}
		else
		{
			remotehost=0;
			for (n=0; n<nias; n++)
			{
			char	a[RFC1035_MAXNAMESIZE];
			char	b[RFC1035_MAXNAMESIZE];

				rfc1035_ntoa(&ias[n], a);
				rfc1035_ntoa(addr, b);

				if (strcmp(a, b) == 0)
				{
					remotehost=buf;
				}
			}
		}
	}
	rfc1035_destroy_resolv(&res);

#else

	{
	struct hostent *he;
	unsigned n;
	struct	in_addr in;

#if	RFC1035_IPV6

		if (IN6_IS_ADDR_V4MAPPED(addr))
			memcpy(&in, (char *)addr + 12, 4);
		else	return;
#else
		in= *addr;
#endif

		he=gethostbyaddr( (char *)&in, sizeof(in), AF_INET);
		if (!he)
		{
			switch (h_errno)	{
			case HOST_NOT_FOUND:
			case NO_DATA:
				remotehost=0;
				break;
			}
		}
		else
		{
			strcpy(buf, he->h_name);
			he=gethostbyname(buf);
			if (!he)
			{
				switch (h_errno)	{
				case HOST_NOT_FOUND:
				case NO_DATA:
					remotehost=0;
					break;
				}
			}
			else for (n=0, remotehost=0; he->h_addr_list[n]; n++)
			{
			struct in_addr hin;

				if (he->h_addrtype != AF_INET ||
					he->h_length < sizeof(hin))
					break;
				memcpy((char *)&hin, he->h_addr_list[n],
					sizeof(hin));
				if (hin.s_addr == in.s_addr)
				{
					remotehost=buf;
					break;
				}
			}

		}
	}
#endif
	if (remotehost)
		mysetenv(env, remotehost);
}

static void mkmymsg(const char *varname, const char *msg)
{
const char *p=getenv("TCPREMOTEIP");
char *q=malloc(strlen(msg)+1+strlen(p));
char *r;

	if (!q)
	{
		perror("malloc");
		exit(1);
	}

	for (r=q; *msg; msg++)
	{
		if (*msg == '@')
		{
			strcpy(r, p);
			while (*r)	r++;
			++msg;
			break;
		}
		*r++=*msg;
	}
	while (*msg)
		*r++=*msg++;
	*r=0;
	mysetenv(varname, q);
	free(q);
}

static void set_allow_variable(const char *varname, const char *msg)
{
	static int found = 0;
	char buf[32];
	mysetenv(varname, ""); /* Whitelist */

	sprintf(buf, "ALLOW_%d", found++);
	mysetenv(buf, varname);

	(void)msg; /* not used, could tweak behavior of -allow */
}

static void set_txt_response(const char *varname,
		      const char *txt)
{
	char *p=malloc(strlen(varname)+20);

	strcat(strcpy(p, varname), "_TXT");
	mysetenv(p, txt);
	free(p);

}

static void set_zone(const char *varname,
	      const char *zone)
{
	char *p=malloc(strlen(varname)+20);

	strcat(strcpy(p, varname), "_ZONE");
	mysetenv(p, zone);
	free(p);

}


static void set_a_response(const char *varname,
		    const struct in_addr *in)
{
	char	buf[RFC1035_NTOABUFSIZE+6];
	char *p;

	rfc1035_ntoa_ipv4(in, buf);

	p=malloc(strlen(varname)+20);

	strcat(strcpy(p, varname), "_IP");
	mysetenv(p, buf);
	free(p);
}

static int is_a_rr(struct rfc1035_reply *replyp,
		   const struct rfc1035_rr *rrptr,
		   const char *wanted_hostname)
{
	char	buf[RFC1035_MAXNAMESIZE+1];

	/*
	** Go through the DNS response, and check every A record
	** in there.
	*/

	rfc1035_replyhostname(replyp, rrptr->rrname, buf);
	if (rfc1035_hostnamecmp(buf, wanted_hostname))	return 0;

	if (rrptr->rrtype != RFC1035_TYPE_A)
		return 0;

	return 1;
}

/*
** Process TXT records in DNSBL lookup response.
*/

static int search_txt_records(struct rfc1035_res *res,
			       int allow,
			       const char *varname,
			       struct rfc1035_reply *replyp,
			       const char *wanted_hostname)
{
	char	buf[RFC1035_MAXNAMESIZE+1];
	int j;

	if ((j=rfc1035_replysearch_all(res,
				       replyp, wanted_hostname,
				       RFC1035_TYPE_TXT,
				       RFC1035_CLASS_IN, 0)) >= 0)
	{
		rfc1035_rr_gettxt(replyp->allrrs[j], 0, buf);

		if (buf[0]) /* is this necessary? */
		{
			if (!allow)
				mysetenv(varname, buf);
			set_txt_response(varname, buf);
			return 1;
		}
	}
	return 0;
}

/*
** check_blocklist is called once for each blocklist query to process.
*/

static void docheckblocklist(struct blocklist_s *p, const char *nameptr)
{
	const char *q;
	const char *varname=p->var;
	char	hostname[RFC1035_MAXNAMESIZE+1];
	int	wanttxt;
	struct rfc1035_reply *replyp;
	struct rfc1035_res res;
	unsigned int i;
	int found;

	hostname[0]=0;
	strncat(hostname, nameptr, RFC1035_MAXNAMESIZE);

	if (!varname)	varname="BLOCK";

	if ((q=getenv(varname)) != 0)	return;
					/* Env var already set */

	rfc1035_init_resolv(&res);

	/*
	** The third parameter has opposite meanings.  For -block, the last
	** component specifies a custom message that overrides any TXT record
	** in the DNS access list.  For -allow, it simply asks for TXT records
	** to be fetched, for use by external software.
	*/

	if (p->allow)
		wanttxt = p->msg != 0;
	else
		wanttxt = p->msg && strcmp(p->msg, "*") == 0;

	(void)rfc1035_resolve_cname(&res,
			hostname,
			wanttxt ? RFC1035_TYPE_TXT:RFC1035_TYPE_A,
			RFC1035_CLASS_IN, &replyp, 0);

	if (!replyp)
	{
		rfc1035_destroy_resolv(&res);
		return;
	}

	found=0;

	for (i=0; i<replyp->ancount+replyp->nscount+replyp->arcount; i++)
	{
		if (!is_a_rr(replyp, replyp->allrrs[i], hostname))
			continue;

		if (p->ia.s_addr != INADDR_ANY &&
		    p->ia.s_addr != replyp->allrrs[i]->rr.inaddr.s_addr)
			continue;

		set_zone(varname, p->display_zone);
		set_a_response(varname, &replyp->allrrs[i]->rr.inaddr);

		/*
		** The -block option was kind enough to supply the
		** error message.
		*/

		if (!p->allow && p->msg && *p->msg)
		{
			mkmymsg(varname, p->msg);
			continue;
		}

		/*
		** search_txt_records takes care of setting varname for
		** -blocks, and we must set it for -allows.
		*/

		if (p->allow)
			set_allow_variable(varname, p->msg);

		if (!search_txt_records(&res, p->allow, varname, replyp,
					hostname) && !p->allow)
		{
			size_t l=strlen(p->zone)+40;
			char *buf=malloc(l+1);

			if (!buf)
			{
				perror("malloc");
				_exit(1);
			}

			buf[snprintf(buf, l, "Blacklisted by %s", p->zone)]=0;

			/*
			** Even though we did not find a TXT record, we're here
			** because of an A record, so for -blocks, we must
			** set varname to something.
			*/
			mkmymsg(varname, buf);
			free(buf);
		}

		found=1;
		break;
	}

	/*
	** Last chance: if all we got is a TXT record, and we were not looking
	** for a specific IP address, then take what we've got.
	*/

	if (p->ia.s_addr == INADDR_ANY && !found)
	{
		if (search_txt_records(&res, p->allow, varname, replyp,
				       hostname))
		{
			/*
			** search_txt_record takes care of setting varname
			** for -blocks, and we must do it for -allows
			*/
			if (p->allow)
				mysetenv(varname, ""); /* Whitelist */
			set_zone(varname, p->display_zone);
		}
	}

	rfc1035_replyfree(replyp);
	rfc1035_destroy_resolv(&res);
}

static void check_blocklist_ipv4(struct blocklist_s *p,
	const struct in_addr *ia)
{
unsigned a,b,c,d;
char	hostname[RFC1035_MAXNAMESIZE+1];
const unsigned char *q=(const unsigned char *)ia;

	/* Calculate DNS query hostname */

	a=q[0];
	b=q[1];
	c=q[2];
	d=q[3];

	/* Silently ignore exceedingly long zones */
	if (snprintf(hostname, sizeof hostname,
		"%u.%u.%u.%u.%s", d, c, b, a, p->zone) <= RFC1035_MAXNAMESIZE)
			docheckblocklist(p, hostname);
}

#if	RFC1035_IPV6

static void check_blocklist(struct blocklist_s *p, const RFC1035_ADDR *ia)
{
	char	hostname[RFC1035_MAXNAMESIZE+1];

	/*
	** 16 byte IPv6 address. 32 nybbles. Each nybble followed by a dot:
	** 64 characters.
	*/

	char	decimal_address[65];
	char	bytebuf[5];
	int i;

	if (IN6_IS_ADDR_V4MAPPED(ia))
	{
	struct in_addr ia4;

		memcpy(&ia4, (const char *)ia + 12, 4);
		check_blocklist_ipv4(p, &ia4);
	}

	decimal_address[0]=0;

	for (i=0; i<16; ++i)
	{
		unsigned char byte=((struct in6_addr *)ia)->s6_addr[15-i];

		sprintf(bytebuf, "%x.%x.",(byte & 0x0F), ((byte >> 4) & 0x0F));
		strcat(decimal_address, bytebuf);
	}

	/* Silently ignore exceedingly long zones */
	if (snprintf(hostname, sizeof hostname,
		     "%s%s", decimal_address, p->zone) <= RFC1035_MAXNAMESIZE)
		docheckblocklist(p, hostname);
}

#else
static void check_blocklist(struct blocklist_s *p, const RFC1035_ADDR *ia)
{
	check_blocklist_ipv4(p, ia);
}
#endif

static void check_drop(int sockfd)
{
	const char *p, *q;
	char *r;

	p=droparg;

	if (p && !*p)
		p="BLOCK";

	for (; p && *p; q=p)
	{
		if (*p == ',')
		{
			q= ++p;
			continue;
		}

		for (q=p; *q; ++q)
			if (*q == ',')
				break;

		r=malloc(q-p+1);

		if (!r)
		{
			perror("malloc");
			_exit(1);
		}

		memcpy(r, p, q-p);
		r[q-p]=0;

		p=getenv(r);
		free(r);

		if (p && *p)
		{
			fprintf(stderr,
				"WARN: dropped blocked connection from %s\n",
				getenv("TCPREMOTEIP"));
			denied(sockfd);
		}
	}
}

static void proxy();

static void run(int fd, const RFC1035_ADDR *addr, int addrport,
	const char *prog, char **argv)
{
RFC1035_NETADDR lsin;
RFC1035_ADDR laddr;
int	lport;

socklen_t	i;
int	ipcnt, ccnt;
char	buf[RFC1035_MAXNAMESIZE+128];
struct blocklist_s *bl;
const char *remoteinfo;
const char *p;

	i=sizeof(lsin);
	if (sox_getsockname(fd, (struct sockaddr *)&lsin, &i) ||
		rfc1035_sockaddrip(&lsin, i, &laddr) ||
		rfc1035_sockaddrport(&lsin, i, &lport))
	{
		fprintf(stderr, "getsockname failed.\n");
		exit(1);
	}

	if (!noidentlookup && (remoteinfo=tcpremoteinfo(
		&laddr, lport,
		addr, addrport, 0)) != 0)
	{
	char	*q=malloc(sizeof("TCPREMOTEINFO=")+strlen(remoteinfo));

		if (!q)
		{
			perror("malloc");
			_exit(1);
		}

		strcat(strcpy(q, "TCPREMOTEINFO="), remoteinfo);
		putenv(q);
	}

/* check if it's an exception to the global ip limit */
	if( (p=getenv("MAXCPERIP")) != NULL )
	{
		int j = atoi(p);

		if( j > 0 )
			maxperip = j;
	}

	for (i=0, ipcnt=ccnt=0; i<nprocs; i++)
	{
	RFC1035_ADDR *psin;
	int	j;

		if (pids[i] == (pid_t)-1)	continue;

		psin=addrs+i;

		for (j=0; j<sizeof(*addr); j++)
			if ( ((char *)addr)[j] != ((char *)psin)[j])
				break;

		if (j >= sizeof(*addr) &&
			++ipcnt >= maxperip)
		{
			rfc1035_ntoa(addr, buf);
			fprintf(stderr,"ALERT: Maximum connection limit reached for %s\n",buf);
			_exit(0);	/* Too many from same IP address */
		}

		if ( j >= sizeof(*addr)-1 &&
			++ccnt >= maxperc)
			_exit(0);	/* Too many from same netblock */

	}

	rfc1035_ntoa(addr, buf);
	mysetenv("TCPREMOTEIP", buf);
	sprintf(buf, "%d", ntohs(addrport));
	mysetenv("TCPREMOTEPORT", buf);
	ip2host(addr, "TCPREMOTEHOST");

	rfc1035_ntoa(&laddr, buf);
	mysetenv("TCPLOCALIP", buf);
	sprintf(buf, "%d", ntohs(lport));
	mysetenv("TCPLOCALPORT", buf);
	ip2host(&laddr, "TCPLOCALHOST");

	for (bl=blocklist; bl; bl=bl->next)
		check_blocklist(bl, addr);

	check_drop(fd);
	sox_close(0);
	sox_close(1);
	dup_and_check(fd);
	dup_and_check(fd);
	sox_close(fd);
	if (stderrarg && strcmp(stderrarg, "socket") == 0)
	{
		sox_close(2);
		dup_and_check(1);
	}
	proxy();
	signal(SIGPIPE, SIG_DFL);

	execv(prog, argv);
	perror(prog);
	exit(1);
}

int main(int argc, char **argv)
{
int argn=init(argc, argv);
int rc;

	if (argn < 0)
	{
		exit(1);
	}
	if (accessarg && openaccess(accessarg))
		perror(accessarg);
	if (accesslocal && !accessarg)
		fprintf(stderr,"-accesslocal requires -access\n");
	rc=doit(argn, argc, argv);
	kill( -getpid(), SIGTERM);
	exit(rc);
	return (0);
}

#if 1

static void proxy()
{
}

#else

/*
** SOCKSv5 does not support wildcards binds, for now, so there's no
** reason to manually proxy anything, yet.
*/


/***************************************************************************

Manual proxy, to support SOCKS encryption.  Because encrypted connection to
the SOCKS server is supported transparently in libsocks5, we can't just
run the app, because we'll lose libsocks5's intercept of read/write, et al

***************************************************************************/

struct proxybuf {
	char buffer[BUFSIZ];
	char	*p;
	int buffered;
	int rfd, wfd;
	} ;

static void proxy_init(struct proxybuf *b, int r, int w)
{
	b->buffered=0;
	b->rfd=r;
	b->wfd=w;
}

static int proxy_setfd(struct proxybuf *b, fd_set *r, fd_set *w, int *max)
{
	/* If we have something buffered, write it out */

	if (b->buffered)
	{
		FD_SET(b->wfd, w);
		if (b->wfd > *max)	*max=b->wfd;
		return (1);
	}

	if (b->rfd < 0)
	{
		if (b->wfd >= 0)
		{
			sox_close(b->wfd);
			b->wfd= -1;
		}
		return (0);	/* Nothing else to do */
	}

	if (b->rfd > *max)	*max=b->rfd;
	FD_SET(b->rfd, r);
	return (1);
}

static void proxy_dofd(struct proxybuf *b, fd_set *r, fd_set *w)
{
	if (b->buffered)
	{
		if (FD_ISSET(b->wfd, w))
		{
		int	n=sox_write(b->wfd, b->p, b->buffered);

			if (n <= 0)
			{
				sox_close(b->rfd);
				sox_close(b->wfd);
				b->rfd=b->wfd= -1;
				b->buffered=0;
				return;
			}
			b->p += n;
			b->buffered -= n;
		}
		return;
	}

	if (b->rfd >= 0 && FD_ISSET(b->rfd, r))
	{
	int	n=sox_read(b->rfd, b->buffer, sizeof(b->buffer));

		if (n <= 0)
		{
			sox_close(b->rfd);
			sox_close(b->wfd);
			b->rfd=b->wfd= -1;
			b->buffered=0;
			return;
		}
		b->p = b->buffer;
		b->buffered=n;
	}
}

static void proxy_do(struct proxybuf *);

static void proxy()
{
int	pipefd0[2], pipefd1[2], pipefd2[2];
pid_t	p, p2;
int	waitstat;
struct proxybuf proxy_[3];

	if (!proxyarg)	return;

	if (pipe(pipefd0) || pipe(pipefd1) || pipe(pipefd2))
	{
		perror("pipe");
		exit(1);
	}

	p=fork();
	if (p == -1)
	{
		perror("fork");
		exit(1);
	}

	/*
	** The parent goes on its merry way, but first makes sure that the
	** child process is OK.
	*/

	if (p)
	{
		while ((p2=wait(&waitstat)) != p)
		{
			if (p2 == -1 && errno != EINTR)
			{
				perror("wait");
				exit(1);
			}
		}
		if (waitstat)
			exit(0);
		sox_close(0);
		sox_close(1);
		sox_close(2);
		errno=EINVAL;
		if (dup_and_check(pipefd0[0]) != 0 ||
			dup_and_check(pipefd1[1]) != 1 ||
			dup_and_check(pipefd2[1]) != 2)
		{
			perror("dup(app)");
			exit(1);
		}
		sox_close(pipefd0[0]);
		sox_close(pipefd0[1]);
		sox_close(pipefd1[0]);
		sox_close(pipefd1[1]);
		sox_close(pipefd2[0]);
		sox_close(pipefd2[1]);
		return;
	}

	p=fork();
	if (p == -1)	exit(1);
	if (p)	exit(0);

	sox_close(pipefd0[0]);
	sox_close(pipefd1[1]);
	sox_close(pipefd2[1]);

	proxy_init(&proxy_[0], 0, pipefd0[1]);
	proxy_init(&proxy_[1], pipefd1[0], 1);
	proxy_init(&proxy_[2], pipefd2[0], 2);
	proxy_do(proxy_);
	exit(0);
}

static void proxy_do(struct proxybuf *p)
{
fd_set	r, w;

	for (;;)
	{
	int	m=0;
	int	rc0, rc1, rc2;

		FD_ZERO(&r);
		FD_ZERO(&w);

		rc0=proxy_setfd(p, &r, &w, &m);
		rc1=proxy_setfd(p+1, &r, &w, &m);
		rc2=proxy_setfd(p+2, &r, &w, &m);

		if (rc0 == 0 && rc1 == 0 && rc2 == 0)
			break;

		if (rc0 == 0 || rc1 == 0 || rc2 == 0)
			alarm(10);

		if (select(m+1, &r, &w, 0, 0) < 0)
		{
			perror("select");
			break;
		}

		proxy_dofd(p, &r, &w);
		proxy_dofd(p+1, &r, &w);
		proxy_dofd(p+2, &r, &w);
	}
}
#endif
