/*
 * $Log: get_local_ip.c,v $
 * Revision 2.6  2010-03-28 16:53:26+05:30  Cprogrammer
 * set other hints flag (ai_socktype, ai_flags, ai_protocol)
 *
 * Revision 2.5  2009-07-10 20:49:35+05:30  Cprogrammer
 * BUG - hints not initialized
 *
 * Revision 2.4  2009-07-09 15:54:22+05:30  Cprogrammer
 * added ipv6 code
 *
 * Revision 2.3  2005-12-29 22:44:46+05:30  Cprogrammer
 * use getEnvConfigStr to set variables from environment variables
 *
 * Revision 2.2  2002-08-25 22:32:45+05:30  Cprogrammer
 * made control dir configurable
 *
 * Revision 2.1  2002-07-27 21:36:26+05:30  Cprogrammer
 * enabled caching of result
 *
 * Revision 1.3  2002-03-30 14:50:09+05:30  Cprogrammer
 * return ip address from QMAILDIR/control/localiphost if present
 *
 * Revision 1.2  2002-03-29 17:24:59+05:30  Cprogrammer
 * removed hardcoding of hostname variable
 *
 * Revision 1.1  2001-12-08 12:36:32+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: get_local_ip.c,v 2.6 2010-03-28 16:53:26+05:30 Cprogrammer Stab mbhangui $";
#endif

char           *
get_local_ip()
{
	static char     hostbuf[MAX_BUFF]; /*- hostname or ip address */
	char            TmpBuf[MAX_BUFF];
	char           *ptr, *qmaildir, *controldir;
	FILE           *fp;
#ifdef HAVE_STRUCT_SOCKADDR_STORAGE
	struct sockaddr *sa;
	struct addrinfo hints;
	struct addrinfo *res, *res0;
	int             error, salen;
#else
	struct hostent *host_data;
#endif

	if (*hostbuf)
		return(hostbuf);
	getEnvConfigStr(&qmaildir, "QMAILDIR", QMAILDIR);
	getEnvConfigStr(&controldir, "CONTROLDIR", "control");
	snprintf(TmpBuf, MAX_BUFF, "%s/%s/localiphost", qmaildir, controldir);
	if ((fp = fopen(TmpBuf, "r"))) {
		if (!fgets(hostbuf, MAX_BUFF - 1, fp)) {
			fclose(fp);
			*hostbuf = 0;
		} else {
			if ((ptr = strchr(hostbuf, '\n')))
				*ptr = 0;
			fclose(fp);
			return(hostbuf);
		}
	}
	if (gethostname(TmpBuf, sizeof(TmpBuf)))
		return((char *) 0);
#ifdef ENABLE_IPV6
	memset(&hints, 0, sizeof(hints));
	/* set-up hints structure */
	hints.ai_family = PF_UNSPEC; /*- Allow IPv4 or IPv6 */
	hints.ai_socktype = SOCK_DGRAM; /*- Datagram socket */
	hints.ai_flags = 0;
	hints.ai_protocol = 0; /*- Any protocol */
	if ((error = getaddrinfo(TmpBuf, 0, &hints, &res0))) {
		fprintf(stderr, "get_local_ip: getaddrinfo: %s\n", gai_strerror(error));
		return((char *) 0);
	} else {
		for (res = res0; res; res = res->ai_next) {
			sa = res->ai_addr;
			salen = res->ai_addrlen;
			/* do what you want */
			/* getnameinfo() case. NI_NUMERICHOST avoids DNS lookup. */
			if ((error = getnameinfo(sa, salen, hostbuf, sizeof(hostbuf), 0, 0, NI_NUMERICHOST)))
				perror("getnameinfo");
			if (res->ai_flags | AI_NUMERICHOST) {
				freeaddrinfo(res0);
				return (hostbuf);
			}
		}
		freeaddrinfo(res0);
		return((char *) 0);
	}
#else
	if (!(host_data = gethostbyname(TmpBuf)))
		return((char *) 0);
	strncpy(hostbuf, inet_ntoa(*((struct in_addr *) host_data->h_addr_list[0])), MAX_BUFF);
#endif
	return(hostbuf);
}

void
getversion_get_local_ip_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
