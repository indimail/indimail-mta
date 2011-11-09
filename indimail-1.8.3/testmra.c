/*
 * $Log: testmra.c,v $
 * Revision 2.13  2011-11-09 19:45:34+05:30  Cprogrammer
 * removed getversion
 *
 * Revision 2.12  2009-02-18 21:33:49+05:30  Cprogrammer
 * removed compiler warning
 *
 * Revision 2.11  2008-07-13 19:48:17+05:30  Cprogrammer
 * use ERESTART only if available
 *
 * Revision 2.10  2005-12-29 22:50:28+05:30  Cprogrammer
 * use getEnvConfigStr to set variables from environment variables
 *
 * Revision 2.9  2005-12-21 09:50:03+05:30  Cprogrammer
 * make gcc 4 happy
 *
 * Revision 2.8  2004-05-25 10:01:59+05:30  Cprogrammer
 * hang problem on IMAP corrected
 *
 * Revision 2.7  2003-09-08 17:26:03+05:30  Cprogrammer
 * reduced default timeout to 60
 *
 * Revision 2.6  2003-09-05 09:40:31+05:30  Cprogrammer
 * corrected problem with testmra not timing out
 *
 * Revision 2.5  2003-07-06 14:16:29+05:30  Cprogrammer
 * added option to specify timeout
 *
 * Revision 2.4  2003-03-24 19:24:53+05:30  Cprogrammer
 * include descriptor 0 if invoked from tty
 *
 * Revision 2.3  2003-01-08 20:02:18+05:30  Cprogrammer
 * added sys/filio.h for FIONBIO definition
 *
 * Revision 2.2  2002-12-11 10:29:00+05:30  Cprogrammer
 * added ERESTART check for errno
 *
 * Revision 2.1  2002-12-09 00:19:36+05:30  Cprogrammer
 * program to test imap/pop3
 *
 */
#include "indimail.h"
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <sys/ioctl.h>
#ifdef sun
#include <sys/filio.h>
#endif

#ifndef	lint
static char     sccsid[] = "$Id: testmra.c,v 2.13 2011-11-09 19:45:34+05:30 Cprogrammer Stab mbhangui $";
#endif

static int      get_options(int, char **, char **, char **, char **, int *, int *, int *, char **, char **);
static void     usage();

int
main(int argc, char **argv)
{
	char           *ptr, *user = 0, *passwd = 0, *host = 0, *command;
	char            buffer[SOCKBUF + 1];
	struct timeval  timeout, Timeout;
	time_t          idle_time;
	int             sockfd, retrycount, retval, dataTimeout, type = 0, readData;
	fd_set          rfds;	/*- File descriptor mask for select -*/
	int             port = 0, keyboard = 0, flag = 0;

	if (get_options(argc, argv, &user, &passwd, &host, &port, &type, &keyboard, &ptr, &command))
		return (1);
#ifdef linux
#include <asm/ioctls.h>
#endif
	if ((sockfd = tcpopen(host, (char *) 0, port)) == -1)
	{
		fprintf(stderr, "tcpopen: %s: %s\n", host, strerror(errno));
		return (1);
	}
	dataTimeout = atoi(ptr);
	idle_time = 0;
	if (dataTimeout < SELECTTIMEOUT)
		Timeout.tv_sec = timeout.tv_sec = 5;
	else
		Timeout.tv_sec = timeout.tv_sec = SELECTTIMEOUT;
	Timeout.tv_usec = timeout.tv_usec = 0;
	for (retrycount = 0;;)
	{
		FD_ZERO(&rfds);
		if(keyboard)
			FD_SET(0, &rfds);
		FD_SET(sockfd, &rfds);
		/*- Is data available from client or server -*/
		if ((retval = select(sockfd + 1, &rfds, (fd_set *) NULL, (fd_set *) NULL, &Timeout)) < 0)
		{
#ifdef ERESTART
			if (errno == EINTR || errno == ERESTART)
#else
			if (errno == EINTR)
#endif
				continue;
			fprintf(stderr, "select: %s\n", strerror(errno));
			(void) close(sockfd);
			return (-1);
		} else
		if (!retval)
		{
			idle_time += timeout.tv_sec;
			if(idle_time > dataTimeout)
			{
				close(sockfd);
				fprintf(stderr, "select: Timeout %ld\n", timeout.tv_sec);
				return (2);
			}
			timeout.tv_sec += 2 * timeout.tv_sec;
			Timeout.tv_sec = timeout.tv_sec;
			continue;
		} else
		{
			if (dataTimeout < SELECTTIMEOUT)
				timeout.tv_sec = 5;
			else
				timeout.tv_sec = SELECTTIMEOUT;
			idle_time = 0;
		}
		if (FD_ISSET(sockfd, &rfds))
		{
			/*- Data from Remote Server -*/
			for (;;)
			{
				errno = 0;
				if ((retval = read(sockfd, buffer, SOCKBUF)) < 0)
				{
#ifdef ERESTART
					if (errno == EINTR || errno == ERESTART)
#else
					if (errno == EINTR)
#endif
						continue;
					else
					if (errno == ENOBUFS && retrycount++ < MAXNOBUFRETRY)
					{
						(void) usleep(1000);
						continue;
					}
#ifdef HPUX_SOURCE
					else
					if (errno == EREMOTERELEASE)
					{
						retval = 0;
						break;
					}
#endif
					fprintf(stderr, "read-dst: %s\n", strerror(errno));
					(void) close(sockfd);
					return (-1);
				}
				break;
			}
			if (!retval)	/*- EOF from server -*/
				break;
			/*- Write data to Client -*/
			if (sockwrite(1, buffer, retval) != retval)
			{
				fprintf(stderr, "sockwrite-src: %s\n", strerror(errno));
				(void) close(sockfd);
				return (-1);
			}
			break;
		} /*- end of FD_ISSET(sockfd, &rfds) -*/
	}
	switch(type)
	{
		case 1:
			filewrt(sockfd, "a1 login %s %s\n", user, passwd);
			break;
		case 2:
			filewrt(sockfd, "USER %s\nPASS %s\n", user, passwd);
			break;
	}
	if (ioctl(0, FIONBIO, &retval) != -1)
	{
		retval = 0;
		if (read(0, "", 0) == -1) ;
		if (ioctl(0, FIONBIO, &retval) == -1)
		{
			fprintf(stderr, "monkey-ioctl-FIONBIO: %s\n", strerror(errno));
			(void) close(sockfd);
			return (1);
		}
	} else
	{
		(void) close(sockfd);
		return (1);
	}
	Timeout.tv_sec = timeout.tv_sec = 5;
	Timeout.tv_usec = timeout.tv_usec = 0;
	for (retrycount = 0;;)
	{
		FD_ZERO(&rfds);
		if(keyboard)
			FD_SET(0, &rfds);
		FD_SET(sockfd, &rfds);
		/*- Is data available from client or server -*/
		if ((retval = select(sockfd + 1, &rfds, (fd_set *) NULL, (fd_set *) NULL, &Timeout)) < 0)
		{
#ifdef ERESTART
			if (errno == EINTR || errno == ERESTART)
#else
			if (errno == EINTR)
#endif
				continue;
			fprintf(stderr, "select: %s\n", strerror(errno));
			(void) close(sockfd);
			return (-1);
		} else
		if (!retval)
		{
			idle_time += timeout.tv_sec;
			if(idle_time > dataTimeout)
			{
				close(sockfd);
				fprintf(stderr, "select: Timeout %ld\n", timeout.tv_sec);
				return (2);
			}
			timeout.tv_sec += 2 * timeout.tv_sec;
			Timeout.tv_sec = timeout.tv_sec;
			continue;
		} else
		{
			timeout.tv_sec = 5;
			idle_time = 0;
		}
		readData = 0;
		if (FD_ISSET(sockfd, &rfds))
		{
			/*- Data from Remote Server -*/
			for (;;)
			{
				errno = 0;
				if ((retval = read(sockfd, buffer, SOCKBUF)) < 0)
				{
#ifdef ERESTART
					if (errno == EINTR || errno == ERESTART)
#else
					if (errno == EINTR)
#endif
						continue;
					else
					if (errno == ENOBUFS && retrycount++ < MAXNOBUFRETRY)
					{
						(void) usleep(1000);
						continue;
					}
#ifdef HPUX_SOURCE
					else
					if (errno == EREMOTERELEASE)
					{
						retval = 0;
						break;
					}
#endif
					fprintf(stderr, "read-dst: %s\n", strerror(errno));
					(void) close(sockfd);
					return (-1);
				}
				break;
			}
			if (!retval)	/*- EOF from server -*/
				break;
			readData = 1;
			/*- Write data to Client -*/
			if (sockwrite(1, buffer, retval) != retval)
			{
				fprintf(stderr, "sockwrite-src: %s\n", strerror(errno));
				(void) close(sockfd);
				return (-1);
			}
		} /*- end of FD_ISSET(sockfd, &rfds) -*/
		if(readData && !flag++)
		{
			if(*command)
 				filewrt(sockfd, "%s\n", command);
			if(!keyboard)
			{
				switch(type)
				{
					case 1:
 						filewrt(sockfd, "a1 logout\n");
						break;
					case 2:
 						filewrt(sockfd, "quit\n");
						break;
				}
			}
		}
		if(!keyboard)
			continue;
		if (FD_ISSET(0, &rfds))
		{
			/*- Data from Client -*/
			for (;;)
			{
				errno = 0;
				if ((retval = read(0, buffer, SOCKBUF)) == -1)
				{
#ifdef ERESTART
					if (errno == EINTR || errno == ERESTART)
#else
					if (errno == EINTR)
#endif
						continue;
					else
					if (errno == ENOBUFS && retrycount++ < MAXNOBUFRETRY)
					{
						(void) usleep(1000);
						continue;
					}
#ifdef HPUX_SOURCE
					else
					if (errno == EREMOTERELEASE)
					{
						retval = 0;
						break;
					}
#endif
					fprintf(stderr, "read-src: %s\n", strerror(errno));
					(void) close(sockfd);
					return (-1);
				}
				break;
			}
			if (!retval)	/*- EOF from client -*/
				break;
			/*- Write to Remote Server -*/
			if (sockwrite(sockfd, buffer, retval) != retval)
			{
				fprintf(stderr, "sockwrite-dst: %s\n", strerror(errno));
				(void) close(sockfd);
				return (-1);
			}
		}	  /*- if (FD_ISSET(0, &rfds)) -*/
	}	/*- end of for -*/
	/*- printf("%s %s %s %d %s\n", user, passwd, host, port, command); -*/
	return (0);
}

static int
get_options(int argc, char **argv, char **user, char **passwd, char **host, 
	int *Port, int *type, int *keyb, char **Timeout, char **command)
{
	int             c;
	char           *ptr;

	*keyb = 0;
	getEnvConfigStr(&ptr, "DATA_TIMEOUT", "60");
	*Timeout = ptr;
	while ((c = getopt(argc, argv, "vkt:u:p:h:P:T:")) != -1)
	{
		switch (c)
		{
		case 'v':
			verbose = 1;
			break;
		case 'k':
			*keyb = 1;
			break;
		case 't':
			if(!strncmp(optarg, "imap", 5))
				*type = 1;
			else
			if(!strncmp(optarg, "pop3", 5))
				*type = 2;
			else
			{
				usage();
				return(1);
			}
			break;
		case 'u':
			*user = optarg;
			break;
		case 'p':
			*passwd = optarg;
			break;
		case 'h':
			*host = optarg;
			break;
		case 'P':
			*Port = atoi(optarg);
			break;
		case 'T':
			*Timeout = optarg;
			break;
		default:
			usage();
			return (1);
		}
	}
	if (optind < argc)
		*command = argv[optind++];
	else
	{
		usage();
		return (1);
	}
	return (0);
}

static void
usage()
{
	fprintf(stderr, "USAGE: testmra -t type -h host -u user -p passwd -P port command\n");
	fprintf(stderr, "       where type is imap or pop3\n");
}

void
getversion_testmra_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
