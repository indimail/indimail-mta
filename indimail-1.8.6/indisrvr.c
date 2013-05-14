/*
 * $Log: indisrvr.c,v $
 * Revision 2.49  2013-05-15 00:43:01+05:30  Cprogrammer
 * fixed compilation warnings
 *
 * Revision 2.48  2012-09-24 19:21:18+05:30  Cprogrammer
 * removed pidfile
 *
 * Revision 2.47  2011-11-09 19:44:41+05:30  Cprogrammer
 * removed getversion
 *
 * Revision 2.46  2011-10-28 14:15:49+05:30  Cprogrammer
 * added auth_method argument to pw_comp
 *
 * Revision 2.45  2011-10-25 20:47:41+05:30  Cprogrammer
 * mgmtgetpass() function has a new status argument
 *
 * Revision 2.44  2011-04-08 17:26:35+05:30  Cprogrammer
 * added HAVE_CONFIG_H
 *
 * Revision 2.43  2011-04-03 20:11:14+05:30  Cprogrammer
 * BUG - getpid not called correctly
 *
 * Revision 2.42  2010-07-22 23:05:38+05:30  Cprogrammer
 * email changed to manvendra@indimail.org
 *
 * Revision 2.41  2010-03-06 14:55:02+05:30  Cprogrammer
 * return error code in ssl translate function correctly
 *
 * Revision 2.40  2009-11-16 21:45:03+05:30  Cprogrammer
 * fix compilation when HAVE_SSL is not defined
 *
 * Revision 2.39  2009-11-13 21:32:53+05:30  Cprogrammer
 * reload cert on SIGHUP
 *
 * Revision 2.38  2009-09-17 10:23:52+05:30  Cprogrammer
 * exit if certfile is missing
 *
 * Revision 2.37  2009-09-16 10:11:30+05:30  Cprogrammer
 * fixed compilation if HAVE_SSL was undefined
 *
 * Revision 2.36  2009-08-11 14:48:57+05:30  Cprogrammer
 * added TLS encryption
 *
 * Revision 2.35  2009-07-09 15:55:09+05:30  Cprogrammer
 * added ipv6 ready code
 *
 * Revision 2.34  2009-02-26 20:25:40+05:30  Cprogrammer
 * added SIGUSR2 to reset verbose flag
 *
 * Revision 2.33  2009-02-24 14:05:32+05:30  Cprogrammer
 * BUG - fixed incorrect passwords get matched (incorrect usage of pw_comp)
 *
 * Revision 2.32  2009-02-18 09:07:14+05:30  Cprogrammer
 * fixed fgets warning
 *
 * Revision 2.31  2009-02-06 11:38:01+05:30  Cprogrammer
 * ignore return value of fgets
 *
 * Revision 2.30  2008-09-11 22:49:06+05:30  Cprogrammer
 * use pw_comp for password comparision
 *
 * Revision 2.29  2008-07-13 19:44:17+05:30  Cprogrammer
 * port for Mac OS X
 * use ERESTART only if available
 *
 * Revision 2.28  2008-06-30 16:16:58+05:30  Cprogrammer
 * removed license code
 *
 * Revision 2.27  2008-06-03 19:45:07+05:30  Cprogrammer
 * conditional compilation of license code
 *
 * Revision 2.26  2008-05-28 16:35:53+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 2.25  2007-12-19 19:37:59+05:30  Cprogrammer
 * change in license file name
 *
 * Revision 2.24  2005-12-21 09:48:25+05:30  Cprogrammer
 * make gcc 4 happy
 *
 * Revision 2.23  2004-05-31 22:39:25+05:30  Cprogrammer
 * added description of adminclient protocol
 *
 * Revision 2.22  2004-05-10 18:20:38+05:30  Cprogrammer
 * added version check for license
 *
 * Revision 2.21  2003-09-16 12:32:52+05:30  Cprogrammer
 * bug fix - use MAX_BUF instead of sizeof
 * error message modified to include username
 *
 * Revision 2.20  2003-09-14 02:00:52+05:30  Cprogrammer
 * added role based administration through checkPerm()
 *
 * Revision 2.19  2003-07-25 09:25:15+05:30  Cprogrammer
 * change in checklicense() function()
 *
 * Revision 2.18  2003-07-21 14:14:07+05:30  Cprogrammer
 * added license verification
 *
 * Revision 2.17  2003-04-12 00:23:40+05:30  Cprogrammer
 * replaced admin_command with structure adminCommands
 *
 * Revision 2.16  2003-02-03 21:55:06+05:30  Cprogrammer
 * made stdin unbuffered
 *
 * Revision 2.15  2003-01-28 23:27:19+05:30  Cprogrammer
 * increased size of buffer for storing commands
 *
 * Revision 2.14  2002-12-26 04:34:57+05:30  Cprogrammer
 * corrected a potential problem with passwd comparision
 *
 * Revision 2.13  2002-12-11 10:28:40+05:30  Cprogrammer
 * added ERESTART check for errno
 *
 * Revision 2.12  2002-12-01 18:52:06+05:30  Cprogrammer
 * added missing usage statement
 *
 * Revision 2.11  2002-11-28 01:04:35+05:30  Cprogrammer
 * added pid in startup/shutdown messages
 *
 * Revision 2.10  2002-11-28 00:45:24+05:30  Cprogrammer
 * used getopt() to get arguments
 * added file descripter 3 for logging error messages
 * added shutdown and other diagnostic/verbose messages
 *
 * Revision 2.9  2002-09-27 19:12:20+05:30  Cprogrammer
 * removed is_already_running() as bind() will prevent multiple versions to run
 *
 * Revision 2.8  2002-09-01 19:45:00+05:30  Cprogrammer
 * added Startup Message
 *
 * Revision 2.7  2002-07-25 10:12:13+05:30  Cprogrammer
 * added removal of pidfile when indisrvr dies
 * added string RETURNSTATUS before the return code to distinguish normal messages from the return code
 *
 * Revision 2.6  2002-07-23 23:41:05+05:30  Cprogrammer
 * removed display of command being executed
 *
 * Revision 2.5  2002-07-23 22:57:47+05:30  Cprogrammer
 * added display of remote peer and commands executed
 *
 * Revision 2.4  2002-07-15 18:41:23+05:30  Cprogrammer
 * changes in hanshake between client and server
 *
 * Revision 2.3  2002-07-15 02:05:29+05:30  Cprogrammer
 * update login status and failure attempts on incorrect passwords
 *
 * Revision 2.2  2002-05-13 02:27:23+05:30  Cprogrammer
 * moved include files inside #ifdef CLUSTERED_SITE
 *
 * Revision 2.1  2002-05-12 01:21:06+05:30  Cprogrammer
 * called vcreate_mgmtaccess_table() to create table mgmtaccess
 *
 * Revision 1.6  2002-02-25 13:55:05+05:30  Cprogrammer
 * conditional compilation for CLUSTERED_SITE
 *
 * Revision 1.5  2002-02-24 22:46:06+05:30  Cprogrammer
 * added getversion
 *
 * Revision 1.4  2001-12-27 01:27:19+05:30  Cprogrammer
 * removed getversion_indisrvr_c
 *
 * Revision 1.3  2001-12-23 01:00:27+05:30  Cprogrammer
 * added conditional compilation for CLUSTERED_SITE
 *
 * Revision 1.2  2001-12-21 00:35:29+05:30  Cprogrammer
 * call vauth_open in case code not compiled with CLUSTERED_SITE
 *
 * Revision 1.1  2001-12-19 21:17:53+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "indimail.h"

#ifndef lint
static char     sccsid[] = "$Id: indisrvr.c,v 2.49 2013-05-15 00:43:01+05:30 Cprogrammer Stab mbhangui $";
#endif

#ifdef CLUSTERED_SITE
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ctype.h>
#ifdef ENABLE_IPV6
#include <netdb.h>
#endif
#ifdef HAVE_SSL
#include <openssl/ssl.h>
#include <openssl/err.h>
#endif
#include <unistd.h>

#define MAXBUF 4096

/*
 * adminclient Protocol
 *
 * s: "Login: "
 * c: "userid\n"
 * s: "Password: "
 * c: "password\n"
 * s: "OK\n"
 * c: "index command arg1 arg2 ...\n"
 * s: <output of above command if any>
 * c: "\n"
 * s: "RETURNSTATUS[return value of command]\n"
 *
 * e.g.
 *
 * Login: admin<lf>
 * Password: xxxxxxxx<lf>
 * OK
 * 7 vuserinfo -n manvendra@indimail.org<lf>
 * name          : manvendra@indimail.org
 * <lf>
 * RETURNSTATUS0
 *
 */
int             Login_User(char *, char *);
int             call_prg();
static void     SigChild(void);
static void     SigTerm();
static void     SigUsr();
static int      get_options(int argc, char **argv, char **, char **, int *);
#ifdef HAVE_SSL
static void     SigHup();
int             translate(SSL *, int, int, int, unsigned int);
#endif

#ifdef HAVE_SSL
static int      usessl = 0;
static char    *certfile;
SSL_CTX        *ctx = (SSL_CTX *) 0;
#endif

char            tbuf[2048];

#ifdef HAVE_SSL
static int
ssl_write(SSL *ssl, char *buf, int len)
{
	int             w;

	while (len)
	{
		if ((w = SSL_write(ssl, buf, len)) == -1)
		{
			if (errno == EINTR)
				continue;
			return -1;	/*- note that some data may have been written */
		}
		if (w == 0);	/*- luser's fault */
		buf += w;
		len -= w;
	}
	return 0;
}

int
translate(SSL *ssl, int out, int clearout, int clearerr, unsigned int iotimeout)
{
	fd_set          rfds;	/*- File descriptor mask for select -*/
	struct timeval  timeout;
	int             flagexitasap;
	int             sslin;
	int             retval, n, r;

	timeout.tv_sec = iotimeout;
	timeout.tv_usec = 0;
	flagexitasap = 0;
	if ((sslin = SSL_get_rfd(ssl)) == -1)
	{
		filewrt(3, "translate: unable to set up SSL connection\n");
		while ((n = ERR_get_error()))
			filewrt(3, "translate: %s\n", ERR_error_string(n, 0));
		return (-1);
	}
	if (SSL_accept(ssl) <= 0)
	{
		filewrt(3, "translate: unable to accept SSL connection\n");
		while ((n = ERR_get_error()))
			filewrt(3, "translate: %s\n", ERR_error_string(n, 0));
		return (-1);
	}
	while (!flagexitasap)
	{
		FD_ZERO(&rfds);
		FD_SET(sslin, &rfds);
		FD_SET(clearout, &rfds);
		FD_SET(clearerr, &rfds);
		if ((retval = select(clearerr > sslin ? clearerr + 1 : sslin + 1, &rfds, (fd_set *) NULL, (fd_set *) NULL, &timeout)) < 0)
		{
#ifdef ERESTART
			if (errno == EINTR || errno == ERESTART)
#else
			if (errno == EINTR)
#endif
				continue;
			filewrt(3, "translate: %s\n", strerror(errno));
			return (-1);
		} else
		if (!retval)
		{
			filewrt(3, "translate: timeout reached without input [%ld sec]\n", timeout.tv_sec);
			return (-1);
		}
		if (FD_ISSET(sslin, &rfds))
		{
			/*- data on sslin */
			if ((n = SSL_read(ssl, tbuf, sizeof(tbuf))) < 0)
			{
				filewrt(3, "translate: unable to read from network: %s\n", strerror(errno));
				flagexitasap = 1;
			} else
			if (n == 0)
				flagexitasap = 1;
			else
			if ((r = sockwrite(out, tbuf, n)) < 0)
			{
				filewrt(3, "translate: unable to write to client: %s\n", strerror(errno));
				return (-1);
			}
		}
		if (FD_ISSET(clearout, &rfds))
		{
			/*- data on clearout */
			if ((n = read(clearout, tbuf, sizeof(tbuf))) < 0)
			{
				filewrt(3, "translate: unable to read from client: %s\n", strerror(errno));
				return (-1);
			} else
			if (n == 0)
				flagexitasap = 1;
			if ((r = ssl_write(ssl, tbuf, n)) < 0)
			{
				filewrt(3, "translate: unable to write to network: %s\n", strerror(errno));
				return (-1);
			}
		}
		if (FD_ISSET(clearerr, &rfds))
		{
			/*- data on clearerr */
			if ((n = read(clearerr, tbuf, sizeof(tbuf))) < 0)
			{
				filewrt(3, "translate: unable to read from client: %s\n", strerror(errno));
				return (-1);
			} else
			if (n == 0)
				flagexitasap = 1;
			if ((r = ssl_write(ssl, tbuf, n)) < 0)
			{
				filewrt(3, "translate: unable to write to network: %s\n", strerror(errno));
				return (-1);
			}
		}
	} /*- while (!flagexitasap) */
	return (0);
}

SSL_CTX *
load_certificate(char *certfile)
{
	SSL_CTX        *myctx = (SSL_CTX *) 0;

    /* setup SSL context (load key and cert into ctx) */
	if (!(myctx = SSL_CTX_new(SSLv23_server_method())))
	{
		fprintf(stderr, "SSL_CTX_new: unable to create SSL context: %s\n",
			ERR_error_string(ERR_get_error(), 0));
		return ((SSL_CTX *) 0);
	}
	/* set prefered ciphers */
	if (getenv("SSL_CIPHER") && !SSL_CTX_set_cipher_list(myctx, getenv("SSL_CIPHER")))
	{
		fprintf(stderr, "SSL_CTX_set_cipher_list: unable to set cipher list: %s\n",
			ERR_error_string(ERR_get_error(), 0));
		SSL_CTX_free(myctx);
		return ((SSL_CTX *) 0);
	}
	if (SSL_CTX_use_certificate_chain_file(myctx, certfile))
	{
		if (SSL_CTX_use_RSAPrivateKey_file(myctx, certfile, SSL_FILETYPE_PEM) != 1)
		{
			fprintf(stderr, "SSL_CTX_use_RSAPrivateKey: unable to load RSA private key: %s\n",
				ERR_error_string(ERR_get_error(), 0));
			SSL_CTX_free(myctx);
			return ((SSL_CTX *) 0);
		}
		if (SSL_CTX_use_certificate_file(myctx, certfile, SSL_FILETYPE_PEM) != 1)
		{
			fprintf(stderr, "SSL_CTX_use_certificate_file: unable to load certificate: %s\n",
			ERR_error_string(ERR_get_error(), 0));
			SSL_CTX_free(myctx);
			return ((SSL_CTX *) 0);
		}
	}
	return (myctx);
}
#endif

int
main(argc, argv)
	int             argc;
	char          **argv;
{
	int             n, socket_desc, pid, backlog;
	char            pgname[MAXBUF];
	char           *port, *ipaddr;
	struct sockaddr_in cliaddress;
	int             addrlen, len, new;
	struct linger   linger;
#ifdef ENABLE_IPV6
	char            hostname[100], servicename[100];
#endif
#ifdef HAVE_SSL
	BIO            *sbio;
	SSL            *ssl;
	int             r, status, retval, pi1[2], pi2[2], pi3[2];
#endif

	if (get_options(argc, argv, &ipaddr, &port, &backlog))
		return(1);
	dup2(2, 3);
	snprintf(pgname, MAX_BUFF, "indiserver.%s%s", ipaddr, port);
	(void) signal(SIGTERM, SigTerm);
	(void) signal(SIGUSR2, SigUsr);
#ifdef HAVE_SSL
	if (usessl == 1)
	{
		if (access(certfile, F_OK))
		{
			fprintf(stderr, "missing certficate: %s: %s\n", certfile, strerror(errno));
			return (1);
		}
		(void) signal(SIGHUP, SigHup);
		SSL_library_init();
		if (!(ctx = load_certificate(certfile)))
			return (1);
	}
#endif
	linger.l_onoff = 1;
	linger.l_linger = 1;
	if ((socket_desc = tcpbind(ipaddr, port, backlog)) == -1)
	{
		fprintf(stderr, "tcpbind: %s\n", strerror(errno));
		return (1);
	}
	len = MAXBUF; /*- for setsockopt */
	addrlen = sizeof(cliaddress);
	(void) signal(SIGCHLD, (void (*)()) SigChild);
#ifdef HAVE_SSL
	filewrt(3, "%d: IndiServer Ready with Address %s:%s backlog %d SSL=%d\n", getpid(), ipaddr, port, backlog, usessl);
#else
	filewrt(3, "%d: IndiServer Ready with Address %s:%s backlog %d SSL=%d\n", getpid(), ipaddr, port, backlog, 0);
#endif
	close(0);
	close(1);
	for (;;)
	{
		if ((new = accept(socket_desc, (struct sockaddr *) &cliaddress, (socklen_t *) &addrlen)) == -1)
		{
			switch (errno)
			{
			case EINTR:
#ifdef ERESTART
			case ERESTART:
#endif
				continue;
			default:
				perror("accept");
				close(socket_desc);
				exit(1);
			}
			break;
		}
#ifdef ENABLE_IPV6
		if (getnameinfo((struct sockaddr *) &cliaddress, addrlen, hostname, sizeof(hostname), servicename, sizeof(servicename), 0) != 0)
			perror("getnameinfo");
		else
		filewrt(3, "%d: Connection from %s:%s\n", getpid(), hostname, servicename);
#else
		filewrt(3, "%d: Connection from %s:%d\n", getpid(), inet_ntoa(cliaddress.sin_addr), ntohs(cliaddress.sin_port));
#endif
		switch (pid = fork())
		{
		case -1:
			close(new);
			continue;
		case 0:
			(void) signal(SIGTERM, SIG_DFL);
			(void) signal(SIGCHLD, SIG_IGN);
			(void) signal(SIGHUP, SIG_IGN);
			close(socket_desc);
			if (setsockopt(new, SOL_SOCKET, SO_LINGER, (char *) &linger, sizeof(linger)) == -1)
			{
				close(new);
				exit(1);
			}
			for (;;)
			{
				if (setsockopt(new, SOL_SOCKET, SO_SNDBUF, (void *) &len, sizeof(int)) == -1)
				{
					if (errno == ENOBUFS)
					{
						usleep(1000);
						continue;
					}
					close(new);
					exit(1);
				}
				break;
			}
			for (;;)
			{
				if (setsockopt(new, SOL_SOCKET, SO_RCVBUF, (void *) &len, sizeof(int)) == -1)
				{
					if (errno == ENOBUFS)
					{
						usleep(1000);
						continue;
					}
					close(new);
					exit(1);
				}
				break;
			}
			dup2(new, 0);
			dup2(new, 1);
			dup2(new, 2);
			if (new != 0 && new != 1 && new != 2)
				close(new);
#ifdef HAVE_SSL
			if (usessl == 1)
			{
				if (pipe(pi1) != 0 || pipe(pi2) != 0 || pipe(pi3) != 0)
				{
					filewrt(3, "unable to create pipe: %s\n", strerror(errno));
					exit(1);
				}
				switch (fork())
				{
				case 0:
					close(pi1[1]);
					close(pi2[0]);
					close(pi3[0]);
					if (dup2(pi1[0], 0) == -1 || dup2(pi2[1], 1) == -1 || dup2(pi3[1], 2) == -1)
					{
						filewrt(3, "unable to set up descriptors: %s\n", strerror(errno));
						exit(1);
					}
					if (pi1[0] != 0)
						close(pi1[0]);
					if (pi2[1] != 1)
						close(pi2[1]);
					if (pi3[1] != 2)
						close(pi1[1]);
					/*
					 * signals are allready set in the parent 
					 */
					n = call_prg();
					close(0);
					close(1);
					close(2);
					close(3);
					_exit(n);
				case -1:
					filewrt(3, "%d: unable to fork: %s\n", getpid(), strerror(errno));
					exit(1);
				default:
					break;
				} /*- switch (fork()) */
				close(pi1[0]);
				close(pi2[1]);
				close(pi3[1]);
				if (!(ssl = SSL_new(ctx)))
				{
					long e;
					while ((e = ERR_get_error()))
						filewrt(3, "%d: %s\n", getpid(), ERR_error_string(e, 0));
					filewrt(3, "%d: unable to set up SSL session\n", getpid());
					SSL_CTX_free(ctx);
					_exit(1);
				}
				SSL_CTX_free(ctx);
				if (!(sbio = BIO_new_socket(0, BIO_NOCLOSE)))
				{
					filewrt(3, "%d: unable to set up BIO socket\n", getpid());
					_exit(1);
				}
				SSL_set_bio(ssl, sbio, sbio); /*- cannot fail */
				n = translate(ssl, pi1[1], pi2[0], pi3[0], 3600);
				SSL_shutdown(ssl);
				SSL_free(ssl);
				for (retval = -1;(r = waitpid(pid, &status, WNOHANG | WUNTRACED));)
				{
#ifdef ERESTART
					if (r == -1 && (errno == EINTR || errno == ERESTART))
#else
					if (r == -1 && errno == EINTR)
#endif
						continue;
					if (WIFSTOPPED(status) || WIFSIGNALED(status))
					{
						if (verbose)
							filewrt(3, "%d: killed by signal %d\n", pid, WIFSTOPPED(status) ? WSTOPSIG(status) : WTERMSIG(status));
						retval = -1;
					} else
					if (WIFEXITED(status))
					{
						retval = WEXITSTATUS(status);
						if (verbose)
							filewrt(3, "%d: normal exit return status %d\n", pid, retval);
					}
					break;
				} /*- for (; pid = waitpid(-1, &status, WNOHANG | WUNTRACED);) -*/
				close(0);
				close(1);
				close(2);
				close(3);
				if (n)
					_exit(n);
				if (retval)
					_exit(retval);
				_exit (0);
			} else
			{
				n = call_prg();
				close(0);
				close(1);
				close(2);
				close(3);
				_exit(n);
			}
#else
			n = call_prg();
			close(0);
			close(1);
			close(2);
			close(3);
			_exit(n);
#endif
		default:
			close(new);
			break;
		} /*- switch (pid = fork()) */
	} /*- for (;;) */
	exit(1);
}

int
call_prg()
{
	char           *ptr;
	char          **Argv;
	char            buffer[MAX_BUFF];
	int             i, status, cmdcount, retval;
	char            username[MAX_BUFF], pass[MAX_BUFF];
	pid_t           pid;

	if ((i = Login_User(username, pass)) == -1)
	{
		fprintf(stderr, "Temporary Authentication Problem\n");
		return (-1);
	} else
	if (i)
		return (1);
	if (!fgets(buffer, sizeof(buffer) - 2, stdin))
	{
		fprintf(stderr, "call_prg: read-cmdbuf: EOF\n");
		return (-1);
	}
	buffer[strlen(buffer) - 1] = 0;
	i = atoi(buffer);
	for (cmdcount = 0; adminCommands[cmdcount].name; cmdcount++);
	if (i > cmdcount || !(ptr = strchr(buffer, ' ')))
	{
		filewrt(2, "Incorrect Syntax %d[%s]\n", i, buffer);
		filewrt(3, "%d: Incorrect Syntax %d[%s]\n", getpid(), i, buffer);
		return (1);
	}
	ptr++;
	(void) signal(SIGCHLD, SIG_DFL);
	switch (pid = fork())
	{
	case -1:
		fprintf(stderr, "Fork failed: %s\n", strerror(errno));
		filewrt(3, "%d: Fork failed: %s\n", getpid(), strerror(errno));
		return(-1);
	case 0:
		(void) signal(SIGCHLD, SIG_DFL);
		if (!(Argv = MakeArgs(ptr)))
		{
			fprintf(stderr, "MakeArgs failed: %s\n", strerror(errno));
			filewrt(3, "%d: MakeArgs failed: %s\n", getpid(), strerror(errno));
			return(-1);
		}
		if (checkPerm(username, adminCommands[i].name, Argv))
		{
			fprintf(stderr, "%s: %s: %s: permission denied\n", username, adminCommands[i].name, ptr);
			filewrt(3, "%s: %s: %s: permission denied\n", username, adminCommands[i].name, ptr);
			exit(1);
		}
		if (verbose)
			filewrt(3, "%d: command %s Args %s\n", getpid(), adminCommands[i].name, ptr);
		execv(adminCommands[i].name, Argv);
		filewrt(3, "%d: %s: %s\n", getpid(), adminCommands[i].name, strerror(errno));
		exit(1);
	default:
		break;
	}
	for (retval = -1;;)
	{
		i = wait(&status);
#ifdef ERESTART
		if (i == -1 && (errno == EINTR || errno == ERESTART))
#else
		if (i == -1 && errno == EINTR)
#endif
			continue;
		else
		if (i == -1)
			break;
		if (WIFSTOPPED(status) || WIFSIGNALED(status))
		{
			if (verbose)
				filewrt(3, "%d: killed by signal %d\n", pid, WIFSTOPPED(status) ? WSTOPSIG(status) : WTERMSIG(status));
			retval = -1;
		} else
		if (WIFEXITED(status))
		{
			retval = WEXITSTATUS(status);
			if (verbose)
				filewrt(3, "%d: normal exit return status %d\n", pid, retval);
		}
		break;
	}
	if (!fgets(buffer, sizeof(buffer) - 2, stdin))
	{
		if (!feof(stdin))
			perror("fgets: stdin");
	}
	if (verbose)
		filewrt(3, "%d: return status %d\n", pid, retval);
	printf("RETURNSTATUS%d\n", retval);
	fflush(stdout);
	return (0);
}

/*
 * return to parent (call_prg) for:
 *  0 - for success
 *  & exit for the foll.
 *  1.) Mysql Prb , 2.) Password incorrect & 3.) If the user is not present
 */
int
Login_User(char *username, char *pass)
{
	char           *ptr, *admin_pass;

	setbuf(stdin, 0);
	printf("Login: ");
	fflush(stdout);
	if (!fgets(username, MAX_BUFF - 2, stdin))
	{
		fprintf(stderr, "EOF username\n");
		return(1);
	}
	for (ptr = username;*ptr;ptr++)
		if (isspace((int) *ptr))
			*ptr = 0;
	printf("Password: ");
	fflush(stdout);
	if (!fgets(pass, MAX_BUFF - 2, stdin))
	{
		fprintf(stderr, "EOF password\n");
		return(1);
	}
	for (ptr = pass;*ptr;ptr++)
	{
		if (isspace((int) *ptr))
			*ptr = 0;
	}
	if (isDisabled(username))
	{
		filewrt(3, "%d: user %s disabled\n", getpid(), username);
		fprintf(stderr, "You are disabled\n");
		return(1);
	}
	if (!(admin_pass = mgmtgetpass(username, 0)))
		return(1);
	if (*admin_pass && !pw_comp(0, (unsigned char *) admin_pass, 0,
		(unsigned char *) pass, 0))
	{
		printf("OK\n");
		fflush(stdout);
		filewrt(3, "%d: user %s logged in\n", getpid(), username);
		return (0);
	}
	filewrt(3, "%d: user %s password incorrect\n", getpid(), username);
	fprintf(stderr, "Password incorrect\n");
	updateLoginFailed(username);
	return (1);
}

static int
get_options(int argc, char **argv, char **ipaddr, char **port, int *backlog)
{
	int             c;

#ifdef HAVE_SSL
	certfile = 0;
#endif
	*ipaddr = *port = 0;
	*backlog = -1;
#ifdef HAVE_SSL
	while ((c = getopt(argc, argv, "vi:p:b:n:")) != -1) 
#else
	while ((c = getopt(argc, argv, "vi:p:b:")) != -1) 
#endif
	{
		switch (c)
		{
		case 'v':
			verbose = 1;
			break;
		case 'i':
			*ipaddr = optarg;
			break;
		case 'p':
			*port = optarg;
			break;
		case 'b':
			*backlog = atoi(optarg);
			break;
#ifdef HAVE_SSL
		case 'n':
			usessl = 1;
			certfile = optarg;
			break;
#endif
		default:
#ifdef HAVE_SSL
			fprintf(stderr, "USAGE: indisrvr -i ipaddr -p port -n certfile -b backlog\n");
#else
			fprintf(stderr, "USAGE: indisrvr -i ipaddr -p port -b backlog\n");
#endif
			break;
		}
	}
	if (!*ipaddr || !*port || !*backlog == -1)
	{
#ifdef HAVE_SSL
		fprintf(stderr, "USAGE: indisrvr -i ipaddr -p port -n certfile -b backlog\n");
#else
		fprintf(stderr, "USAGE: indisrvr -i ipaddr -p port -b backlog\n");
#endif
		return(1);
	}
	return(0);
}

static void
SigTerm()
{
	filewrt(3, "%d: IndiServer going down on SIGTERM\n", getpid());
	exit(0);
}

static void
SigChild(void)
{
	int             status;
	pid_t           pid;

	for (;(pid = waitpid(-1, &status, WNOHANG | WUNTRACED));)
	{
#ifdef ERESTART
		if (pid == -1 && (errno == EINTR || errno == ERESTART))
#else
		if (pid == -1 && errno == EINTR)
#endif
			continue;
		break;
	} /*- for (; pid = waitpid(-1, &status, WNOHANG | WUNTRACED);) -*/
	(void) signal(SIGCHLD, (void (*)()) SigChild);
	return;
}

static void
SigUsr(void)
{
	filewrt(3, "%d Resetting Verbose flag to %d\n", (int) getpid(), verbose ? 0 : 1);
	verbose = (verbose ? 0 : 1);
	(void) signal(SIGUSR2, (void(*)()) SigUsr);
	errno = EINTR;
	return;
}

#ifdef HAVE_SSL
static void
SigHup(void)
{
	filewrt(3, "%d: IndiServer received SIGHUP\n", getpid());
	ctx = load_certificate(certfile);
	(void) signal(SIGHUP, (void(*)()) SigHup);
	errno = EINTR;
	return;
}
#endif
#else
int
main()
{
	fprintf(stderr, "IndiMail not configured with --enable-user-cluster=y\n");
	return(1);
}
#endif

void
getversion_indisrvr_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
