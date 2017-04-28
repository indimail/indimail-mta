/*
 * $Log: proxypop3.c,v $
 * Revision 2.8  2017-03-13 14:06:44+05:30  Cprogrammer
 * replaced INDIMAILDIR with PREFIX
 *
 * Revision 2.7  2010-03-06 15:47:33+05:30  Cprogrammer
 * use COURIERTLS env variable to execute TLS enabler program
 *
 * Revision 2.6  2010-03-06 14:56:00+05:30  Cprogrammer
 * added STARTTLS capability
 *
 * Revision 2.5  2003-02-08 21:21:31+05:30  Cprogrammer
 * corrected ambiguous return value
 *
 * Revision 2.4  2002-12-06 01:55:50+05:30  Cprogrammer
 * bug fix, prevent SIGSEGV if BADLOGINS is not defined
 *
 * Revision 2.3  2002-11-28 00:45:04+05:30  Cprogrammer
 * compilation correction for clustered domain
 *
 * Revision 2.2  2002-06-25 02:24:52+05:30  Cprogrammer
 * set timeout for inputs
 *
 * Revision 2.1  2002-04-24 20:40:20+05:30  Cprogrammer
 * implemented CAPA
 *
 * Revision 1.1  2002-04-08 04:35:47+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <signal.h>
#include <ctype.h>
#include <unistd.h>
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: proxypop3.c,v 2.8 2017-03-13 14:06:44+05:30 Cprogrammer Stab mbhangui $";
#endif

#ifdef CLUSTERED_SITE

static void     bye();

int
main(int argc, char **argv)
{
	char           *p, *remoteip, *local_port, *destport, *user = 0;
	char            badlogins[MAX_BUFF], buf[BUFSIZ];
	int             c, len;
	char           *binqqargs[7];

	if ((p = strrchr(argv[0], '/')))
		p++;
	else
		p = argv[0];
	if (argc != 3)
	{
		fprintf(stderr, "USAGE: %s pop3d Maildir\n", p);
		return(1);
	}
	if (!(remoteip = getenv("TCPREMOTEIP")))
	{
		fprintf(stderr, "ERR: TCPREMOTEIP not set\n");
		fflush(stderr);
		_exit(1);
	} else
	if (!(local_port = getenv("TCPLOCALPORT")))
	{
		fprintf(stderr, "ERR: TCPLOCALPORT not set\n");
		fflush(stderr);
		exit(1);
	}
	if (!(destport = getenv("DESTPORT")))
	{
		fprintf(stderr, "ERR: DESTPORT not set\n");
		fflush(stderr);
		_exit(1);
	} 
	signal(SIGALRM, bye);
	if (AuthModuser(argc, argv, 60, 5))
	{
		if (!getenv("SSLERATOR"))
		{
			fprintf(stderr, "INFO: Connection, remoteip=[%s]\n", remoteip);
			printf("+OK POP3 Server Ready.\r\n");
		}
	} else
	{
		if (!(p = (char *) getenv("BADLOGINS")))
		{
			snprintf(badlogins, MAX_BUFF, "BADLOGINS=1");
			p = "1";
		}
		else
			snprintf(badlogins, MAX_BUFF, "BADLOGINS=%d", atoi(p) + 1);
		putenv(badlogins);
		if (atoi(p) > 3)
			_exit(1);
		fprintf(stderr, "ERR: LOGIN FAILED, remoteip=[%s]\n", remoteip);
		printf("-ERR Login failed.\r\n");
	}
	fflush(stdout);
	alarm(0);
	for (;;)
	{
		alarm(60);
		if (!fgets(buf, sizeof(buf), stdin))
			return(1);
		alarm(0);
		c = 1;
		for (p = buf; *p; p++)
		{
			if (*p == '\n')
				break;

			if (*p == ' ' || *p == '\t')
				c = 0;
			if (c)
				*p = toupper((int) (unsigned char) *p);
		}
		if (*p)
			*p = 0;
		else
			while ((c = getchar()) != EOF && c != '\n') ;
		p = strtok(buf, " \t\r");
		if (p)
		{
			if (!strcmp(p, "QUIT"))
			{
				printf("+OK Phir Khab Miloge.\r\n");
				fflush(stdout);
				break;
			} else
			if (!strcmp(p, "NOOP") || !strcmp(p, "RSET") || !strcmp(p, "STAT") || !strcmp(p, "LIST")
				|| !strcmp(p, "RETR") || !strcmp(p, "DELE") || !strcmp(p, "TOP") || !strcmp(p, "UIDL"))
			{
				printf("-ERR command valid only in transaction state.\r\n");
				fflush(stdout);
				continue;
			} else
			if (!strcmp(p, "CAPA"))
			{
				pop3d_capability();
				fflush(stdout);
				continue;
			} else
			if (!strcmp(p, "STLS"))
			{
				char           *ptr;

				putenv("AUTHARGC=0");
				for (c = 0; c < argc; c++)
				{
					char namebuf[56];
					sprintf(namebuf, "AUTHARGV%d=", c);
					unsetenv(namebuf);
				}
				unsetenv("BADLOGINS");
				unsetenv("POP3_STARTTLS");
				alarm(0);
				p = getenv("COURIERTLS");
				if (!(ptr = strrchr(p, '/')))
					ptr = p;
				else
					ptr++;
				if (!p || !strcmp(ptr, "sslerator"))
				{
					binqqargs[0] = PREFIX"/bin/sslerator";
					binqqargs[1] = argv[0];
					binqqargs[2] = argv[1]; 
					binqqargs[3] = argv[2];
					binqqargs[4] = 0;
					putenv("BANNER=+OK Begin SSL/TLS negotiation now.\r\n");
				} else
				{
					if (!(binqqargs[0] = strdup(p)))
					{
						perror("malloc");
						_exit(1);
					}
					unsetenv("COURIERTLS");
					binqqargs[1] = "-remotefd=0";
					binqqargs[2] = "-server";
					binqqargs[3] = argv[0];
					binqqargs[4] = argv[1]; 
					binqqargs[5] = argv[2];
					binqqargs[6] = 0;
					putenv("SSLERATOR=1");
					printf("+OK Begin SSL/TLS negotiation now.\r\n");
					fflush(stdout);
				}
				execv(*binqqargs, binqqargs);
				fprintf(stderr, "execv: %s: %s\n", *binqqargs, strerror(errno));
				continue;
			} else
			if (!strcmp(p, "USER"))
			{
				p = strtok(0, " \t\r");
				if (p)
				{
					if (user)
						free(user);
					if (!(user = malloc(len = (strlen(p) + 1))))
					{
						perror("malloc");
						_exit(1);
					}
					scopy(user, p, len);
					printf("+OK Password required.\r\n");
					fflush(stdout);
					continue;
				}
			} else
			if (!strcmp(p, "PASS"))
			{
				p = strtok(0, "\t\r");
				if (!user || p == 0)
				{
					printf("-ERR USER/PASS required.\r\n");
					fflush(stdout);
					continue;
				}
			} else
			{
				printf("-ERR Invalid command.\r\n");
				fflush(stdout);
				continue;
			} /*- proxylogin should normally never return */
			return(proxylogin(argv, destport, user, p, remoteip, 0, 2));
		}
		printf("-ERR Invalid command.\r\n");
		fflush(stdout);
	} /* while (alarm(300), fgets(buf, sizeof(buf), stdin)) */
	return (0);
}

static void bye()
{
	fprintf(stderr, "ERR: TIMEOUT\n");
	exit(0);
}
#else
int
main()
{
	printf("IndiMail not configured with --enable-user-cluster=y\n");
	return(0);
}
#endif

void
getversion_proxypop3_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
	return;
}
