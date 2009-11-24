/*
 * $Log: udpopen.c,v $
 * Revision 2.1  2002-07-31 14:54:50+05:30  Cprogrammer
 * function to open a udp connection
 *
 */

#ifndef	lint
static char     sccsid[] = "$Id: udpopen.c,v 2.1 2002-07-31 14:54:50+05:30 Cprogrammer Stab mbhangui $";
#endif

#include "indimail.h"
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>

int
udpopen(rhost, servicename)
	char           *rhost, *servicename;
{
	int             fd, retval;
	struct hostent *hp;	/*- pointer to host info for nameserver host */
	struct servent *sp;	/*- pointer to service information */
	struct sockaddr_in servaddr_in;
#ifndef LONGLONG /*- 64 Bit OS DEC OSF1, Solaris 2.8 */
	unsigned int    inaddr;
#else
	unsigned long   inaddr;
#endif

	/*- clear out address structures */
	memset((char *) &servaddr_in, 0, sizeof(struct sockaddr_in));
	servaddr_in.sin_family = AF_INET;
	if(isnum(servicename))
		servaddr_in.sin_port = htons(atoi(servicename));
	else
	if ((sp = getservbyname(servicename, "udp")))
		servaddr_in.sin_port = sp->s_port;
	else
	{
		fprintf(stderr, "%s/udp: No such service\n", servicename);
		errno = EINVAL;
		return (-1);
	}
	if ((inaddr = inet_addr(rhost)) != INADDR_NONE) /*- It's a dotted decimal */
		(void) memcpy((char *) &servaddr_in.sin_addr, (char *) &inaddr, sizeof(inaddr));
	else
	if ((hp = gethostbyname(rhost)) == NULL)
	{
		fprintf(stderr, "gethostbyname: %s: No such Host\n", rhost);
		errno = EINVAL;
		return (-1);
	} else
		(void) memcpy((char *) &servaddr_in.sin_addr, hp->h_addr, hp->h_length);
	/*- Create the socket. */
	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
		return (-1);
	if((retval = connect(fd, (struct sockaddr *) &servaddr_in, sizeof(servaddr_in))) == -1)
	{
		close(fd);
		return (-1);
	}
	return(fd);
}

void
getversion_udpopen_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
