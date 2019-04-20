/*
 * $Log: inquery.c,v $
 * Revision 2.22  2018-09-11 10:34:57+05:30  Cprogrammer
 * fixed compiler warnings
 *
 * Revision 2.21  2017-03-27 08:53:59+05:30  Cprogrammer
 * added FIFODIR variable for location of infifo
 *
 * Revision 2.20  2017-03-13 14:01:57+05:30  Cprogrammer
 * replaced qmaildir with sysconfdir
 *
 * Revision 2.19  2016-05-17 17:09:39+05:30  Cprogrammer
 * use control directory set by configure
 *
 * Revision 2.18  2016-01-12 23:46:38+05:30  Cprogrammer
 * fixed setting of InFifo file
 *
 * Revision 2.17  2016-01-12 14:20:31+05:30  Cprogrammer
 * new logic for selecting fifo
 *
 * Revision 2.16  2010-04-11 22:21:19+05:30  Cprogrammer
 * replaced LPWD_QUERY with LIMIT_QUERY for domain limits
 *
 * Revision 2.15  2008-05-28 21:55:09+05:30  Cprogrammer
 * removed LDAP_QUERY
 *
 * Revision 2.14  2005-12-29 22:41:16+05:30  Cprogrammer
 * use getEnvConfigStr to set variables from environment variables
 *
 * Revision 2.13  2004-02-18 14:23:49+05:30  Cprogrammer
 * added domain query
 *
 * Revision 2.12  2002-12-12 01:25:23+05:30  Cprogrammer
 * added code for random balancing of dbqueries
 *
 * Revision 2.11  2002-10-19 18:24:04+05:30  Cprogrammer
 * initialize permanent error in the begining
 *
 * Revision 2.10  2002-10-18 14:53:53+05:30  Cprogrammer
 * added load balancing code
 *
 * Revision 2.9  2002-09-20 23:54:25+05:30  Cprogrammer
 * added timeout for writes
 *
 * Revision 2.8  2002-08-25 22:30:37+05:30  Cprogrammer
 * made control dir configurable
 *
 * Revision 2.7  2002-08-03 00:35:39+05:30  Cprogrammer
 * added local passwd query for local authentication only (by IMAP and POP3 daemons)
 *
 * Revision 2.6  2002-07-22 13:10:47+05:30  Cprogrammer
 * corrected erroneous error message
 *
 * Revision 2.5  2002-07-15 19:46:40+05:30  Cprogrammer
 * added LDAP_QUERY
 *
 * Revision 2.4  2002-07-07 21:12:39+05:30  Cprogrammer
 * improved error reporting
 *
 * Revision 2.3  2002-07-04 00:33:02+05:30  Cprogrammer
 * interpret userNotFound on value of bytes
 *
 * Revision 2.2  2002-07-01 13:31:50+05:30  Cprogrammer
 * corrected location for control file timeoutread
 *
 * Revision 2.1  2002-06-26 03:17:30+05:30  Cprogrammer
 * correction for non-distributed code
 *
 * Revision 1.3  2002-04-10 11:58:27+05:30  Cprogrammer
 * added variable readTimeout
 *
 * Revision 1.2  2002-04-10 10:23:57+05:30  Cprogrammer
 * removed for(;;) loop for timeoutread
 *
 * Revision 1.1  2002-04-10 02:58:05+05:30  Cprogrammer
 * Initial revision
 *
 */

#ifndef	lint
static char     sccsid[] = "$Id: inquery.c,v 2.22 2018-09-11 10:34:57+05:30 Cprogrammer Exp mbhangui $";
#endif

#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include "indimail.h"

/*- 
 *  Format of Query Buffer
 *  |len of string - int|QueryType - 1|NULL - 1|EmailId - len1|NULL - 1|Fifo - len2|NULL - 1|Ip - len3|NULL - 1|
 */
void           *
inquery(char query_type, char *email, char *ip)
{
	int             rfd, wfd, len1, len2, len3, bytes, idx, readTimeout, writeTimeout, pipe_size, tmperrno, relative;
	static int      intBuf;
	char            QueryBuf[MAX_BUFF + sizeof(int)], myFifo[MAX_BUFF], InFifo[MAX_BUFF + 25], TmpBuf[MAX_BUFF + 12];
	char           *sysconfdir, *controldir, *infifo_dir, *infifo;
	static char    *pwbuf;
	FILE           *fp;
	void            (*sig_pipe_save) ();

	userNotFound = 0;
	switch(query_type)
	{
		case RELAY_QUERY:
			if(!ip || !*ip)
			{
				errno = EINVAL;
				return ((void *) 0);
			}
		case USER_QUERY:
		case PWD_QUERY:
#ifdef CLUSTERED_SITE
		case HOST_QUERY:
#endif
		case ALIAS_QUERY:
#ifdef ENABLE_DOMAIN_LIMITS
		case LIMIT_QUERY:
#endif
		case DOMAIN_QUERY:
			break;
		default:
			errno = EINVAL;
			return ((void *) 0);
	}
	/* - Prepare the Query Buffer for the Daemon */
	QueryBuf[sizeof(int)] = query_type;
	QueryBuf[1 + sizeof(int)] = 0;
	scopy(QueryBuf + 2 + sizeof(int), email, MAX_BUFF - 2);
	len1 = slen(email);
	snprintf(myFifo, MAX_BUFF, "/tmp/%d.%ld", getpid(), time(0));
	len2 = slen(myFifo);
	scopy(QueryBuf + 2 + len1 + 1 + sizeof(int), myFifo, MAX_BUFF - (2 + len1 + 1));
	if(ip && *ip)
	{
		scopy(QueryBuf + 2 + len1 + 1 + len2 + 1 + sizeof(int), ip, MAX_BUFF - (2 + len1 + 1 + len2 + 1));
		len3 = slen(ip);
	} else
		len3 = 0;
	QueryBuf[2 + len1 + 1 + len2 + 1 + len3 + sizeof(int)] = 0;
	bytes = sizeof(char) + len1 + len2 + len3 + 4;
	*((int *) QueryBuf) = bytes;
	bytes += sizeof(int);

	getEnvConfigStr(&sysconfdir, "SYSCONFDIR", SYSCONFDIR);
	getEnvConfigStr(&controldir, "CONTROLDIR", CONTROLDIR);
	getEnvConfigStr(&infifo_dir, "FIFODIR", INDIMAILDIR"/inquery");
	relative = *infifo_dir == '/' ? 0 : 1;
	getEnvConfigStr(&infifo, "INFIFO", INFIFO);
	/*- Open the Fifos */
	if (*infifo == '/' || *infifo == '.') {
		snprintf(TmpBuf, MAX_BUFF, "%s", infifo);
		scopy(InFifo, TmpBuf, MAX_BUFF);
	} else {
		if (relative)
			snprintf(TmpBuf, MAX_BUFF, INDIMAILDIR"%s/%s", infifo_dir, infifo);
		else {
			if (indimailuid == -1 || indimailgid == -1)
				GetIndiId(&indimailuid, &indimailgid);
			r_mkdir(infifo_dir, 0775, indimailuid, indimailgid);
			snprintf(TmpBuf, MAX_BUFF, "%s/%s", infifo_dir, infifo);
		}
		for (idx = 1;;idx++) {
			snprintf(InFifo, sizeof(InFifo) - 1, "%s.%d", TmpBuf, idx);
			if(access(InFifo, F_OK))
				break;
		}
#ifdef RANDOM_BALANCING
		srand(getpid() + time(0));
#endif
#ifdef RANDOM_BALANCING
		snprintf(InFifo, sizeof(InFifo) - 1, "%s.%d", TmpBuf, 1 + (int) ((float) (idx - 1) * rand()/(RAND_MAX + 1.0)));
#else
		snprintf(InFifo, sizeof(InFifo) - 1, "%s.%ld", TmpBuf, 1 + (time(0) % (idx - 1)));
#endif
	}
	if(verbose)
		printf("Using INFIFO=%s\n", InFifo);
	if ((wfd = open(InFifo, O_WRONLY | O_NDELAY, 0)) == -1)
	{
		perror(InFifo);
		return ((void *) 0);
	} else 
	if(bytes > (pipe_size = fpathconf(wfd, _PC_PIPE_BUF)))
	{
		errno = EMSGSIZE;
		return ((void *) 0);
	} else
	if(FifoCreate(myFifo) == -1)
	{
		tmperrno = errno;
		perror(myFifo);
		close(wfd);
		errno = tmperrno;
		return ((void *) 0);
	} else
	if ((rfd = open(myFifo, O_RDONLY | O_NDELAY, 0)) == -1)
	{
		tmperrno = errno;
		perror(myFifo);
		close(wfd);
		unlink(myFifo);
		errno = tmperrno;
		return ((void *) 0);
	} else
	if ((sig_pipe_save = signal(SIGPIPE, SIG_IGN)) == SIG_ERR)
	{
		tmperrno = errno;
		close(rfd);
		close(wfd);
		unlink(myFifo);
		errno = tmperrno;
		return ((void *) 0);
	} 
	if (relative)
		snprintf(TmpBuf, MAX_BUFF, "%s/%s/timeoutwrite", sysconfdir, controldir);
	else
		snprintf(TmpBuf, MAX_BUFF, "%s/timeoutwrite", controldir);
	if((fp = fopen(TmpBuf, "r")))
	{
		if(fgets(TmpBuf, MAX_BUFF - 2, fp))
		{
			if(sscanf(TmpBuf, "%d", &writeTimeout) != 1)
				writeTimeout = 4;
		}
		fclose(fp);
	} else
		writeTimeout = 4;
	if(timeoutwrite(writeTimeout, wfd, QueryBuf, bytes) != bytes)
	{
		tmperrno = errno;
		perror("write-fifo");
		signal(SIGPIPE, sig_pipe_save);
		close(wfd);
		close(rfd);
		unlink(myFifo);
		errno = tmperrno;
		return ((void *) 0);
	}
	signal(SIGPIPE, sig_pipe_save);
	close(wfd);
	switch(query_type)
	{
		case USER_QUERY:
		case RELAY_QUERY:
		case PWD_QUERY:
#ifdef CLUSTERED_SITE
		case HOST_QUERY:
#endif
		case ALIAS_QUERY:
#ifdef ENABLE_DOMAIN_LIMITS
		case LIMIT_QUERY:
#endif
		case DOMAIN_QUERY:
			if (relative)
				snprintf(TmpBuf, MAX_BUFF, "%s/%s/timeoutread", sysconfdir, controldir);
			else
				snprintf(TmpBuf, MAX_BUFF, "%s/timeoutread", controldir);
			if((fp = fopen(TmpBuf, "r")))
			{
				if(fgets(TmpBuf, MAX_BUFF - 2, fp))
				{
					if(sscanf(TmpBuf, "%d", &readTimeout) != 1)
						readTimeout = 4;
				}
				fclose(fp);
			} else
				readTimeout = 4;
			if((idx = timeoutread(readTimeout, rfd, (char *) &intBuf, sizeof(int))) == -1 || !idx)
			{
				tmperrno = errno;
				perror("read-intBuf");
				close(rfd);
				unlink(myFifo);
				errno = tmperrno;
				return((void *) 0);
			} else
			if(intBuf == -1)
			{
				close(rfd);
				unlink(myFifo);
				errno = 0;
				return((void *) 0);
			} else
			if(intBuf > pipe_size)
			{
				close(rfd);
				unlink(myFifo);
				errno = EMSGSIZE;
				return((void *) 0);
			}
			switch(query_type)
			{
				case USER_QUERY:
				case RELAY_QUERY:
					close(rfd);
					unlink(myFifo);
					return(&intBuf);
				case PWD_QUERY:
#ifdef ENABLE_DOMAIN_LIMITS
				case LIMIT_QUERY:
#endif
					if(!intBuf)
					{
						close(rfd);
						unlink(myFifo);
						errno = 0;
						return((void *) 0);
					} else
					if(!(pwbuf = (char *) realloc(pwbuf, intBuf + 1)))
					{
						tmperrno = errno;
						perror("realloc");
						close(rfd);
						unlink(myFifo);
						errno = tmperrno;
						return((void *) 0);
					} 
					if((idx = timeoutread(readTimeout, rfd, pwbuf, intBuf)) == -1 || !idx)
					{
						tmperrno = errno;
						perror("read-PWD_QUERY");
						close(rfd);
						unlink(myFifo);
						errno = tmperrno;
						return((void *) 0);
					}
					close(rfd);
					unlink(myFifo);
#ifdef ENABLE_DOMAIN_LIMITS
					if (query_type == PWD_QUERY)
						return(strToPw(pwbuf, intBuf));
					else
						return (pwbuf);
#else
					return(strToPw(pwbuf, intBuf));
#endif
				case ALIAS_QUERY:
#ifdef CLUSTERED_SITE
				case HOST_QUERY:
#endif
				case DOMAIN_QUERY:
					if(!intBuf)
					{
						close(rfd);
						unlink(myFifo);
						userNotFound = 1;
						errno = 0;
						return((void *) 0);
					} else
					if(!(pwbuf = (char *) realloc(pwbuf, intBuf + 1)))
					{
						tmperrno = errno;
						fprintf(stderr, "query_type=%d: realloc: %s\n", query_type, strerror(errno));
						close(rfd);
						unlink(myFifo);
						errno = tmperrno;
						return((void *) 0);
					} 
					if((idx = timeoutread(readTimeout, rfd, pwbuf, intBuf)) == -1 || !idx)
					{
						tmperrno = errno;
						perror("read-HOST_QUERY");
						close(rfd);
						unlink(myFifo);
						errno = tmperrno;
						return((void *) 0);
					}
					close(rfd);
					unlink(myFifo);
					return(pwbuf);
					break;
				default:
					break;
			}
		default:
			break;
	} /*- switch(query_type) */
	close(rfd);
	unlink(myFifo);
	return((void *) 0);
}

void
getversion_inquery_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}