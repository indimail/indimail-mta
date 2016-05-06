/*
** Copyright 1998 - 2011 Double Precision, Inc.
** See COPYING for distribution information.
*/

#ifndef	rfc1035_h
#define	rfc1035_h



#if	HAVE_CONFIG_H
#include "rfc1035/config.h"
#endif

#include	"random128/random128.h"
#include	"md5/md5.h"

#include	<stdio.h>
#include	<sys/types.h>
#include	<sys/socket.h>
#if HAVE_NETINET_IN_H
#include	<netinet/in.h>
#endif

#ifdef  __cplusplus
extern "C" {
#endif

#if RFC1035_IPV6
typedef struct in6_addr	RFC1035_ADDR;
typedef struct sockaddr_in6 RFC1035_SOCKADDR;
typedef struct sockaddr_storage RFC1035_NETADDR;
#define RFC1035_ADDRANY		in6addr_any
#else
typedef	struct in_addr	RFC1035_ADDR;
typedef struct sockaddr_in	RFC1035_SOCKADDR;
typedef struct sockaddr		RFC1035_NETADDR;

extern struct in_addr rfc1035_addr_any;
#define RFC1035_ADDRANY		rfc1035_addr_any
#endif

#define RFC1035_TYPE_A		1
#define RFC1035_TYPE_NS		2
#define RFC1035_TYPE_MD		3
#define RFC1035_TYPE_MF		4
#define RFC1035_TYPE_CNAME	5
#define RFC1035_TYPE_SOA	6
#define RFC1035_TYPE_MB		7
#define RFC1035_TYPE_MG		8
#define RFC1035_TYPE_MR		9
#define RFC1035_TYPE_NULL	10
#define RFC1035_TYPE_WKS	11
#define RFC1035_TYPE_PTR	12
#define RFC1035_TYPE_HINFO	13
#define RFC1035_TYPE_MINFO	14
#define RFC1035_TYPE_MX		15
#define RFC1035_TYPE_TXT	16

#define	RFC1035_TYPE_AAAA	28	/* RFC 1886. Even if we don't have
					IPv6 */

#define RFC1035_TYPE_OPT	41
#define RFC1035_TYPE_RRSIG	46

#define	RFC1035_TYPE_AXFR	252
#define	RFC1035_TYPE_MAILB	253
#define	RFC1035_TYPE_MAILA	254
#define	RFC1035_TYPE_ANY	255

void rfc1035_type_itostr(int, void (*)(const char *, void *), void *);
int rfc1035_type_strtoi(const char *);

#define	RFC1035_CLASS_IN	1
#define	RFC1035_CLASS_CSNET	2
#define	RFC1035_CLASS_CHAOS	3
#define	RFC1035_CLASS_HESIOD	4
#define	RFC1035_CLASS_ANY	255

const char *rfc1035_class_itostr(int);
int rfc1035_class_strtoi(const char *);

#define	RFC1035_OPCODE_QUERY	0
#define	RFC1035_OPCODE_IQUERY	1
#define	RFC1035_OPCODE_STATUS	2

const char *rfc1035_opcode_itostr(int);
int rfc1035_opcode_strtoi(const char *);

#define RFC1035_RCODE_NOERROR	0
#define	RFC1035_RCODE_FORMAT	1
#define	RFC1035_RCODE_SERVFAIL	2
#define	RFC1035_RCODE_NXDOMAIN	3
#define	RFC1035_RCODE_UNIMPLEMENTED	4
#define	RFC1035_RCODE_REFUSED	5

const char *rfc1035_rcode_itostr(int);
int rfc1035_rcode_strtoi(const char *);

struct rfc1035_query {
	const char *name;
	unsigned qtype, qclass;
	} ;

struct rfc1035_reply;	/* Defined below */
struct rfc1035_rr;	/* Defined below */

/*
** The init family of functions perform various initializations.  Calling them
** is optional, as librfc1035.a will use defaults if not specified.
*/

#define	RFC1035_MAXNS	10
#define	RFC1035_DEFAULT_INITIAL_TIMEOUT	5
#define	RFC1035_DEFAULT_MAXIMUM_BACKOFF	3

		/* Resolver state */
struct rfc1035_res {

	RFC1035_ADDR nameservers[RFC1035_MAXNS];
	int rfc1035_nnameservers;

	char *rfc1035_defaultdomain;
	int norecursive; /* Do not set the recursive flag, for specialized apps */
	int dnssec_payload_size; /* Enable dnssec requests */

	unsigned rfc1035_good_ns;
	unsigned rfc1035_timeout_initial;	/* Initial timeout */
	unsigned rfc1035_timeout_backoff;	/* Maximum exponential backoff */

	random128binbuf randseed;
	MD5_DIGEST	randbuf;
	unsigned randptr;
	} ;

extern struct rfc1035_res rfc1035_default_resolver;

void rfc1035_init_timeout(struct rfc1035_res *, unsigned, unsigned);
					/*
					** Specify timeout in seconds,
					** and maximum exponential backoff.
					*/
void rfc1035_init_ns(struct rfc1035_res *, const RFC1035_ADDR *, unsigned);
		/* Specify nameservers to query (max 10) */

void rfc1035_init_norecursive(struct rfc1035_res *, int);
	/* Set the no-recursive flag, if you don't want the NS to do recursive queries on your behalf */

void rfc1035_init_dnssec_enable(struct rfc1035_res *, int);
	/* Enable/disable dnssec/edns0 */

void rfc1035_init_edns_payload(struct rfc1035_res *, int);
	/* Set edns0 payload size */

void rfc1035_init_resolv(struct rfc1035_res *);
	/* Read /etc/resolv.conf for nameservers */

void rfc1035_destroy_resolv(struct rfc1035_res *);
	/* Destroy the resolver object */

	/*
	** Most people will only need to call rfc1035_resolve or
	** rfc1035_resolve_cname.  The return value from _resolve functions
	** should be interpreted as follows:
	**     NULL - internal failure (treat it as a soft DNS error)
	**     ptr->rcode = RFC1035_RCODE_NOERROR - success
	**     ptr->rcode = RFC1035_RCODE_NXDOMAIN - hard DNS error
	**     ptr->rcode = RFC1035_RCODE_TEMPFAIL - soft DNS error
	**     ptr->rcode = any other value - log abnormal result,
	**                  handle as soft DNS error
	*/

struct rfc1035_reply *rfc1035_resolve(
	struct rfc1035_res *,	/* Pointer to a resolver structure */
	int,		/* Opcode, see above. */
	const char *,	/* Query name */
	unsigned,	/* Query type, see above. */
	unsigned);	/* Query class, see above. */

	/*
	** Multiple queries.  Most servers don't support this.
	*/

struct rfc1035_reply *rfc1035_resolve_multiple(
	struct rfc1035_res *,	/* Pointer to a resolver structure */
	int,	/* opcode */
	const struct rfc1035_query *,	/* Array of queries */
	unsigned);			/* Array size */

/*
** rfc1035_resolve_cname is like _resolve, but starts with the default
** servers, and automatically reissues the query if the received response
** is a CNAME.  If successful, it returns an INDEX into the allrrs array
** containing the first answer.  To get the next answers, call
** rfc1035_replysearch_all with return value+1.  If not succesfull, -1
** is returned.
**
** Note - if the returned index points to a CNAME, this is because a CNAME
** pointed to another CNAME -- it's BAD!
**
** It takes a POINTER to the 'id' counter, which is incremented if a
** second query needs to be issued.
**
** If takes a POINTER to the rfc1035_reply structure, which will be either
** null on exit, or point to the reply received.  The pointer MAY be not null
** even if the return value is -1.
**
** Suggested logic:
**    Return value >= 0, succesfull query.
**    Return value <= -1, ptr is not null, and rcode = RFC1035_RCODE_NXDOMAIN,
**        hard DNS error.
**    Return value <= -1, ptr is not null, and rcode = RFC1035_RCODE_TEMPFAIL,
**        soft DNS error.
**    Other situations: log the abnormal result, handle as soft DNS error.
*/

#define	RFC1035_ERR_CNAME_RECURSIVE	-2
	/* Specific error code for a recursive CNAME record - prohibited */

int rfc1035_resolve_cname(
		struct rfc1035_res *,	/* Pointer to a resolver structure */
		char *,			/* RFC1035_MAXNAMESIZE buffer with
					** the name to query */
		unsigned,		/* Query type */
		unsigned,		/* Query class */
		struct rfc1035_reply **, /* Ptr set to reply received */
		int); /* Extended flags: */

#define RFC1035_X_RANDOMIZE 1    /* Randomize query results */

	/*
	** Always call replyfree when done.
	*/

void rfc1035_replyfree(struct rfc1035_reply *);

	/*
	** !!!ALL!!! the const char * pointers in rfc1035_reply are NOT
	** standard C strings, but DNS-compressed strings.  Call
	** replyhostname to translate those to C strings.
	*/

const char *rfc1035_replyhostname(
	const struct rfc1035_reply *,	/* The reply */
	const char *,	/* The const char ptr */

	char *);	/* Buffer where to put hostname.  All strings are
			** guaranteed to fit into RFC1035_MAXNAMESIZE+1 byte
			** buffer.
			** replyhostname returns this pointer.
			*/

/* Some value added code, look up A and PTR records. */

int rfc1035_a(struct rfc1035_res *,
	const char *,		/* Host name */
	RFC1035_ADDR **,		/* We allocate array of IP addresses */
	unsigned *);			/* We return # of IP addresses here */

int rfc1035_ptr(struct rfc1035_res *,
	const RFC1035_ADDR *,	/* Query PTR for this address */
	char *);		/* Result - RFC1035_MAXNAMESIZE+1 buf */

int rfc1035_ptr_x(struct rfc1035_res *res, const RFC1035_ADDR *addr,
		  void (*cb_func)(const char *, void *),
		  void *cb_arg); /* Invoke a callback function instead
				 ** (multiple callbacks possible)
				 */

/* ---------------------- */

	/* Replyuncompress is a lower-level function taking a pointer to
	** the const char *.  When it returns, the const char * is advanced
	** past the end of the compressed string in the DNS data.
	** replyuncompress returns its third argument, or NULL if there was
	** an error. */

const char *rfc1035_replyuncompress(const char **,
	const struct rfc1035_reply *, char *);

#define	RFC1035_MAXNAMESIZE	255


	/*
	** Compare two hostnames.  Return 0 if they match, non-zero if they
	** don't.
	*/

int rfc1035_hostnamecmp(const char *, const char *);

	/*
	** After we receive a reply, search for the answer there.  Returns
	** an index in the respective section, or -1 if not found.
	** If we find a CNAME, we return a pointer to it instead, so make
	** sure to check for that!
	*/

int rfc1035_replysearch_an(
	const struct rfc1035_res *,	/* The resolver */
	const struct rfc1035_reply *,	/* The reply */
	const char *, 			/* Hostname to search */
	unsigned,			/* Type */
	unsigned,			/* Class */
	int);				/* Starting position, 1st time use 0 */

int rfc1035_replysearch_ns(
	const struct rfc1035_res *,	/* The resolver */
	const struct rfc1035_reply *,	/* The reply */
	const char *, 			/* Hostname to search */
	unsigned,			/* Type */
	unsigned,			/* Class */
	int);				/* Starting position, 1st time use 0 */

int rfc1035_replysearch_all(
	const struct rfc1035_res *,	/* The resolver */
	const struct rfc1035_reply *,	/* The reply */
	const char *, 			/* Hostname to search */
	unsigned,			/* Type */
	unsigned,			/* Class */
	int);				/* Starting position, 1st time use 0 */

/*
** Low level functions follow.
*/

	/*
	** rfc1035_mkquery() constructs a query to be sent.  The query is
	** composed by REPEATEDLY running the caller-provided function,
	** which will be called REPEATEDLY to build the query, part by part.
	*/

int rfc1035_mkquery(struct rfc1035_res *,	/* resolver structure */
			unsigned,	/* opcode */

#define	RFC1035_RESOLVE_RECURSIVE 1	/* Ask nameserver to do the recursion */

			const struct rfc1035_query *,	/* questions */
			unsigned,	/* Number of questions */
			void (*)(const char *, unsigned, void *),
					/* Function - called repetitively
					** to build the query */
			void *);	/* Third arg to function */

/**************************************************************************/
/* Low level input/output functions.  Most people won't need to use these */
/**************************************************************************/

int rfc1035_open_udp(int *af);		/* Create a UDP socket */

int rfc1035_send_udp(int,	/* File descriptor from rfc1035_open */
		const struct sockaddr *, int, /* Send to this name server */
		const char *,	/* The query */
		unsigned);	/* Query length */
	/*
	** Returns 0, or non-zero if failed.
	*/

int rfc1035_wait_reply(int,	/* File descriptor from rfc1035_open */
	unsigned);	/* Number of seconds to wait, use 0 for default */
	/* Returns 0 when reply is waiting, non-0 if timeout expired */

int rfc1035_wait_query(int,	/* File descriptor from rfc1035_open */
	unsigned);	/* Number of seconds to wait, use 0 for default */
	/* Like reply, but we select for writing */

char *rfc1035_recv_udp(int,	/* File descriptor from rfc1035_open */
	const struct sockaddr *, int,
				/* Expecting reply from this IP address  */
	int *,		/* * will be set to point to # of bytes received */
	const char *);	/* Original query, used to validate id # */
	/* Returns ptr to dynamically allocated memory containing the reply,
	** or NULL if error.  Errno will be set EAGAIN if we should try
	** again, because the message received was not in response
	** to the query.
	*/

char *rfc1035_query_udp(struct rfc1035_res *,
	int,	/* file descriptor */
	const struct sockaddr *, int,	/* Attempt number */
	const char *,		/* query */
	unsigned,		/* query length */
	int *,			/* # of bytes received */
	unsigned);		/* # of seconds to wait for response */

	/*
	** Jumbo function: sends the indicated query via UDP, waits for
	** a validated reply.  Returns pointer to dynamically allocated
	** memory with the reply.  Returns NULL if there was an error.
	** errno will be set to EAGAIN if the response timed out
	** (if the UDP stack returns an error, we fake an EAGAIN).
	** After getting EAGAIN, attempt number should be incremented,
	** and we should try again.
	** Fake ETIMEDOUT is returned if no more attempts are possible.
	*/

int rfc1035_open_tcp(struct rfc1035_res *, const RFC1035_ADDR *);
				/*
				** Create a TCP socket for this attempt #,
				** returns negative for a failure, and sets
				** errno.
				*/

	/*
	** Attempt to transmit the indicated query on this TCP socket.
	** Return 0 for successfull transmission.
	*/

int rfc1035_send_tcp(int,	/* file descriptor */
		const char *,	/* query */
		unsigned);	/* query length */

	/*
	** Attempt to receive a reply on this TCP socket.
	** Returns pointer to dynamically malloced memory, or null if error.
	*/

char *rfc1035_recv_tcp(struct rfc1035_res *,
		int,	/* file descriptor */
		int *,		/* * initialized to contain msg length */
		unsigned);	/* # of seconds to wait for a response */

char *rfc1035_query_tcp(struct rfc1035_res *,
	int,	/* file descriptor */
	const char *,		/* query */
	unsigned,		/* query length */
	int *,			/* * initialized to contain msg length */
	unsigned);		/* # of seconds to wait for response */

/*************************************************/
/* Parse a raw response into a useful structure. */
/*************************************************/

struct rfc1035_rr {
	const char *rrname;  /* NOT a null term str, a ptr into the raw resp */
	unsigned rrtype, rrclass;
	RFC1035_UINT32  ttl;
	unsigned rdlength;
	const char    *rdata;	/* Raw data, parsed record follows: */

	union {
		struct {
			const char *hinfo_str;
			const char *os_str;
			} hinfo;

		struct in_addr inaddr;		/* A */
#if RFC1035_IPV6
		struct in6_addr in6addr;	/* AAAA */
#endif

		const char *domainname;
				/* CNAME, MB, MD, MF, MG, MR, NS, PTR */

		struct {
			const char *rmailbx_label;
			const char *emailbx_label;
			} minfo;

		struct {
			unsigned preference;
			const char *mx_label;
			} mx;

		struct {
			const char *mname_label;
			const char *rname_label;
			RFC1035_UINT32 serial;
			RFC1035_UINT32 refresh;
			RFC1035_UINT32 retry;
			RFC1035_UINT32 expire;
			RFC1035_UINT32 minimum;
			} soa;

		struct {
			RFC1035_UINT16 type_covered;
			unsigned char algorithm;
			unsigned char labels;
			RFC1035_UINT16 original_ttl;
			RFC1035_UINT32 signature_expiration;
			RFC1035_UINT32 signature_inception;
			RFC1035_UINT16 key_tag;
			const char *signer_name;
			const char *signature;
			RFC1035_UINT16 signature_len;
		} rrsig;

		/* As are just represented by rdata/rdlength */
		/* TXTs are parsed directly from rdata/rdlength */
		/* WKS are parsed directly */

		} rr;
	} ;

struct rfc1035_reply {
	struct	rfc1035_reply	*next;	/* AXFRs have a linked list here */

	const	char	*reply;	/* The raw reply, for convenience's sake */
	unsigned	replylen;	/* The length of the reply */
	char		*mallocedbuf;	/* If not NULL, dynamically allocated
					** memory that holds the reply.
					*/

	RFC1035_NETADDR server_addr;	/* Replying server */

	unsigned char qr;
	unsigned char opcode;
	unsigned char aa;
	unsigned char tc;
	unsigned char rd;
	unsigned char ra;
	unsigned char ad;
	unsigned char cd;
	unsigned char rcode;
	unsigned qdcount;
	unsigned ancount;
	unsigned nscount;
	unsigned arcount;

	struct rfc1035_query *qdptr;	/* sizeof qdcount */
	struct rfc1035_rr *anptr;
	struct rfc1035_rr *nsptr;
	struct rfc1035_rr *arptr;

	struct rfc1035_rr **allrrs;	/* Pointers to all RR records,
					** add ancount+nscount+arcount for
					** the size of the array */
	} ;

struct rfc1035_reply *rfc1035_replyparse(const char *, unsigned);
void rfc1035_rr_rand_an(struct rfc1035_reply *rr);
void rfc1035_rr_rand_ns(struct rfc1035_reply *rr);
void rfc1035_rr_rand_ar(struct rfc1035_reply *rr);
void rfc1035_rr_rand(struct rfc1035_reply *rr);

void rfc1035_dump(struct rfc1035_reply *, FILE *);

const char *rfc1035_fmttime(unsigned long, char *);
#define	RFC1035_MAXTIMEBUFSIZE	40

char *rfc1035_dumprrdata(struct rfc1035_reply *, struct rfc1035_rr *);

int rfc1035_rr_gettxt(struct rfc1035_rr *,
		      int,
		      char buf[256]);

/*
** I ignore any possible bugs in the resolver functions, and roll my own,
** for IPv4.
*/

void rfc1035_ntoa_ipv4(const struct in_addr *in, char *buf);
int rfc1035_aton_ipv4(const char *p, struct in_addr *in4);

#if RFC1035_IPV6
void rfc1035_ipv6to4(struct in_addr *, const struct in6_addr *);
void rfc1035_ipv4to6(struct in6_addr *, const struct in_addr *);
#endif

/*
** Extract network address from a socket address.
*/

int rfc1035_sockaddrip(const RFC1035_NETADDR *,	/* Socket address buffer */
		int,				/* Length of address */
		RFC1035_ADDR *);		/* Address saved here */

int rfc1035_sockaddrport(const RFC1035_NETADDR *, /* Socket address buffer */
		int,				/* Length of address */
		int *);				/* Port saved here */

#if RFC1035_IPV6
#define	RFC1035_NTOABUFSIZE	INET6_ADDRSTRLEN
#else
#define	RFC1035_NTOABUFSIZE	16
#endif

void rfc1035_ntoa(const RFC1035_ADDR *, char *);
int rfc1035_aton(const char *, RFC1035_ADDR *);

/*
** New function that compares two addresses -- handles both IPv4 and IPv6:
*/

int rfc1035_same_ip(const void *, int, const void *, int);

int     rfc1035_bindsource(int sockfd,	/* Socket fd */
	const struct sockaddr *addr,	/* Buffer to socket address */
	int addrlen);			/* Size of socket address */

/*
** First try to create an IPv6 socket, then, if we fail, an IPv4 socket.
*/

int	rfc1035_mksocket(int sock_type,		/* socket type to create */
			int sock_protocol,	/* socket protocol to create */
			int *af);	/* If succeed, address family created */

/*
** Take the destination address, and create a socket address structure
** suitable for connecting to this address.
*/

int	rfc1035_mkaddress(int af,		/* AF_INET or AF_INET6 */

	/* buf is initiailized to the sender's ip address. */

	RFC1035_NETADDR *buf,		/* Buffer for the created address */
	const RFC1035_ADDR *addr,	/* Network address */
	int port,			/* Network port (network byte order) */
	const struct sockaddr **ptr,	/* Will point to buf */
	int *len);			/* Will be size of socket address */

/*
** A convenient interface to the SIOCGIFCONF ioctl.
*/

struct rfc1035_ifconf {
	struct rfc1035_ifconf *next;
	char *ifname;
	RFC1035_ADDR ifaddr;
};

struct rfc1035_ifconf *rfc1035_ifconf(int *errflag);
void rfc1035_ifconf_free(struct rfc1035_ifconf *ifconf_list);

#ifdef  __cplusplus
}
#endif

#endif
