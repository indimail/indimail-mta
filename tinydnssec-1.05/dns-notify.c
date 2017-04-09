/*
 * Usage:  dns-notify ...
 *
 *	This programs notifies DNS slave servers to check for updates
 *	of zone information.  I wrote this to help with interoperation
 *	with our tinydns server and our BIND slave servers.  There are
 *	some Perl utilities that do the same thing, but I didn't have
 *	perl installed, and it seemed easier to write this than to install
 *	PERL + Net::DNS + patch to get it working.  Besides, if C is good
 *	enough for djbdns, ...
 *
 * Written by
 *
 *		Joseph Tam <tam (at) math (dot) ubc (dot) ca>
 *		Department of Mathematics
 *		University of British Columbia
 *		Canada V6T 1Z2
 *
 *		Last updated: 2011-01-12
 *
 * Permission is freely given to use, distribute or mangle.
 *
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <signal.h>
#include <setjmp.h>
#include "dns-notify.h"

int		s=0,				// Socket handle
		nretries=3,			// Number of times to retry
		try,				// Current number of attempts
		timeout=2,			// Number of seconds to wait for response
		verbose=0;			// Toggle verbose flag: default = no
char		source[MAXHOST] = "0.0.0.0",	// Source host: default = primary IP
		dest[MAXHOST] = "";		// Destination host: no default
sigjmp_buf	env;				// Jump buffer


//
// Help message.
//
void Usage (char *thisprog) {
	fprintf(stderr,
	"\n"
	"Usage:  %s [-h|-s<source>|-d<dest>|-r<retries>|-t<timeout>|-v|zone...] ...\n"
	"\n"
	"	-h	This help message\n"
	"	-s<ip>	Source IP of notify messages; useful if host is\n"
	"		multi-homed and is not zone serving from its primary\n"
	"		interface. [default=host's primary IP]\n"
	"	-d<ip>	IP or hostname to send notify messages to. [default=none]\n"
	"	-r<int>	Maximum retries ofnotify request. [default=3]\n"
	"	-t<int>	Timeout (in seconds) for responses before retry. [default=2]\n"
	"	-v	Toggle verbose diagnostic flag. [default=quiet]\n"
	"	zone...	Zone(s) to update.\n"
	"\n"
	"Arguments are acted upon in the order they are given.  Multiple zones\n"
	"on different slave servers can be notified:\n"
	"\n"
	"	-dslave1 zone1 -dslave2 zone2 ...\n"
	"\n"
	"If a set of zones are replicated by multiple slave servers, you'll have\n"
	"repeat the set for each server:\n"
	"\n"
	"	-dslave1 zones ... -dslave2 zones ...\n"
	"\n"
	,thisprog);
}


//
// Extract rcode from reply packet and return string message.
//
const char *ResponseCode(const struct DNS_header *h) {

	static char	mys[80];

	switch (h->rcode) {
		/* Grabbed from RFC2929 */
		case 0:		strcpy(mys,"OK (No error)"); break;
		case 1:		strcpy(mys,"Format error"); break;
		case 2:		strcpy(mys,"Server failure"); break;
		case 3:		strcpy(mys,"Non-existent domain"); break;
		case 4:		strcpy(mys,"Not implemented"); break;
		case 5:		strcpy(mys,"Query refused"); break;
		case 6:		strcpy(mys,"Name exists, but shouldn't"); break;
		case 7:		strcpy(mys,"RR Exists, but shouldn't"); break;
		case 8:		strcpy(mys,"RR doesn't exist, but should"); break;
		case 9:		strcpy(mys,"Server not authoratative for zone"); break;
		case 10:	strcpy(mys,"Name not contained in zone"); break;
		case 15:	strcpy(mys,"Bad OPT version"); break;
		/* There are others, but you're not likely to get them ... */
		default:	sprintf(mys,"Unknown rcode (%d)",h->rcode);
	}

	return mys;
}


//
// Create network socket to send/receive DNS messages.
//
void OpenSocket (char *myname)
{
	struct hostent		*h;		/* Host entry for self */
	struct sockaddr_in	me;		/* Socket addresses of source host */

	// Close previously opened socket
	if (s) close(s);

	if ((s=socket(PF_INET,SOCK_DGRAM,0))<0) {
		perror("socket");
		exit(1);
	}

	if (myname) {
		me.sin_family = AF_INET;
		if ((h=gethostbyname(myname))==NULL) {
			fprintf(stderr,"Can't open source host (%s)\n",myname);
			perror("gethostbyname");
			exit(1);
		}
		memcpy(&me.sin_addr,h->h_addr,h->h_length);
		//me.sin_port = 55555; // Uncomment to fix source port

		if (bind(s,(struct sockaddr *)&me,sizeof(me)) != 0) {
			perror("bind");
			exit(1);
		}
	}
}


//
// Output verbose decompilation of DNS query/reply.
//
void PrintDNSHeader (const char *title, struct DNS_header *h) {
	char		mys[80];

	fprintf(stderr,
		"\n\t=== %s ===\n"
		"\tQuery ID            = 0x%04x\n"
		"\tQuery/Response      = %s\n"
		,title,h->id,h->qr? "response":"query");

	switch (h->opcode) {
		case 0:		strcpy(mys,"Query"); break;
		case 1:		strcpy(mys,"Inverse query"); break;
		case 2:		strcpy(mys,"Status"); break;
		case 4:		strcpy(mys,"Notify"); break;
		case 5:		strcpy(mys,"Update"); break;
		default:	sprintf(mys,"Unknown opcode (%d)",h->opcode);
	}

	fprintf(stderr,
		"\tOpcode              = %s\n"
		"\tAuth. answer        = %sauthorative\n"
		"\tTruncated           = %s\n"
		"\tRecursion desired   = %s\n"
		"\tRecursion available = %s\n"
		"\tResponse code       = %s\n"
		"\tQuestion count      = %d\n"
		"\tAnswer count        = %d\n"
		"\tNameserver RR count = %d\n"
		"\tAdditional RR count = %d\n"
		,mys,h->aa? "":"non-",h->trunc?"yes":"no",h->rd?"yes":"no",h->ra?"yes":"no",
		ResponseCode(h),
		h->qdcount,h->ancount,h->nscount,h->arcount);
}


//
// Create DNS NOTIFY packet.
//
void MakeNotifyPacket (const char *zone, unsigned char packet[MAXPACK], int *packetlen)
{
	struct DNS_header	dh;
	struct DNS_query	dq;
	unsigned		zlen;
	unsigned char		*zp;
	register int		i, k;


	zlen = strlen(zone)+2;
	*packetlen = sizeof(dh) + zlen + sizeof(dq);

	if (*packetlen >= MAXPACK) {
		fprintf(stderr,"Packet size is too large: %d >= %d\n",*packetlen,MAXPACK);
		exit(1);
	}

	dh.id		= rand();
	dh.qr		= 0;
	dh.opcode	= 4;
	dh.aa		= 1;
	dh.trunc	= 0;
	dh.rd		= 0;
	dh.ra		= 0;
	dh.z		= 0;
	dh.rcode	= 0;
	dh.qdcount	= 1;
	dh.ancount	= 0;
	dh.nscount	= 0;
	dh.arcount	= 0;
	memcpy(packet,&dh,sizeof(dh));

	/* Make zone labels */
	zp = &packet[sizeof(dh)];
	memcpy(&zp[1],zone,zlen-1);
	zp[0] = '.';
	k = 0;

	while (zp[k]) {
		for (i=k+1; zp[i] && zp[i]!='.'; i++) ;
		zp[k] = i-k-1;
		k = i;
	}

	dq.qtype	= 6;
	dq.qclass	= 1;
	memcpy(&packet[sizeof(dh)+zlen],&dq,sizeof(dq));
}


//
// Timeout handler for UDP reply.
//
static void timeout_handler (int sig) {
	fputs("timeout.\n",stderr);
	try++;
	siglongjmp(env,sig);
}


//
// Send NOTIFY packet and wait for response.
//
void SendPacket (const char *slave_name, unsigned char packet[MAXPACK], const int packetlen) {
	struct hostent		*h;
	struct sockaddr_in	slave_addr;	/* Socket addresses of slave server */
	unsigned char		response[MAXPACK];
	int			resp_len, slen;

	slave_addr.sin_family = AF_INET;
	slave_addr.sin_port = DOMAIN_PORT;

	if ((h=gethostbyname(slave_name)) == NULL) {
		fprintf(stderr,"Can't open connection to slave host (%s)\n",slave_name);
		perror("gethostbyname");
		exit(1);
	}
	memcpy(&slave_addr.sin_addr,h->h_addr,h->h_length);

	if (verbose)
		PrintDNSHeader("Send", (struct DNS_header *) packet);

	try = 1;
	sigsetjmp(env,1);
	signal(SIGALRM,timeout_handler);
	alarm(timeout);

	if (try<=nretries) {

		fprintf(stderr,"\ttry#%d: ",try);

		if (sendto(s,packet,packetlen,0,(struct sockaddr *)&slave_addr,sizeof(slave_addr)) < 0) {
			perror("sendto");
			exit(1);
		}
		fputs("sent, ",stderr);

		//
		// When a NOTIFY message is sent, a response should be received which is
		// is the same as the query, but with the qr flag set.
		//
		slen = sizeof(slave_addr);
		if ((resp_len=recvfrom(s,response,MAXPACK,0,(struct sockaddr *)&slave_addr,&slen)) < 0) {
			perror("recvfrom");
			exit(1);
		}

		if (verbose)
			PrintDNSHeader("Reply",(struct DNS_header *)&response);
		else
			fprintf(stderr,"reply [%s].\n",
				ResponseCode((const struct DNS_header *)&response));
	} else
		fputs("\tGive up.\n",stderr);
}


//
// Main routine: parse arguments, and carry out requested actions.
//
int main (int argc, char *argv[])
{
	unsigned char	packet[MAXPACK];
	int		packetlen;
	register int	i;


	if (argc==1) {
		Usage(argv[0]);
		exit(0);
	}

	/* Seed PRNG to get query IDs */
	srand(getpid());

	for (i=1; i <argc; i++)
		if (*argv[i]=='-')
			switch (argv[i][1]) {
				case 'd':
					strncpy(dest,&argv[i][2],MAXHOST);
					break;

				case 'h':
					Usage(argv[0]);
					break;

				case 'r':
					nretries = atoi(&argv[i][2]);
					break;

				case 's':
					strncpy(source,&argv[i][2],MAXHOST);
					OpenSocket(source);
					break;

				case 't':
					timeout = atoi(&argv[i][2]);
					break;

				case 'v':
					verbose = !verbose;
					break;

				default:
					fprintf(stderr,"Unknown option: %s\n",argv[i]);
					break;
			}
		else {
			if (!dest[0]) {
				fputs("\tNo destination set.\n",stderr);
				continue;
			}
			fprintf(stderr,"Zone=%s (%s=>%s)\n",argv[i],source,dest);
			MakeNotifyPacket(argv[i],packet,&packetlen);
			SendPacket(dest,packet,packetlen);
		}
}
