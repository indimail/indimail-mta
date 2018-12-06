/*
 * socket.c -- socket library functions
 *
 * Copyright 1998 by Eric S. Raymond.
 * For license terms, see the file COPYING in this directory.
 */

#include "config.h"
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <ctype.h> /* isspace() */
#ifdef HAVE_MEMORY_H
#include <memory.h>
#endif /* HAVE_MEMORY_H */
#include <sys/types.h>
#include <sys/stat.h>
#ifndef HAVE_NET_SOCKET_H
#include <sys/socket.h>
#else
#include <net/socket.h>
#endif
#include <sys/un.h>
#include <netinet/in.h>
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#include <netdb.h>
#if defined(STDC_HEADERS)
#include <stdlib.h>
#endif
#if defined(HAVE_UNISTD_H)
#include <unistd.h>
#endif
#if defined(HAVE_STDARG_H)
#include <stdarg.h>
#else
#include <varargs.h>
#endif
#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

#include "socket.h"
#include "fetchmail.h"
#include "getaddrinfo.h"
#include "i18n.h"
#include "sdump.h"

/* Defines to allow BeOS and Cygwin to play nice... */
#ifdef __BEOS__
static char peeked;
#define fm_close(a)  closesocket(a)
#define fm_write(a,b,c)  send(a,b,c,0)
#define fm_peek(a,b,c)   recv(a,b,c,0)
#define fm_read(a,b,c)   recv(a,b,c,0)
#else
#define fm_close(a)  close(a)
#define fm_write(a,b,c)  write(a,b,c)
#define fm_peek(a,b,c)   recv(a,b,c, MSG_PEEK)
#ifdef __CYGWIN__
#define fm_read(a,b,c)   cygwin_read(a,b,c)
static ssize_t cygwin_read(int sock, void *buf, size_t count);
#else /* ! __CYGWIN__ */
#define fm_read(a,b,c)   read(a,b,c)
#endif /* __CYGWIN__ */
#endif

/* We need to define h_errno only if it is not already */
#ifndef h_errno
# if !HAVE_DECL_H_ERRNO
extern int h_errno;
# endif
#endif /* ndef h_errno */

#ifdef HAVE_SOCKETPAIR
static char *const *parse_plugin(const char *plugin, const char *host, const char *service)
{
	char **argvec;
	const char *c, *p;
	char *cp, *plugin_copy;
	unsigned int plugin_copy_len;
	unsigned int plugin_offset = 0, plugin_copy_offset = 0;
	unsigned int i, s = 2 * sizeof(char*), host_count = 0, service_count = 0;
	unsigned int plugin_len = strlen(plugin);
	unsigned int host_len = strlen(host);
	unsigned int service_len = strlen(service);

	for (c = p = plugin; *c; c++)
	{	if (isspace((unsigned char)*c) && !isspace((unsigned char)*p))
			s += sizeof(char*);
		if (*p == '%' && *c == 'h')
			host_count++;
		if (*p == '%' && *c == 'p')
			service_count++;
		p = c;
	}

	plugin_copy_len = plugin_len + host_len * host_count + service_len * service_count;
	plugin_copy = (char *)malloc(plugin_copy_len + 1);
	if (!plugin_copy)
	{
		report(stderr, GT_("fetchmail: malloc failed\n"));
		return NULL;
	}

	while (plugin_offset < plugin_len && plugin_copy_offset < plugin_copy_len)
	{	if ((plugin[plugin_offset] == '%') && (plugin[plugin_offset + 1] == 'h'))
		{	strcpy(plugin_copy + plugin_copy_offset, host);
			plugin_offset += 2;
			plugin_copy_offset += host_len;
		}
		else if ((plugin[plugin_offset] == '%') && (plugin[plugin_offset + 1] == 'p'))
		{	strcpy(plugin_copy + plugin_copy_offset, service);
			plugin_offset += 2;
			plugin_copy_offset += service_len;
		}
		else
		{	plugin_copy[plugin_copy_offset] = plugin[plugin_offset];
			plugin_offset++;
			plugin_copy_offset++;
		}
	}
	plugin_copy[plugin_copy_len] = 0;

	/* XXX FIXME - is this perhaps a bit too simplistic to chop down the argument strings without any respect to quoting?
	 * better write a generic function that tracks arguments instead... */
	argvec = (char **)malloc(s);
	if (!argvec)
	{
		free(plugin_copy);
		report(stderr, GT_("fetchmail: malloc failed\n"));
		return NULL;
	}
	memset(argvec, 0, s);
	for (p = cp = plugin_copy, i = 0; *cp; cp++)
	{	if ((!isspace((unsigned char)*cp)) && (cp == p ? 1 : isspace((unsigned char)*p))) {
			argvec[i] = cp;
			i++;
		}
		p = cp;
	}
	for (cp = plugin_copy; *cp; cp++)
	{	if (isspace((unsigned char)*cp))
			*cp = 0;
	}
	return argvec;
}

static int handle_plugin(const char *host,
			 const char *service, const char *plugin)
/* get a socket mediated through a given external command */
{
    int fds[2];
    char *const *argvec;

    /*
     * The author of this code, Felix von Leitner <felix@convergence.de>, says:
     * he chose socketpair() instead of pipe() because socketpair creates 
     * bidirectional sockets while allegedly some pipe() implementations don't.
     */
    if (socketpair(AF_UNIX,SOCK_STREAM,0,fds))
    {
	report(stderr, GT_("fetchmail: socketpair failed\n"));
	return -1;
    }
    switch (fork()) {
	case -1:
		/* error */
		report(stderr, GT_("fetchmail: fork failed\n"));
		return -1;
	case 0:	/* child */
		/* fds[1] is the parent's end; close it for proper EOF
		** detection */
		(void) close(fds[1]);
		if ( (dup2(fds[0],0) == -1) || (dup2(fds[0],1) == -1) ) {
			report(stderr, GT_("dup2 failed\n"));
			_exit(EXIT_FAILURE);
		}
		/* fds[0] is now connected to 0 and 1; close it */
		(void) close(fds[0]);
		if (outlevel >= O_VERBOSE)
		    report(stderr, GT_("running %s (host %s service %s)\n"), plugin, host, service);
		argvec = parse_plugin(plugin,host,service);
		if (argvec == NULL)
			_exit(EXIT_FAILURE);
		execvp(*argvec, argvec);
		report(stderr, GT_("execvp(%s) failed\n"), *argvec);
		_exit(EXIT_FAILURE);
		break;
	default:	/* parent */
		/* NOP */
		break;
    }
    /* fds[0] is the child's end; close it for proper EOF detection */
    (void) close(fds[0]);
    return fds[1];
}
#endif /* HAVE_SOCKETPAIR */

/** Set socket to SO_KEEPALIVE. \return 0 for success. */
int SockKeepalive(int sock) {
    int keepalive = 1;
    return setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &keepalive, sizeof keepalive);
}

int UnixOpen(const char *path)
{
    int sock = -1;
    struct sockaddr_un ad;
    memset(&ad, 0, sizeof(ad));
    ad.sun_family = AF_UNIX;
    strncpy(ad.sun_path, path, sizeof(ad.sun_path)-1);

    sock = socket( AF_UNIX, SOCK_STREAM, 0 );
    if (sock < 0)
    {
	h_errno = 0;
	return -1;
    }

    /* Socket opened saved. Usefull if connect timeout 
     * because it can be closed.
     */
    mailserver_socket_temp = sock;

    if (connect(sock, (struct sockaddr *) &ad, sizeof(ad)) < 0)
    {
	int olderr = errno;
	fm_close(sock);	/* don't use SockClose, no traffic yet */
	h_errno = 0;
	errno = olderr;
	sock = -1;
    }

    /* No connect timeout, then no need to set mailserver_socket_temp */
    mailserver_socket_temp = -1;

    return sock;
}

int SockOpen(const char *host, const char *service,
	     const char *plugin, struct addrinfo **ai0)
{
    struct addrinfo *ai, req;
    int i, acterr = 0;
    int ord;
    char errbuf[8192] = "";

#ifdef HAVE_SOCKETPAIR
    if (plugin)
	return handle_plugin(host,service,plugin);
#endif /* HAVE_SOCKETPAIR */

    memset(&req, 0, sizeof(struct addrinfo));
    req.ai_socktype = SOCK_STREAM;
#ifdef AI_ADDRCONFIG
    req.ai_flags = AI_ADDRCONFIG;
#endif

    i = fm_getaddrinfo(host, service, &req, ai0);
    if (i) {
	report(stderr, GT_("getaddrinfo(\"%s\",\"%s\") error: %s\n"),
		host, service, gai_strerror(i));
	if (i == EAI_SERVICE)
	    report(stderr, GT_("Try adding the --service option (see also FAQ item R12).\n"));
	return -1;
    }

    /* NOTE a Linux bug here - getaddrinfo will happily return 127.0.0.1
     * twice if no IPv6 is configured */
    i = -1;
    for (ord = 0, ai = *ai0; ai; ord++, ai = ai->ai_next) {
	char buf[256]; /* hostname */
	char pb[256];  /* service name */
	int gnie;      /* getnameinfo result code */

	gnie = getnameinfo(ai->ai_addr, ai->ai_addrlen, buf, sizeof(buf), NULL, 0, NI_NUMERICHOST);
	if (gnie)
	    snprintf(buf, sizeof(buf), GT_("unknown (%s)"), gai_strerror(gnie));
	gnie = getnameinfo(ai->ai_addr, ai->ai_addrlen, NULL, 0, pb, sizeof(pb), NI_NUMERICSERV);
	if (gnie)
	    snprintf(pb, sizeof(pb), GT_("unknown (%s)"), gai_strerror(gnie));

	if (outlevel >= O_VERBOSE)
	    report_build(stdout, GT_("Trying to connect to %s/%s..."), buf, pb);
	i = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
	if (i < 0) {
	    int e = errno;
	    /* mask EAFNOSUPPORT errors, they confuse users for
	     * multihomed hosts */
	    if (errno != EAFNOSUPPORT)
		acterr = errno;
	    if (outlevel >= O_VERBOSE)
		report_complete(stdout, GT_("cannot create socket: %s\n"), strerror(e));
	    snprintf(errbuf+strlen(errbuf), sizeof(errbuf)-strlen(errbuf),\
		     GT_("name %d: cannot create socket family %d type %d: %s\n"), ord, ai->ai_family, ai->ai_socktype, strerror(e));
	    continue;
	}

	SockKeepalive(i);

	/* Save socket descriptor.
	 * Used to close the socket after connect timeout. */
	mailserver_socket_temp = i;

	if (connect(i, (struct sockaddr *) ai->ai_addr, ai->ai_addrlen) < 0) {
	    int e = errno;

	    /* additionally, suppress IPv4 network unreach errors */
	    if (e != EAFNOSUPPORT)
		acterr = errno;

	    if (outlevel >= O_VERBOSE)
		report_complete(stdout, GT_("connection failed.\n"));
	    if (outlevel >= O_VERBOSE)
		report(stderr, GT_("connection to %s:%s [%s/%s] failed: %s.\n"), host, service, buf, pb, strerror(e));
	    snprintf(errbuf+strlen(errbuf), sizeof(errbuf)-strlen(errbuf), GT_("name %d: connection to %s:%s [%s/%s] failed: %s.\n"), ord, host, service, buf, pb, strerror(e));
	    fm_close(i);
	    i = -1;
	    continue;
	} else {
	    if (outlevel >= O_VERBOSE)
		report_complete(stdout, GT_("connected.\n"));
	}

	/* No connect timeout, then no need to set mailserver_socket_temp */
	mailserver_socket_temp = -1;

	break;
    }

    fm_freeaddrinfo(*ai0);
    *ai0 = NULL;

    if (i == -1) {
	report(stderr, GT_("Connection errors for this poll:\n%s"), errbuf);
	errno = acterr;
    }

    return i;
}


#if defined(HAVE_STDARG_H)
int SockPrintf(int sock, const char* format, ...)
{
#else
int SockPrintf(sock,format,va_alist)
int sock;
char *format;
va_dcl {
#endif

    va_list ap;
    char buf[8192];

#if defined(HAVE_STDARG_H)
    va_start(ap, format) ;
#else
    va_start(ap);
#endif
    vsnprintf(buf, sizeof(buf), format, ap);
    va_end(ap);
    return SockWrite(sock, buf, strlen(buf));

}

#ifdef SSL_ENABLE
#define OPENSSL_NO_DEPRECATED 23
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/x509v3.h>
#include <openssl/rand.h>

#define fm_MIN_OPENSSL_VER 0x1000200fL

#ifdef LIBRESSL_VERSION_NUMBER
#pragma message "WARNING - LibreSSL is unsupported. Use at your own risk."
#endif

#if OPENSSL_VERSION_NUMBER < fm_MIN_OPENSSL_VER
#error Your OpenSSL version must be at least 1.0.2 release. Older OpenSSL versions are unsupported.
#else
/*
#define __fm_ossl_ver(x) #x
#define _fm_ossl_ver(x) __fm_ossl_ver(x)
#pragma message "Building with OpenSSL headers version " _fm_ossl_ver(OPENSSL_VERSION_NUMBER) ", " OPENSSL_VERSION_TEXT
*/
#endif

static void report_SSL_errors(FILE *stream)
{
    unsigned long err;

    while (0ul != (err = ERR_get_error())) {
	char *errstr = ERR_error_string(err, NULL);
	report(stream, GT_("OpenSSL reported: %s\n"), errstr);
    }
}

/* override ERR_print_errors_fp to our own implementation */
#undef ERR_print_errors_fp
#define ERR_print_errors_fp(stream) report_SSL_errors((stream))

static	SSL_CTX *_ctx[FD_SETSIZE];
static	SSL *_ssl_context[FD_SETSIZE];

static SSL	*SSLGetContext( int );
#endif /* SSL_ENABLE */

int SockWrite(int sock, const char *buf, int len)
{
    int n, wrlen = 0;
#ifdef	SSL_ENABLE
    SSL *ssl;
#endif

    while (len)
    {
#ifdef SSL_ENABLE
	if( NULL != ( ssl = SSLGetContext( sock ) ) )
		n = SSL_write(ssl, buf, len);
	else
#endif /* SSL_ENABLE */
	    n = fm_write(sock, buf, len);
        if (n <= 0)
            return -1;
        len -= n;
	wrlen += n;
	buf += n;
    }
    return wrlen;
}

int SockRead(int sock, char *buf, int len)
{
    char *newline, *bp = buf;
    int n;
#ifdef	SSL_ENABLE
    SSL *ssl;
#endif

    if (--len < 1)
	return(-1);
#ifdef __BEOS__
    if (peeked != 0){
        (*bp) = peeked;
        bp++;
        len--;
        peeked = 0;
    }
#endif        
    do {
	/* 
	 * The reason for these gymnastics is that we want two things:
	 * (1) to read \n-terminated lines,
	 * (2) to return the true length of data read, even if the
	 *     data coming in has embedded NULS.
	 */
#ifdef	SSL_ENABLE
	if( NULL != ( ssl = SSLGetContext( sock ) ) ) {
		/* Hack alert! */
		/* OK...  SSL_peek works a little different from MSG_PEEK
			Problem is that SSL_peek can return 0 if there
			is no data currently available.  If, on the other
			hand, we lose the socket, we also get a zero, but
			the SSL_read then SEGFAULTS!  To deal with this,
			we'll check the error code any time we get a return
			of zero from SSL_peek.  If we have an error, we bail.
			If we don't, we read one character in SSL_read and
			loop.  This should continue to work even if they
			later change the behavior of SSL_peek
			to "fix" this problem...  :-(	*/
		if ((n = SSL_peek(ssl, bp, len)) < 0) {
			(void)SSL_get_error(ssl, n);
			return(-1);
		}
		if( 0 == n ) {
			/* SSL_peek says no data...  Does he mean no data
			or did the connection blow up?  If we got an error
			then bail! */
			if (0 != SSL_get_error(ssl, n)) {
				return -1;
			}
			/* We didn't get an error so read at least one
				character at this point and loop */
			n = 1;
			/* Make sure newline start out NULL!
			 * We don't have a string to pass through
			 * the strchr at this point yet */
			newline = NULL;
		} else if ((newline = (char *)memchr(bp, '\n', n)) != NULL)
			n = newline - bp + 1;
		/* Matthias Andree: SSL_read can return 0, in that case
		 * we must call SSL_get_error to figure if there was
		 * an error or just a "no data" condition */
		if ((n = SSL_read(ssl, bp, n)) <= 0) {
			if ((n = SSL_get_error(ssl, n))) {
				return(-1);
			}
		}
		/* Check for case where our single character turned out to
		 * be a newline...  (It wasn't going to get caught by
		 * the strchr above if it came from the hack...  ). */
		if( NULL == newline && 1 == n && '\n' == *bp ) {
			/* Got our newline - this will break
				out of the loop now */
			newline = bp;
		}
	}
	else
#endif /* SSL_ENABLE */
	{

#ifdef __BEOS__
	    if ((n = fm_read(sock, bp, 1)) <= 0)
#else
	    if ((n = fm_peek(sock, bp, len)) <= 0)
#endif
		return (-1);
	    if ((newline = (char *)memchr(bp, '\n', n)) != NULL)
		n = newline - bp + 1;
#ifndef __BEOS__
	    if ((n = fm_read(sock, bp, n)) == -1)
		return(-1);
#endif /* __BEOS__ */
	}
	bp += n;
	len -= n;
    } while 
	    (!newline && len);
    *bp = '\0';

    return bp - buf;
}

int SockPeek(int sock)
/* peek at the next socket character without actually reading it */
{
    int n;
    char ch;
#ifdef	SSL_ENABLE
    SSL *ssl;
#endif

#ifdef	SSL_ENABLE
	if( NULL != ( ssl = SSLGetContext( sock ) ) ) {
		n = SSL_peek(ssl, &ch, 1);
		if (n < 0) {
			(void)SSL_get_error(ssl, n);
			return -1;
		}
		if( 0 == n ) {
			/* This code really needs to implement a "hold back"
			 * to simulate a functioning SSL_peek()...  sigh...
			 * Has to be coordinated with the read code above.
			 * Next on the list todo...	*/

			/* SSL_peek says 0...  Does that mean no data
			or did the connection blow up?  If we got an error
			then bail! */
			if(0 != SSL_get_error(ssl, n)) {
				return -1;
			}

			/* Haven't seen this case actually occur, but...
			   if the problem in SockRead can occur, this should
			   be possible...  Just not sure what to do here.
			   This should be a safe "punt" the "peek" but don't
			   "punt" the "session"... */

			return 0;	/* Give him a '\0' character */
		}
	}
	else
#endif /* SSL_ENABLE */
	    n = fm_peek(sock, &ch, 1);
	if (n == -1)
		return -1;

#ifdef __BEOS__
    peeked = ch;
#endif
    return(ch);
}

#ifdef SSL_ENABLE

static	char *_ssl_server_cname = NULL;
static	int _check_fp;
static	char *_check_digest;
static 	char *_server_label;
static	int _depth0ck;
static	int _firstrun;
static	int _prev_err;
static	int _verify_ok;

SSL *SSLGetContext( int sock )
{
	if( sock < 0 || (unsigned)sock > FD_SETSIZE )
		return NULL;
	if( _ctx[sock] == NULL )
		return NULL;
	return _ssl_context[sock];
}

/* ok_return (preverify_ok) is 1 if this stage of certificate verification
   passed, or 0 if it failed. This callback lets us display informative
   errors, and perform additional validation (e.g. CN matches) */
static int SSL_verify_callback( int ok_return, X509_STORE_CTX *ctx, int strict )
{
#define SSLverbose (((outlevel) >= O_DEBUG) || ((outlevel) >= O_VERBOSE && (depth) == 0)) 
	char buf[257];
	X509 *x509_cert;
	int err, depth, i;
	unsigned char digest[EVP_MAX_MD_SIZE];
	char text[EVP_MAX_MD_SIZE * 3 + 1], *tp, *te;
	const EVP_MD *digest_tp;
	unsigned int dsz, esz;
	X509_NAME *subj, *issuer;
	char *tt;

	x509_cert = X509_STORE_CTX_get_current_cert(ctx);
	err = X509_STORE_CTX_get_error(ctx);
	depth = X509_STORE_CTX_get_error_depth(ctx);

	subj = X509_get_subject_name(x509_cert);
	issuer = X509_get_issuer_name(x509_cert);

	if (outlevel >= O_VERBOSE) {
		if (depth == 0 && SSLverbose)
			report(stdout, GT_("Server certificate:\n"));
		else {
			if (_firstrun) {
				_firstrun = 0;
				if (SSLverbose)
					report(stdout, GT_("Certificate chain, from root to peer, starting at depth %d:\n"), depth);
			} else {
				if (SSLverbose)
					report(stdout, GT_("Certificate at depth %d:\n"), depth);
			}
		}

		if (SSLverbose) {
			if ((i = X509_NAME_get_text_by_NID(issuer, NID_organizationName, buf, sizeof(buf))) != -1) {
				report(stdout, GT_("Issuer Organization: %s\n"), (tt = sdump(buf, i)));
				xfree(tt);
				if ((size_t)i >= sizeof(buf) - 1)
					report(stdout, GT_("Warning: Issuer Organization Name too long (possibly truncated).\n"));
			} else
				report(stdout, GT_("Unknown Organization\n"));
			if ((i = X509_NAME_get_text_by_NID(issuer, NID_commonName, buf, sizeof(buf))) != -1) {
				report(stdout, GT_("Issuer CommonName: %s\n"), (tt = sdump(buf, i)));
				xfree(tt);
				if ((size_t)i >= sizeof(buf) - 1)
					report(stdout, GT_("Warning: Issuer CommonName too long (possibly truncated).\n"));
			} else
				report(stdout, GT_("Unknown Issuer CommonName\n"));
		}
	}

	if ((i = X509_NAME_get_text_by_NID(subj, NID_commonName, buf, sizeof(buf))) != -1) {
		if (SSLverbose) {
			report(stdout, GT_("Subject CommonName: %s\n"), (tt = sdump(buf, i)));
			xfree(tt);
		}
		if ((size_t)i >= sizeof(buf) - 1) {
			/* Possible truncation. In this case, this is a DNS name, so this
			 * is really bad. We do not tolerate this even in the non-strict case. */
			report(stderr, GT_("Bad certificate: Subject CommonName too long!\n"));
			return (0);
		}
		if ((size_t)i > strlen(buf)) {
			/* Name contains embedded NUL characters, so we complain. This is likely
			 * a certificate spoofing attack. */
			report(stderr, GT_("Bad certificate: Subject CommonName contains NUL, aborting!\n"));
			return 0;
		}
	}

	if (depth == 0) { /* peer certificate */
		if (!_depth0ck) {
			_depth0ck = 1;
		}

		if ((i = X509_NAME_get_text_by_NID(subj, NID_commonName, buf, sizeof(buf))) != -1) {
			if (_ssl_server_cname != NULL) {
				char *p1 = buf;
				char *p2 = _ssl_server_cname;
				int matched = 0;
				STACK_OF(GENERAL_NAME) *gens;

				/* RFC 2595 section 2.4: find a matching name
				 * first find a match among alternative names */
				gens = (STACK_OF(GENERAL_NAME) *)X509_get_ext_d2i(x509_cert, NID_subject_alt_name, NULL, NULL);
				if (gens) {
					int j, r;
					for (j = 0, r = sk_GENERAL_NAME_num(gens); j < r; ++j) {
						const GENERAL_NAME *gn = sk_GENERAL_NAME_value(gens, j);
						if (gn->type == GEN_DNS) {
							char *pp1 = (char *)gn->d.ia5->data;
							char *pp2 = _ssl_server_cname;
							if (outlevel >= O_VERBOSE) {
								report(stdout, GT_("Subject Alternative Name: %s\n"), (tt = sdump(pp1, (size_t)gn->d.ia5->length)));
								xfree(tt);
							}
							/* Name contains embedded NUL characters, so we complain. This
							 * is likely a certificate spoofing attack. */
							if ((size_t)gn->d.ia5->length != strlen(pp1)) {
								report(stderr, GT_("Bad certificate: Subject Alternative Name contains NUL, aborting!\n"));
								sk_GENERAL_NAME_free(gens);
								return 0;
							}
							if (name_match(pp1, pp2)) {
							    matched = 1;
							}
						}
					}
					GENERAL_NAMES_free(gens);
				}
				if (name_match(p1, p2)) {
					matched = 1;
				}
				if (!matched) {
					if (strict || SSLverbose) {
						report(stderr,
								GT_("Server CommonName mismatch: %s != %s\n"),
								(tt = sdump(buf, i)), _ssl_server_cname );
						xfree(tt);
					}
					ok_return = 0;
				}
			} else if (ok_return) {
				report(stderr, GT_("Server name not set, could not verify certificate!\n"));
				if (strict) return (0);
			}
		} else {
			if (outlevel >= O_VERBOSE)
				report(stdout, GT_("Unknown Server CommonName\n"));
			if (ok_return && strict) {
				report(stderr, GT_("Server name not specified in certificate!\n"));
				return (0);
			}
		}
		/* Print the finger print. Note that on errors, we might print it more than once
		 * normally; we kluge around that by using a global variable. */
		if (_check_fp == 1) {
			unsigned dp;

			_check_fp = -1;
			digest_tp = EVP_md5();
			if (digest_tp == NULL) {
				report(stderr, GT_("EVP_md5() failed!\n"));
				return (0);
			}
			if (!X509_digest(x509_cert, digest_tp, digest, &dsz)) {
				report(stderr, GT_("Out of memory!\n"));
				return (0);
			}
			tp = text;
			te = text + sizeof(text);
			for (dp = 0; dp < dsz; dp++) {
				esz = snprintf(tp, te - tp, dp > 0 ? ":%02X" : "%02X", digest[dp]);
				if (esz >= (size_t)(te - tp)) {
					report(stderr, GT_("Digest text buffer too small!\n"));
					return (0);
				}
				tp += esz;
			}
			if (outlevel > O_NORMAL)
			    report(stdout, GT_("%s key fingerprint: %s\n"), _server_label, text);
			if (_check_digest != NULL) {
				if (strcasecmp(text, _check_digest) == 0) {
				    if (outlevel > O_NORMAL)
					report(stdout, GT_("%s fingerprints match.\n"), _server_label);
				} else {
				    report(stderr, GT_("%s fingerprints do not match!\n"), _server_label);
				    return (0);
				}
			} /* if (_check_digest != NULL) */
		} /* if (_check_fp) */
	} /* if (depth == 0 && !_depth0ck) */

	if (err != X509_V_OK && err != _prev_err && !(_check_fp != 0 && _check_digest && !strict)) {
		char *tmp;
		int did_rep_err = 0;
		_prev_err = err;

                report(stderr, GT_("Server certificate verification error: %s\n"), X509_verify_cert_error_string(err));
                /* We gave the error code, but maybe we can add some more details for debugging */

		switch (err) {
		/* actually we do not want to lump these together, but
		 * since OpenSSL flipped the meaning of these error
		 * codes in the past, and they do hardly make a
		 * practical difference because servers need not provide
		 * the root signing certificate, we don't bother telling
		 * users the difference:
		 */
		case X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT_LOCALLY:
		case X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT:
			X509_NAME_oneline(issuer, buf, sizeof(buf));
			buf[sizeof(buf) - 1] = '\0';
			report(stderr, GT_("Broken certification chain at: %s\n"), (tmp = sdump(buf, strlen(buf))));
			xfree(tmp);
			report(stderr, GT_(	"This could mean that the server did not provide the intermediate CA's certificate(s), "
						"which is nothing fetchmail could do anything about.  For details, "
						"please see the README.SSL-SERVER document that ships with fetchmail.\n"));
			did_rep_err = 1;
			/* FALLTHROUGH */
		case X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT:
		case X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN:
			if (!did_rep_err) {
			    X509_NAME_oneline(issuer, buf, sizeof(buf));
			    buf[sizeof(buf) - 1] = '\0';
			    report(stderr, GT_("Missing trust anchor certificate: %s\n"), (tmp = sdump(buf, strlen(buf))));
			    xfree(tmp);
			}
			report(stderr, GT_(	"This could mean that the root CA's signing certificate is not in the "
						"trusted CA certificate location, or that c_rehash needs to be run "
						"on the certificate directory. For details, please "
						"see the documentation of --sslcertpath and --sslcertfile in the manual page.\n"));
			break;
		default:
			break;
		}
	}
	/*
	 * If not in strict checking mode (--sslcertck), override this
	 * and pretend that verification had succeeded.
	 */
	_verify_ok &= ok_return;
	if (!strict)
		ok_return = 1;
	return (ok_return);
}

static int SSL_nock_verify_callback( int ok_return, X509_STORE_CTX *ctx )
{
	return SSL_verify_callback(ok_return, ctx, 0);
}

static int SSL_ck_verify_callback( int ok_return, X509_STORE_CTX *ctx )
{
	return SSL_verify_callback(ok_return, ctx, 1);
}


/* get commonName from certificate set in file.
 * commonName is stored in buffer namebuffer, limited with namebufferlen
 */
static const char *SSLCertGetCN(const char *mycert,
                                char *namebuffer, size_t namebufferlen)
{
	const char *ret       = NULL;
	BIO        *certBio   = NULL;
	X509       *x509_cert = NULL;
	X509_NAME  *certname  = NULL;

	if (namebuffer && namebufferlen > 0) {
		namebuffer[0] = 0x00;
		certBio = BIO_new_file(mycert,"r");
		if (certBio) {
			x509_cert = PEM_read_bio_X509(certBio,NULL,NULL,NULL);
			BIO_free(certBio);
		}
		if (x509_cert) {
			certname = X509_get_subject_name(x509_cert);
			if (certname &&
			    X509_NAME_get_text_by_NID(certname, NID_commonName,
						      namebuffer, namebufferlen) > 0)
				ret = namebuffer;
			X509_free(x509_cert);
		}
	}
	return ret;
}

#if defined(LIBRESSL_VERSION_NUMBER) || OPENSSL_VERSION_NUMBER < 0x1010000fL
/* OSSL_proto_version_logic for OpenSSL 1.0.x and LibreSSL */
static int OSSL10X_proto_version_logic(int sock, const char **myproto, int *avoid_ssl_versions)
{
	if (!*myproto) {
	    *myproto = "auto";
	}

	if (!strcasecmp("ssl3", *myproto)) {
#if (HAVE_DECL_SSLV3_CLIENT_METHOD > 0) && (0 == OPENSSL_NO_SSL3 + 0)
		_ctx[sock] = SSL_CTX_new(SSLv3_client_method());
		*avoid_ssl_versions &= ~SSL_OP_NO_SSLv3;
#else
		report(stderr, GT_("Your OpenSSL version does not support SSLv3.\n"));
		return -1;
#endif
	} else if (!strcasecmp("ssl3+", *myproto)) {
		*avoid_ssl_versions &= ~SSL_OP_NO_SSLv3;
		*myproto = NULL;
	} else if (!strcasecmp("tls1", *myproto)) {
		_ctx[sock] = SSL_CTX_new(TLSv1_client_method());
	} else if (!strcasecmp("tls1+", *myproto)) {
		*myproto = NULL;
#if defined(TLS1_1_VERSION)
	} else if (!strcasecmp("tls1.1", *myproto)) {
		_ctx[sock] = SSL_CTX_new(TLSv1_1_client_method());
	} else if (!strcasecmp("tls1.1+", *myproto)) {
		*myproto = NULL;
		*avoid_ssl_versions |= SSL_OP_NO_TLSv1;
#else
	} else if(!strcasecmp("tls1.1",*myproto) || !strcasecmp("tls1.1+", *myproto)) {
		report(stderr, GT_("Your OpenSSL version does not support TLS v1.1.\n"));
		return -1;
#endif
#if defined(TLS1_2_VERSION)
	} else if (!strcasecmp("tls1.2", *myproto)) {
		_ctx[sock] = SSL_CTX_new(TLSv1_2_client_method());
	} else if (!strcasecmp("tls1.2+", *myproto)) {
		*myproto = NULL;
		*avoid_ssl_versions |= SSL_OP_NO_TLSv1 | SSL_OP_NO_TLSv1_1;
#else
	} else if(!strcasecmp("tls1.2",*myproto) || !strcasecmp("tls1.2+", *myproto)) {
		report(stderr, GT_("Your OpenSSL version does not support TLS v1.2.\n"));
		return -1;
#endif
#if defined(TLS1_3_VERSION)
	} else if (!strcasecmp("tls1.3", *myproto)) {
		_ctx[sock] = SSL_CTX_new(TLSv1_3_client_method());
	} else if (!strcasecmp("tls1.3+", *myproto)) {
		*myproto = NULL;
		*avoid_ssl_versions |= SSL_OP_NO_TLSv1 | SSL_OP_NO_TLSv1_1 | SSL_OP_NO_TLSv1_2;
#else
	} else if(!strcasecmp("tls1.3",*myproto) || !strcasecmp("tls1.3+", *myproto)) {
		report(stderr, GT_("Your OpenSSL version does not support TLS v1.3.\n"));
		return -1;
#endif
	} else if (!strcasecmp("ssl23", *myproto)
	        || 0 == strcasecmp("auto", *myproto))
	{
		*myproto = NULL;
	} else {
		report(stderr,
		        GT_("Invalid SSL protocol '%s' specified, using default autoselect (auto).\n"),
		        *myproto);
		*myproto = NULL;
	}
	return 0;
}
#define OSSL_proto_version_logic(a,b,c) OSSL10X_proto_version_logic((a),(b),(c))
#undef OSSL110_API
#else
/* implementation for OpenSSL 1.1.0 */
#define OSSL110_API 1
static int OSSL110_proto_version_logic(int sock, const char **myproto,
        int *avoid_ssl_versions)
{
	/* NOTE - this code MUST NOT set myproto to NULL, else the
	 * SSL_...set_..._proto_version() call becomes ineffective. */
	_ctx[sock] = SSL_CTX_new(TLS_client_method());
	SSL_CTX_set_min_proto_version(_ctx[sock], TLS1_VERSION);

	if (!*myproto) {
	    *myproto = "auto";
	}

	if (!strcasecmp("ssl3", *myproto)) {
#if (0 == OPENSSL_NO_SSL3 + 0)
		SSL_CTX_set_min_proto_version(_ctx[sock], SSL3_VERSION);
		SSL_CTX_set_max_proto_version(_ctx[sock], SSL3_VERSION);
		*avoid_ssl_versions &= ~SSL_OP_NO_SSLv3;
#else
		report(stderr, GT_("Your OpenSSL version does not support SSLv3.\n"));
		return -1;
#endif
	} else if (!strcasecmp("ssl3+", *myproto)) {
		SSL_CTX_set_min_proto_version(_ctx[sock], SSL3_VERSION);
		*avoid_ssl_versions &= ~SSL_OP_NO_SSLv3;
	} else if (!strcasecmp("tls1", *myproto)) {
		SSL_CTX_set_max_proto_version(_ctx[sock], TLS1_VERSION);
	} else if (!strcasecmp("tls1+", *myproto)) {
		/* do nothing, min_proto_version is already at TLS1_VERSION */
#if defined(TLS1_1_VERSION)
	} else if (!strcasecmp("tls1.1", *myproto)) {
		SSL_CTX_set_min_proto_version(_ctx[sock], TLS1_1_VERSION);
		SSL_CTX_set_max_proto_version(_ctx[sock], TLS1_1_VERSION);
	} else if (!strcasecmp("tls1.1+", *myproto)) {
		SSL_CTX_set_min_proto_version(_ctx[sock], TLS1_1_VERSION);
#else
	} else if(!strcasecmp("tls1.1",*myproto) || !strcasecmp("tls1.1+", *myproto)) {
		report(stderr, GT_("Your OpenSSL version does not support TLS v1.1.\n"));
		return -1;
#endif
#if defined(TLS1_2_VERSION)
	} else if (!strcasecmp("tls1.2", *myproto)) {
		SSL_CTX_set_min_proto_version(_ctx[sock], TLS1_2_VERSION);
		SSL_CTX_set_max_proto_version(_ctx[sock], TLS1_2_VERSION);
	} else if (!strcasecmp("tls1.2+", *myproto)) {
		SSL_CTX_set_min_proto_version(_ctx[sock], TLS1_2_VERSION);
#else
	} else if(!strcasecmp("tls1.2",*myproto) || !strcasecmp("tls1.2+", *myproto)) {
		report(stderr, GT_("Your OpenSSL version does not support TLS v1.2.\n"));
		return -1;
#endif
#if defined(TLS1_3_VERSION)
	} else if (!strcasecmp("tls1.3", *myproto)) {
		SSL_CTX_set_min_proto_version(_ctx[sock], TLS1_3_VERSION);
		SSL_CTX_set_max_proto_version(_ctx[sock], TLS1_3_VERSION);
	} else if (!strcasecmp("tls1.3+", *myproto)) {
		SSL_CTX_set_min_proto_version(_ctx[sock], TLS1_3_VERSION);
#else
	} else if(!strcasecmp("tls1.3",*myproto) || !strcasecmp("tls1.3+", *myproto)) {
		report(stderr, GT_("Your OpenSSL version does not support TLS v1.3.\n"));
		return -1;
#endif
	} else if (!strcasecmp("ssl23", *myproto)
	        || 0 == strcasecmp("auto", *myproto))
	{
		/* do nothing */
	} else {
		/* This should not happen. */
		report(stderr,
		        GT_("Invalid SSL protocol '%s' specified, using default autoselect (auto).\n"),
		        *myproto);
		report(stderr, "fetchmail internal error in OSSL110_proto_version_logic\n");
		abort();
	}
	return 0;
}
#define OSSL_proto_version_logic(a,b,c) OSSL110_proto_version_logic((a),(b),(c))
#endif

/* performs initial SSL handshake over the connected socket
 * uses SSL *ssl global variable, which is currently defined
 * in this file
 */
int SSLOpen(int sock, char *mycert, char *mykey, const char *myproto, int certck,
    char *cacertfile, char *certpath,
    char *fingerprint, char *servercname, char *label, char **remotename)
{
        struct stat randstat;
        int i;
	int avoid_ssl_versions = SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3;
	long sslopts = SSL_OP_ALL | SSL_OP_SINGLE_DH_USE;
	int ssle_connect = 0;
	long ver;

#ifndef OSSL110_API
	SSL_load_error_strings();
	SSL_library_init();
	OpenSSL_add_all_algorithms(); /* see Debian Bug#576430 and manpage */
	ver = SSLeay();
#else
	ver = OpenSSL_version_num();
#endif

	if (ver < OPENSSL_VERSION_NUMBER) {
	    report(stderr, GT_("Loaded OpenSSL library %#lx older than headers %#lx, refusing to work.\n"), (long)ver, (long)(OPENSSL_VERSION_NUMBER));
	    return -1;
	}

	if (ver > OPENSSL_VERSION_NUMBER && outlevel >= O_VERBOSE) {
	    report(stdout, GT_("Loaded OpenSSL library %#lx newer than headers %#lx, trying to continue.\n"), (long)ver, (long)(OPENSSL_VERSION_NUMBER));
	}

        if (stat("/dev/random", &randstat)  &&
            stat("/dev/urandom", &randstat)) {
          /* Neither /dev/random nor /dev/urandom are present, so add
             entropy to the SSL PRNG a hard way. */
          for (i = 0; i < 10000  &&  ! RAND_status (); ++i) {
            char buf[4];
            struct timeval tv;
            gettimeofday (&tv, 0);
            buf[0] = tv.tv_usec & 0xF;
            buf[2] = (tv.tv_usec & 0xF0) >> 4;
            buf[3] = (tv.tv_usec & 0xF00) >> 8;
            buf[1] = (tv.tv_usec & 0xF000) >> 12;
            RAND_add (buf, sizeof buf, 0.1);
          }
        }

	if( sock < 0 || (unsigned)sock > FD_SETSIZE ) {
		report(stderr, GT_("File descriptor out of range for SSL") );
		return( -1 );
	}

	/* Make sure a connection referring to an older context is not left */
	_ssl_context[sock] = NULL;
	{
		int rc = OSSL_proto_version_logic(sock, &myproto, &avoid_ssl_versions);
		if (rc) return rc;
	}
	/* do not combine into an else { } as myproto may be nulled above! */
	if (!myproto) {
		/* SSLv23 is a misnomer and will in fact use the best
		 available protocol, subject to SSL_OP_NO* constraints. */
		_ctx[sock] = SSL_CTX_new(SSLv23_client_method());
	}
	if(_ctx[sock] == NULL) {
		unsigned long ec = ERR_peek_last_error();
		ERR_print_errors_fp(stderr);
		if (ERR_GET_REASON(ec) == SSL_R_NULL_SSL_METHOD_PASSED) {
		    report(stderr, GT_("Note that some distributions disable older protocol versions in weird non-standard ways. Try a newer protocol version.\n"));
		}
		return(-1);
	}

	{
	    char *tmp = getenv("FETCHMAIL_DISABLE_CBC_IV_COUNTERMEASURE");
	    if (tmp == NULL || *tmp == '\0' || strspn(tmp, " \t") == strlen(tmp))
		sslopts &= ~ SSL_OP_DONT_INSERT_EMPTY_FRAGMENTS;
	}

	SSL_CTX_set_options(_ctx[sock], sslopts | avoid_ssl_versions);

	if (certck) {
		SSL_CTX_set_verify(_ctx[sock], SSL_VERIFY_PEER, SSL_ck_verify_callback);
	} else {
		/* In this case, we do not fail if verification fails. However,
		 * we provide the callback for output and possible fingerprint
		 * checks. */
		SSL_CTX_set_verify(_ctx[sock], SSL_VERIFY_PEER, SSL_nock_verify_callback);
	}

	/* Check which trusted X.509 CA certificate store(s) to load */
	{
		char *tmp;
		int want_default_cacerts = 0;

		/* Load user locations if any is given */
		if (certpath || cacertfile)
			SSL_CTX_load_verify_locations(_ctx[sock],
						cacertfile, certpath);
		else
			want_default_cacerts = 1;

		tmp = getenv("FETCHMAIL_INCLUDE_DEFAULT_X509_CA_CERTS");
		if (want_default_cacerts || (tmp && tmp[0])) {
			SSL_CTX_set_default_verify_paths(_ctx[sock]);
		}
	}
	
	_ssl_context[sock] = SSL_new(_ctx[sock]);
	
	if(_ssl_context[sock] == NULL) {
		ERR_print_errors_fp(stderr);
		SSL_CTX_free(_ctx[sock]);
		_ctx[sock] = NULL;
		return(-1);
	}
	
	/* This static is for the verify callback */
	_ssl_server_cname = servercname;
	_server_label = label;
	_check_fp = 1;
	_check_digest = fingerprint;
	_depth0ck = 0;
	_firstrun = 1;
	_verify_ok = 1;
	_prev_err = -1;

	/*
	 * Support SNI, some servers (googlemail) appear to require it.
	 */
	{
	    long r;
	    r = SSL_set_tlsext_host_name(_ssl_context[sock], servercname);

	    if (0 == r) {
		/* handle error */
		report(stderr, GT_("Warning: SSL_set_tlsext_host_name(%p, \"%s\") failed (code %#lx), trying to continue.\n"), (void *)_ssl_context[sock], servercname, r);
		ERR_print_errors_fp(stderr);
	    }
	}

	if( mycert || mykey ) {

	/* Ok...  He has a certificate file defined, so lets declare it.  If
	 * he does NOT have a separate certificate and private key file then
	 * assume that it's a combined key and certificate file.
	 */
		char buffer[256];
		
		if( !mykey )
			mykey = mycert;
		if( !mycert )
			mycert = mykey;

		if ((!*remotename || !**remotename) && SSLCertGetCN(mycert, buffer, sizeof(buffer))) {
			free(*remotename);
			*remotename = xstrdup(buffer);
		}
		SSL_use_certificate_file(_ssl_context[sock], mycert, SSL_FILETYPE_PEM);
		SSL_use_RSAPrivateKey_file(_ssl_context[sock], mykey, SSL_FILETYPE_PEM);
	}

	if (SSL_set_fd(_ssl_context[sock], sock) == 0 
	    || (ssle_connect = SSL_connect(_ssl_context[sock])) < 1) {
		int e = errno;
		unsigned long ssle_err_from_queue = ERR_peek_error();
		unsigned long ssle_err_from_get_error = SSL_get_error(_ssl_context[sock], ssle_connect);
		ERR_print_errors_fp(stderr);
		if (SSL_ERROR_SYSCALL == ssle_err_from_get_error && 0 == ssle_err_from_queue) {
		    if (0 == ssle_connect) {
			report(stderr, GT_("Server shut down connection prematurely during SSL_connect().\n"));
		    } else if (ssle_connect < 0) {
			report(stderr, GT_("System error during SSL_connect(): %s\n"), strerror(e));
		    }
		}
		SSL_free( _ssl_context[sock] );
		_ssl_context[sock] = NULL;
		SSL_CTX_free(_ctx[sock]);
		_ctx[sock] = NULL;
		return(-1);
	}

	if (outlevel >= O_VERBOSE) {
	    SSL_CIPHER const *sc;
	    int bitsmax, bitsused;

	    const char *ver;

	    ver = SSL_get_version(_ssl_context[sock]);

	    sc = SSL_get_current_cipher(_ssl_context[sock]);
	    if (!sc) {
		report (stderr, GT_("Cannot obtain current SSL/TLS cipher - no session established?\n"));
	    } else {
		bitsused = SSL_CIPHER_get_bits(sc, &bitsmax);
		report(stdout, GT_("SSL/TLS: using protocol %s, cipher %s, %d/%d secret/processed bits\n"),
			ver, SSL_CIPHER_get_name(sc), bitsused, bitsmax);
	    }
	}

	/* Paranoia: was the callback not called as we expected? */
	if (!_depth0ck) {
		report(stderr, GT_("Certificate/fingerprint verification was somehow skipped!\n"));

		if (fingerprint != NULL || certck) {
			if( NULL != SSLGetContext( sock ) ) {
				/* Clean up the SSL stack */
				SSL_shutdown( _ssl_context[sock] );
				SSL_free( _ssl_context[sock] );
				_ssl_context[sock] = NULL;
				SSL_CTX_free(_ctx[sock]);
				_ctx[sock] = NULL;
			}
			return(-1);
		}
	}

	if (!certck && !fingerprint &&
		(SSL_get_verify_result(_ssl_context[sock]) != X509_V_OK || !_verify_ok)) {
		report(stderr, GT_("Warning: the connection is insecure, continuing anyways. (Better use --sslcertck!)\n"));
	}

	return(0);
}
#endif

int SockClose(int sock)
/* close a socket gracefully */
{
#ifdef	SSL_ENABLE
    if( NULL != SSLGetContext( sock ) ) {
        /* Clean up the SSL stack */
        SSL_shutdown( _ssl_context[sock] );
        SSL_free( _ssl_context[sock] );
        _ssl_context[sock] = NULL;
	SSL_CTX_free(_ctx[sock]);
	_ctx[sock] = NULL;
    }
#endif

    /* if there's an error closing at this point, not much we can do */
    return(fm_close(sock));	/* this is guarded */
}

#ifdef __CYGWIN__
/*
 * Workaround Microsoft Winsock recv/WSARecv(..., MSG_PEEK) bug.
 * See http://sources.redhat.com/ml/cygwin/2001-08/msg00628.html
 * for more details.
 */
static ssize_t cygwin_read(int sock, void *buf, size_t count)
{
    char *bp = (char *)buf;
    size_t n = 0;

    if ((n = read(sock, bp, count)) == (size_t)-1)
	return(-1);

    if (n != count) {
	size_t n2 = 0;
	if (outlevel >= O_VERBOSE)
	    report(stdout, GT_("Cygwin socket read retry\n"));
	n2 = read(sock, bp + n, count - n);
	if (n2 == (size_t)-1 || n + n2 != count) {
	    report(stderr, GT_("Cygwin socket read retry failed!\n"));
	    return(-1);
	}
    }

    return count;
}
#endif /* __CYGWIN__ */
