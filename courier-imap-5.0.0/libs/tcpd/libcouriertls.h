/*
** Copyright 2002-2008 Double Precision, Inc.
** See COPYING for distribution information.
*/

#ifndef libcouriertls_h
#define libcouriertls_h

#include	"config.h"
#include	<stdio.h>
#include	<string.h>
#include	<sys/types.h>
#include	<sys/time.h>
#if	HAVE_SYS_SELECT_H
#include	<sys/select.h>
#endif
#if	HAVE_UNISTD_H
#include	<unistd.h>
#endif
#if	HAVE_FCNTL_H
#include	<fcntl.h>
#endif

#ifdef COURIERTCPD_EXPOSE_OPENSSL

#define	DEBUG_SAFESTACK	1	/* For openssl 0.9.6 */

#include	<openssl/ssl.h>
#include	<openssl/err.h>

typedef SSL_CTX *ssl_context;
typedef SSL *ssl_handle;
#else

struct ssl_context_t;
struct ssl_handle_t;

typedef struct ssl_context_t *ssl_context;
typedef struct ssl_handle_t *ssl_handle;
#endif

#ifdef  __cplusplus
extern "C" {
#endif


/*
**                   High level TLS interface library
**
** This library implements a higher level OpenSSL API, taking care of any gory
** low level details.
*/

/*
** This tls_info structure must be initialized before calling tls_create().
*/

struct CACHE;

struct tls_info {

	/*
	** For SSL/TLS clients that wish to validate the server's certificate,
	** set 'peer_verify_domain' to the required certificate CN field.
	** Leave peer_verify_domain at NULL if server's certificate should
	** not be verified.
	*/

	const char *peer_verify_domain;

	/*
	** If the following function is not NULL, it will be called
	** after SSL negotiation completes.  If this function returns 0,
	** the SSL connection gets torn down.
	*/

	int (*connect_callback)(ssl_handle , void *);

	/*
	** When libcouriertls.a feels the urge to report an error, this
	** function gets called with an error message.
	*/

	void (*tls_err_msg)(const char *err_msg,
			    void *app_data /* see below */);

	/*
	** The plethora of OpenSSL configuration settings are wrapped up
	** in this function.  libcouriertls.a will call the following function
	** to obtain a particular configuration setting, represented by a
	** label.  The function should return the setting's configuration
	** value, as a text string.
	**
	** In most cases, calling getenv() will be sufficient here.
	** See below for a complete list of currently defined configuration
	** settings.
	*/

	const char * (*getconfigvar)(const char *, void *);

	/*
	** Retrieve client SSL TLS certificates. If this function pointer is
	** not NULL, this callback gets invoked repeatedly, with the first
	** parameter starting at zero and increasing until the callback
	** function returns 0.
	**
	** A non-zero return means that the callback function initialized
	** *cert_array_ret and *cert_array_size_ret to a pointer to a
	** PEM-formatted client SSL certificate, and its size in bytes.
	** The PEM file should contain a "BEGIN CERTIFICATE" followed by a
	** "BEGIN * PRIVATE KEY" (passphrase-protected keys and certs are
	** not yet supported. A zero return means that a cert/key is not
	** available.
	**
	** The first parameter is the certificate index number. 0 puts
	** the first available client certificate into cert_array_ret
	** and cert_array_size_ret and returns non-zero, or a zero return
	** if no SSL client certificates are available. If a client certificate
	** is returned, the callback function MAY get invoked again with
	** the first parameter set to 1, to retrieve the second client
	** certificate (if available). This continues until the callback
	** function returns zero, or until the returned SSL certificate's
	** issuer matches the acceptable issuers, as requested by the peer.
	**
	** The callback function cannot expect that all available SSL client
	** certs will get retrieved. If a suitable cert is found, no more
	** will be requested.
	*/

	int (*getpemclientcert4ca)(size_t i,
				   const char **cert_array_ret,
				   size_t *cert_array_size_ret,
				   void *dummy_arg);
	/*
	** If this callback function is defined, it gets invoked before
	** the first SSL client certificate gets requested. Its typical
	** purpose is to load all the available client certificates into
	** readily-available memory buffer of some sorts.
	*/
	void (*loadpemclientcert4ca)(void *dummy_arg);

	/*
	** Once a suitable SSL certificate is returned, or after all
	** certificates are returned (getpemclientcert4ca returned zero),
	** this function gets invoked. Its typical purpose is to unload
	** all memory buffers used by loaded client certs, and release all
	** memory allocated by loadpemclient4ca.
	*/

	void (*releasepemclientcert4ca)(void *dummy_arg);

	/*
	** app_data is a transparent pointer that's passed along as the last
	** argument to the callback functions above.
	*/

	void *app_data;

	/*
	** Everything below is internal data.
	*/
	struct CACHE *tlscache;
	int isserver;

	int connect_interrupted;
	int accept_interrupted;

	int certificate_verified;
};

/*
** tls_get_default_info() returns a default tls_info structure, with their
** default values.
*/
const struct tls_info *tls_get_default_info();

/*
** Create or destroy an SSL context.  'isserver' is non-zero for a server
** context, zero for a client context.  tls_create() makes a copy of the
** tls_info structure, for its own internal use.  The functions and data
** in the tls_info structure will continue to be used until the context is
** destroyed, from the internally-maintained copy, though.
**
** Do not call tls_destroy until all sessions are similarly destroyed.
*/

ssl_context tls_create(int isserver, const struct tls_info *);
void tls_destroy(ssl_context ctx);

/*
** SSL connect/disconnect.  tls_connect() creates a new SSL connection on
** an existing file descriptor.
*/

ssl_handle tls_connect(ssl_context ctx, int fd);

void tls_disconnect(ssl_handle ssl, int fd);

/*
** Return non-zero if connection is still in progress
*/

int tls_connecting(ssl_handle );

/*
** Return non-zero if the certificate was verified
*/

int tls_certificate_verified(ssl_handle);

/*
** Once an SSL/TLS session is established, use the following structure to
** read or write to the SSL/TLS socket.  The tls_transfer function reads
** and/or writes to the SSL/TLS socket, simultaneously.
**
** To read SSL data, set readptr to point to the buffer, and readleft to the
** # of bytes in the buffer.  Both readptr and readleft are updated if data
** was read from the socket by tls_transfer().  tls_transfer() will not read
** from the socket if readleft is zero.
**
** To write SSL data, set writeptr and writeleft to point to the buffer to
** be written out.  tls_transfer() will update writeptr/writeleft, if it
** wrote succesfully.  tls_transfer() may end up writing out only a portion
** of the buffer.  Do not reset writeptr and writeleft until tls_transfer()
** updates writeleft to 0, which indicates that the data has been written out
** succesfully.
**
** A tls_transfer_info object is initialized by tls_connect().
*/

struct tls_transfer_info {

	char *readptr;
	size_t readleft;

	const char *writeptr;
	size_t writeleft;

	int read_interrupted;
	int write_interrupted;

	int shutdown;
	int shutdown_interrupted;
};

#define tls_transfer_init(i) memset((i), 0, sizeof(*i));

/*
**  Read and/or write from the SSL/TLS socket.  tls_transfer() updates
**  the info object, after reading or writing 'ssl', on 'fd'.
**
**  tls_transfer returns 0 if the read/write operation was processed
**  succesfully (the write operation may not be written out in entirety,
**  check writeleft).
**
**  tls_transfer returns a negative value if there was an SSL/TLS protocol
**  error, or the SSL/TLS connection closed. The SSL connection should be
**  destroyed by calling tls_disconnect().
**
**  tls_transfer returns a positive value if tls_transfer() could not complete
**  a read or a write operation because no data was available on
**  the socket and/or the socket's output buffer is full.
**  The file descriptor sets 'r' and 'w' are updated to indicate the
**  desired socket state, and the tls_inprogress() macro will return true.
**
**  tls_transfer returns a positive value, and tls_inprogress() macro will
**  return false if there was nothing to read or write on the socket
**  (both readleft and writeleft were zero).
*/

int	tls_transfer(struct tls_transfer_info *info, ssl_handle ssl, int fd,
		     fd_set *r, fd_set *w);

#define tls_inprogress(s) ((s)->read_interrupted || (s)->write_interrupted || \
			(s)->shutdown_interrupted)

void tls_dump_connection_info(ssl_handle ssl,
			      int server,
			      void (*dump_func)(const char *, int cnt, void *),
			      void *dump_arg);

char *tls_get_encryption_desc(ssl_handle ssl);

char *tls_cert_name(const char *buf, size_t buf_size);

/*
** Start orderly SSL/TLS connection disconnect.
*/

#define tls_closing(s) ((s)->shutdown_interrupted=1)
#define tls_isclosing(s) ((s)->shutdown_interrupted)
#define tls_isclosed(s) ((s)->shutdown)


int tls_validate_pem_cert(const char *buf, size_t buf_size);

#ifdef  __cplusplus
}
#endif

/******************  Configuration variables ******************************


TLS_PROTOCOL sets the protocol version.  The possible versions are:

SSL2 - SSLv2
SSL3 - SSLv3
TLS1 - TLS1


TLS_CIPHER_LIST optionally sets the list of ciphers to be used by the
OpenSSL library.  In most situations you can leave TLS_CIPHER_LIST
undefined.

TLS_TIMEOUT - session timeout fot TLS1

TLS_DHCERTFILE - PEM file that stores our Diffie-Hellman cipher pair.
When OpenSSL is compiled to use Diffie-Hellman ciphers instead of RSA
you must generate a DH pair that will be used.  In most situations the
DH pair is to be treated as confidential, and the file specified by
TLS_DHCERTFILE must not be world-readable.

TLS_CERTFILE - PEM file that stores the RSA secret key and certificate.
TLS_CERTFILE is required for SSL/TLS servers, and is optional for SSL/TLS
clients.  TLS_CERTFILE is usually treated as confidential, and must not be
world-readable.

TLS_TRUSTCERTS=pathname - load trusted root certificates from pathname.
pathname can be a file or a directory. If a file, the file should
contain a list of trusted certificates, in PEM format. If a
directory, the directory should contain the trusted certificates,
in PEM format, one per file and hashed using OpenSSL's c_rehash
script. TLS_TRUSTCERTS is used by SSL/TLS clients (by specifying
the -domain option) and by SSL/TLS servers (TLS_VERIFYPEER is set
to PEER or REQUIREPEER).

TLS_VERIFYPEER - how to verify client certificates.  The possible values of
this setting are:

NONE - do not verify anything
PEER - verify the client certificate, if one's presented
REQUIREPEER - require a client certificate, fail if one's not presented

TLS_CACHEFILE - if defined, specifies the pathname to a file that is used to
SSL/TLS sessions.  Some clients that open multiple SSL connections can take
advantage of SSL/TLS session caching.

TLS_CACHESIZE - the size of the TLS_CACHEFILE to create, if it does not
exist.

TLS_INTCACHESIZE - the size of the internal OpenSSL SSL session cache.
OpenSSL's documentations states that the default size is 20,000 sessions.
Use this configuration setting to reduce the default size in order to reduce
the memory footprint of SSL-enabled processes.

TCPLOCALIP - the local IP address.  Used in server settings.  If
TLS_CERTFILE/TLS_DHCERTFILE does not exist, append ".TCPLOCALIP" and try
again.

*************************************************************************/

#endif
