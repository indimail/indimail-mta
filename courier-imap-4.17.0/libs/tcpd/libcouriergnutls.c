/*
** Copyright 2007-2013 Double Precision, Inc.
** See COPYING for distribution information.
*/
#include	"config.h"
#include	"argparse.h"
#include	"spipe.h"
#include	"libcouriertls.h"
#include	"tlscache.h"
#include	"soxwrap/soxwrap.h"
#include	<gnutls/gnutls.h>
#ifndef HAVE_GNUTLS3
#include	<gnutls/extra.h>
#endif
#include	<gnutls/x509.h>
#include	<gnutls/openpgp.h>
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

struct oid_name {
	const char *oid;
	const char *name;
};

static struct oid_name oid_name_list[]={
	{"2.5.4.0","objectClass"},
	{"2.5.4.2","knowledgeInformation"},
	{"2.5.4.3","cn"},
	{"2.5.4.4","sn"},
	{"2.5.4.5","serialNumber"},
	{"2.5.4.6","c"},
	{"2.5.4.7","l"},
	{"2.5.4.8","st"},
	{"2.5.4.9","street"},
	{"2.5.4.10","o"},
	{"2.5.4.11","ou"},
	{"2.5.4.12","title"},
	{"2.5.4.13","description"},
	{"2.5.4.14","searchGuide"},
	{"2.5.4.15","businessCategory"},
	{"2.5.4.16","postalAddress"},
	{"2.5.4.17","postalCode"},
	{"2.5.4.18","postOfficeBox"},
	{"2.5.4.19","physicalDeliveryOfficeName"},
	{"2.5.4.20","telephoneNumber"},
	{"2.5.4.21","telexNumber"},
	{"2.5.4.22","teletexTerminalIdentifier"},
	{"2.5.4.23","facsimileTelephoneNumber"},
	{"2.5.4.24","x121Address"},
	{"2.5.4.25","internationaliSDNNumber"},
	{"2.5.4.26","registeredAddress"},
	{"2.5.4.27","destinationIndicator"},
	{"2.5.4.28","preferredDeliveryMethod"},
	{"2.5.4.29","presentationAddress"},
	{"2.5.4.30","supportedApplicationContext"},
	{"2.5.4.31","member"},
	{"2.5.4.32","owner"},
	{"2.5.4.33","roleOccupant"},
	{"2.5.4.35","userPassword"},
	{"2.5.4.36","userCertificate"},
	{"2.5.4.37","cACertificate"},
	{"2.5.4.38","authorityRevocationList"},
	{"2.5.4.39","certificateRevocationList"},
	{"2.5.4.40","crossCertificatePair"},
	{"2.5.4.41","name"},
	{"2.5.4.42","givenName"},
	{"2.5.4.43","initials"},
	{"2.5.4.44","generationQualifier"},
	{"2.5.4.45","x500UniqueIdentifier"},
	{"2.5.4.46","dnQualifier"},
	{"2.5.4.47","enhancedSearchGuide"},
	{"2.5.4.48","protocolInformation"},
	{"2.5.4.49","distinguishedName"},
	{"2.5.4.50","uniqueMember"},
	{"2.5.4.51","houseIdentifier"},
	{"2.5.4.52","supportedAlgorithms"},
	{"2.5.4.53","deltaRevocationList"},
	{"2.5.4.54","dmdName"},
	{"2.5.4.65","pseudonym"},
	{"0.9.2342.19200300.100.1.3","mail"},
	{"0.9.2342.19200300.100.1.25","dc"},
	{"0.9.2342.19200300.100.1.1","uid"},
	{"1.3.6.1.1.3.1","uidObject"},
	{"1.2.840.113549.1.9.1","emailaddress"},
};


struct ssl_context_t {
	int isserver;
	struct tls_info info_cpy;
	const char *priority_list;

	char *certfile;
	char *dhfile;

	char *trustcerts;

	int verify_cert;
	int fail_if_no_cert;
};

struct ssl_handle_t {
	struct tls_info info_cpy;
	ssl_context ctx;
	gnutls_anon_client_credentials_t anonclientcred;
	gnutls_anon_server_credentials_t anonservercred;
	gnutls_certificate_credentials_t xcred;
	gnutls_dh_params_t dhparams;
	int dhparams_initialized;
	gnutls_session_t session;

	gnutls_x509_privkey_t x509_key;

	gnutls_openpgp_crt_t pgp_crt;
	gnutls_openpgp_privkey_t pgp_key;
};

static void nonsslerror(struct tls_info *info, const char *pfix)
{
        char errmsg[256];

        strcpy(errmsg, "couriertls: ");
        strncat(errmsg, pfix, 200);
        strcat(errmsg, ": ");
        strncat(errmsg, strerror(errno), 255 - strlen(errmsg));

        (*info->tls_err_msg)(errmsg, info->app_data);
}

static const char *safe_getenv(ssl_context context, const char *n,
			       const char *def)
{
	const char *v=(*context->info_cpy.getconfigvar)
		(n, context->info_cpy.app_data);

	if (!v)	v="";

	if (!*v)
		v=def;
	return (v);
}

static void log_2stderr( int level, const char *s)
{
	fprintf(stderr, "%s", s);
}

ssl_context tls_create(int isserver, const struct tls_info *info)
{
	static int first=1;

	ssl_context p=malloc(sizeof(struct ssl_context_t));
	char *certfile=NULL;
	char debug_flag;

	if (!p)
		return NULL;

	memset(p, 0, sizeof(*p));

	p->isserver=isserver;
	p->info_cpy=*info;
	p->info_cpy.certificate_verified=0;

	debug_flag=*safe_getenv(p, "TLS_DEBUG", "");

	if (first)
	{
		if (gnutls_check_version(LIBGNUTLS_VERSION) == NULL)
		{
			fprintf(stderr, "GnuTLS version mismatch\n");
			free(p);
			errno=EINVAL;
			return (NULL);
		}

		first=0;

		if (debug_flag)
		{
			gnutls_global_set_log_function(log_2stderr);
			gnutls_global_set_log_level(9);
		}

		if (gnutls_global_init() < 0)
		{
			fprintf(stderr, "gnutls_global_init() failed\n");
			free(p);
			errno=EINVAL;
			return (NULL);
		}

#ifndef HAVE_GNUTLS3
		if (gnutls_global_init_extra() < 0)
		{
			gnutls_global_deinit();
			fprintf(stderr, "gnutls_global_init() failed\n");
			free(p);
			errno=EINVAL;
			return (NULL);
		}
#endif
	}

	p->priority_list=safe_getenv(p, "TLS_PRIORITY",
				     "NORMAL:-CTYPE-OPENPGP");

	if ((certfile=strdup(safe_getenv(p, "TLS_CERTFILE", ""))) == NULL ||
	    (p->trustcerts=strdup(safe_getenv(p, "TLS_TRUSTCERTS", "")))
	    == NULL)
	{
		if (certfile)
			free(certfile);
		tls_destroy(p);
		return NULL;
	}

	if (*certfile)
	{
		p->certfile=certfile;
		certfile=NULL;
	}

	if (certfile)
		free(certfile);

	if ((certfile=strdup(safe_getenv(p, "TLS_DHPARAMS", ""))) != NULL &&
	    *certfile)
	{
		p->dhfile=certfile;
	}
	else
	{
		if (certfile)
			free(certfile);
	}

	switch (*safe_getenv(p, "TLS_VERIFYPEER", "P")) {
	case 'n':
	case 'N':
		p->verify_cert=0;
		p->fail_if_no_cert=0;
		break;
	case 'p':
	case 'P':		/* PEER */
		p->verify_cert=1;
		p->fail_if_no_cert=0;
		break;
	case 'r':
	case 'R':		/* REQUIREPEER */
		p->verify_cert=1;
		p->fail_if_no_cert=1;
		break;
	}

	if (info->peer_verify_domain)
		p->verify_cert=p->fail_if_no_cert=1;

	{
		const char *filename=safe_getenv(p, "TLS_CACHEFILE", "");
		const char *cachesize=safe_getenv(p, "TLS_CACHESIZE", "");
		off_t cachesize_l;

		if (filename && *filename)
		{
			cachesize_l= cachesize ? (off_t)atol(cachesize):0;

			if (cachesize_l <= 0)
				cachesize_l=512L * 1024;
			if ((p->info_cpy.tlscache=tls_cache_open(filename,
								 cachesize_l))
			    == NULL)
			{
				nonsslerror(&p->info_cpy, filename);
				tls_destroy(p);
				return NULL;
			}
		}
	}

#if 0
	int session_timeout=atoi(safe_getenv(ctx, "TLS_TIMEOUT"));
#endif
	return p;
}

void tls_destroy(ssl_context p)
{
	if (p->certfile)
		free(p->certfile);
	if (p->dhfile)
		free(p->dhfile);
	if (p->trustcerts)
		free(p->trustcerts);

	if (p->info_cpy.tlscache)
		tls_cache_close(p->info_cpy.tlscache);
	free(p);
}

int tls_certificate_verified(ssl_handle ssl)
{
	return ssl->info_cpy.certificate_verified;
}

static int read_cert_dir(const char *cert_dir,
			 int (*cb_func)(const char *filename,
					struct stat *stat_buf,
					void *arg),
			 void *arg)
{
	DIR *dirp;
	struct dirent *de;
	int rc=0;

	if ((dirp=opendir(cert_dir)) == NULL)
		return 0;

	while ((de=readdir(dirp)) != NULL)
	{
		char *buf;
		struct stat stat_buf;

		if (de->d_name[0] == '.')
			continue;

		buf=malloc(strlen(cert_dir)+strlen(de->d_name)+2);

		if (!buf)
			continue;

		strcat(strcat(strcpy(buf, cert_dir), "/"), de->d_name);

		if (lstat(buf, &stat_buf) < 0 || !S_ISREG(stat_buf.st_mode))
		{
			free(buf);
			continue;
		}

		rc=(*cb_func)(buf, &stat_buf, arg);
		free(buf);
		if (rc)
			break;
	}
	closedir(dirp);
	return rc;
}

static int cnt_cert_size(const char *filename,
			 struct stat *stat_buf,
			 void *arg)
{
	*(size_t *)arg += stat_buf->st_size;
	return 0;
}

struct cert_buf_ptr {
	char *ptr;
	size_t cnt;
};

static int save_cert_to_buf(const char *filename,
			    struct stat *stat_buf,
			    void *arg)
{
	struct cert_buf_ptr *p=(struct cert_buf_ptr *)arg;
	FILE *fp;

	if (p->cnt < stat_buf->st_size)
		return 1;

	fp=fopen(filename, "r");

	if (fp)
	{
		if (stat_buf->st_size &&
		    fread(p->ptr, stat_buf->st_size, 1, fp) != 1)
		{
			fclose(fp);
			return 1;
		}
		fclose(fp);
	}
	p->ptr += stat_buf->st_size;
	p->cnt -= stat_buf->st_size;
	return 0;
}


static int add_certificates(gnutls_certificate_credentials_t xcred,
			    const char *certfile)
{
	struct stat stat_buf;
	struct cert_buf_ptr ptr;
	gnutls_datum_t datum_ptr;

	if (!certfile || !*certfile || stat(certfile, &stat_buf) < 0)
		return 0;

	if (S_ISREG(stat_buf.st_mode))
	{
		return gnutls_certificate_set_x509_trust_file(xcred, certfile,
							      GNUTLS_X509_FMT_PEM);
	}

	if (!S_ISDIR(stat_buf.st_mode))
		return 0;

	ptr.cnt=0;

	if (read_cert_dir(certfile, cnt_cert_size, &ptr.cnt))
		return 0;

	datum_ptr.data=malloc(ptr.cnt+1);
	datum_ptr.size=ptr.cnt;

	if (!datum_ptr.data)
		return 0;

	ptr.ptr=(char *)datum_ptr.data;

	if (read_cert_dir(certfile, save_cert_to_buf, &ptr) ||
	    ptr.cnt)
	{
		free(datum_ptr.data);
		return 0;
	}
	*ptr.ptr=0;

	gnutls_certificate_set_x509_trust_mem(xcred, &datum_ptr,
					      GNUTLS_X509_FMT_PEM);
	free(datum_ptr.data);

	return 0;
}

static void tls_free_session_keys(ssl_handle ssl)
{
	if (ssl->x509_key)
		gnutls_x509_privkey_deinit(ssl->x509_key);

	if (ssl->pgp_crt)
		gnutls_openpgp_key_deinit(ssl->pgp_crt);

	if (ssl->pgp_key)
		gnutls_openpgp_privkey_deinit(ssl->pgp_key);

	ssl->x509_key=NULL;
	ssl->pgp_crt=NULL;
	ssl->pgp_key=NULL;

}

static void tls_free_session(ssl_handle ssl)
{
	gnutls_deinit(ssl->session);
	gnutls_certificate_free_credentials(ssl->xcred);
	gnutls_anon_free_client_credentials(ssl->anonclientcred);
	gnutls_anon_free_server_credentials(ssl->anonservercred);
	gnutls_dh_params_deinit(ssl->dhparams);
	tls_free_session_keys(ssl);
	free(ssl);
}

static int chk_error(int rc, ssl_handle ssl, int fd, fd_set *r, fd_set *w,
		     int *result_rc)
{
	if (rc == GNUTLS_E_SUCCESS)
	{
		if (result_rc)
			*result_rc=0;
		return 0;
	}

	if (rc == GNUTLS_E_WARNING_ALERT_RECEIVED)
		return 1;

	if (rc == GNUTLS_E_FATAL_ALERT_RECEIVED)
	{
		const char *alert=
			gnutls_alert_get_name(gnutls_alert_get(ssl->session));
		(*ssl->info_cpy.tls_err_msg)(alert, ssl->info_cpy.app_data);

		if (result_rc)
			*result_rc= -1;
		return 0;
	}

	if (rc == GNUTLS_E_AGAIN || rc == GNUTLS_E_INTERRUPTED)
	{
		fd_set *p=gnutls_record_get_direction(ssl->session)
			? w:r;

		if (p)
			FD_SET(fd, p);

		if (result_rc)
			*result_rc=1;
		return 0;
	}

	if (result_rc)
	{
		(*ssl->info_cpy.tls_err_msg)(gnutls_strerror(rc),
					     ssl->info_cpy.app_data);
		*result_rc= -1;
	}
	return 0;
}

static int verify_client(ssl_handle ssl, int fd)
{
	unsigned int status;
	int rc;
	const gnutls_datum_t *cert_list;
	unsigned int cert_list_size;

	if (!ssl->ctx->verify_cert)
		return 0;

	cert_list = gnutls_certificate_get_peers(ssl->session, &cert_list_size);
	if (cert_list == NULL || cert_list_size == 0)
	{
		if (ssl->ctx->fail_if_no_cert)
		{
			(*ssl->info_cpy.tls_err_msg)
				("No certificate supplied by peer",
				 ssl->info_cpy.app_data);
			return GNUTLS_E_INSUFFICIENT_CREDENTIALS;
		}
		return 0;
	}

	status=0;
	rc=gnutls_certificate_verify_peers2(ssl->session, &status);

	if (rc)
	{
		(*ssl->info_cpy.tls_err_msg)
			("Peer certificate verification failed",
			 ssl->info_cpy.app_data);
		return GNUTLS_E_INSUFFICIENT_CREDENTIALS;
	}

	if (status)
	{
		(*ssl->info_cpy.tls_err_msg)
			(status & GNUTLS_CERT_REVOKED ?
			 "Peer's certificate is revoked":
			 status & GNUTLS_CERT_SIGNER_NOT_FOUND ?
			 "Peer's certificate not signed by a trusted authority":
			 status & GNUTLS_CERT_SIGNER_NOT_CA ?
			 "Invalid peer certificate authority":
			 status & GNUTLS_CERT_INSECURE_ALGORITHM ?
			 "Peer's certificate does not use a secure checksum":
			 "Invalid peer certificate",
			 ssl->info_cpy.app_data);
		return GNUTLS_E_INSUFFICIENT_CREDENTIALS;
	}

	if (gnutls_certificate_type_get(ssl->session) == GNUTLS_CRT_X509)
	{
		gnutls_x509_crt_t cert;

		if (gnutls_x509_crt_init(&cert) < 0)
		{
			(*ssl->info_cpy.tls_err_msg)
				("Error initializing certificate",
				 ssl->info_cpy.app_data);
			return GNUTLS_E_INSUFFICIENT_CREDENTIALS;
		}

		if (gnutls_x509_crt_import(cert, &cert_list[0],
					   GNUTLS_X509_FMT_DER) < 0)
		{
			(*ssl->info_cpy.tls_err_msg)
				("Error parsing certificate",
				 ssl->info_cpy.app_data);
			gnutls_x509_crt_deinit (cert);
			return GNUTLS_E_INSUFFICIENT_CREDENTIALS;
		}


		if (gnutls_x509_crt_get_expiration_time(cert) < time(NULL))
		{
			(*ssl->info_cpy.tls_err_msg)
				("Expired certificate",
				 ssl->info_cpy.app_data);
			gnutls_x509_crt_deinit (cert);
			return GNUTLS_E_INSUFFICIENT_CREDENTIALS;
		}

		if (gnutls_x509_crt_get_activation_time(cert) > time(NULL))
		{
			(*ssl->info_cpy.tls_err_msg)
				("Certificate not activated",
				 ssl->info_cpy.app_data);
			gnutls_x509_crt_deinit (cert);
			return GNUTLS_E_INSUFFICIENT_CREDENTIALS;
		}

		if (ssl->info_cpy.peer_verify_domain &&
		    *ssl->info_cpy.peer_verify_domain &&
		    !gnutls_x509_crt_check_hostname(cert,
						    ssl->info_cpy
						    .peer_verify_domain
						    ))
		{
			char hostname[256];
			size_t hostname_size=sizeof(hostname);
			const char *errmsg_txt="Certificate owner mismatch: ";
			char *errmsg_buf;

			if (gnutls_x509_crt_get_dn_by_oid(cert,
							  "2.5.4.3", 0,
							  0, hostname,
							  &hostname_size) < 0)
				strcpy(hostname,"(unknown)");

			errmsg_buf=malloc(strlen(errmsg_txt)+
					  strlen(hostname)+10);

			if (errmsg_buf)
				strcat(strcpy(errmsg_buf, errmsg_txt),
				       hostname);

			(*ssl->info_cpy.tls_err_msg)
				(errmsg_buf ? errmsg_buf: strerror(errno),
				 ssl->info_cpy.app_data);
			gnutls_x509_crt_deinit (cert);

			if (errmsg_buf)
				free(errmsg_buf);
			return GNUTLS_E_INSUFFICIENT_CREDENTIALS;
		}

		gnutls_x509_crt_deinit (cert);
	}
	else if (gnutls_certificate_type_get(ssl->session)==GNUTLS_CRT_OPENPGP)
	{
		gnutls_openpgp_crt_t cert;

		if (gnutls_openpgp_key_init(&cert) < 0)
		{
			(*ssl->info_cpy.tls_err_msg)
				("Error initializing certificate",
				 ssl->info_cpy.app_data);
			gnutls_openpgp_key_deinit(cert);
			return GNUTLS_E_INSUFFICIENT_CREDENTIALS;
		}

		if (gnutls_openpgp_key_import(cert, &cert_list[0],
					      GNUTLS_OPENPGP_FMT_RAW) < 0)
		{
			(*ssl->info_cpy.tls_err_msg)
				("Error parsing certificate",
				 ssl->info_cpy.app_data);
			gnutls_openpgp_key_deinit (cert);
			return GNUTLS_E_INSUFFICIENT_CREDENTIALS;
		}

		if (gnutls_openpgp_key_get_creation_time(cert) > time(NULL))
		{
			(*ssl->info_cpy.tls_err_msg)
				("Certificate not activated",
				 ssl->info_cpy.app_data);
			gnutls_openpgp_key_deinit (cert);
			return GNUTLS_E_INSUFFICIENT_CREDENTIALS;
		}

		if (gnutls_openpgp_key_get_expiration_time(cert) < time(NULL))
		{
			(*ssl->info_cpy.tls_err_msg)
				("Expired certificate",
				 ssl->info_cpy.app_data);
			gnutls_openpgp_key_deinit (cert);
			return GNUTLS_E_INSUFFICIENT_CREDENTIALS;
		}

		if (ssl->info_cpy.peer_verify_domain &&
		    *ssl->info_cpy.peer_verify_domain &&
		    !gnutls_openpgp_key_check_hostname(cert,
						       ssl->info_cpy
						       .peer_verify_domain))

		{
			char *hostname;
			size_t hostnamesiz=0;
			const char *errmsg_txt=
				"Certificate owner mismatch: ";
			char *errmsg_buf;

			gnutls_openpgp_key_get_name(cert, 0, NULL,
						    &hostnamesiz);

			hostname=malloc(hostnamesiz);

			if (hostname)
			{
				*hostname=0;
				gnutls_openpgp_key_get_name(cert,
							    0, hostname,
							    &hostnamesiz);
			}

			errmsg_buf=malloc(strlen(errmsg_txt)+
					  strlen(hostname ?
						 hostname:"")+100);

			if (errmsg_buf)
				strcat(strcpy(errmsg_buf, errmsg_txt),
					       hostname ?
					       hostname:"(unknown)");

			(*ssl->info_cpy.tls_err_msg)
				(errmsg_buf ? errmsg_buf:strerror(errno),
				 ssl->info_cpy.app_data);
			if (errmsg_buf)
				free(errmsg_buf);
			if (hostname)
				free(hostname);
			gnutls_openpgp_key_deinit (cert);
			return GNUTLS_E_INSUFFICIENT_CREDENTIALS;
		}
		gnutls_openpgp_key_deinit (cert);
	}
	else
	{
		(*ssl->info_cpy.tls_err_msg)
			("No certificate supplied by peer",
			 ssl->info_cpy.app_data);
		return GNUTLS_E_INSUFFICIENT_CREDENTIALS;
	}

	ssl->info_cpy.certificate_verified=1;
	return 0;
}

static int dohandshake(ssl_handle ssl, int fd, fd_set *r, fd_set *w)
{
	int rc;

	while (chk_error(gnutls_handshake(ssl->session),
			 ssl, fd, r, w, &rc))
		;

	if (rc == 0)
	{
		ssl->info_cpy.connect_interrupted=0;


		if (verify_client(ssl, fd))
			return -1;

		if (ssl->info_cpy.connect_callback != NULL &&
		    !(*ssl->info_cpy.connect_callback)(ssl,
						       ssl->info_cpy.app_data))
			return (-1);
	}
	return rc;
}

static char *check_cert(const char *filename,
			gnutls_certificate_type_t cert_type,
			const char *req_dn,
			int isvirtual)
{
	if (!filename || !*filename)
		return NULL;

	while (*req_dn)
	{
		char *p=malloc(strlen(filename)+strlen(req_dn)+10);

		if (!p)
			return NULL;

		strcat(strcat(strcpy(p, filename), "."), req_dn);

		if (cert_type == GNUTLS_CRT_OPENPGP)
			strcat(p, ".pgp");

		if (access(p, R_OK) == 0)
			return p;

		free(p);

		if (!isvirtual)
			break;

		if ((req_dn=strchr(req_dn, '.')) == NULL)
			break;
		++req_dn;
	}

	{
		char *p=malloc(strlen(filename)+10);

		if (!p)
			return NULL;

		strcpy(p, filename);

		if (cert_type == GNUTLS_CRT_OPENPGP)
			strcat(p, ".pgp");

		if (access(p, R_OK) == 0)
			return p;

		free(p);
	}
	return NULL;
}

static int read_file(const char *file,
		     gnutls_datum_t *filebuf)
{
	FILE *fp;
	struct stat stat_buf;

	filebuf->data=NULL;

	if ((fp=fopen(file, "r")) == NULL ||
	    fstat(fileno(fp), &stat_buf) < 0)
	{
		if (fp)
			fclose(fp);
		return GNUTLS_E_FILE_ERROR;
	}

	if ((filebuf->data=malloc(stat_buf.st_size)) == NULL)
	{
		fclose(fp);
		return GNUTLS_E_MEMORY_ERROR;
	}

	if (fread(filebuf->data, filebuf->size=stat_buf.st_size, 1, fp) != 1)
	{
		if (fp)
			fclose(fp);
		return GNUTLS_E_FILE_ERROR;
	}
	return 0;
}

static void release_file(gnutls_datum_t *filebuf)
{
	if (filebuf->data)
		free(filebuf->data);
	filebuf->data=NULL;
}

static int set_cert(ssl_handle ssl,
		    gnutls_session_t session,
		    gnutls_retr2_st *st,
		    const char *certfilename)
{
	int rc;
	gnutls_datum_t filebuf;
	unsigned int cert_cnt;

	st->ncerts=0;
	st->deinit_all=0;
	tls_free_session_keys(ssl);

	if ((rc=read_file(certfilename, &filebuf)) < 0)
		return rc;

	switch (st->cert_type) {
	case GNUTLS_CRT_X509:

		cert_cnt=0;

		if ((rc=gnutls_x509_privkey_init(&ssl->x509_key)) < 0 ||
		    (rc=gnutls_x509_privkey_import(ssl->x509_key, &filebuf,
						   GNUTLS_X509_FMT_PEM)) < 0)
			break;

		rc=gnutls_x509_crt_list_import(NULL, &cert_cnt,
					       &filebuf,
					       GNUTLS_X509_FMT_PEM,
					       GNUTLS_X509_CRT_LIST_IMPORT_FAIL_IF_EXCEED);

		if (rc != GNUTLS_E_SHORT_MEMORY_BUFFER)
			break;

		st->ncerts=cert_cnt+1;
		st->cert.x509=gnutls_malloc(st->ncerts*sizeof(*st->cert.x509));

		rc=gnutls_x509_crt_list_import(st->cert.x509, &st->ncerts,
					       &filebuf,
					       GNUTLS_X509_FMT_PEM, 0);

		if (rc < 0)
		{
			st->ncerts=0;
			gnutls_free(st->cert.x509);
			st->cert.x509=0;
			break;
		}
		st->ncerts=rc;
		st->key.x509=ssl->x509_key;
		ssl->x509_key=0;
		st->deinit_all=1;

		break;
	case GNUTLS_CRT_OPENPGP:
		if ((rc=gnutls_openpgp_key_init(&ssl->pgp_crt)) < 0 ||
		    (rc=gnutls_openpgp_privkey_init(&ssl->pgp_key)) < 0 ||
		    (rc=gnutls_openpgp_key_import(ssl->pgp_crt, &filebuf,
						  GNUTLS_OPENPGP_FMT_BASE64))
		    < 0 ||
		    (rc=gnutls_openpgp_privkey_import(ssl->pgp_key, &filebuf,
						      GNUTLS_OPENPGP_FMT_BASE64,
						      NULL, 0)) < 0)
			break;
		st->cert.pgp=ssl->pgp_crt;
		st->ncerts=1;
		st->key.pgp=ssl->pgp_key;
		break;
	default:
		break;
	}

	release_file(&filebuf);
	return 0;
}

static int get_server_cert(gnutls_session_t session,
			   const gnutls_datum_t * req_ca_rdn, int nreqs,
			   const gnutls_pk_algorithm_t * sign_algos,
			   int sign_algos_length, gnutls_retr2_st *st)

{
	ssl_handle ssl=(ssl_handle)gnutls_session_get_ptr(session);
	int vhost_idx;
	char *vhost_buf;
	size_t vhost_max_size=0;
	size_t vhost_size;
	unsigned int type=GNUTLS_NAME_DNS;
	char *certfilename=NULL;
	int rc;

	st->cert_type=gnutls_certificate_type_get(session);

	for (vhost_idx=0; vhost_size=0,
		     gnutls_server_name_get(session, NULL, &vhost_size, &type,
					    vhost_idx) ==
		     GNUTLS_E_SHORT_MEMORY_BUFFER; ++vhost_idx)
	{
		if (++vhost_size > vhost_max_size)
			vhost_max_size=vhost_size;
	}

	vhost_buf=malloc(vhost_max_size);

	if (!vhost_buf)
		return GNUTLS_E_MEMORY_ERROR;

	for (vhost_idx=0; vhost_size=vhost_max_size,
		     gnutls_server_name_get(session, vhost_buf, &vhost_size,
					    &type,
					    vhost_idx) == GNUTLS_E_SUCCESS;
	     ++vhost_idx)
	{
		char *p;

		for (p=vhost_buf; *p; p++)
			if (*p == '/')
				*p='.'; /* Script kiddie check */

		if (ssl->ctx->certfile)
			certfilename=check_cert(ssl->ctx->certfile,
						st->cert_type,
						vhost_buf, 1);

		if (certfilename)
			break;
	}

	if (!certfilename)
	{
		if (ssl->ctx->certfile)
			certfilename=check_cert(ssl->ctx->certfile,
						st->cert_type,
						safe_getenv(ssl->ctx,
							    "TCPLOCALIP", ""),
						0);
	}

	if (!certfilename)
		return 0;

	rc=set_cert(ssl, session, st, certfilename);
	free(certfilename);
	return rc;
}


static int pick_client_cert(gnutls_session_t session,
			    const gnutls_datum_t * req_ca_rdn, int nreqs,
			    const gnutls_pk_algorithm_t * sign_algos,
			    int sign_algos_length, gnutls_retr2_st *st)
{
	ssl_handle ssl=(ssl_handle)gnutls_session_get_ptr(session);
	int i, j;
	const char *cert_array;
	size_t cert_array_size;
	int rc=0;

	if (ssl->info_cpy.getpemclientcert4ca == NULL)
		return 0;

	if (st->cert_type != GNUTLS_CRT_X509)
		return 0;

	if (ssl->info_cpy.loadpemclientcert4ca)
		(*ssl->info_cpy.loadpemclientcert4ca)(ssl->info_cpy.app_data);

	for (j=0; (*ssl->info_cpy.getpemclientcert4ca)(j, &cert_array,
						       &cert_array_size,
						       ssl->info_cpy.app_data);
	     ++j)
	{
		gnutls_datum_t data;
		unsigned int cert_cnt=0;
		gnutls_x509_crt_t *certbuf;
		size_t issuer_buf_size=0;
		char *issuer_rdn;
		gnutls_x509_privkey_t pk;

		data.data=(unsigned char *)cert_array;
		data.size=cert_array_size;
		gnutls_x509_privkey_init(&pk);
		if (gnutls_x509_privkey_import(pk, &data,
					       GNUTLS_X509_FMT_PEM)
		    != GNUTLS_E_SUCCESS)
		{
			gnutls_x509_privkey_deinit(pk);
			continue;
		}

		data.data=(void *)cert_array;
		data.size=cert_array_size;;

		gnutls_x509_crt_list_import(NULL, &cert_cnt, &data,
					    GNUTLS_X509_FMT_PEM,
					    GNUTLS_X509_CRT_LIST_IMPORT_FAIL_IF_EXCEED);
		if (cert_cnt == 0)
		{
			gnutls_x509_privkey_deinit(pk);
			continue;
		}

		certbuf=gnutls_malloc(sizeof(*certbuf)*cert_cnt);

		if (!certbuf)
		{
			gnutls_x509_privkey_deinit(pk);
			continue;
		}

		if (gnutls_x509_crt_list_import(certbuf, &cert_cnt, &data,
						GNUTLS_X509_FMT_PEM,
						GNUTLS_X509_CRT_LIST_IMPORT_FAIL_IF_EXCEED) < 0)
		{
			free(certbuf);
			gnutls_x509_privkey_deinit(pk);
			continue;
		}


		gnutls_x509_crt_get_issuer_dn(certbuf[0], NULL,
					      &issuer_buf_size);

		++issuer_buf_size;

		issuer_rdn=gnutls_malloc(issuer_buf_size+1);

		if (gnutls_x509_crt_get_issuer_dn(certbuf[0], issuer_rdn,
						  &issuer_buf_size)
		    != GNUTLS_E_SUCCESS)
		{
			gnutls_free(issuer_rdn);
			issuer_rdn=0;
		}
		else
			issuer_rdn[issuer_buf_size]=0;

		for (i=0; issuer_rdn && i<nreqs; i++)
		{
			size_t buf_size=0;
			char *ca_rdn;

			gnutls_x509_rdn_get(&req_ca_rdn[i], NULL, &buf_size);

			++buf_size;

			ca_rdn=gnutls_malloc(buf_size+1);

			if (gnutls_x509_rdn_get(&req_ca_rdn[i], ca_rdn, &buf_size) !=
			    GNUTLS_E_SUCCESS)
			{
				gnutls_free(ca_rdn);
				continue;
			}

			ca_rdn[buf_size]=0;

			if (strcmp(ca_rdn, issuer_rdn) == 0)
				break;
			gnutls_free(ca_rdn);
		}

		st->ncerts=0;
		if (issuer_rdn && i < nreqs)
		{
			st->cert.x509=certbuf;
			st->ncerts=cert_cnt;
			st->deinit_all=1;
			st->key.x509=pk;
			cert_cnt=0;
			rc=1;
		}
		else
		{
			gnutls_x509_privkey_deinit(pk);
			while (cert_cnt)
				gnutls_x509_crt_deinit(certbuf[--cert_cnt]);
			gnutls_free(certbuf);
		}
		gnutls_free(issuer_rdn);
		if (rc)
			break;
	}

	return rc;
}

static int get_client_cert(gnutls_session_t session,
			   const gnutls_datum_t * req_ca_rdn, int nreqs,
			   const gnutls_pk_algorithm_t * sign_algos,
			   int sign_algos_length, gnutls_retr2_st *st)
{
	ssl_handle ssl=(ssl_handle)gnutls_session_get_ptr(session);
	int rc;
	char *certfilename=NULL;

	rc= 0;
	st->cert_type=gnutls_certificate_type_get(session);

	if (ssl->ctx->certfile)
		certfilename=check_cert(ssl->ctx->certfile,
					st->cert_type, "", 0);

	st->ncerts=0;
	st->deinit_all=0;

	if (certfilename)
	{
		rc=set_cert(ssl, session, st, certfilename);
		free(certfilename);
	}
	else
	{
		rc=pick_client_cert(session, req_ca_rdn, nreqs, sign_algos,
				    sign_algos_length, st);
		if (rc > 0)
			rc=0;
	}
	return rc;
}

static int read_dh_params(gnutls_dh_params_t dhparams,
			  const char *filename,
			  int *dhparams_initialized)
{
	int rc;
	gnutls_datum_t filebuf;

	if (*dhparams_initialized)
		return 0;

	if (!filename)
		return 0;

	rc=read_file(filename, &filebuf);

	if (rc == 0)
	{
		if (gnutls_dh_params_import_pkcs3(dhparams, &filebuf,
						  GNUTLS_X509_FMT_PEM) == 0)
			*dhparams_initialized=1;
		release_file(&filebuf);
	}
	return 0;
}

static int db_store_func(void *dummy, gnutls_datum_t key,
			 gnutls_datum_t data)
{
	char *p=malloc(key.size + data.size + sizeof(int));

	if (!p)
		return -1;

	memcpy(p, &key.size, sizeof(key.size));
	memcpy(p+sizeof(key.size), key.data, key.size);
	memcpy(p+sizeof(key.size)+key.size, data.data, data.size);

	tls_cache_add(((ssl_handle)dummy)->info_cpy.tlscache, p,
		      key.size + data.size + sizeof(int));
	free(p);
	return 0;
}

static int do_cache_remove(void *rec, size_t recsize, int *doupdate, void *arg)
{
	char *recptr=rec;
	gnutls_datum_t *key=(gnutls_datum_t *)arg;

	if (recsize >= key->size + sizeof(key->size))
	{
		gnutls_datum_t dummy;

		memcpy(&dummy.size, recptr, sizeof(dummy.size));

		if (dummy.size == key->size &&
		    memcmp(recptr + sizeof(key->size),
			   key->data, key->size) == 0)
		{
			dummy.size= -1;
			memcpy(recptr, &dummy.size, sizeof(dummy.size));
			*doupdate=1;
			return 1;
		}
	}
	return 0;
}

static int db_remove_func(void *dummy, gnutls_datum_t key)
{
	tls_cache_walk(((ssl_handle)dummy)->info_cpy.tlscache,
		       do_cache_remove, &key);
	return 0;
}

struct db_retrieve_s {
	gnutls_datum_t ret;
	gnutls_datum_t *key;
};

static int do_cache_retrieve(void *rec, size_t recsize, int *doupdate,
			     void *arg)
{
	char *recptr=rec;
	struct db_retrieve_s *ret=(struct db_retrieve_s *)arg;

	if (recsize >= ret->key->size + sizeof(ret->key->size))
	{
		gnutls_datum_t dummy;

		memcpy(&dummy.size, recptr, sizeof(dummy.size));

		if (dummy.size == ret->key->size &&
		    memcmp(recptr+sizeof(dummy.size),
			   ret->key->data,
			   ret->key->size) == 0)
		{
			ret->ret.size=recsize-sizeof(dummy.size)-ret->key->size;

			ret->ret.data=gnutls_malloc(ret->ret.size);

			if (ret->ret.data)
				memcpy(ret->ret.data,
				       (void *)(recptr+sizeof(dummy.size)
						+ret->key->size),
				       ret->ret.size);
			else
				ret->ret.size=0;

			return 1;
		}
	}
	return 0;
}

static gnutls_datum_t db_retrieve_func(void *dummy, gnutls_datum_t key)
{
	struct db_retrieve_s drs;

	drs.ret.data=NULL;
	drs.ret.size=0;
	drs.key= &key;

	tls_cache_walk(((ssl_handle)dummy)->info_cpy.tlscache,
		       do_cache_retrieve, &drs);
	return drs.ret;
}

ssl_handle tls_connect(ssl_context ctx, int fd)
{
	ssl_handle ssl=malloc(sizeof(struct ssl_handle_t));

	if (!ssl)
		return NULL;

	memset(ssl, 0, sizeof(*ssl));

	ssl->info_cpy=ctx->info_cpy;
	ssl->ctx=ctx;

	if (ctx->info_cpy.peer_verify_domain && !*ctx->trustcerts)
	{
		errno=ENOENT;
		(*ctx->info_cpy.tls_err_msg)( "TLS_TRUSTCERTS not set",
					      ctx->info_cpy.app_data);
		free(ssl);
		return NULL;
	}

        if (fcntl(fd, F_SETFL, O_NONBLOCK))
        {
                nonsslerror(&ctx->info_cpy, "fcntl");
                return (NULL);
        }

#ifdef  SO_KEEPALIVE

        {
        int     dummy;

                dummy=1;

                if (setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE,
                        (const char *)&dummy, sizeof(dummy)) < 0)
                {
                        nonsslerror(&ctx->info_cpy, "setsockopt");
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
                        nonsslerror(&ctx->info_cpy, "setsockopt");
                        return (NULL);
                }
        }
#endif

	if (gnutls_anon_allocate_client_credentials(&ssl->anonclientcred) < 0)
	{
		free(ssl);
		return NULL;
	}

	if (gnutls_anon_allocate_server_credentials(&ssl->anonservercred) < 0)
	{
		gnutls_anon_free_client_credentials(ssl->anonclientcred);
		free(ssl);
		return NULL;
	}

	if (gnutls_certificate_allocate_credentials(&ssl->xcred) < 0)
	{
		gnutls_anon_free_server_credentials(ssl->anonservercred);
		gnutls_anon_free_client_credentials(ssl->anonclientcred);
		free(ssl);
		return NULL;
	}

	if (gnutls_dh_params_init(&ssl->dhparams) < 0)
	{
		gnutls_certificate_free_credentials(ssl->xcred);
		gnutls_anon_free_server_credentials(ssl->anonservercred);
		gnutls_anon_free_client_credentials(ssl->anonclientcred);
		free(ssl);
		return NULL;
	}

	if (gnutls_init (&ssl->session,
			 ctx->isserver ? GNUTLS_SERVER:GNUTLS_CLIENT) < 0)
	{
		gnutls_certificate_free_credentials(ssl->xcred);
		gnutls_anon_free_server_credentials(ssl->anonservercred);
		gnutls_anon_free_client_credentials(ssl->anonclientcred);
		free(ssl);
		return NULL;
	}

	{
		const char *p=getenv("TLS_MIN_DH_BITS");
		unsigned int n=atoi(p ? p:"0");

		if (n)
			gnutls_dh_set_prime_bits(ssl->session, n);
	}

	gnutls_session_set_ptr(ssl->session, ssl);

        gnutls_handshake_set_private_extensions(ssl->session, 1);
        gnutls_certificate_set_verify_flags(ssl->xcred,
                                            GNUTLS_VERIFY_ALLOW_X509_V1_CA_CRT |

                                            /*
                                            GNUTLS_VERIFY_DO_NOT_ALLOW_SAME |
                                            GNUTLS_VERIFY_ALLOW_ANY_X509_V1_CA_C
RT |

                                            GNUTLS_VERIFY_ALLOW_SIGN_RSA_MD2 |
                                            GNUTLS_VERIFY_ALLOW_SIGN_RSA_MD5 |*/
                                            0);

        gnutls_certificate_set_verify_limits(ssl->xcred, 16384, 10);
	ssl->dhparams_initialized=0;

	if (gnutls_priority_set_direct(ssl->session, ctx->priority_list,
				       NULL) < 0 ||
	    read_dh_params(ssl->dhparams, ctx->dhfile,
			   &ssl->dhparams_initialized) < 0 ||
	    read_dh_params(ssl->dhparams, ctx->certfile,
			   &ssl->dhparams_initialized) < 0 ||
	    add_certificates(ssl->xcred, ctx->trustcerts) < 0 ||
	    gnutls_credentials_set(ssl->session, GNUTLS_CRD_ANON,
				   ctx->isserver
				   ? (void *)ssl->anonservercred
				   : (void *)ssl->anonclientcred)
	    < 0 ||
	    gnutls_credentials_set(ssl->session, GNUTLS_CRD_CERTIFICATE,
				   ssl->xcred) < 0 ||

	    (ctx->info_cpy.peer_verify_domain &&
	     gnutls_server_name_set(ssl->session, GNUTLS_NAME_DNS,
				    ctx->info_cpy.peer_verify_domain,
				    strlen(ctx->info_cpy.peer_verify_domain))
	     < 0)
	    )
	{
		tls_free_session(ssl);
		return NULL;
	}

	if (ssl->dhparams_initialized)
	{
		gnutls_certificate_set_dh_params(ssl->xcred, ssl->dhparams);

		gnutls_anon_set_server_dh_params(ssl->anonservercred,
						 ssl->dhparams);
	}

	if (ctx->isserver)
	{
		if (ctx->verify_cert)
			gnutls_certificate_server_set_request(ssl->session,
							      ctx->fail_if_no_cert ?
							      GNUTLS_CERT_REQUIRE:
							      GNUTLS_CERT_REQUEST);
		gnutls_certificate_set_retrieve_function(ssl->xcred,
							 get_server_cert);
	}
	else gnutls_certificate_set_retrieve_function(ssl->xcred,
						      get_client_cert);

	gnutls_transport_set_ptr(ssl->session,(gnutls_transport_ptr_t)
				 GNUTLS_CAST_PTR_T fd);

	if (ssl->ctx->info_cpy.tlscache)
	{
		gnutls_db_set_ptr(ssl->session, ssl);

		gnutls_db_set_cache_expiration(ssl->session, 3600);

		gnutls_db_set_remove_function(ssl->session,
					      db_remove_func);
		gnutls_db_set_retrieve_function(ssl->session,
						db_retrieve_func);
		gnutls_db_set_store_function(ssl->session,
					     db_store_func);
	}

	ssl->info_cpy.connect_interrupted=1;

	if (dohandshake(ssl, fd, NULL, NULL) < 0)
	{
		tls_disconnect(ssl, fd);
		return NULL;
	}

	return ssl;
}

void tls_disconnect(ssl_handle ssl, int fd)
{
	fcntl(fd, F_SETFL, 0);
	gnutls_bye(ssl->session, GNUTLS_SHUT_RDWR);
	tls_free_session(ssl);
}

int	tls_transfer(struct tls_transfer_info *t, ssl_handle ssl, int fd,
		     fd_set *r, fd_set *w)
{
	if (ssl->info_cpy.connect_interrupted)
	{
		if (dohandshake(ssl, fd, r, w) < 0)
			return -1;

		return 0;
	}

	if (t->shutdown)
		return -1;

	if (t->shutdown_interrupted)
	{
		while (chk_error(gnutls_bye(ssl->session, GNUTLS_SHUT_RDWR),
				 ssl, fd, r, w, NULL))
			;

		if ((r && FD_ISSET(fd, r)) ||
		    (w && FD_ISSET(fd, w)))
		{
			return 1;
		}


		t->shutdown_interrupted=0;
		t->shutdown= -1;
		return -1;
	}

	if(0) printf("readleft=%d writeleft=%d\nread_interrupted=%d read_interrupted=%d\n",
		    (int)t->readleft,(int)t->writeleft,
		    t->read_interrupted,t->write_interrupted);

	if (!t->write_interrupted && t->readleft > 0 && t->writeleft == 0)
	{
		int rc;
		ssize_t n;

		do
		{
			n=gnutls_record_recv(ssl->session, t->readptr,
					     t->readleft);

			if (n >= 0)
			{
				if (n == 0)
				{
					t->shutdown=1;
					return -1;
				}

				t->readptr += n;
				t->readleft -= n;
				return 0;
			}

			if ((int)n == GNUTLS_E_REHANDSHAKE)
			{
				ssl->info_cpy.connect_interrupted=1;

				return tls_transfer(t, ssl, fd, r, w);
			}

		} while (chk_error((int)n, ssl, fd, r, w, &rc));

		if (rc < 0)
		{
			t->shutdown_interrupted=1;
			return tls_transfer(t, ssl, fd, r, w);
		}
	} else if (t->writeleft > 0)
	{
		int rc;
		ssize_t n;

		t->write_interrupted=0;

		do
		{
			n=gnutls_record_send(ssl->session, (void *)t->writeptr,
					     t->writeleft);

			if (n >= 0)
			{
				if (n == 0)
				{
					t->shutdown=1;
					return -1;
				}

				t->writeptr += n;
				t->writeleft -= n;
				return 0;
			}

			if ((int)n == GNUTLS_E_REHANDSHAKE)
			{
				t->write_interrupted=1;
				ssl->info_cpy.connect_interrupted=1;

				return tls_transfer(t, ssl, fd, r, w);
			}

		} while (chk_error((int)n, ssl, fd, r, w, &rc));

		if (rc < 0)
		{
			t->shutdown=1;
			return -1;
		}
		t->write_interrupted=1;
	}
	else
	{
		FD_SET(fd, r);
		FD_SET(fd, w);
	}
	return (1);
}

int tls_connecting(ssl_handle ssl)
{
	return ssl->info_cpy.connect_interrupted;
}

static const char *dump_dn(gnutls_x509_crt_t cert,
			   int (*get_dn_func)(gnutls_x509_crt_t cert, int indx,
					      void *oid, size_t * sizeof_oid),
			   int (*get_dnval_func)(gnutls_x509_crt_t cert,
						 const char *oid, int indx,
						 unsigned int raw_flag,
						 void *buf, size_t *sizeof_buf),
			   void (*dump_func)(const char *, int cnt, void *),
			   void *dump_arg)
{
	int idx;
	size_t bufsiz;
	size_t maxnamesize;
	size_t maxvalsize;
	char *oidname;
	char *oidval;
	int oidcnt;

	maxnamesize=0;
	maxvalsize=0;

	oidcnt=0;

	while (bufsiz=0, (*get_dn_func)(cert, oidcnt, NULL, &bufsiz)
	       == GNUTLS_E_SHORT_MEMORY_BUFFER)
	{
		if (bufsiz > maxnamesize)
			maxnamesize=bufsiz;
		++oidcnt;
	}

	oidname=malloc(maxnamesize);

	if (!oidname)
		return strerror(errno);

	for (idx=0; idx<oidcnt; ++idx)
	{
		int vidx;
		int rc;

		bufsiz=maxnamesize;

		if ((rc=(*get_dn_func)(cert, idx, oidname, &bufsiz)) < 0)
		{
			free(oidname);
			return gnutls_strerror(rc);
		}

		vidx=0;

		while (bufsiz=0,
		       (*get_dnval_func)(cert, oidname, vidx, 0,
					 NULL, &bufsiz)
		       == GNUTLS_E_SHORT_MEMORY_BUFFER)
		{
			if (bufsiz > maxvalsize)
				maxvalsize=bufsiz;
			++vidx;
		}
	}

	oidval=malloc(maxvalsize);

	if (!oidval)
	{
		free(oidname);
		return strerror(errno);
	}

	for (idx=0; idx<oidcnt; ++idx)
	{
		int vidx;
		int rc;
		size_t i;
		const char *oidname_str;

		bufsiz=maxnamesize;

		if ((rc=(*get_dn_func)(cert, idx, oidname, &bufsiz)) < 0)
		{
			free(oidval);
			free(oidname);
			return gnutls_strerror(rc);
		}

		oidname_str=oidname;

		for (i=0; i<sizeof(oid_name_list)/sizeof(oid_name_list[0]);
		     ++i)
		{
			if (strcmp(oid_name_list[i].oid, oidname) == 0)
			{
				oidname_str=oid_name_list[i].name;
				break;
			}
		}

		vidx=0;

		while (bufsiz=maxvalsize,
		       (*get_dnval_func)(cert, oidname, vidx, 0,
					 oidval, &bufsiz) >= 0)
		{
			(*dump_func)("   ", -1, dump_arg);
			(*dump_func)(oidname_str, -1, dump_arg);
			(*dump_func)("=", -1, dump_arg);
			(*dump_func)(oidval, -1, dump_arg);
			(*dump_func)("\n", -1, dump_arg);
			++vidx;
		}
	}

	free(oidval);
	free(oidname);
	return NULL;
}

static void print_time(const char *name, time_t t,
		       void (*dump_func)(const char *, int cnt, void *),
		       void *dump_arg)
{
	struct tm *tmptr=gmtime(&t);
	char buf[256];

	strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", tmptr);

	(*dump_func)(name, -1, dump_arg);
	(*dump_func)(": ", 2, dump_arg);
	(*dump_func)(buf, -1, dump_arg);
	(*dump_func)("\n", 1, dump_arg);
}

static void tls_dump_connection_info_x509(ssl_handle ssl,
					  int server,
					  void (*dump_func)(const char *,
							    int cnt, void *),
					  void *dump_arg);

static void dump_cipher_name(gnutls_session_t session,
			     void (*dump_func)(const char *,
					       int cnt, void *),
			     void *dump_arg);

void tls_dump_connection_info(ssl_handle ssl,
			      int server,
			      void (*dump_func)(const char *, int cnt, void *),
			      void *dump_arg)
{
	if (gnutls_certificate_type_get (ssl->session) == GNUTLS_CRT_X509)
		tls_dump_connection_info_x509(ssl, server, dump_func,
					      dump_arg);

	(*dump_func)("Version: ", -1, dump_arg);
	(*dump_func)
		(gnutls_protocol_get_name(gnutls_protocol_get_version(ssl->session)),
		 -1, dump_arg);
	(*dump_func)("\n", 1, dump_arg);

	{
		char buf[10];

		(*dump_func)("Bits: ", -1, dump_arg);

		snprintf(buf, sizeof(buf), "%d", (int)
			 gnutls_cipher_get_key_size(gnutls_cipher_get(ssl->session))
			 *8);
		buf[sizeof(buf)-1]=0;

		(*dump_func)(buf, -1, dump_arg);
		(*dump_func)("\n", 1, dump_arg);
	}

	(*dump_func)("Cipher: ", -1, dump_arg);
	dump_cipher_name(ssl->session, dump_func, dump_arg);
	(*dump_func)("\n", 1, dump_arg);
}

static void dump_cipher_name(gnutls_session_t session,
			     void (*dump_func)(const char *,
					       int cnt, void *),
			     void *dump_arg)
{
	gnutls_kx_algorithm_t kx_algo;
	gnutls_cipher_algorithm_t cipher_algo;
	gnutls_mac_algorithm_t mac_algo;
	const char *cipher_name;

	kx_algo=gnutls_kx_get(session);
	cipher_algo=gnutls_cipher_get(session);
	mac_algo=gnutls_mac_get(session);
	cipher_name=gnutls_cipher_suite_get_name(kx_algo, cipher_algo,
						 mac_algo);

	if (cipher_name)
		(*dump_func)(cipher_name, -1, dump_arg);
	else
	{
		gnutls_compression_method_t comp;

		(*dump_func)(gnutls_kx_get_name(kx_algo), -1, dump_arg);

		(*dump_func)("-", 1, dump_arg);
		(*dump_func)(gnutls_certificate_type_get_name(gnutls_certificate_type_get(session)),
			     -1, dump_arg);

		(*dump_func)("-", 1, dump_arg);
		(*dump_func)(gnutls_cipher_get_name(cipher_algo), -1,
			     dump_arg);

		if ((comp=gnutls_compression_get(session))
		    != GNUTLS_COMP_NULL)
		{
			(*dump_func)("/", 1, dump_arg);
			(*dump_func)(gnutls_compression_get_name(comp),
				     -1, dump_arg);
		}

		(*dump_func)("-", 1, dump_arg);
		(*dump_func)(gnutls_mac_get_name(gnutls_mac_get(session)),
			     -1, dump_arg);
	}
}

static void tls_dump_connection_info_x509(ssl_handle ssl,
					  int server,
					  void (*dump_func)(const char *,
							    int cnt, void *),
					  void *dump_arg)
{
	const gnutls_datum_t *cert_list;
	unsigned int cert_list_size;
	gnutls_x509_crt_t *cert;

	cert_list=gnutls_certificate_get_peers(ssl->session, &cert_list_size);

	if (cert_list)
	{
		unsigned int i;

		cert=malloc(sizeof (*cert) * cert_list_size);

		for (i = 0; i<cert_list_size; i++)
		{
			gnutls_x509_crt_init(&cert[i]);
			gnutls_x509_crt_import(cert[i],
					       &cert_list[i],
					       GNUTLS_X509_FMT_DER);
		}

		for (i = 0; i < cert_list_size; i++)
		{
			time_t notbefore;
			time_t notafter;

			(*dump_func)("Subject:\n", -1, dump_arg);

			dump_dn(cert[i],
				gnutls_x509_crt_get_dn_oid,
				gnutls_x509_crt_get_dn_by_oid,
				dump_func, dump_arg);
			(*dump_func)("\n", 1, dump_arg);


#if 0
			(*dump_func)("Issuer:\n", -1, dump_arg);

			dump_dn(cert[i],
				gnutls_x509_crt_get_issuer_dn_oid,
				gnutls_x509_crt_get_issuer_dn_by_oid,
				dump_func, dump_arg);
			(*dump_func)("\n", 1, dump_arg);
#endif

			notbefore=gnutls_x509_crt_get_activation_time(cert[i]);
			notafter=gnutls_x509_crt_get_expiration_time(cert[i]);
			print_time("Not-Before", notbefore,
				   dump_func, dump_arg);
			print_time("Not-After",  notafter,
				   dump_func, dump_arg);
		}

		for (i = 0; i < cert_list_size; i++)
			gnutls_x509_crt_deinit(cert[i]);
		free(cert);
	}
}

static void gen_encryption_desc(gnutls_session_t session,
				void (*dump_func)(const char *,
						  int cnt, void *),
				void *dump_arg);

static void cnt_desc_size(const char *str, int s, void *ptr)
{
	if (s < 0)
		s=strlen(str);

	*(size_t *)ptr += s;
}

static void save_desc(const char *str, int s, void *ptr)
{
	if (s < 0)
		s=strlen(str);

	memcpy(*(char **)ptr, str, s);
	*(char **)ptr += s;
}

char *tls_get_encryption_desc(ssl_handle ssl)
{
	size_t n=1;
	char *buf;

	gen_encryption_desc(ssl->session, cnt_desc_size, &n);

	buf=malloc(n);

	if (buf)
	{
		char *ptr=buf;
		gen_encryption_desc(ssl->session, save_desc, &ptr);
		*ptr=0;
	}
	return buf;
}

static void gen_encryption_desc(gnutls_session_t session,
				void (*dump_func)(const char *,
						  int cnt, void *),
				void *dump_arg)
{
	char buf[10];

	(*dump_func)(gnutls_protocol_get_name(gnutls_protocol_get_version(session)),
		     -1, dump_arg);
	(*dump_func)(",", 1, dump_arg);
	snprintf(buf, sizeof(buf), "%d",
		 (int)gnutls_cipher_get_key_size(gnutls_cipher_get(session))
		 *8);
	buf[sizeof(buf)-1]=0;
	(*dump_func)(buf, -1, dump_arg);
	(*dump_func)("bits,", -1, dump_arg);
	dump_cipher_name(session, dump_func, dump_arg);
}


/* ------------------- */

int tls_validate_pem_cert(const char *buf, size_t buf_size)
{
	gnutls_datum_t dat;
	unsigned int cert_cnt=0;
 	gnutls_x509_crt_t *certbuf;

	dat.data=(void *)buf;
	dat.size=buf_size;

	gnutls_x509_crt_list_import(NULL, &cert_cnt, &dat,
				    GNUTLS_X509_FMT_PEM,
				    GNUTLS_X509_CRT_LIST_IMPORT_FAIL_IF_EXCEED);

	if (cert_cnt == 0)
		return 0;
	certbuf=malloc(sizeof(*certbuf)*cert_cnt);

	if (!certbuf)
		return 0;

	if (gnutls_x509_crt_list_import(certbuf, &cert_cnt, &dat,
					GNUTLS_X509_FMT_PEM,
					GNUTLS_X509_CRT_LIST_IMPORT_FAIL_IF_EXCEED) < 0)
		return 0;

	while (cert_cnt)
		gnutls_x509_crt_deinit(certbuf[--cert_cnt]);
	free(certbuf);
	return (1);
}

char *tls_cert_name(const char *buf, size_t buf_size)
{
	gnutls_datum_t dat;
	unsigned int cert_cnt=0;
 	gnutls_x509_crt_t *certbuf;
	char *p=0;
	size_t p_size;


	dat.data=(void *)buf;
	dat.size=buf_size;

	gnutls_x509_crt_list_import(NULL, &cert_cnt, &dat,
				    GNUTLS_X509_FMT_PEM,
				    GNUTLS_X509_CRT_LIST_IMPORT_FAIL_IF_EXCEED);

	if (cert_cnt == 0)
		return 0;
	certbuf=malloc(sizeof(*certbuf)*cert_cnt);

	if (!certbuf)
		return 0;

	if (gnutls_x509_crt_list_import(certbuf, &cert_cnt, &dat,
					GNUTLS_X509_FMT_PEM,
					GNUTLS_X509_CRT_LIST_IMPORT_FAIL_IF_EXCEED) < 0)
		return 0;

	p_size=0;
	gnutls_x509_crt_get_dn(certbuf[0], NULL, &p_size);
	++p_size;
	p=malloc(p_size+1);

	if (p)
	{
		if (gnutls_x509_crt_get_dn(certbuf[0], p, &p_size)
		    != GNUTLS_E_SUCCESS)
		{
			free(p);
			p=0;
		}
		else p[p_size]=0;
	}

	while (cert_cnt)
		gnutls_x509_crt_deinit(certbuf[--cert_cnt]);
	free(certbuf);
	return p;
}
