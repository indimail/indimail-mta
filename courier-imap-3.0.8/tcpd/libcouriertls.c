/*
** Copyright 2000-2002 Double Precision, Inc.
** See COPYING for distribution information.
*/
#include	"config.h"
#include	"argparse.h"
#include	"spipe.h"
#include	"libcouriertls.h"
#include	"tlscache.h"
#include	"rfc1035/rfc1035.h"
#include	"soxwrap/soxwrap.h"
#ifdef  getc
#undef  getc
#endif
#include	<stdio.h>
#include	<string.h>
#include	<stdlib.h>
#include	<ctype.h>
#include	<netdb.h>
#if HAVE_DIRENT_H
#include <dirent.h>
#define NAMLEN(dirent) strlen((dirent)->d_name)
#else
#define dirent direct
#define NAMLEN(dirent) (dirent)->d_namlen
#if HAVE_SYS_NDIR_H
#include <sys/ndir.h>
#endif
#if HAVE_SYS_DIR_H
#include <sys/dir.h>
#endif
#if HAVE_NDIR_H
#include <ndir.h>
#endif
#endif
#if	HAVE_UNISTD_H
#include	<unistd.h>
#endif
#if	HAVE_FCNTL_H
#include	<fcntl.h>
#endif
#include	<errno.h>
#if	HAVE_SYS_TYPES_H
#include	<sys/types.h>
#endif
#if	HAVE_SYS_STAT_H
#include	<sys/stat.h>
#endif
#include	<sys/socket.h>
#include	<arpa/inet.h>

#include	<sys/time.h>

/***** TODO *****/

/* #define TLSCACHEDEBUG */

static const char *safe_getenv(const struct tls_info *info, const char *n)
{
	const char *v=(*info->getconfigvar)(n, info->app_data);

	if (!v)	v="";
	return (v);
}

static int get_peer_verify_level(const struct tls_info *info)
{
	int peer_verify_level=SSL_VERIFY_PEER;
		/* SSL_VERIFY_NONE */
		/* SSL_VERIFY_PEER */
		/* SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT */
	const char *s=safe_getenv(info, "TLS_VERIFYPEER");

	if (info->peer_verify_domain)
		return SSL_VERIFY_PEER|SSL_VERIFY_FAIL_IF_NO_PEER_CERT;

	switch (*s)	{
	case 'n':
	case 'N':		/* NONE */
		peer_verify_level=SSL_VERIFY_NONE;
		break;
	case 'p':
	case 'P':		/* PEER */
		peer_verify_level=SSL_VERIFY_PEER;
		break;
	case 'r':
	case 'R':		/* REQUIREPEER */
		peer_verify_level=
			SSL_VERIFY_PEER|SSL_VERIFY_FAIL_IF_NO_PEER_CERT;
		break;
	}
	return (peer_verify_level);
}

static int ssl_verify_callback(int goodcert, X509_STORE_CTX *x509)
{
	SSL *ssl=
		X509_STORE_CTX_get_ex_data(x509,
					   SSL_get_ex_data_X509_STORE_CTX_idx()
					   );
	const struct tls_info *info=SSL_get_app_data(ssl);

	if (info->peer_verify_domain || get_peer_verify_level(info))
	{
		if (!goodcert)
			return (0);
	}

	return (1);
}

static int verifypeer(const struct tls_info *info, SSL *ssl)
{
	X509 *x=NULL;
	X509_NAME *subj=NULL;
	int nentries, j;
	char domain[256];
	char *p;
	char errmsg[1000];

	if (!info->peer_verify_domain)
		return (1);

	if (info->isserver)
	{
		x=SSL_get_peer_certificate(ssl);

		if (x)
			subj=X509_get_subject_name(x);
	}
	else
	{
		STACK_OF(X509) *peer_cert_chain=SSL_get_peer_cert_chain(ssl);

		if (peer_cert_chain && peer_cert_chain->stack.num > 0)
		{
			X509 *xx=(X509 *)peer_cert_chain->stack.data[0];

			if (xx)
				subj=X509_get_subject_name(xx);
		}
	}

	
	nentries=0;
	if (subj)
		nentries=X509_NAME_entry_count(subj);

	domain[0]=0;
	for (j=0; j<nentries; j++)
	{
		const char *obj_name;
		X509_NAME_ENTRY *e;
		ASN1_OBJECT *o;
		ASN1_STRING *d;

		int dlen;
		unsigned char *ddata;

		e=X509_NAME_get_entry(subj, j);
		if (!e)
			continue;

		o=X509_NAME_ENTRY_get_object(e);
		d=X509_NAME_ENTRY_get_data(e);

		if (!o || !d)
			continue;

		obj_name=OBJ_nid2sn(OBJ_obj2nid(o));

		dlen=ASN1_STRING_length(d);
		ddata=ASN1_STRING_data(d);

		if (strcasecmp(obj_name, "CN") == 0)
		{
			if (dlen >= sizeof(domain)-1)
				dlen=sizeof(domain)-1;

			memcpy(domain, ddata, dlen);
			domain[dlen]=0;
		}
	}

	if (x)
		X509_free(x);
	p=domain;

	if (*p == '*')
	{
		int	pl, l;

		pl=strlen(++p);
		l=strlen(info->peer_verify_domain);

		if (*p == '.' && pl <= l &&
		    strcasecmp(info->peer_verify_domain+l-pl, p) == 0)
			return (1);
	}
	else if (strcasecmp(info->peer_verify_domain, p) == 0)
		return (1);

	strcpy(errmsg, "couriertls: Mismatched SSL certificate: CN=");
	strcat(errmsg, domain);
	strcat(errmsg, " (expected ");
	strncat(errmsg, info->peer_verify_domain, 256);
	strcat(errmsg, ")");
	(*info->tls_err_msg)(errmsg, info->app_data);
	return (0);
}

#ifndef NO_RSA

static RSA *rsa_callback(SSL *s, int export, int keylength)
{
	return (RSA_generate_key(keylength,RSA_F4,NULL,NULL));
}

#endif

static void nonsslerror(const struct tls_info *info, const char *pfix)
{
	char errmsg[256];

	strcpy(errmsg, "couriertls: ");
	strncat(errmsg, pfix, 200);
	strcat(errmsg, ": ");
	strncat(errmsg, strerror(errno), 255 - strlen(errmsg));

	(*info->tls_err_msg)(errmsg, info->app_data);
}

static void sslerror(const struct tls_info *info, const char *pfix, int rc)
{
	char errmsg[256];
	char errmsgbuf2[300];
	int errnum=ERR_get_error();

	if (errnum == 0)
	{
		if (rc == 0)
		{
			(*info->tls_err_msg)("DEBUG: Unexpected SSL connection shutdown.",
					     info->app_data);
			return;
		}

		nonsslerror(info, pfix);
		return;
	}

	ERR_error_string_n(errnum, errmsg, sizeof(errmsg)-1);

	errmsg[sizeof(errmsg)-1]=0;

	strcpy(errmsgbuf2, "couriertls: ");
	strncat(errmsgbuf2, pfix, 200);
	strcat(errmsgbuf2, ": ");
	strncat(errmsgbuf2, errmsg, 299 - strlen(errmsgbuf2));

	(*info->tls_err_msg)(errmsgbuf2, info->app_data);
}

static void init_session_cache(struct tls_info *, SSL_CTX *);

static int process_rsacertfile(SSL_CTX *ctx, const char *filename)
{
#ifndef	NO_RSA

	const struct tls_info *info=SSL_CTX_get_app_data(ctx);

	SSL_CTX_set_tmp_rsa_callback(ctx, rsa_callback);

	if(!SSL_CTX_use_certificate_chain_file(ctx, filename))
	{
		sslerror(info, filename, -1);
		return (0);
	}

	if(!SSL_CTX_use_RSAPrivateKey_file(ctx, filename, SSL_FILETYPE_PEM))
	{
		sslerror(info, filename, -1);
		return (0);
	}
#endif
	return (1);
}


static int process_dhcertfile(SSL_CTX *ctx, const char *filename)
{
#ifndef	NO_DH

	const struct tls_info *info=SSL_CTX_get_app_data(ctx);
	BIO	*bio;
	DH	*dh;
	int	cert_done=0;

	if(!SSL_CTX_use_certificate_chain_file(ctx, filename))
	{
		sslerror(info, filename, -1);
		return (0);
	}

	if ((bio=BIO_new_file(filename, "r")) != 0)
	{
		if ((dh=PEM_read_bio_DHparams(bio, NULL, NULL, NULL)) != 0)
		{
			SSL_CTX_set_tmp_dh(ctx, dh);
			cert_done=1;
			DH_free(dh);
		}
		else
			sslerror(info, filename, -1);
		BIO_free(bio);
	}
	else
		sslerror(info, filename, -1);

	if (!cert_done)
	{
		(*info->tls_err_msg)("couriertls: DH init failed!",
				     info->app_data);

		return (0);
	}

	if(!SSL_CTX_use_PrivateKey_file(ctx, filename, SSL_FILETYPE_PEM))
	{
		sslerror(info, filename, -1);
		return (0);
	}
#endif
	return (1);
}

static int process_certfile(SSL_CTX *ctx, const char *certfile, const char *ip,
			    int (*func)(SSL_CTX *, const char *))
{
	if (ip && *ip)
	{
		char *test_file;

		if (strncmp(ip, "::ffff:", 7) == 0 && strchr(ip, '.'))
			return (process_certfile(ctx, certfile, ip+7, func));

		test_file= malloc(strlen(certfile)+strlen(ip)+2);

		strcpy(test_file, certfile);
		strcat(test_file, ".");
		strcat(test_file, ip);

		if (access(test_file, R_OK) == 0)
		{
			int rc= (*func)(ctx, test_file);

			free(test_file);
			return rc;
		}
		free(test_file);
	}

	return (*func)(ctx, certfile);
}

SSL_CTX *tls_create(int isserver, const struct tls_info *info)
{
	SSL_CTX *ctx;
	const char *protocol=safe_getenv(info, "TLS_PROTOCOL");
	const char *ssl_cipher_list=safe_getenv(info, "TLS_CIPHER_LIST");
	int session_timeout=atoi(safe_getenv(info, "TLS_TIMEOUT"));
	const char *dhcertfile=safe_getenv(info, "TLS_DHCERTFILE");
	const char *certfile=safe_getenv(info, "TLS_CERTFILE");
	const char *s;
	struct stat stat_buf;
	const char *peer_cert_dir=NULL;
	const char *peer_cert_file=NULL;
	int n;
	struct tls_info *info_copy;

	if (!*ssl_cipher_list)
		ssl_cipher_list=NULL;

	if (!*dhcertfile)
		dhcertfile=NULL;

	if (!*certfile)
		certfile=NULL;

	s=safe_getenv(info, "TLS_TRUSTCERTS");
	if (s && stat(s, &stat_buf) == 0)
	{
		if (S_ISDIR(stat_buf.st_mode))
			peer_cert_dir=s;
		else
			peer_cert_file=s;
	}
	else if (info->peer_verify_domain)
	{
		errno=ENOENT;
		nonsslerror(info, "TLS_TRUSTCERTS not set");
		return (NULL);
	}

	SSL_load_error_strings();
	SSLeay_add_ssl_algorithms();

	info_copy=malloc(sizeof(struct tls_info));

	if (info_copy == NULL)
	{
		nonsslerror(info, "malloc");
		return (NULL);
	}

	memcpy(info_copy, info, sizeof(*info_copy));
	info_copy->isserver=isserver;

	ctx=SSL_CTX_new(protocol && strcmp(protocol, "SSL2") == 0
							? SSLv2_method():
		protocol && strcmp(protocol, "SSL3") == 0 ? SSLv23_method():
		TLSv1_method());

	if (!ctx)
	{
		free(info_copy);
		nonsslerror(info, "SSL_CTX_NEW");
		return (0);
	}
	SSL_CTX_set_app_data(ctx, info_copy);
	SSL_CTX_set_options(ctx, SSL_OP_ALL);

	if (ssl_cipher_list)
		SSL_CTX_set_cipher_list(ctx, ssl_cipher_list);
	SSL_CTX_set_timeout(ctx, session_timeout);

	info_copy->tlscache=NULL;
	init_session_cache(info_copy, ctx);


	s = safe_getenv(info, "TCPLOCALIP");

	if (certfile && !process_certfile(ctx, certfile, s,
					  process_rsacertfile))
	{
		tls_destroy(ctx);
		return (NULL);
	}

	if (dhcertfile && !process_certfile(ctx, dhcertfile, s,
					    process_dhcertfile))
	{
		tls_destroy(ctx);
		return (NULL);
	}

	SSL_CTX_set_session_cache_mode(ctx, SSL_SESS_CACHE_BOTH);

	n=atoi(safe_getenv(info, "TLS_INTCACHESIZE"));

	if (n > 0)
		SSL_CTX_sess_set_cache_size(ctx, n);

	if (peer_cert_dir || peer_cert_file)
	{
		if ((!SSL_CTX_set_default_verify_paths(ctx))
			|| (!SSL_CTX_load_verify_locations(ctx, peer_cert_file,
				peer_cert_dir)))
		{
			sslerror(info, peer_cert_dir, -1);
			tls_destroy(ctx);
			return (0);
		}

		if (isserver && peer_cert_file)
		{
			SSL_CTX_set_client_CA_list(ctx,
						   SSL_load_client_CA_file
						   (peer_cert_file));
		}

		if (isserver && peer_cert_dir)
		{
			DIR *dirp;
			struct dirent *de;
			X509 *x;

			dirp=opendir(peer_cert_dir);
			while (dirp && (de=readdir(dirp)) != NULL)
			{
				const char *p;
				char *q;
				FILE *fp;

				p=strrchr(de->d_name, '.');
				if (!p[0] || !p[1])
					continue;
				while (*++p)
				{
					if (strchr("0123456789", *p) == NULL)
						break;
				}
				if (*p)
					continue;

				q=malloc(strlen(peer_cert_dir)
					 +strlen(de->d_name) + 4);
				if (!q)
				{
					nonsslerror(info, "malloc");
					exit(1);
				}

				strcat(strcat(strcpy(q, peer_cert_dir),
					      "/"), de->d_name);

				fp=fopen(q, "r");
				if (!fp)
				{
					nonsslerror(info, q);
					exit(1);
				}
				free(q);

				while ((x=PEM_read_X509(fp, NULL, NULL, NULL)))
				{
					SSL_CTX_add_client_CA(ctx,x);
					X509_free(x);
				}
				fclose(fp);
			}
			if (dirp)
				closedir(dirp);
                }
	}
	SSL_CTX_set_verify(ctx, get_peer_verify_level(info),
			   ssl_verify_callback);
	return (ctx);
}

void tls_destroy(SSL_CTX *ctx)
{
	struct tls_info *info=SSL_CTX_get_app_data(ctx);

	SSL_CTX_flush_sessions(ctx, 0); /* OpenSSL bug, 2002-08-07 */

	SSL_CTX_free(ctx);

	if (info->tlscache)
	{
		tls_cache_close(info->tlscache);
		info->tlscache=NULL;
	}
	free(info);
}

static int cache_add(SSL *ssl, SSL_SESSION *sess);

static SSL_SESSION *cache_get(SSL *ssl, unsigned char *id, int id_len,
			      int *copyflag);
static void cache_del(SSL_CTX *ctx, SSL_SESSION *ssl);

static void init_session_cache(struct tls_info *info, SSL_CTX *ctx)
{
	const char *filename=safe_getenv(info, "TLS_CACHEFILE");
	const char *cachesize=safe_getenv(info, "TLS_CACHESIZE");
	off_t cachesize_l;

	if (!filename || !*filename)
	{
		SSL_CTX_set_session_cache_mode(ctx, SSL_SESS_CACHE_OFF);
		return;
	}

	if (info->tlscache == NULL)
	{
		cachesize_l= cachesize ? (off_t)atol(cachesize):0;

		if (cachesize_l <= 0)
			cachesize_l=512L * 1024;
		if ((info->tlscache=tls_cache_open(filename, cachesize_l))
		    == NULL)
		{
			nonsslerror(info, filename);
			return;
		}
	}

        SSL_CTX_set_session_cache_mode(ctx, SSL_SESS_CACHE_BOTH);
	SSL_CTX_sess_set_new_cb(ctx, cache_add);
	SSL_CTX_sess_set_get_cb(ctx, cache_get);
	SSL_CTX_sess_set_remove_cb(ctx, cache_del);
}

static int cache_add(SSL *ssl, SSL_SESSION *sess)
{
	struct tls_info *info=SSL_get_app_data(ssl);
	unsigned char buffer[BUFSIZ];
	unsigned char *ucp;
	time_t timeout= (time_t)SSL_SESSION_get_time(sess)
		+ SSL_SESSION_get_timeout(sess);
	void *session_id=(void *)sess->session_id;
	size_t session_id_len=sess->session_id_length;
	size_t sess_len=i2d_SSL_SESSION(sess, NULL);

	if (sizeof(timeout) + sizeof(session_id_len) + session_id_len +
	    sess_len > sizeof(buffer))
	{
		fprintf(stderr, "WARN: starttls.c: buffer not big enough to cache SSL_SESSION\n");
		return (0);	/* Too big */
	}

	memcpy(buffer, &timeout, sizeof(timeout));
	memcpy(buffer+sizeof(timeout), &session_id_len,
	       sizeof(session_id_len));
	memcpy(buffer+sizeof(timeout)+sizeof(session_id_len),
	       session_id, session_id_len);
	ucp=buffer+sizeof(timeout)+
		sizeof(session_id_len)+session_id_len;

	i2d_SSL_SESSION(sess, &ucp);
	if (tls_cache_add(info->tlscache, buffer,
			  (size_t)(sizeof(timeout) +
				   sizeof(session_id_len) +
				   session_id_len + sess_len)))
		perror("ALERT: tls_cache_add: ");

#ifdef TLSCACHEDEBUG
	fprintf(stderr, "INFO: TLSCACHE: added\n");
#endif
	return 0;
}

struct walk_info {
	unsigned char *id;
	int id_len;
	int *copyflag;
	SSL_SESSION *ret;
	time_t now;
};

static int get_func(void *rec, size_t recsize,
		    int *doupdate, void *arg);

static SSL_SESSION *cache_get(SSL *ssl, unsigned char *id, int id_len,
			      int *copyflag)
{
	const struct tls_info *info=SSL_get_app_data(ssl);
	struct walk_info wi;

	wi.id=id;
	wi.id_len=id_len;
	wi.copyflag=copyflag;
	wi.ret=NULL;
	time(&wi.now);
	if (tls_cache_walk(info->tlscache, get_func, &wi) < 0)
		perror("ALERT: tls_cache_walk: ");

#ifdef TLSCACHEDEBUG
	fprintf(stderr, "INFO: TLSCACHE: session %s\n",
		wi.ret ? "found":"not found");
#endif
	return wi.ret;
}

static int get_func(void *rec, size_t recsize,
		    int *doupdate, void *arg)
{
	unsigned char *recp=(unsigned char *)rec;
	struct walk_info *wi=(struct walk_info *)arg;
	time_t timeout;
	size_t session_id_len;

	unsigned char *sess;

	if (recsize < sizeof(timeout)+sizeof(session_id_len))
		return (0);

	memcpy(&timeout, recp, sizeof(timeout));

	if (timeout <= wi->now)
		return (0);

	memcpy(&session_id_len, recp + sizeof(timeout),
	       sizeof(session_id_len));

	if (session_id_len != (size_t)wi->id_len ||
	    memcmp(recp + sizeof(timeout) + sizeof(session_id_len),
		   wi->id, session_id_len))
		return (0);

	sess=recp + sizeof(timeout) + sizeof(session_id_len) + session_id_len;

	wi->ret=d2i_SSL_SESSION(NULL, &sess, recsize - sizeof(timeout) -
				sizeof(session_id_len) - session_id_len);

	*wi->copyflag=0;
	return 1;
}

static int del_func(void *rec, size_t recsize,
		    int *doupdate, void *arg);

static void cache_del(SSL_CTX *ctx, SSL_SESSION *sess)
{
	const struct tls_info *info=SSL_CTX_get_app_data(ctx);
	struct walk_info wi;

	wi.now=0;

	wi.id=(unsigned char *)sess->session_id;
	wi.id_len=sess->session_id_length;
	if (tls_cache_walk(info->tlscache, del_func, &wi) < 0)
		perror("ALERT: tls_cache_walk: ");
}

static int del_func(void *rec, size_t recsize,
		    int *doupdate, void *arg)
{
	unsigned char *recp=(unsigned char *)rec;
	struct walk_info *wi=(struct walk_info *)arg;
	time_t timeout;
	size_t session_id_len;

	if (recsize < sizeof(timeout)+sizeof(session_id_len))
		return (0);

	memcpy(&timeout, recp, sizeof(timeout));

	if (timeout <= wi->now)
		return (0);

	memcpy(&session_id_len, recp + sizeof(timeout),
	       sizeof(session_id_len));

	if (session_id_len != (size_t)wi->id_len ||
	    memcmp(recp + sizeof(timeout) + sizeof(session_id_len),
		   wi->id, session_id_len))
		return (0);

	timeout=0;
	memcpy(recp, &timeout, sizeof(timeout));
	*doupdate=1;
#ifdef TLSCACHEDEBUG
	fprintf(stderr, "INFO: TLSCACHE: deleted\n");
#endif
	return (1);
}


/* ----------------------------------------------------------------- */

SSL *tls_connect(SSL_CTX *ctx, int fd)
{
	struct tls_info *info=SSL_CTX_get_app_data(ctx);
	SSL *ssl;
	int rc;

	/*
	**  Initialize a tls_transfer_info object.
	*/

	if (fcntl(fd, F_SETFL, O_NONBLOCK))
	{
		nonsslerror(info, "fcntl");
		return (NULL);
	}

#ifdef  SO_KEEPALIVE

	{
	int	dummy;

		dummy=1;

		if (setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE,
			(const char *)&dummy, sizeof(dummy)) < 0)
                {
                        nonsslerror(info, "setsockopt");
			return (NULL);
                }
	}
#endif

#ifdef  SO_LINGER
	{
	struct linger l;

		l.l_onoff=0;
		l.l_linger=0;

		if (setsockopt(fd, SOL_SOCKET, SO_LINGER,
			(const char *)&l, sizeof(l)) < 0)
		{
			nonsslerror(info, "setsockopt");
			return (NULL);
		}
	}
#endif

	if (!(ssl=SSL_new(ctx)))
	{
		sslerror(info, "SSL_new", -1);
		return (NULL);
	}

	SSL_set_app_data(ssl, info);

	SSL_set_fd(ssl, fd);
	info->accept_interrupted=0;
	info->connect_interrupted=0;

	if (info->isserver)
	{
		SSL_set_accept_state(ssl);
		if ((rc=SSL_accept(ssl)) > 0)
		{
			if (!verifypeer(info, ssl))
			{
				tls_disconnect(ssl, fd);
				return (NULL);
			}

			if (info->connect_callback != NULL &&
			    !(*info->connect_callback)(ssl, info->app_data))
			{
				tls_disconnect(ssl, fd);
				return (NULL);
			}

			return ssl;
		}
		info->accept_interrupted=1;
	}
	else
	{
		SSL_set_connect_state(ssl);

		if ((rc=SSL_connect(ssl)) > 0)
		{
			if (!verifypeer(info, ssl))
			{
				tls_disconnect(ssl, fd);
				return (NULL);
			}

			if (info->connect_callback != NULL &&
			    !(*info->connect_callback)(ssl, info->app_data))
			{
				tls_disconnect(ssl, fd);
				return (NULL);
			}
			return (ssl);
		}
		info->connect_interrupted=1;
	}

	switch (SSL_get_error(ssl, rc))	{
	case SSL_ERROR_WANT_WRITE:
	case SSL_ERROR_WANT_READ:
		break;
	default:
		sslerror(info, "connect", rc);
		tls_disconnect(ssl, fd);
		return NULL;
	}

	return (ssl);
}

/* --------------------------------------- */

int	tls_transfer(struct tls_transfer_info *t, SSL *ssl, int fd,
		     fd_set *r, fd_set *w)
{
	struct tls_info *info=SSL_get_app_data(ssl);
	int n;

	if (info->connect_interrupted)
	{
		n=SSL_connect(ssl);

		switch (SSL_get_error(ssl, n))	{
		case SSL_ERROR_NONE:
			info->connect_interrupted=0;
			break;
		case SSL_ERROR_WANT_WRITE:
			FD_SET(fd, w);
			return (1);
		case SSL_ERROR_WANT_READ:
			FD_SET(fd, r);
			return (1);
		default:
			info->connect_interrupted=0;
			t->shutdown=1;
			sslerror(info, "connect", n);
			return (-1);
		}

		if (!verifypeer(info, ssl))
		{
			info->connect_interrupted=0;
			t->shutdown=1;
			return (-1);
		}
		if (info->connect_callback != NULL &&
		    !(*info->connect_callback)(ssl, info->app_data))
		{
			info->connect_interrupted=0;
			t->shutdown=1;
			return (-1);
		}
	}
	else if (info->accept_interrupted)
	{
		n=SSL_accept(ssl);

		switch (SSL_get_error(ssl, n))	{
		case SSL_ERROR_NONE:
			info->accept_interrupted=0;
			break;
		case SSL_ERROR_WANT_WRITE:
			FD_SET(fd, w);
			return (1);
		case SSL_ERROR_WANT_READ:
			FD_SET(fd, r);
			return (1);
		default:
			info->accept_interrupted=0;
			t->shutdown=1;
			sslerror(info, "accept", n);
			return (-1);
		}

		if (!verifypeer(info, ssl))
		{
			info->accept_interrupted=0;
			t->shutdown=1;
			return (-1);
		}

		if (info->connect_callback != NULL &&
		    !(*info->connect_callback)(ssl, info->app_data))
		{
			info->accept_interrupted=0;
			t->shutdown=1;
			return (-1);
		}
	}

	if (t->shutdown)
		return -1;

	if (t->shutdown_interrupted && !t->read_interrupted &&
	    !t->write_interrupted)
	{
		n=SSL_shutdown(ssl);
		if (n > 0)
		{
			t->shutdown_interrupted=0;
			t->shutdown=1;
			return -1;
		}

		switch (SSL_get_error(ssl, n))	{
		case SSL_ERROR_WANT_WRITE:
			FD_SET(fd, w);
			break;
		case SSL_ERROR_WANT_READ:
			FD_SET(fd, r);
			break;
		default:
			t->shutdown_interrupted=0;
			t->shutdown= -1;
			return -1;
		}
		return 1;
	}

	if (!t->write_interrupted && t->readleft > 0)
	{
		n=SSL_read(ssl, t->readptr, t->readleft);

		switch (SSL_get_error(ssl, n))	{
		case SSL_ERROR_NONE:
			break;
		case SSL_ERROR_WANT_WRITE:
			t->read_interrupted=1;
			FD_SET(fd, w);
			return (1);
		case SSL_ERROR_WANT_READ:
			FD_SET(fd, r);
			n=0;
			break;
		case SSL_ERROR_WANT_X509_LOOKUP:
			n=0;
			break;
		case SSL_ERROR_ZERO_RETURN:
			t->shutdown=1;
			return (-1);
		default:
			sslerror(info, "read", n);
			return (-1);
		}
		t->read_interrupted=0;
		t->readptr += n;
		t->readleft -= n;

		if (n > 0)
			return (0);
	}

	if (!t->read_interrupted && t->writeleft > 0)
	{
		n=SSL_write(ssl, t->writeptr, t->writeleft);

		switch (SSL_get_error(ssl, n))	{
		case SSL_ERROR_NONE:
			break;
		case SSL_ERROR_WANT_WRITE:
			FD_SET(fd, w);
			n=0;
			break;
		case SSL_ERROR_WANT_READ:
			t->write_interrupted=1;
			FD_SET(fd, r);
			return (1);
		case SSL_ERROR_ZERO_RETURN:
			t->shutdown=1;
			return (-1);
		case SSL_ERROR_WANT_X509_LOOKUP:
			n=0;
			break;
		default:
			return (-1);
		}
		t->write_interrupted=0;
		t->writeptr += n;
		t->writeleft -= n;

		if (n > 0)
			return (0);
	}

	return (1);
}

int tls_connecting(SSL *ssl)
{
	struct tls_info *info=(struct tls_info *)SSL_get_app_data(ssl);

	return info->accept_interrupted || info->connect_interrupted;
}

