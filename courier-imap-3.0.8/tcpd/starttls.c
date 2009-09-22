/*
** Copyright 2000-2003 Double Precision, Inc.
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
#define	DEBUG_SAFESTACK	1	/* For openssl 0.9.6 */

#include	<openssl/ssl.h>
#include	<openssl/err.h>

#if TIME_WITH_SYS_TIME
#include        <sys/time.h>
#include        <time.h>
#else
#if HAVE_SYS_TIME_H
#include        <sys/time.h>
#else
#include        <time.h>
#endif
#endif
#include	<locale.h>

static const char rcsid[]="$Id: starttls.c,v 1.34 2003/09/01 19:30:23 mrsam Exp $";

/* Command-line options: */
const char *clienthost=0;
const char *clientport=0;

const char *server=0;
const char *localfd=0;
const char *remotefd=0;
const char *statusfd=0;
const char *tcpd=0;
const char *peer_verify_domain=0;
const char *fdprotocol=0;
static FILE *errfp;
static FILE *statusfp;

const char *printx509=0;

static void ssl_errmsg(const char *errmsg, void *dummy)
{
	fprintf(errfp, "%s\n", errmsg);
}

static void nonsslerror(const char *pfix)
{
	fprintf(errfp, "%s: %s\n", pfix, strerror(errno));
}

void docopy(SSL *ssl, int sslfd, int stdinfd, int stdoutfd)
{
	struct tls_transfer_info transfer_info;

	char from_ssl_buf[BUFSIZ], to_ssl_buf[BUFSIZ];
	char *fromptr;
	int rc;

	fd_set	fdr, fdw;
	int	maxfd=sslfd;

	if (fcntl(stdinfd, F_SETFL, O_NONBLOCK)
	    || fcntl(stdoutfd, F_SETFL, O_NONBLOCK)
	    )
	{
		nonsslerror("fcntl");
		return;
	}

	if (maxfd < stdinfd)	maxfd=stdinfd;
	if (maxfd < stdoutfd)	maxfd=stdoutfd;

	tls_transfer_init(&transfer_info);

	transfer_info.readptr=fromptr=from_ssl_buf;

	for (;;)
	{
		if (transfer_info.readptr == fromptr)
		{
			transfer_info.readptr=fromptr=from_ssl_buf;
			transfer_info.readleft=sizeof(from_ssl_buf);
		}
		else
			transfer_info.readleft=0;

		FD_ZERO(&fdr);
		FD_ZERO(&fdw);

		rc=tls_transfer(&transfer_info, ssl, sslfd, &fdr, &fdw);

		if (rc == 0)
			continue;
		if (rc < 0)
			break;

		if (!tls_inprogress(&transfer_info))
		{
			if (transfer_info.readptr > fromptr)
				FD_SET(stdoutfd, &fdw);

			if (transfer_info.writeleft == 0)
				FD_SET(stdinfd, &fdr);
		}

		if (select(maxfd+1, &fdr, &fdw, 0, 0) <= 0)
		{
			if (errno != EINTR)
			{
				nonsslerror("select");
				break;
			}
			continue;
		}

		if (FD_ISSET(stdoutfd, &fdw) &&
		    transfer_info.readptr > fromptr)
		{
			rc=write(stdoutfd, fromptr,
				 transfer_info.readptr - fromptr);

			if (rc <= 0)
				break;

			fromptr += rc;
		}

		if (FD_ISSET(stdinfd, &fdr) && transfer_info.writeleft == 0)
		{
			rc=read(stdinfd, to_ssl_buf, sizeof(to_ssl_buf));
			if (rc <= 0)
				break;

			transfer_info.writeptr=to_ssl_buf;
			transfer_info.writeleft=rc;
		}
	}

	tls_closing(&transfer_info);

	for (;;)
	{
		FD_ZERO(&fdr);
		FD_ZERO(&fdw);

		if (tls_transfer(&transfer_info, ssl, sslfd, &fdr, &fdw) < 0)
			break;

		if (select(maxfd+1, &fdr, &fdw, 0, 0) <= 0)
		{
			if (errno != EINTR)
			{
				nonsslerror("select");
				break;
			}
			continue;
		}
	}
}

#define MAXDOMAINSIZE	256

static time_t asn1toTime(ASN1_TIME *asn1Time)
{
	struct tm tm;
	int offset;

	if (asn1Time == NULL || asn1Time->length < 13)
		return 0;

	memset(&tm, 0, sizeof(tm));

#define N2(n)	((asn1Time->data[n]-'0')*10 + asn1Time->data[(n)+1]-'0')

#define CPY(f,n) (tm.f=N2(n))

	CPY(tm_year,0);

	if(tm.tm_year < 50)
		tm.tm_year += 100; /* Sux */

	CPY(tm_mon, 2);
	--tm.tm_mon;
	CPY(tm_mday, 4);
	CPY(tm_hour, 6);
	CPY(tm_min, 8);
	CPY(tm_sec, 10);

	offset=0;

	if (asn1Time->data[12] != 'Z')
	{
		if (asn1Time->length < 17)
			return 0;

		offset=N2(13)*3600+N2(15)*60;

		if (asn1Time->data[12] == '-')
			offset= -offset;
	}

#undef N2
#undef CPY

	return mktime(&tm)-offset;
}

static void dump_x509(X509 *x509, FILE *printx509_fp)
{
	X509_NAME *subj=X509_get_subject_name(x509);
	int nentries, j;
	time_t timestamp;
	static const char gcc_shutup[]="%Y-%m-%d %H:%M:%S";

	if (!subj)
		return;

	if (printx509_fp)
		fprintf(printx509_fp, "Subject:\n");

	nentries=X509_NAME_entry_count(subj);
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

		if (printx509_fp)
		{
			fprintf(printx509_fp, "   %s=", obj_name);
			fwrite(ddata, dlen, 1, printx509_fp);
			fprintf(printx509_fp, "\n");
		}
	}
	if (printx509_fp)
		fprintf(printx509_fp, "\n");

	timestamp=asn1toTime(X509_get_notBefore(x509));

	if (timestamp && printx509_fp)
	{
		struct tm *tm=localtime(&timestamp);
		char buffer[500];

		buffer[strftime(buffer, sizeof(buffer)-1, gcc_shutup,
				tm)]=0;

		fprintf(printx509_fp, "Not-Before: %s\n", buffer);
	}

	timestamp=asn1toTime(X509_get_notAfter(x509));
	if (timestamp && printx509_fp)
	{
		struct tm *tm=localtime(&timestamp);
		char buffer[500];

		buffer[strftime(buffer, sizeof(buffer)-1, gcc_shutup,
				tm)]=0;

		fprintf(printx509_fp, "Not-After: %s\n", buffer);
	}
}

static int verify_connection(SSL *ssl, void *dummy)
{
	FILE	*printx509_fp=NULL;
	int	printx509_fd=0;
	SSL_CIPHER *cipher;

	if (printx509)
	{
		printx509_fd=atoi(printx509);

		printx509_fp=fdopen(printx509_fd, "w");
                if (!printx509_fp)
                        nonsslerror("fdopen");
	}


	{
		STACK_OF(X509) *peer_cert_chain=SSL_get_peer_cert_chain(ssl);
		int i;

		if (server)
		{
			X509 *x=SSL_get_peer_certificate(ssl);

			if (x)
			{
				dump_x509(x, printx509_fp);
				X509_free(x);
			}
		}

		for (i=0; peer_cert_chain && i<peer_cert_chain->stack.num; i++)
			dump_x509((X509 *)peer_cert_chain->stack.data[i],
				  printx509_fp);
	}

	cipher=SSL_get_current_cipher(ssl);

	if (cipher)
	{
		static char protocolbuf[256];

		const char *c;

		c=SSL_CIPHER_get_version(cipher);
		if (c && printx509_fp)
			fprintf(printx509_fp, "Version: %s\n", c);

		strcpy(protocolbuf, "TLS_CONNECTED_PROTOCOL=");
		strncat(protocolbuf, c ? c:"unknown", 40);

		if (printx509_fp)
			fprintf(printx509_fp, "Bits: %d\n",
				SSL_CIPHER_get_bits(cipher, NULL));

		sprintf(protocolbuf+strlen(protocolbuf), ",%dbits",
			SSL_CIPHER_get_bits(cipher, NULL));

		c=SSL_CIPHER_get_name(cipher);

		if (c && printx509_fp)
			fprintf(printx509_fp, "Cipher: %s\n", c);

		c=SSL_CIPHER_get_name(cipher);
		strncat(strcat(protocolbuf, ","), c ? c:"unknown", 40);
		putenv(protocolbuf);
	}

	if (printx509_fp)
	{
		fclose(printx509_fp);
		close(printx509_fd);
	}

	if (statusfp)
	{
		fclose(statusfp);
		errfp=stderr;
		statusfp=NULL;
	}
	return 1;
}


/* ----------------------------------------------------------------------- */

static void startclient(int argn, int argc, char **argv, int fd,
	int *stdin_fd, int *stdout_fd)
{
pid_t	p;
int	streampipe[2];

	if (localfd)
	{
		*stdin_fd= *stdout_fd= atoi(localfd);
		return;
	}

	if (argn >= argc)	return;		/* Interactive */

	if (libmail_streampipe(streampipe))
	{
		nonsslerror("libmail_streampipe");
		exit(1);
	}
	if ((p=fork()) == -1)
	{
		nonsslerror("fork");
		close(streampipe[0]);
		close(streampipe[1]);
		exit(1);
	}
	if (p == 0)
	{
	char **argvec;
	int n;

		close(fd);	/* Child process doesn't need it */
		close(0);
		dup(streampipe[1]);
		close(1);
		dup(streampipe[1]);
		close(streampipe[0]);
		close(streampipe[1]);

		argvec=malloc(sizeof(char *)*(argc-argn+1));
		if (!argvec)
		{
			nonsslerror("malloc");
			exit(1);
		}
		for (n=0; n<argc-argn; n++)
			argvec[n]=argv[argn+n];
		argvec[n]=0;
		execvp(argvec[0], argvec);
		nonsslerror(argvec[0]);
		exit(1);
	}
	close(streampipe[1]);

	*stdin_fd= *stdout_fd= streampipe[0];
}

static int connectremote(const char *host, const char *port)
{
int	fd;

RFC1035_ADDR addr;
int	af;
RFC1035_ADDR *addrs;
unsigned	naddrs, n;

RFC1035_NETADDR addrbuf;
const struct sockaddr *saddr;
int     saddrlen;
int	port_num;

	port_num=atoi(port);
	if (port_num <= 0)
	{
	struct servent *servent;

		servent=getservbyname(port, "tcp");

		if (!servent)
		{
			fprintf(errfp, "%s: invalid port.\n", port);
			return (-1);
		}
		port_num=servent->s_port;
	}
	else
		port_num=htons(port_num);

	if (rfc1035_aton(host, &addr) == 0) /* An explicit IP addr */
	{
		if ((addrs=malloc(sizeof(addr))) == 0)
		{
			nonsslerror("malloc");
			return (-1);
		}
		memcpy(addrs, &addr, sizeof(addr));
		naddrs=1;
	}
	else
	{
		if (rfc1035_a(&rfc1035_default_resolver, host, &addrs,
			      &naddrs))
		{
			fprintf(errfp, "%s: not found.\n", host);
			return (-1);
		}
	}

        if ((fd=rfc1035_mksocket(SOCK_STREAM, 0, &af)) < 0)
        {
                nonsslerror("socket");
                return (-1);
        }

	for (n=0; n<naddrs; n++)
	{
		if (rfc1035_mkaddress(af, &addrbuf, addrs+n, port_num,
			&saddr, &saddrlen))	continue;

		if (sox_connect(fd, saddr, saddrlen) == 0)
			break;
	}
	free(addrs);

	if (n >= naddrs)
	{
		close(fd);
		nonsslerror("connect");
		return (-1);
	}

	return (fd);
}

static int connect_completed(SSL *ssl, int fd)
{
	struct tls_transfer_info transfer_info;
	tls_transfer_init(&transfer_info);

	while (tls_connecting(ssl))
	{
		fd_set	fdr, fdw;

		FD_ZERO(&fdr);
		FD_ZERO(&fdw);
		if (tls_transfer(&transfer_info, ssl,
				 fd, &fdr, &fdw) < 0)
			return (0);

		if (!tls_connecting(ssl))
			break;

		if (select(fd+1, &fdr, &fdw, 0, 0) <= 0)
		{
			if (errno != EINTR)
			{
				nonsslerror("select");
				return (0);
			}
		}
	}
	return (1);
}

static int dossl(int fd, int argn, int argc, char **argv)
{
	SSL_CTX *ctx;
	SSL	*ssl;

	int	stdin_fd, stdout_fd;
	struct tls_info info= *tls_get_default_info();

	info.peer_verify_domain=peer_verify_domain;
	info.tls_err_msg=ssl_errmsg;
	info.connect_callback= &verify_connection;
	info.app_data=NULL;

	ctx=tls_create(server ? 1:0, &info);
	if (ctx == 0)	return (1);

	ssl=tls_connect(ctx, fd);

	if (!ssl)
	{
		close(fd);
		return (1);
	}

	if (!connect_completed(ssl, fd))
		return 1;

	stdin_fd=0;
	stdout_fd=1;

	startclient(argn, argc, argv, fd, &stdin_fd, &stdout_fd);

	docopy(ssl, fd, stdin_fd, stdout_fd);

	tls_disconnect(ssl, fd);
	close(fd);
	tls_destroy(ctx);
	return (0);
}

struct protoreadbuf {
	char buffer[512];
	char *bufptr;
	int bufleft;

	char line[256];
} ;

#define PRB_INIT(p) ( (p)->bufptr=0, (p)->bufleft=0)

static char protoread(int fd, struct protoreadbuf *prb)
{
	fd_set fds;
	struct timeval tv;

	FD_ZERO(&fds);
	FD_SET(fd, &fds);

	tv.tv_sec=60;
	tv.tv_usec=0;

	if (select(fd+1, &fds, NULL, NULL, &tv) <= 0)
	{
		nonsslerror("select");
		exit(1);
	}

	if ( (prb->bufleft=read(fd, prb->buffer, sizeof(prb->buffer))) <= 0)
	{
		errno=ECONNRESET;
		nonsslerror("read");
		exit(1);
	}

	prb->bufptr= prb->buffer;

	--prb->bufleft;
	return (*prb->bufptr++);
}

#define PRB_GETCH(fd,prb) ( (prb)->bufleft-- > 0 ? *(prb)->bufptr++:\
				protoread( (fd), (prb)))

static const char *prb_getline(int fd, struct protoreadbuf *prb)
{
	int i=0;
	char c;

	while ((c=PRB_GETCH(fd, prb)) != '\n')
	{
		if ( i < sizeof (prb->line)-1)
			prb->line[i++]=c;
	}
	prb->line[i]=0;
	return (prb->line);
}

static void prb_write(int fd, struct protoreadbuf *prb, const char *p)
{
	printf("%s", p);
	while (*p)
	{
		int l=write(fd, p, strlen(p));

		if (l <= 0)
		{
			nonsslerror("write");
			exit(1);
		}
		p += l;
	}
}

static int goodimap(const char *p)
{
	if (*p == 'x' && p[1] && isspace((int)(unsigned char)p[1]))
		++p;
	else
	{
		if (*p != '*')
			return (0);
		++p;
	}
	while (*p && isspace((int)(unsigned char)*p))
		++p;
	if (strncasecmp(p, "BAD", 3) == 0)
	{
		exit(1);
	}

	if (strncasecmp(p, "BYE", 3) == 0)
	{
		exit(1);
	}

	if (strncasecmp(p, "NO", 2) == 0)
	{
		exit(1);
	}

	return (strncasecmp(p, "OK", 2) == 0);
}

static void imap_proto(int fd)
{
	struct protoreadbuf prb;
	const char *p;

	PRB_INIT(&prb);

	do
	{
		p=prb_getline(fd, &prb);
		printf("%s\n", p);

	} while (!goodimap(p));

	prb_write(fd, &prb, "x STARTTLS\r\n");

	do
	{
		p=prb_getline(fd, &prb);
		printf("%s\n", p);
	} while (!goodimap(p));
}

static void pop3_proto(int fd)
{
	struct protoreadbuf prb;
	const char *p;

	PRB_INIT(&prb);

	p=prb_getline(fd, &prb);
	printf("%s\n", p);

	prb_write(fd, &prb, "STLS\r\n");

	p=prb_getline(fd, &prb);
	printf("%s\n", p);
}

static void smtp_proto(int fd)
{
	struct protoreadbuf prb;
	const char *p;

	PRB_INIT(&prb);

	do
	{
		p=prb_getline(fd, &prb);
		printf("%s\n", p);
	} while ( ! ( isdigit((int)(unsigned char)p[0]) && 
		      isdigit((int)(unsigned char)p[1]) &&
		      isdigit((int)(unsigned char)p[2]) &&
		      (p[3] == 0 || isspace((int)(unsigned char)p[3]))));
	if (strchr("123", *p) == 0)
		exit(1);

	prb_write(fd, &prb, "STARTTLS\r\n");

	do
	{
		p=prb_getline(fd, &prb);
		printf("%s\n", p);
	} while ( ! ( isdigit((int)(unsigned char)p[0]) && 
		      isdigit((int)(unsigned char)p[1]) &&
		      isdigit((int)(unsigned char)p[2]) &&
		      (p[3] == 0 || isspace((int)(unsigned char)p[3]))));
	if (strchr("123", *p) == 0)
		exit(1);

}

int main(int argc, char **argv)
{
int	argn;
int	fd;
static struct args arginfo[] = {
	{ "host", &clienthost },
	{ "localfd", &localfd},
	{ "port", &clientport },
	{ "printx509", &printx509},
	{ "remotefd", &remotefd},
	{ "server", &server},
	{ "tcpd", &tcpd},
	{ "verify", &peer_verify_domain},
	{ "statusfd", &statusfd},
	{ "protocol", &fdprotocol},
	{0}};
void (*protocol_func)(int)=0;

	setlocale(LC_ALL, "");
	errfp=stderr;
 
	argn=argparse(argc, argv, arginfo);

	if (statusfd)
		statusfp=fdopen(atoi(statusfd), "w");

	if (statusfp)
		errfp=statusfp;

	if (fdprotocol)
	{
		if (strcmp(fdprotocol, "smtp") == 0)
			protocol_func= &smtp_proto;
		else if (strcmp(fdprotocol, "imap") == 0)
			protocol_func= &imap_proto;
		else if (strcmp(fdprotocol, "pop3") == 0)
			protocol_func= &pop3_proto;
		else
		{
			fprintf(stderr, "--protocol=%s - unknown protocol.\n",
				fdprotocol);
			exit(1);
		}
	}

	if (tcpd)
	{
		close(1);
		dup(2);
		fd=0;
	}
	else if (remotefd)
		fd=atoi(remotefd);
	else if (clienthost && clientport)
		fd=connectremote(clienthost, clientport);
	else
	{
		fprintf(errfp, "%s: specify remote location.\n",
			argv[0]);
		return (1);
	}

	if (fd < 0)	return (1);
	if (protocol_func)
		(*protocol_func)(fd);
	return (dossl(fd, argn, argc, argv));
}
