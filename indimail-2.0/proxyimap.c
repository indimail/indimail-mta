/*
 * $Log: proxyimap.c,v $
 * Revision 2.8  2010-03-06 15:46:52+05:30  Cprogrammer
 * use COURIERTLS env variable to execute TLS enabler program
 *
 * Revision 2.7  2010-03-06 14:55:33+05:30  Cprogrammer
 * added STLS capability
 *
 * Revision 2.6  2003-02-08 21:21:15+05:30  Cprogrammer
 * corrected ambiguous return value
 *
 * Revision 2.5  2002-12-06 01:54:45+05:30  Cprogrammer
 * bug fix, prevent SIGSEGV if BADLOGINS is not defined
 *
 * Revision 2.4  2002-11-28 00:44:51+05:30  Cprogrammer
 * compilation correction for clustered domain
 *
 * Revision 2.3  2002-07-05 00:37:52+05:30  Cprogrammer
 * initialization of variables
 *
 * Revision 2.2  2002-06-25 02:24:40+05:30  Cprogrammer
 * set timeout for inputs
 *
 * Revision 2.1  2002-04-24 15:10:35+05:30  Cprogrammer
 * added NOOP and CAPABILITY
 *
 * Revision 1.1  2002-04-08 04:35:43+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: proxyimap.c,v 2.8 2010-03-06 15:46:52+05:30 Cprogrammer Stab mbhangui $";
#endif

#ifdef CLUSTERED_SITE

static void     bye();

int
main(int argc, char **argv)
{
	char            buf[BUFSIZ], username[MAX_BUFF], password[MAX_BUFF], imaptag[MAX_BUFF], dummy2[MAX_BUFF];
	char            badlogins[MAX_BUFF];
	int             c, ret;
	char           *ptr, *remoteip, *local_port, *destport;
	const char     *tag = getenv("IMAPLOGINTAG");
	char           *binqqargs[7];

	if ((ptr = strrchr(argv[0], '/')))
		ptr++;
	else
		ptr = argv[0];
	if (argc != 3)
	{
		fprintf(stderr, "USAGE: %s imapd Maildir\n", ptr);
		return(1);
	}
	remoteip = local_port = destport = (char *) 0;
	if (!(remoteip = getenv("TCPREMOTEIP")))
	{
		fprintf(stderr, "ERR: TCPREMOTEIP not set\n");
		fflush(stderr);
		_exit(1);
	} else
	if (!(local_port = getenv("TCPLOCALPORT")))
	{
		fprintf(stderr, "ERR: TCPLOCALPORT not set\n");
		_exit(1);
	} else
	if (!(destport = getenv("DESTPORT")))
	{
		fprintf(stderr, "ERR: DESTPORT not set\n");
		_exit(1);
	} 
	signal(SIGALRM, bye);
	if (AuthModuser(argc, argv, 60, 5))
	{
		printf("* OK IMAP4rev1 Server Ready.\r\n");
		fprintf(stderr, "INFO: Connection, remoteip=[%s]\n", remoteip);
		putenv("BADLOGINS=0");
	} else
	{
		if (!(ptr = (char *) getenv("BADLOGINS")))
		{
			snprintf(badlogins, MAX_BUFF, "BADLOGINS=1");
			ptr = "1";
		}
		else
			snprintf(badlogins, MAX_BUFF, "BADLOGINS=%d", atoi(ptr) + 1);
		putenv(badlogins);
		if (atoi(ptr) > 3)
			_exit(1);
		printf("%s NO Login failed.\r\n", tag ? tag : "");
		fprintf(stderr, "ERR: LOGIN FAILED, remoteip=[%s] badlogin=[%d]\n", remoteip, atoi(ptr) + 1);
	}
	fflush(stdout);
	alarm(0);
	for (;;)
	{
		alarm(60);
		if (!fgets(buf, sizeof(buf), stdin))
			return(1);
		alarm(0);
		ret = sscanf(buf, "%s %s %s %s", imaptag, dummy2, username, password);
		if (ret >= 2)
		{
			for (ptr = dummy2;*ptr;ptr++)
			{
				if (islower((int) *ptr))
					*ptr = toupper((int) *ptr);
			}
			if (!strcmp(dummy2, "LOGIN")) /*- proxylogin should normally never return */
				return(proxylogin(argv, destport, username, password, remoteip, imaptag, 1));
			else
			if (!strcmp(dummy2, "NOOP"))
			{
				printf("%s OK NOOP completed\r\n", imaptag);
				fflush(stdout);
				continue;
			} else
			if (!strcmp(dummy2, "STARTTLS"))
			{
				static char     namebuf[56];
				char           *p;

				putenv("AUTHARGC=0");
				for (c = 0; c < argc; c++)
				{
					sprintf(namebuf, "AUTHARGV%d=", c);
					unsetenv(namebuf);
				}
				unsetenv("BADLOGINS");
				unsetenv("IMAP_STARTTLS");
				alarm(0);
				ptr = getenv("COURIERTLS");
				if (!(p = strrchr(ptr, '/')))
					p = ptr;
				else
					p++;
				if (!ptr || !strcmp(p, "sslerator"))
				{
					binqqargs[0] = INDIMAILDIR"/bin/sslerator";
					binqqargs[1] = argv[0];
					binqqargs[2] = argv[1]; 
					binqqargs[3] = argv[2];
					binqqargs[4] = 0;
					snprintf(namebuf, sizeof(namebuf), "BANNER=%s Begin SSL/TLS negotiation now.\r\n", imaptag);
					putenv(namebuf);
				} else
				{
					if (!(binqqargs[0] = strdup(ptr)))
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
					printf("%s Begin SSL/TLS negotiation now.\r\n", imaptag);
					fflush(stdout);
				}
				execv(*binqqargs, binqqargs);
				fprintf(stderr, "execv: %s: %s\n", *binqqargs, strerror(errno));
				continue;
			} else
			if (!strcmp(dummy2, "CAPABILITY"))
			{
				imapd_capability();
				printf("%s OK CAPABILITY completed\r\n", imaptag);
				fflush(stdout);
				continue;
			} else
			if (!strcmp(dummy2, "LOGOUT"))
			{
				printf("* BYE IMAP4rev1 server shutting down\r\n");
				printf(" OK LOGOUT completed\r\n");
				fflush(stdout);
				break;
			}
		}
		printf("* NO Error in IMAP command received by server.\n");
		fflush(stdout);
	}
	return(0);
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
getversion_proxyimap_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
	return;
}
