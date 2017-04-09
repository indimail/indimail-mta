/* RFC1035, RFC1996 used as references */

struct DNS_header {
	unsigned short	id;		/* Query ID */
	unsigned	qr:1;		/* Reponse flag: 0=query */
	unsigned	opcode:4;	/* Opcode: query type (4=notify) */
	unsigned	aa:1;		/* Authoritative answer flag: 1=yes */
	unsigned	trunc:1;	/* Truncation flag: 0=no */
	unsigned	rd:1;		/* Recursion desired flag: 0=no */
	unsigned	ra:1;		/* Recursion available: 0=no */
	unsigned	z:3;		/* Reserved for future use: 0 */
	unsigned	rcode:4;	/* Response code: 0=no error */
	unsigned short	qdcount;	/* Number of entries in question section (usually 1) */
	unsigned short	ancount;	/* Number of entries in answer section (usually 0) */
	unsigned short	nscount;	/* Number of entries in name server RR in authority section (usually 0) */
	unsigned short	arcount;	/* Number of entries in additional RR section (usually 0) */
};

struct DNS_query {
	/* Zone labels goes here, then ... */
	unsigned short	qtype;		/* Query type: 6=T_SOA */
	unsigned short	qclass;		/* Query class: 1=IN */
};

#define	MAXPACK		512
#define	MAXHOST		128
/* Thanks to Johann Haider <jhaider@zid.tuwien.ac.at> for reminding
 * that this value needs to be converted to big-endian for x86 hosts. */
#define	DOMAIN_PORT	htons(53)
