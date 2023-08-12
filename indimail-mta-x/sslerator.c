/*
 * $Id: sslerator.c,v 1.11 2023-08-13 00:43:40+05:30 Cprogrammer Exp mbhangui $
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
#include <tls.h>
#include <strmsg.h>
#include <scan.h>
#include <stralloc.h>
#include <wait.h>
#include <openreadclose.h>
#include <timeoutread.h>
#include <timeoutwrite.h>
#include "control.h"
#include "auto_sysconfdir.h"

#define FATAL           "sslerator: fatal: "
#define DEFAULT_TIMEOUT 300

static int      usessl = 0, verbose = 0, sslin, sslout, timeoutconn,
				timeoutdata, starttls_smtp = 0;
#ifdef SSL_OP_ALLOW_CLIENT_RENEGOTIATION
static int      client_renegotiation;
#endif
const char     *ssl_err_str = 0;
SSL_CTX        *ctx = (SSL_CTX *) 0;
SSL            *ssl = NULL;
static stralloc saciphers;
static stralloc certdir_s;

static void
usage()
{
	strerr_warn1("usage: sslerator [options] prog [args]\n"
			"options\n"
		 	" [ -d certdir ]\n"
			" [ -n certfile ]\n"
			" [ -c cipherfile ]\n"
			" [ -M TLS methods ]\n"
			" [ -C cafile ]\n"
			" [ -r crlfile ]\n"
			" [ -t timeoutdata ]\n"
			" [ -T timeoutconn ]\n"
			" [ -f fd ]\n"
#ifdef SSL_OP_ALLOW_CLIENT_RENEGOTIATION
			" [ -N ] allow client-side renegotiation\n"
#endif
			" [ -s ] (starttls smtp)\n"
			" [ -v ] (verbose)\n"
			" prog [ args ]", 0);
	_exit(100);
}

ssize_t
saferead(int fd, char *buf, int len)
{
	return (tlsread(fd, buf, len, timeoutdata));
}

ssize_t
safewrite(int fd, char *buf, int len)
{
	return (tlswrite(fd, buf, len, timeoutdata));
}

int
sslio(SSL *myssl, int err_to_net, int out, int clearout, int clearerr, unsigned int iotimeout)
{
	fd_set          rfds;	/*- File descriptor mask for select -*/
	struct timeval  timeout;
	char            strnum[FMT_ULONG];
	int             flagexitasap, retval, n, r;
	char            tbuf[2048];

	timeout.tv_sec = iotimeout;
	timeout.tv_usec = 0;
	flagexitasap = 0;
	if ((sslin = SSL_get_rfd(myssl)) == -1 || (sslout = SSL_get_wfd(myssl)) == -1) {
		strerr_warn2(FATAL, "sslio: unable to set up SSL connection", 0);
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
			strerr_warn2(FATAL, "sslio: ", &strerr_sys);
			return (1);
		} else
		if (!retval) {
			strnum[fmt_ulong(strnum, timeout.tv_sec)] = 0;
			strerr_warn4(FATAL, "sslio: timeout reached without input [", strnum, " sec]", 0);
			return (0);
		}
		if (FD_ISSET(clearout, &rfds)) {
			/*- data on clearout */
			if ((n = read(clearout, tbuf, sizeof(tbuf))) < 0) {
				strerr_warn2(FATAL, "sslio: unable to read from client: ", &strerr_sys);
				return (1);
			} else
			if (n == 0)
				flagexitasap = 1;
			if ((r = safewrite(sslout, tbuf, n)) < 0) {
				strerr_warn2(FATAL, "sslio: unable to write to network: ", &strerr_tls);
				return (1);
			}
		}
		if (FD_ISSET(clearerr, &rfds)) {
			/*- data on clearerr */
			if ((n = read(clearerr, tbuf, sizeof(tbuf))) < 0) {
				strerr_warn2(FATAL, "sslio: unable to read from client: ", &strerr_sys);
				return (1);
			} else
			if (n == 0)
				flagexitasap = 1;
			if ((r = (err_to_net ? safewrite(sslout, tbuf, n) : write(2, tbuf, n))) < 0) {
				strerr_warn3(FATAL, "sslio: unable to write to ",
						err_to_net ? "network: " : "stderr: ", &strerr_tls);
				return (1);
			}
		}
		if (FD_ISSET(sslin, &rfds)) {
			/*- data on sslin */
			if ((n = saferead(sslin, tbuf, sizeof(tbuf))) < 0) {
				strerr_warn2(FATAL, "sslio: unable to read from network: ", &strerr_tls);
				flagexitasap = 1;
			} else
			if (n == 0)
				flagexitasap = 1;
			else
			if ((r = write(out, tbuf, n)) < 0) {
				strerr_warn2(FATAL, "sslio: unable to write to client: ", &strerr_sys);
				return (1);
			}
		}
	}
	return (0);
}

static int
get_options(int argc, char **argv, char **certdir, char **certfile,
		char **cipherfile, char **cafile, char **crlfile, char **tls_method,
		char **banner, int *err_to_net, int *fd, char ***pgargs)
{
	int             c;

	*certdir = *cipherfile = *banner = *certfile = *cafile = *crlfile = *tls_method = 0;
	timeoutdata = DEFAULT_TIMEOUT;
	timeoutconn = 60;
	*err_to_net = 0;
	*fd = 0;
#ifdef SSL_OP_ALLOW_CLIENT_RENEGOTIATION
	while ((c = getopt(argc, argv, "vsNd:n:f:t:T:b:c:C:r:")) != opteof) {
#else
	while ((c = getopt(argc, argv, "vsd:n:f:t:T:b:c:C:r:")) != opteof) {
#endif
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
		case 'd':
			*certdir = optarg;
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
		case 'r':
			*crlfile = optarg;
			break;
		case 'M':
			*tls_method = optarg;
			break;
#ifdef SSL_OP_ALLOW_CLIENT_RENEGOTIATION
		case 'N':
			client_renegotiation = 1;
			break;
#endif
		case 't':
			scan_int(optarg, &timeoutdata);
			break;
		case 'T':
			scan_int(optarg, &timeoutconn);
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
	char          **pgargs, *certfile, *cipherfile, *banner, *ciphers,
				   *cafile, *crlfile, *certdir, *tls_method;
	char            strnum1[FMT_ULONG], strnum2[FMT_ULONG], tbuf[2048];
	int             status, werr, r, ret, n, sock, err_to_net, pid,
					pi1[2], pi2[2], pi3[2];
	struct stat     st;

	if (get_options(argc, argv, &certdir, &certfile, &cipherfile, &cafile, &crlfile,
				&tls_method, &banner, &err_to_net, &sock, &pgargs))
		_exit (100);
	if (timeoutdata == DEFAULT_TIMEOUT && control_readint(&timeoutdata, "timeoutremote") == -1)
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
	if (!certdir && !(certdir = env_get("CERTDIR"))) {
		if (!stralloc_copys(&certdir_s, auto_sysconfdir) ||
				!stralloc_catb(&certdir_s, "/certs", 6) ||
				!stralloc_0(&certdir_s))
			strerr_die2x(111, FATAL, "out of memory");
		certdir = certdir_s.s;
	}
	set_certdir(certdir);
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

	if (starttls_smtp) { /*- special case for SMTP */
		if ((r = timeoutread(timeoutdata, pi2[0], tbuf, sizeof(tbuf) - 1)) == -1) {
  			if (errno == error_timeout)
				strerr_warn1("454 timed out: greeting failed\r\n", 0);
			else
				strerr_warn1("454 greeting failed: ", &strerr_sys);
			SSL_free(ssl);
			_exit(111);
		}
		scan_int(tbuf, &n);
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
	if (!(ctx = tls_init(tls_method, certfile, cafile , crlfile,
			ciphers, server)))
		strerr_die2x(111, FATAL, "unable to set initialize TLS");
#ifdef SSL_OP_ALLOW_CLIENT_RENEGOTIATION
	if (client_renegotiation)
		SSL_CTX_set_options(ctx, SSL_OP_ALLOW_CLIENT_RENEGOTIATION);
#endif
	if (!(ssl = tls_session(ctx, sock)))
		strerr_die2x(111, FATAL, "unable to create TLS session");
	SSL_CTX_free(ctx);
	ctx = NULL;
	if (tls_accept(timeoutconn, sock, sock, ssl))
		strerr_die2x(111, FATAL, "unable to accept SSL connection");
	n = sslio(ssl, err_to_net, pi1[1], pi2[0], pi3[0], timeoutdata);
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
		if (!(r = wait_handler(status, &werr)) && werr) {
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
		break;
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
	static char    *x = "$Id: sslerator.c,v 1.11 2023-08-13 00:43:40+05:30 Cprogrammer Exp mbhangui $";

	x++;
}

/*
 * $Log: sslerator.c,v $
 * Revision 1.11  2023-08-13 00:43:40+05:30  Cprogrammer
 * use strerr_tls for ssl/tls error
 *
 * Revision 1.10  2023-03-26 08:23:16+05:30  Cprogrammer
 * fixed code for wait_handler
 *
 * Revision 1.9  2023-01-11 08:18:09+05:30  Cprogrammer
 * added -N option to allow client side renegotiation
 *
 * Revision 1.8  2023-01-03 19:47:46+05:30  Cprogrammer
 * replace internal TLS function with TLS functions from libqmail
 *
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
