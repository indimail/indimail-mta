/*
 * $Log: initsvc.c,v $
 * Revision 2.9  2009-06-18 16:14:48+05:30  Cprogrammer
 * use launchtl on Mac OS
 *
 * Revision 2.8  2009-02-03 20:17:40+05:30  Cprogrammer
 * fix erroneously error being returned during first time install of svscan service on systems
 * having upstart
 *
 * Revision 2.7  2009-01-25 12:16:26+05:30  Cprogrammer
 * do not install upstart service for svscan for -off argument
 *
 * Revision 2.6  2008-09-12 20:09:13+05:30  Cprogrammer
 * display job control file
 *
 * Revision 2.5  2008-07-27 15:42:16+05:30  Cprogrammer
 * enable upstart by copying /var/indimail/bin/upstart
 *
 * Revision 2.4  2008-07-18 21:49:29+05:30  Cprogrammer
 * changes for upstart init
 *
 * Revision 2.3  2005-12-29 22:45:08+05:30  Cprogrammer
 * use getEnvConfigStr to set variables from environment variables
 *
 * Revision 2.2  2005-05-30 10:40:05+05:30  Cprogrammer
 * added error conditions
 *
 * Revision 2.1  2003-11-20 23:31:31+05:30  Cprogrammer
 * inittab utility
 *
 */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: initsvc.c,v 2.9 2009-06-18 16:14:48+05:30 Cprogrammer Stab mbhangui $";
#endif

int
main(int argc, char **argv)
{
	FILE           *fp;
	char           *qmaildir, *ptr;
	char            buffer[2048];
	int             flag, found, colonCount;
	long            pos;
	static char    *initargs[3] = { "/sbin/init", "q", 0 };

	if (argc != 2)
	{
		fprintf(stderr, "usage: initsvc -on|-off|-status|-print\n");
		return (1);
	}
	if (!strncmp(argv[1], "-on", 4))
		flag = 1;
	else
	if (!strncmp(argv[1], "-off", 5))
		flag = 2;
	else
	if (!strncmp(argv[1], "-status", 8))
		flag = 3;
	else
	if (!strncmp(argv[1], "-print", 7))
		flag = 4;
	else
	{
		fprintf(stderr, "usage: initsvc -on|-off|-status|-print\n");
		return (1);
	}
	getEnvConfigStr(&qmaildir, "QMAILDIR", QMAILDIR);
	if (!access("/etc/event.d", F_OK)) /*- Upstart */
	{
		/* Install upstart */
		if (access("/etc/event.d/svscan", F_OK))
		{
			if (flag == 2)
				return (0);
			printf("Installing upstart service\n");
			if (chdir(QMAILDIR) || chdir("boot"))
			{
				fprintf(stderr, "chdir %s/boot: %s\n", QMAILDIR, strerror(errno));
				return(1);
			} else
			if (fappend("upstart", "/etc/event.d/svscan", "w", 0644, 0, getgid()))
			{
				fprintf(stderr, "fappend %s %s: %s\n", "upstart", "/etc/event.d/svscan", strerror(errno));
				return(1);
			}
		} 
		switch(flag)
		{
			case 1:
				execl("/sbin/initctl", "initctl", "start", "svscan", (char *) 0);
				perror("/sbin/initctl");
				break;
			case 2:
				execl("/sbin/initctl", "initctl", "stop", "svscan", (char *) 0);
				perror("/sbin/initctl");
				break;
			case 3:
				execl("/sbin/initctl", "initctl",  "list", "svscan", (char *) 0);
				perror("/sbin/initctl");
				break;
			case 4:
				execl("/bin/sh", "sh", "-c",  "/bin/cat /etc/event.d/svscan;/bin/ls -l /etc/event.d/svscan", (char *) 0);
				perror("/bin/ls");
				break;
		}
		return(1);
	} 
#ifdef DARWIN
	else
	if (!access("/bin/launchctl", X_OK))
	{
		/* Install indimail.plist */
		if (access("/System/Library/LaunchDaemons/indimail.plist", F_OK))
		{
			if (flag == 2)
				return (0);
			printf("Installing indimail.plist\n");
			if (chdir(QMAILDIR) || chdir("boot"))
			{
				fprintf(stderr, "chdir %s/boot: %s\n", QMAILDIR, strerror(errno));
				return(1);
			} else
			if (fappend("indimail.plist", "/System/Library/LaunchDaemons/indimail.plist", "w", 0644, 0, getgid()))
			{
				fprintf(stderr, "fappend %s %s: %s\n", "upstart", "/etc/event.d/svscan", strerror(errno));
				return(1);
			}
		} 
		switch(flag)
		{
			case 1:
				execl("/bin/launchctl", "launchctl", "load", "/System/Library/LaunchDaemons/indimail.plist", (char *) 0);
				perror("/bin/launchctl");
				break;
			case 2:
				execl("/bin/launchctl", "launchctl", "unload", "/System/Library/LaunchDaemons/indimail.plist", (char *) 0);
				perror("/bin/launchctl");
				break;
			case 3:
				execl("/bin/launchctl", "launchctl",  "list", "indimail", (char *) 0);
				perror("/bin/launchctl");
				break;
			case 4:
				execl("/bin/sh", "sh", "-c",
				"/bin/cat /System/Library/LaunchDaemons/indimail.plist;/bin/ls -l /System/Library/LaunchDaemons/indimail.plist",
				(char *) 0);
				perror("/bin/ls");
				break;
		}
		return(1);
	} /*- if (!access("/bin/launchctl", X_OK)) */
#endif
	if (!(fp = fopen("/etc/inittab", "r")))
	{
		perror("/etc/inittab");
		return (1);
	}
	/*
	 * SV:345:respawn:/var/qmail/bin/svscanboot /service /service1 <>/dev/console 2<>/dev/console
	 */
	for (found = 0;;)
	{
		if (!fgets(buffer, sizeof(buffer) - 2, fp))
			break;
		if ((ptr = strstr(buffer, "SV:345:")) && strstr(buffer, "svscanboot"))
		{
			if(flag == 3)
			{
				fclose(fp);
				for(colonCount = 0;*ptr;ptr++)
				{
					if(*ptr == ':')
						colonCount++;
					if(colonCount == 2)
					{
						if(!memcmp(ptr + 1, "respawn", 7))
							return(0);
						else
						if(!memcmp(ptr + 1, "off", 3))
							return(1);
						else
							return(2);
					}
				}
				return(2);
			} else
			if(flag == 4)
			{
				printf("%s", buffer);
				fclose(fp);
				return(0);
			}
			found = 1;
			break;
		}
	}
	if(flag == 3)
		return(2);
	if (!(fp = freopen("/etc/inittab", found ? "r+" : "a", fp)))
	{
		perror("/etc/inittab");
		return (1);
	}
	for (;;)
	{
		if((pos = ftell(fp)) == -1)
		{
			perror("ftell");
			fclose(fp);
			return(1);
		}
		if (!fgets(buffer, sizeof(buffer) - 2, fp))
			break;
		if (strstr(buffer, "SV:345:") && strstr(buffer, "svscanboot"))
			break;
	}
	if (fseek(fp, pos, SEEK_SET) == -1)
	{
		perror("fseek");
		fclose(fp);
		return (1);
	}
	/*
	 * SV:345:respawn:/var/qmail/bin/svscanboot /service /service1 <>/dev/console 2<>/dev/console
	 */
	snprintf(buffer, sizeof(buffer), "SV:345:%s:%s/bin/svscanboot%s/service /service1 <>/dev/console 2<>/dev/console",
			 flag == 1 ? "respawn" : "off", qmaildir, flag == 1 ? " " : "     ");
	fprintf(fp, "%s\n", buffer);
	if (fclose(fp))
	{
		perror("fclose");
		return (1);
	}
	execv(*initargs, initargs);
	fprintf(stderr, "execv: %s: %s\n", *initargs, strerror(errno));
	return(1);
}

void
getversion_creinit_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
