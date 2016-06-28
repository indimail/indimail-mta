/*
 * $Log: adminclient.c,v $
 * Revision 2.6  2009-08-11 16:22:21+05:30  Cprogrammer
 * added certfile argument for tls encryption
 *
 * Revision 2.5  2003-04-12 00:22:34+05:30  Cprogrammer
 * corrected bug with assignment of commands
 * replaced admin_command with structure
 *
 * Revision 2.4  2003-02-03 21:53:29+05:30  Cprogrammer
 * added option input_read to allow data on stdin to be passed to indisrvr
 *
 * Revision 2.3  2002-11-28 00:43:04+05:30  Cprogrammer
 * moved variable 'command' as admin_command to variables.c
 *
 * Revision 2.2  2002-10-12 22:54:16+05:30  Cprogrammer
 * adminclient need not run only in clustered environment
 *
 * Revision 2.1  2002-07-25 12:26:06+05:30  Cprogrammer
 * program for issuing adming commands to indisrvr
 *
 */
#include "indimail.h"

#ifndef lint
static char     sccsid[] = "$Id: adminclient.c,v 2.6 2009-08-11 16:22:21+05:30 Cprogrammer Stab mbhangui $";
#endif

#ifdef CLUSTERED_SITE
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

int             auth_admin(char *, char *, char *, char *, char *);
int             adminCmmd(int, int, char *, int);

int
main(int argc, char **argv)
{
	char           *ptr, *admin_user, *admin_pass, *admin_host, *admin_port, *cmmd,
				   *cmdptr1, *cmdptr2, *certfile;
	char            cmdbuf[MAX_BUFF], cmdName[MAX_BUFF];
	int             sfd, idx, input_read;

	certfile = admin_user = admin_pass = admin_host = admin_port = cmmd = (char *) 0;
	input_read = 0;
	if((ptr = strrchr(argv[0], '/')))
		ptr++;
	else
		ptr = argv[0];
	for (idx = 1; idx < argc; idx++)
	{
		if (argv[idx][0] != '-')
			continue;
		switch (argv[idx][1])
		{
		case 'h':
			admin_host = *(argv + idx + 1);
			break;
		case 'p':
			admin_port = *(argv + idx + 1);
			break;
		case 'u':
			admin_user = *(argv + idx + 1);
			break;
		case 'P':
			admin_pass = *(argv + idx + 1);
			break;
		case 'c':
			cmmd = *(argv + idx + 1);
			break;
		case 'i':
			input_read = 1;
			break;
		case 'n':
			certfile = *(argv + idx + 1);
			break;
		case 'v':
			verbose = 1;
			break;
		default:
			fprintf(stderr, "USAGE: %s [-h adminHost -p adminPort -u adminUser -P adminPasswd] [-n certfile] [-i] -c cmmd [-v]\n", ptr);
			fprintf(stderr, "invalid option %c\n", argv[idx][1]);
			return(1);
		}
	}
	if(!admin_host && !(admin_host = (char *) getenv("ADMIN_HOST")))
	{
		fprintf(stderr, "USAGE: %s [-h adminHost -p adminPort -u adminUser -P adminPasswd] [-n certfile] [-i] -c cmmd [-v]\n", ptr);
		return(1);
	} else
	if(!admin_port && !(admin_port = (char *) getenv("ADMIN_PORT")))
	{
		fprintf(stderr, "USAGE: %s [-h adminHost -p adminPort -u adminUser -P adminPasswd] [-n certfile] [-i] -c cmmd [-v]\n", ptr);
		return(1);
	} else
	if(!admin_user && !(admin_user = (char *) getenv("ADMIN_USER")))
	{
		fprintf(stderr, "USAGE: %s [-h adminHost -p adminPort -u adminUser -P adminPasswd] [-n certfile] [-i] -c cmmd [-v]\n", ptr);
		return(1);
	} else
	if(!admin_pass && !(admin_pass = (char *) getenv("ADMIN_PASS")))
	{
		fprintf(stderr, "USAGE: %s [-h adminHost -p adminPort -u adminUser -P adminPasswd] [-n certfile] [-i] -c cmmd [-v]\n", ptr);
		return(1);
	} else
	if(!cmmd)
	{
		fprintf(stderr, "USAGE: %s [-h adminHost -p adminPort -u adminUser -P adminPasswd] [-n certfile] [-i] -c cmmd [-v]\n", ptr);
		return(1);
	}
	if(verbose)
		printf("connecting to %s:%s as %s\n", admin_host, admin_port, admin_user);
	if ((sfd = auth_admin(admin_user, admin_pass, admin_host, admin_port, certfile)) == -1)
	{
		perror("auth_admin");
		return (1);
	}
	if(verbose)
		printf("connected\n");
	for(cmdptr1 = cmmd;*cmdptr1 && isspace((int) *cmdptr1);cmdptr1++);
	for(cmdptr2 = cmdName;*cmdptr1 && !isspace((int) *cmdptr1);*cmdptr2++ = *cmdptr1++);
	*cmdptr2 = 0;
	if((cmdptr1 = strrchr(cmdName, '/')))
		cmdptr1++;
	else
		cmdptr1 = cmdName;
	for (idx = 0; adminCommands[idx].name; idx++)
	{
		if((cmdptr2 = strrchr(adminCommands[idx].name, '/')))
			cmdptr2++;
		else
			cmdptr2 = adminCommands[idx].name;
		if(!strncmp(cmdptr1, cmdptr2, slen(cmdptr2) + 1))
		{
			snprintf(cmdbuf, sizeof(cmdbuf), "%d %s", idx, cmmd);
			if(verbose)
				printf("executing command no %d [%s]\n", idx, cmmd);
			return (adminCmmd(sfd, input_read, cmdbuf, strlen(cmdbuf)));
		}
	}
	fprintf(stderr, "Invalid or unauthorized command %s\n", cmmd);
	return(1);
}
#else
int
main()
{
	fprintf(stderr, "IndiMail not configured with --enable-user-cluster=y\n");
	return(1);
}
#endif

void
getversion_adminclient_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
