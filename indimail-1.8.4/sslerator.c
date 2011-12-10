/*
 * $Log: sslerator.c,v $
 * Revision 2.2  2010-04-15 12:49:45+05:30  Cprogrammer
 * include string.h for Mac OS X
 *
 * Revision 2.1  2010-03-06 16:10:18+05:30  Cprogrammer
 * ssl enabler utility
 *
 */
#include "indimail.h"

#ifndef lint
static char     sccsid[] = "$Id: sslerator.c,v 2.2 2010-04-15 12:49:45+05:30 Cprogrammer Stab mbhangui $";
#endif

#ifdef HAVE_SSL
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#define MAXBUF 4096

int             translate(SSL *, int, int, int, int, unsigned int);
static int      get_options(int, char **, int *, char ***);

static int      usessl = 0;
static char    *certfile;
SSL_CTX        *ctx = (SSL_CTX *) 0;

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
translate(SSL *ssl, int err_to_net, int out, int clearout, int clearerr, unsigned int iotimeout)
{
	fd_set          rfds;	/*- File descriptor mask for select -*/
	struct timeval  timeout;
	int             flagexitasap;
	int             sslin;
	int             retval, n, r;
	char            tbuf[2048];

	timeout.tv_sec = iotimeout;
	timeout.tv_usec = 0;
	flagexitasap = 0;
	if (SSL_accept(ssl) <= 0)
	{
		fprintf(stderr, "translate: unable to accept SSL connection\n");
		while ((n = ERR_get_error()))
			fprintf(stderr, "sslerator: %s\n", ERR_error_string(n, 0));
		return (1);
	}
	if ((sslin = SSL_get_rfd(ssl)) == -1)
	{
		fprintf(stderr, "translate: unable to set up SSL connection\n");
		while ((n = ERR_get_error()))
			fprintf(stderr, "sslerator: %s\n", ERR_error_string(n, 0));
		return (1);
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
			fprintf(stderr, "translate: %s\n", strerror(errno));
			return (1);
		} else
		if (!retval)
		{
			fprintf(stderr, "translate: timeout reached without input [%ld sec]\n", timeout.tv_sec);
			return (0);
		}
		if (FD_ISSET(sslin, &rfds))
		{
			/*- data on sslin */
			if ((n = SSL_read(ssl, tbuf, sizeof(tbuf))) < 0)
			{
				fprintf(stderr, "translate: unable to read from network: %s\n", strerror(errno));
				flagexitasap = 1;
			} else
			if (n == 0)
				flagexitasap = 1;
			else
			if ((r = sockwrite(out, tbuf, n)) < 0)
			{
				fprintf(stderr, "translate: unable to write to client: %s\n", strerror(errno));
				return (1);
			}
		}
		if (FD_ISSET(clearout, &rfds))
		{
			/*- data on clearout */
			if ((n = read(clearout, tbuf, sizeof(tbuf))) < 0)
			{
				fprintf(stderr, "translate: unable to read from client: %s\n", strerror(errno));
				return (1);
			} else
			if (n == 0)
				flagexitasap = 1;
			if ((r = ssl_write(ssl, tbuf, n)) < 0)
			{
				fprintf(stderr, "translate: unable to write to network: %s\n", strerror(errno));
				return (1);
			}
		}
		if (FD_ISSET(clearerr, &rfds))
		{
			/*- data on clearerr */
			if ((n = read(clearerr, tbuf, sizeof(tbuf))) < 0)
			{
				fprintf(stderr, "translate: unable to read from client: %s\n", strerror(errno));
				return (1);
			} else
			if (n == 0)
				flagexitasap = 1;
			if ((r = (err_to_net ? ssl_write(ssl, tbuf, n) : sockwrite(2, tbuf, n))) < 0)
			{
				fprintf(stderr, "translate: unable to write to %s: %s\n", 
						err_to_net ? "network" : "stderr", strerror(errno));
				return (1);
			}
		}
	}
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

static int
get_options(int argc, char **argv, int *err_to_net, char ***pgargs)
{
	int             c;

	certfile = 0;
	*err_to_net = 0;
	while ((c = getopt(argc, argv, "vn:")) != -1) 
	{
		switch (c)
		{
		case 'v':
			verbose = 1;
			break;
		case 'e':
			*err_to_net = 1;
			break;
		case 'n':
			certfile = optarg;
			break;
		default:
			fprintf(stderr, "USAGE: sslerator [-t][-n certfile][-v] prog [args]\n");
			return (1);
		}
	}
	if (!certfile)
		certfile = getenv("TLS_CERTFILE");
	if (certfile && *certfile)
		usessl = 1;
	if (getenv("TCPREMOTEIP"))
		*err_to_net = 0;
	if (optind < argc)
		*pgargs = argv + optind;
	else
	{
		fprintf(stderr, "USAGE: sslerator [-t][-n certfile][-v] prog [args]\n");
		return (1);
	}
	return(0);
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

int
main(argc, argv)
	int             argc;
	char          **argv;
{
	BIO            *sbio;
	SSL            *ssl;
	char          **pgargs, *ptr;
	int             status, r, ret, n, err_to_net, pid, pi1[2], pi2[2], pi3[2];

	if(get_options(argc, argv, &err_to_net, &pgargs))
		return(1);
	if (usessl == 0)
	{
		execv(pgargs[0], pgargs);
		fprintf(stderr, "execv: %s: %s\n", pgargs[0], strerror(errno));
		return(1);
	}
	(void) signal(SIGCHLD, (void (*)()) SigChild);
	if (pipe(pi1) != 0 || pipe(pi2) != 0 || pipe(pi3) != 0)
	{
		fprintf(stderr, "sslerator: unable to create pipe: %s\n", strerror(errno));
		exit(1);
	}
	switch (pid = fork())
	{
		case -1:
			fprintf(stderr, "sslerator: fork: %s\n", strerror(errno));
			return (1);
		case 0:
			close(pi1[1]);
			close(pi2[0]);
			close(pi3[0]);
			if (dup2(pi1[0], 0) == -1 || dup2(pi2[1], 1) == -1 || dup2(pi3[1], 2) == -1)
			{
				fprintf(stderr, "unable to set up descriptors: %s\n", strerror(errno));
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
			putenv("SSLERATOR=1");
			execv(pgargs[0], pgargs);
			fprintf(stderr, "execv: %s: %s\n", pgargs[0], strerror(errno));
			close(0);
			close(1);
			close(2);
			_exit(1);
			break;
		default:
			break;
	} /*- switch (pid = fork()) */
	close(pi1[0]);
	close(pi2[1]);
	close(pi3[1]);
	if (access(certfile, F_OK))
	{
		fprintf(stderr, "missing certficate: %s: %s\n", certfile, strerror(errno));
		return (1);
	}
	SSL_library_init();
	if (!(ctx = load_certificate(certfile)))
		return (1);
	if (!(ssl = SSL_new(ctx)))
	{
		long e;
		while ((e = ERR_get_error()))
			fprintf(stderr, "%d: %s\n", getpid(), ERR_error_string(e, 0));
		fprintf(stderr, "%d: unable to set up SSL session\n", getpid());
		SSL_CTX_free(ctx);
		_exit(1);
	}
	SSL_CTX_free(ctx);
	if (!(sbio = BIO_new_socket(0, BIO_NOCLOSE)))
	{
		fprintf(stderr, "%d: unable to set up BIO socket\n", getpid());
		_exit(1);
	}
	SSL_set_bio(ssl, sbio, sbio); /*- cannot fail */
	if ((ptr = getenv("BANNER")))
	{
		puts(ptr);
		fflush(stdout);
	}
	n = translate(ssl, err_to_net, pi1[1], pi2[0], pi3[0], 3600);
	SSL_shutdown(ssl);
	SSL_free(ssl);
	for (ret = -1;(r = waitpid(pid, &status, WNOHANG | WUNTRACED));)
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
				fprintf(stderr, "%d: killed by signal %d\n", pid, WIFSTOPPED(status) ? WSTOPSIG(status) : WTERMSIG(status));
			ret = -1;
		} else
		if (WIFEXITED(status))
		{
			ret = WEXITSTATUS(status);
			if (verbose)
				fprintf(stderr, "%d: normal exit return status %d\n", pid, ret);
		}
		break;
	} /*- for (; pid = waitpid(-1, &status, WNOHANG | WUNTRACED);) -*/
	if (n)
		_exit(n);
	if (ret)
		_exit (ret);
	_exit (0);
}
#else
int
main(argc, argv)
	int             argc;
	char          **argv;
{
	fprintf(stderr, "SSL support not detected. HAVE_SSL not defined\n");
	return (1);
}
#endif

void
getversion_sslator_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
