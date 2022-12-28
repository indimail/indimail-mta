/*
 * $Id: sslerator.c,v 1.7 2022-12-28 17:52:17+05:30 Cprogrammer Exp mbhangui $
 */
#ifdef TLS
#include <unistd.h>
#include <signal.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <sgetopt.h>
#include <strerr.h>
#include <error.h>
#include <fmt.h>
#include <env.h>
#include <str.h>
#include <strmsg.h>
#include <scan.h>
#include <stralloc.h>
#include <wait.h>
#include <openreadclose.h>
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
static stralloc saciphers;

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
			if (errno == error_intr || errno == error_restart)
#else
			if (errno == error_intr)
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

SSL_CTX        *
set_tls_method(char *tls_method)
{
	SSL_CTX        *myctx;
#if OPENSSL_VERSION_NUMBER < 0x10100000L
	int             method = 4;	/*- (1 unused) 2 = SSLv23, 3=SSLv3, 4 = TLSv1, 5=TLSv1.1, 6=TLSv1.2 */
#else
	int             method = 0;	/*- (1,2 unused) 3=SSLv3, 4 = TLSv1, 5=TLSv1.1, 6=TLSv1.2, 7=TLSv1.3 */
#endif

	if (tls_method) {
#if OPENSSL_VERSION_NUMBER < 0x10100000L
		if (str_equal(tls_method, "SSLv23"))
			method = 2;
		else
		if (str_equal(tls_method, "SSLv3"))
			method = 3;
		else
#endif
		if (str_equal(tls_method, "TLSv1"))
			method = 4;
		else
		if (str_equal(tls_method, "TLSv1_1"))
			method = 5;
		else
		if (str_equal(tls_method, "TLSv1_2"))
			method = 6;
		else
		if (str_equal(tls_method, "TLSv1_3"))
			method = 7;
		else {
			strerr_warn1("set_tls_method: Invalid TLS method configured", 0);
#if OPENSSL_VERSION_NUMBER < 0x10100000L
			strerr_warn1("set_tls_method: Supported methods: SSLv23, SSLv3, TLSv1, TLSv1_1, TLSv1_2", 0);
#else
			strerr_warn1("set_tls_method: Supported methods: TLSv1, TLSv1_1, TLSv1_2 TLSv1_3", 0);
#endif
			return ((SSL_CTX *) NULL);
		}
	}
	SSL_library_init();
	/*- a new SSL context with the bare minimum of options */
#if OPENSSL_VERSION_NUMBER < 0x10100000L
	if (method == 2 && !(myctx = SSL_CTX_new(SSLv23_server_method()))) {
		strerr_warn1("set_tls_method: TLS not available: unable to initialize SSLv23 ctx", 0);
		return ((SSL_CTX *) NULL);
	} else
	if (method == 3 && !(myctx = SSL_CTX_new(SSLv3_server_method()))) {
		strerr_warn1("set_tls_method: TLS not available: unable to initialize SSLv3 ctx", 0);
		return ((SSL_CTX *) NULL);
	} else
#if defined(TLSV1_SERVER_METHOD) || defined(TLS1_VERSION)
	if (method == 4 && !(myctx = SSL_CTX_new(TLSv1_server_method()))) {
		strerr_warn1("set_tls_method: TLS not available: unable to initialize TLSv1 ctx", 0);
		return ((SSL_CTX *) NULL);
	} else
#else
	if (method == 4) {
		strerr_warn1("set_tls_method: TLS not available: TLSv1 method not available", 0);
		return ((SSL_CTX *) NULL);
	} else
#endif
#if defined(TLSV1_1_SERVER_METHOD) || defined(TLS1_1_VERSION)
	if (method == 5 && !(myctx = SSL_CTX_new(TLSv1_1_server_method()))) {
		strerr_warn1("set_tls_method: TLS not available: unable to initialize TLSv1_1 ctx", 0);
		return ((SSL_CTX *) NULL);
	} else
#else
	if (method == 5) {
		strerr_warn1("set_tls_method: TLS not available: TLSv1_1_server_method not available", 0);
		return ((SSL_CTX *) NULL);
	} else
#endif
#if defined(TLSV1_2_SERVER_METHOD) || defined(TLS1_2_VERSION)
	if (method == 6 && !(myctx = SSL_CTX_new(TLSv1_2_server_method()))) {
		strerr_warn1("set_tls_method: TLS not available: unable to initialize TLSv1_2 ctx", 0);
		return ((SSL_CTX *) NULL) ;
	} else
#else
	if (method == 6) {
		strerr_warn1("set_tls_method: TLS not available: TLSv1_2_server_method not available", 0);
		return ((SSL_CTX *) NULL) ;
	}
#endif
#else /*- OPENSSL_VERSION_NUMBER < 0x10100000L */
	myctx = SSL_CTX_new(TLS_server_method());
	/*-
	 * Currently supported versions are SSL3_VERSION, TLS1_VERSION, TLS1_1_VERSION,
	 * TLS1_2_VERSION, TLS1_3_VERSION for TLS
	 */
	switch (method)
	{
	case 2:
		strerr_warn1("set_tls_method: TLS not available: SSLv23_server_method not available", 0);
		return ((SSL_CTX *) NULL) ;
	case 3:
		SSL_CTX_set_min_proto_version(myctx, SSL3_VERSION);
		SSL_CTX_set_max_proto_version(myctx, SSL3_VERSION);
		break;
	case 4:
		SSL_CTX_set_min_proto_version(myctx, TLS1_VERSION);
		SSL_CTX_set_max_proto_version(myctx, TLS1_VERSION);
		break;
	case 5:
		SSL_CTX_set_min_proto_version(myctx, TLS1_1_VERSION);
		SSL_CTX_set_max_proto_version(myctx, TLS1_1_VERSION);
		break;
	case 6:
		SSL_CTX_set_min_proto_version(myctx, TLS1_2_VERSION);
		SSL_CTX_set_max_proto_version(myctx, TLS1_2_VERSION);
		break;
	}
	/*- POODLE Vulnerability */
	SSL_CTX_set_options(myctx, SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3);
#endif /*- OPENSSL_VERSION_NUMBER < 0x10100000L */
	return myctx;
}

SSL_CTX *
load_certificate(char *certfile, char *cafile, char *tls_method)
{
	SSL_CTX        *myctx = (SSL_CTX *) 0;

    /* setup SSL context (load key and cert into ctx) */
	if (!(myctx = set_tls_method(tls_method))) {
		strerr_warn3(FATAL, "SSL_CTX_new: unable to create SSL context: ",
			ERR_error_string(ERR_get_error(), 0), 0);
		return ((SSL_CTX *) 0);
	}
	if (SSL_CTX_use_certificate_chain_file(myctx, certfile) != 1) {
		strerr_warn5(FATAL, "SSL_CTX_use_certificate_chain_file: ", certfile, ": ",
				ERR_error_string(ERR_get_error(), 0), 0);
		SSL_CTX_free(myctx);
		return ((SSL_CTX *) 0);
	}
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
	if (SSL_CTX_use_PrivateKey_file(myctx, certfile, SSL_FILETYPE_PEM) != 1) {
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
	if (cafile && 1 != SSL_CTX_load_verify_locations(myctx, cafile, 0)) {
		strerr_warn4("SSL_CTX_load_verify_locations: Unable to use ca certificate: ",
				cafile, ": ", ERR_error_string(ERR_get_error(), 0), 0);
		SSL_CTX_free(myctx);
		return ((SSL_CTX *) NULL);
	}
	return (myctx);
}

static void
usage()
{
	strerr_warn1("usage: sslerator [options] prog [args]\n"
			"options\n"
			" [ -n certfile ]\n"
			" [ -c cipherfile ]\n"
			" [ -M TLS methods ]\n"
			" [ -C cafile ]\n"
			" [ -t timeout ]\n"
			" [ -f fd ]\n"
			" [ -s ] (starttls smtp)\n"
			" [ -v ] (verbose)\n"
			" prog [ args ]", 0);
	_exit(100);
}

static int
get_options(int argc, char **argv, char **certfile, char **cipherfile,
		char **cafile, char **tls_method, char **banner, int *timeout,
		int *err_to_net, int *fd, char ***pgargs)
{
	int             c;

	*cipherfile = *banner = *certfile = *cafile = *tls_method = 0;
	*timeout = DEFAULT_TIMEOUT;
	*err_to_net = 0;
	*fd = 0;
	while ((c = getopt(argc, argv, "vsn:f:t:b:c:C:")) != opteof) {
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
		case 'c':
			*cipherfile = optarg;
			break;
		case 'C':
			*cafile = optarg;
			break;
		case 'M':
			*tls_method = optarg;
			break;
		case 't':
			scan_int(optarg, timeout);
			break;
		default:
			usage();
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
	else
		usage();
	return (0);
}

int
main(int argc, char **argv)
{
	BIO            *sbio;
	char          **pgargs, *certfile, *cipherfile, *banner, *ciphers,
				   *cafile, *tls_method;
	char            strnum1[FMT_ULONG], strnum2[FMT_ULONG], tbuf[2048];
	int             status, werr, r, ret, n, sock, err_to_net, pid,
					pi1[2], pi2[2], pi3[2];
	struct stat     st;

	if (get_options(argc, argv, &certfile, &cipherfile, &cafile, &tls_method,
				&banner, &timeoutssl, &err_to_net, &sock, &pgargs))
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
	if (!(ctx = load_certificate(certfile, cafile, tls_method)))
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
	if (cipherfile) {
		if (lstat(cipherfile, &st) == -1)
			strerr_die4sys(111, FATAL, "lstat: ", cipherfile, ": ");
		if (openreadclose(cipherfile, &saciphers, st.st_size) == -1)
			strerr_die3sys(111, FATAL, cipherfile, ": ");
		if (saciphers.s[saciphers.len - 1] == '\n')
			saciphers.s[saciphers.len - 1] = 0;
		else
		if (!stralloc_0(&saciphers))
			strerr_die2x(111, FATAL, "out of memory");
		for (ciphers = saciphers.s; *ciphers; ciphers++)
			if (isspace(*ciphers)) {
				*ciphers = 0;
				break;
			}
		ciphers = saciphers.s;
	} else
	if (!(ciphers = env_get("TLS_CIPHER_LIST")))
		ciphers = "PROFILE=SYSTEM";
	if (!SSL_set_cipher_list(ssl, ciphers)) {
		strerr_warn5(FATAL, "unable to set ciphers: ", ciphers,
				": ", ERR_error_string(ERR_get_error(), 0), 0);
		SSL_free(ssl);
		_exit (111);
	}
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
	for (ret = -1;;) {
		if (!(r = waitpid(pid, &status, WUNTRACED|WCONTINUED)))
			break;
		else
		if (r == -1) {
#ifdef ERESTART
			if (errno == error_intr || errno == error_restart)
#else
			if (errno == error_intr)
#endif
				continue;
			if (verbose) {
				strnum1[fmt_ulong(strnum1, pid)] = 0;
				strerr_warn3(FATAL, strnum1, ": waitpid error: ", &strerr_sys);
			}
			break;
		}
		if (r != pid) {
			strnum1[fmt_ulong(strnum1, pid)] = 0;
			strerr_warn3(FATAL, strnum1, ": waitpid surprise: ", 0);
			break;
		}
		if (!(r = wait_handler(status, &werr))) {
			if (verbose) {
				strnum1[fmt_ulong(strnum1, pid)] = 0;
				strnum2[fmt_int(strnum2, werr)] = 0;
				strerr_warn4(FATAL, strnum1, wait_stopped(status) ? ": stopped by signal " : ": started by signal ", strnum2, 0);
			}
			continue;
		} else
		if (werr == -1) {
			if (verbose) {
				strnum1[fmt_ulong(strnum1, pid)] = 0;
				strerr_warn3(FATAL, strnum1, ": internal wait handler error", 0);
			}
		} else
		if (werr) {
			if (verbose) {
				strnum1[fmt_ulong(strnum1, pid)] = 0;
				strnum2[fmt_int(strnum2, WTERMSIG(status))] = 0;
				strerr_warn4(FATAL, strnum1, ": killed by signal ", strnum2, 0);
			}
		} else {
			if (verbose) {
				strnum1[fmt_ulong(strnum1, pid)] = 0;
				strnum2[fmt_int(strnum2, r)] = 0;
				strerr_warn4(FATAL, strnum1, ": normal exit. return status ", strnum2, 0);
			}
			ret = r;
		}
	} /*- for (; pid = waitpid(-1, &status, WNOHANG | WUNTRACED);) -*/
	if (n)
		_exit(n);
	if (ret)
		_exit (ret);
	_exit (0);
}
#else
#include <strerr.h>
#define FATAL           "sslerator: fatal: "
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
	static char    *x = "$Id: sslerator.c,v 1.7 2022-12-28 17:52:17+05:30 Cprogrammer Exp mbhangui $";

	x++;
}

/*
 * $Log: sslerator.c,v $
 * Revision 1.7  2022-12-28 17:52:17+05:30  Cprogrammer
 * refactored code
 * added options to specify cipher file, CA file and TLS method
 *
 * Revision 1.6  2022-12-18 12:30:06+05:30  Cprogrammer
 * handle wait status with details
 *
 * Revision 1.5  2022-08-21 17:59:24+05:30  Cprogrammer
 * fix compilation error when TLS is not defined in conf-tls
 *
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
