/*
 * $Log: greylist.c,v $
 * Revision 1.3  2009-10-28 13:34:29+05:30  Cprogrammer
 * fix scan_ip_port()
 * remove newline as delimiter
 *
 * Revision 1.2  2009-08-29 15:28:45+05:30  Cprogrammer
 * send the entire RCPT list in one packet
 *
 * Revision 1.1  2009-08-25 16:31:56+05:30  Cprogrammer
 * Initial revision based on code by Richard Andrews
 *
 */
#include "stralloc.h"
#include "ip.h"
#include "byte.h"
#include "scan.h"
#include "greylist.h"
#include "timeoutread.h"
#include "timeoutwrite.h"
#include "error.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>


static int 
fn_handler(errfn, timeoutfn, option)
	void              (*errfn)();
	void              (*timeoutfn)();
	int               option;
{
	if (!option)
		(*errfn)();
	else
		(*timeoutfn)();
	return (-1);
}

/*
 * Takes a string specifying IP address and port, separated by ':' If IP
 * address and/or port are missing, supplied defaults are used.
 */
int 
scan_ip_port(s, defaultip, defaultport, ipp, portp)
	char           *s, *defaultip;
	ip_addr        *ipp;
	unsigned int    defaultport, *portp;
{
	int             n;
	unsigned long   port;	/* long because of scan_ulong */

	if (!s)
	{
		ip_scan(defaultip, ipp);
		port = defaultport;
	} else {
		if (!(n = ip_scan(s, ipp)))
			ip_scan(defaultip, ipp);
		if (!(*(s + n) == ':' && scan_ulong(s + n + 1, &port)))
			port = defaultport;
	}
	*portp = (unsigned int) port;
	return (0);
}

int
connect_udp(ip, port, errfn)
	ip_addr           ip;
	unsigned int      port;
	void              (*errfn)();
{
	int               fd;
	struct sockaddr_in sout;

	byte_zero((char *) &sout, sizeof(sout));
	sout.sin_port = htons(port);
	sout.sin_family = AF_INET;
	byte_copy((char *) &sout.sin_addr, 4, (char *) &ip);
	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
		return (errfn ? fn_handler(errfn, 0, 0) : -1);
	if (connect(fd, (struct sockaddr *)&sout, sizeof(sout)) < 0)
		return (errfn ? fn_handler(errfn, 0, 0) : -1);
	return (fd);
}

int 
query_skt(fd, queryp, responsep, maxresponsesize, timeout, timeoutfn, errfn)
	int             fd;
	stralloc       *queryp;
	char           *responsep;
	int             maxresponsesize, timeout;
	void            (*errfn) (), (*timeoutfn) ();
{
	int             r = 0;

	if (timeoutwrite(timeout, fd, queryp->s, queryp->len) < 0)
		return (errfn ? fn_handler(errfn, 0, 0) : -1);
	if ((r = timeoutread(timeout, fd, responsep, maxresponsesize)) == -1)
	{
  		if (errno == error_timeout)
		{
			responsep[0] = 2;
			return (errfn ? fn_handler(errfn, timeoutfn, 1) : -1);
		}
		return (errfn ? fn_handler(errfn, 0, 0) : -1);
	}
	return (r);
}

/*
 * Check given greylist triple: Pass connectingip+from+tolist to greydaemon
 * on IP address gip. timeoutfn and errfn passed in case of error. Note that
 * greylist may be called more than once during a single SMTP session (=
 * qmail-smtpd instance).
 */
stralloc        chkpacket = {0};

int 
greylist(gip, connectingip, from, tolist, tolen, timeoutfn, errfn)
	char           *gip, *connectingip, *from, *tolist;
	int             tolen;
	void            (*timeoutfn) (), (*errfn) (); /*- errfn must _exit */
{
	int             r, port;
	ip_addr         ip;
	char            rbuf[2];
	static int      sockfd = -1;

	if (!gip)
	{
		errno = EINVAL;
		return (errfn ? fn_handler(errfn, 0, 0) : -1);
	}
	if (sockfd == -1)
	{
		if (scan_ip_port(gip, DEFAULTGREYIP, DEFAULTGREYPORT, &ip, &port) == -1)
			return (errfn ? fn_handler(errfn, 0, 0) : -1);
		if ((sockfd = connect_udp(ip, port, errfn)) == -1)
			return (errfn ? fn_handler(errfn, 0, 0) : -1);
	}
	if (!stralloc_copys(&chkpacket, "I"))
		return (-2); /*- ENOMEM */
	if (!stralloc_cats(&chkpacket, connectingip))
		return (-2);
	if (!stralloc_0(&chkpacket))
		return (-2);
	if (!stralloc_append(&chkpacket, "F"))
		return (-2);
	if (!stralloc_cats(&chkpacket, from))
		return (-2);
	if (!stralloc_0(&chkpacket))
		return (-2);
	if (!stralloc_catb(&chkpacket, tolist, tolen))
		return (-2);
#if 0 /*- causes problem with greydaemon */
	if (!stralloc_append(&chkpacket, "\n")) /* newline to end a record */
		return (-2);
#endif
	if ((r = query_skt(sockfd, &chkpacket, rbuf, sizeof rbuf, GREYTIMEOUT, timeoutfn, errfn)) == -1)
		return -1;	/*- Permit connection (soft fail) - probably timeout */
	if (rbuf[0] == 0)
		return (0);	/* greylist */
	return (1);
}

void
getversion_greylist_c()
{
	static char    *x = "$Id: greylist.c,v 1.3 2009-10-28 13:34:29+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
