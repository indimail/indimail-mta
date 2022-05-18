/*
 * $Log: sslerator.c,v $
 * Revision 1.4  2022-05-18 13:30:24+05:30  Cprogrammer
 * openssl 3.0.0 port
 *
 * Revision 1.3  2021-03-04 23:02:07+05:30  Cprogrammer
 * fixed usage strings
 *
 * Revision 1.2  2021-03-02 10:41:58+05:30  Cprogrammer
 * renamed SSL_CIPHER to TLS_CIPHER_LIST
 *
 * Revision 1.1  2020-11-17 16:22:57+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <stdio.h>
#ifdef TLS
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <sgetopt.h>
#include <strerr.h>
#include <error.h>
#include <fmt.h>
#include <env.h>
#include <strmsg.h>
#include <scan.h>
#include <timeoutread.h>
#include <timeoutwrite.h>
#include "control.h"
#include "ssl_timeoutio.h"

#define FATAL           "sslerator: fatal: "
#define DEFAULT_TIMEOUT 300

int             translate(SSL *, int, int, int, int, unsigned int);

static int      usessl = 0, verbose = 0, sslin, sslout, timeoutssl, starttls_smtp = 0;
const char     *ssl_err_str = 0;
SSL_CTX        *ctx = (SSL_CTX *) 0;
SSL            *ssl = NULL;

const char     *
ssl_error()
{
	int             r = ERR_get_error();
	if (!r)
		return "unknown error";
	SSL_load_error_strings();
	return ERR_error_string(r, NULL);
}

const char     *
ssl_error_str()
{
	const char     *err = ssl_error();

	if (err)
		return err;
	if (!errno)
		return "unknown error";
	return (errno == error_timeout) ? "timed out" : error_str(errno);
}

ssize_t
saferead(int fd, char *buf, int len)
{
	int             r;

	if (usessl) {
		if ((r = ssl_timeoutread(timeoutssl, sslin, sslin, ssl, buf, len)) < 0)
			ssl_err_str = ssl_error_str();
	} else
		r = timeoutread(timeoutssl, sslin, buf, len);
	return r;
}

ssize_t
safewrite(int fd, char *buf, int len)
{
	int             r;

	if (usessl) {
		if ((r = ssl_timeoutwrite(timeoutssl, sslout, sslout, ssl, buf, len)) < 0)
			ssl_err_str = ssl_error_str();
	} else
		r = timeoutwrite(timeoutssl, sslout, buf, len);
	return r;
}

int
translate(SSL *myssl, int err_to_net, int out, int clearout, int clearerr, unsigned int iotimeout)
{
	fd_set          rfds;	/*- File descriptor mask for select -*/
	struct timeval  timeout;
	char            strnum[FMT_ULONG];
	int             flagexitasap, retval, n, r;
	char            tbuf[2048];

	timeout.tv_sec = iotimeout;
	timeout.tv_usec = 0;
	flagexitasap = 0;
	if (SSL_accept(myssl) <= 0) {
		strerr_warn2(FATAL, "translate: unable to accept SSL connection", 0);
		while ((n = ERR_get_error()))
			strerr_warn2(FATAL, ERR_error_string(n, 0), 0);
		return (1);
	}
	if ((sslin = SSL_get_rfd(myssl)) == -1 || (sslout = SSL_get_wfd(myssl)) == -1) {
		strerr_warn2(FATAL, "translate: unable to set up SSL connection", 0);
		while ((n = ERR_get_error()))
			strerr_warn2(FATAL, ERR_error_string(n, 0), 0);
		return (1);
	}
	while (!flagexitasap) {
		FD_ZERO(&rfds);
		FD_SET(sslin, &rfds);
		FD_SET(clearout, &rfds);
		FD_SET(clearerr, &rfds);
		if ((retval = select(clearerr > sslin ? clearerr + 1 : sslin + 1, &rfds, (fd_set *) NULL, (fd_set *) NULL, &timeout)) < 0) {
#ifdef ERESTART
			if (errno == EINTR || errno == ERESTART)
#else
			if (errno == EINTR)
#endif
				continue;
			strerr_warn2(FATAL, "translate: ", &strerr_sys);
			return (1);
		} else
		if (!retval) {
			strnum[fmt_ulong(strnum, timeout.tv_sec)] = 0;
			strerr_warn4(FATAL, "translate: timeout reached without input [", strnum, " sec]", 0);
			return (0);
		}
		if (FD_ISSET(clearout, &rfds)) {
			/*- data on clearout */
			if ((n = read(clearout, tbuf, sizeof(tbuf))) < 0) {
				strerr_warn2(FATAL, "translate: unable to read from client: ", &strerr_sys);
				return (1);
			} else
			if (n == 0)
				flagexitasap = 1;
			if ((r = safewrite(sslout, tbuf, n)) < 0) {
				strerr_warn2(FATAL, "translate: unable to write to network: ", &strerr_sys);
				return (1);
			}
		}
		if (FD_ISSET(clearerr, &rfds)) {
			/*- data on clearerr */
			if ((n = read(clearerr, tbuf, sizeof(tbuf))) < 0) {
				strerr_warn2(FATAL, "translate: unable to read from client: ", &strerr_sys);
				return (1);
			} else
			if (n == 0)
				flagexitasap = 1;
			if ((r = (err_to_net ? safewrite(sslout, tbuf, n) : write(2, tbuf, n))) < 0) {
				strerr_warn3(FATAL, "translate: unable to write to ",
						err_to_net ? "network: " : "stderr: ", &strerr_sys);
				return (1);
			}
		}
		if (FD_ISSET(sslin, &rfds)) {
			/*- data on sslin */
			if ((n = saferead(sslin, tbuf, sizeof(tbuf))) < 0) {
				strerr_warn3(FATAL, "translate: unable to read from network: ",
						ERR_error_string(SSL_get_error(myssl, n), 0), 0);
				flagexitasap = 1;
			} else
			if (n == 0)
				flagexitasap = 1;
			else
			if ((r = write(out, tbuf, n)) < 0) {
				strerr_warn2(FATAL, "translate: unable to write to client: ", &strerr_sys);
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
#ifdef CRYPTO_POLICY_NON_COMPLIANCE
	char           *ptr;
#endif

    /* setup SSL context (load key and cert into ctx) */
	if (!(myctx = SSL_CTX_new(SSLv23_server_method()))) {
		strerr_warn3(FATAL, "SSL_CTX_new: unable to create SSL context: ",
			ERR_error_string(ERR_get_error(), 0), 0);
		return ((SSL_CTX *) 0);
	}
	/* set prefered ciphers */
#ifdef CRYPTO_POLICY_NON_COMPLIANCE
	ptr = env_get("TLS_CIPHER_LIST");
	if (ptr && !SSL_CTX_set_cipher_list(myctx, ptr)) {
		strer_warn3("SSL_CTX_set_cipher_list: unable to set cipher list: ", ptr,
			ERR_error_string(ERR_get_error(), 0), 0);
		SSL_CTX_free(myctx);
		return ((SSL_CTX *) 0);
	}
#endif
	if (SSL_CTX_use_certificate_chain_file(myctx, certfile)) {
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
		if (SSL_CTX_use_PrivateKey_file(ctx, certfile, SSL_FILETYPE_PEM) != 1) {
#else
		if (SSL_CTX_use_RSAPrivateKey_file(myctx, certfile, SSL_FILETYPE_PEM) != 1) {
#endif
			strerr_warn3(FATAL, "SSL_CTX_use_RSAPrivateKey: unable to load RSA private key: ",
				ERR_error_string(ERR_get_error(), 0), 0);
			SSL_CTX_free(myctx);
			return ((SSL_CTX *) 0);
		}
		if (SSL_CTX_use_certificate_file(myctx, certfile, SSL_FILETYPE_PEM) != 1) {
			strerr_warn3(FATAL, "SSL_CTX_use_certificate_file: unable to load certificate: ",
			ERR_error_string(ERR_get_error(), 0), 0);
			SSL_CTX_free(myctx);
			return ((SSL_CTX *) 0);
		}
	}
	return (myctx);
}

static int
get_options(int argc, char **argv, char **certfile, char **banner, int *timeout, int *err_to_net, int *fd, char ***pgargs)
{
	int             c;

	*banner = *certfile = 0;
	*timeout = DEFAULT_TIMEOUT;
	*err_to_net = 0;
	*fd = 0;
	while ((c = getopt(argc, argv, "vsn:f:t:b:")) != opteof) {
		switch (c)
		{
		case 'v':
			verbose = 1;
			break;
		case 'e':
			*err_to_net = 1;
			break;
		case 's':
			starttls_smtp = 1;
			break;
		case 'b':
			*banner = optarg;
		case 'f':
			scan_int(optarg, fd);
			break;
		case 'n':
			*certfile = optarg;
			break;
		case 't':
			scan_int(optarg, timeout);
			break;
		default:
			strerr_warn1("usage: sslerator [-n certfile][-t timeout][-s][-f fd][-v] prog [args]", 0);
			strerr_warn1("       s - starttls smtp", 0);
			strerr_warn1("       v - verbose", 0);
			return (1);
		}
	}
	if (!*certfile)
		*certfile = env_get("TLS_CERTFILE");
	if (*certfile && **certfile)
		usessl = 1;
	if (env_get("TCPREMOTEIP"))
		*err_to_net = 0;
	if (optind < argc)
		*pgargs = argv + optind;
	else {
		strerr_warn1("usage: sslerator [-n certfile][-t timeout][-s][-f fd][-v] prog [args]", 0);
		strerr_warn1("       s - starttls smtp", 0);
		strerr_warn1("       v - verbose", 0);
		return (1);
	}
	return (0);
}

int
main(argc, argv)
	int             argc;
	char          **argv;
{
	BIO            *sbio;
	char          **pgargs, *ptr, *certfile, *banner;
	char            strnum1[FMT_ULONG], strnum2[FMT_ULONG], tbuf[2048];
	int             status, r, ret, n, sock, err_to_net, pid, pi1[2], pi2[2], pi3[2];

	if (get_options(argc, argv, &certfile, &banner, &timeoutssl, &err_to_net, &sock, &pgargs))
		_exit (100);
	if (timeoutssl == DEFAULT_TIMEOUT && control_readint(&timeoutssl, "timeoutremote") == -1)
		strerr_die2sys(111, FATAL, "timeoutremote: ");
	if (usessl == 0) {
		execv(pgargs[0], pgargs);
		strerr_die4sys(111, FATAL, "execv: ", pgargs[0], ": ");
	}
	if (pipe(pi1) != 0 || pipe(pi2) != 0 || pipe(pi3) != 0)
		strerr_die2sys(111, FATAL, "unable to create pipe: ");
	switch (pid = fork())
	{
		case -1:
			strerr_die2sys(111, FATAL, "fork: ");
			return (1);
		case 0:
			close(pi1[1]);
			close(pi2[0]);
			close(pi3[0]);
			if (dup2(pi1[0], 0) == -1 || dup2(pi2[1], 1) == -1 || dup2(pi3[1], 2) == -1) {
				strerr_die2sys(111, FATAL, "unable to set up descriptors: ");
				_exit(1);
			}
			if (pi1[0] != 0)
				close(pi1[0]);
			if (pi2[1] != 1)
				close(pi2[1]);
			if (pi3[1] != 2)
				close(pi1[1]);
			env_put("SSLERATOR=1");
			execv(pgargs[0], pgargs);
			strerr_die4sys(111, FATAL, "execv: ", pgargs[0], ": ");
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
		strerr_die4sys(111, FATAL, "missing certficate: ", certfile, ": ");
	SSL_library_init();
	if (!(ctx = load_certificate(certfile)))
		_exit (111);
	if (!(ssl = SSL_new(ctx))) {
		long e;
		strnum1[fmt_ulong(strnum1, getpid())] = 0;
		while ((e = ERR_get_error()))
			strerr_warn4(FATAL, strnum1, ": ", ERR_error_string(e, 0), 0);
		SSL_CTX_free(ctx);
		_exit(111);
	}
	SSL_CTX_free(ctx);
#ifndef CRYPTO_POLICY_NON_COMPLIANCE
	if (!(ptr = env_get("TLS_CIPHER_LIST")))
		ptr = "PROFILE=SYSTEM";
	if (!SSL_set_cipher_list(ssl, ptr)) {
		strerr_warn5(FATAL, "unable to set ciphers: ", ptr,
				": ", ERR_error_string(ERR_get_error(), 0), 0);
		SSL_free(ssl);
		_exit (111);
	}
#endif
	if (!(sbio = BIO_new_socket(sock, BIO_NOCLOSE))) {
		strnum1[fmt_ulong(strnum1, getpid())] = 0;
		strerr_warn3(FATAL, strnum1, ": unable to set up BIO socket", 0);
		SSL_free(ssl);
		_exit(111);
	}
	SSL_set_bio(ssl, sbio, sbio); /*- cannot fail */
	if (starttls_smtp) { /*- special case for SMTP */
		if ((r = timeoutread(timeoutssl, pi2[0], tbuf, sizeof(tbuf) - 1)) == -1) {
  			if (errno == error_timeout)
				strerr_warn1("454 timed out: greeting failed\r\n", 0);
			else
				strerr_warn1("454 greeting failed: ", &strerr_sys);
			SSL_free(ssl);
			_exit(111);
		}
		/*- tbuf[r] = 0; -*/
		scan_int(tbuf, &n);
		/*- strmsg_out1(tbuf); -*/
		if (write(1, tbuf, r) != r) {
			strerr_warn1("454 write failed: ", &strerr_sys);
			SSL_free(ssl);
			_exit(111);
		}
		if (n != 220) {
			SSL_free(ssl);
			_exit(n);
		}
	} else
	if (banner || (banner = env_get("BANNER")))
		strmsg_out2(banner, "\n");
	n = translate(ssl, err_to_net, pi1[1], pi2[0], pi3[0], timeoutssl);
	SSL_shutdown(ssl);
	SSL_free(ssl);
	ssl = 0;
	for (ret = -1;(r = waitpid(pid, &status, WNOHANG | WUNTRACED));) {
#ifdef ERESTART
		if (r == -1 && (errno == EINTR || errno == ERESTART))
#else
		if (r == -1 && errno == EINTR)
#endif
			continue;
		if (WIFSTOPPED(status) || WIFSIGNALED(status)) {
			if (verbose) {
				strnum1[fmt_ulong(strnum1, getpid())] = 0;
				strnum2[fmt_int(strnum2, WIFSTOPPED(status) ? WSTOPSIG(status) : WTERMSIG(status))] = 0;
				strerr_warn4(FATAL, strnum1, ": killed by signal ", strnum2, 0);
			}
			ret = -1;
		} else
		if (WIFEXITED(status)) {
			ret = WEXITSTATUS(status);
			if (verbose) {
				strnum1[fmt_ulong(strnum1, getpid())] = 0;
				strnum2[fmt_int(strnum2, ret)] = 0;
				strerr_warn4(FATAL, strnum1, ": normal exit return status ", strnum2, 0);
			}
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
	strerr_warn2(FATAL, "SSL support not detected. HAVE_SSL not defined", 0);
	return (1);
}
#endif

void
getversion_sslerator_c()
{
	static char    *x = "$Id: sslerator.c,v 1.4 2022-05-18 13:30:24+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
