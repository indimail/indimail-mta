/*
 * $Log: initsvc.c,v $
 * Revision 2.14  2011-07-04 17:42:39+05:30  Cprogrammer
 * fix for virtual machines where /dev/console gives problem
 *
 * Revision 2.13  2011-05-26 23:54:18+05:30  Cprogrammer
 * change in svscanboot usage
 *
 * Revision 2.12  2010-06-05 14:33:04+05:30  Cprogrammer
 * port for fedora 13
 *
 * Revision 2.11  2010-05-18 08:24:43+05:30  Cprogrammer
 * fixed -list option for Mac OS X
 *
 * Revision 2.10  2009-11-17 20:14:50+05:30  Cprogrammer
 * run initsvc only for root
 *
 * Revision 2.9  2009-06-18 16:14:48+05:30  Cprogrammer
 * use launchctl on Mac OS
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
#include <fcntl.h>
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: initsvc.c,v 2.14 2011-07-04 17:42:39+05:30 Cprogrammer Exp mbhangui $";
#endif

#define SV_ON    1
#define SV_OFF   2
#define SV_STAT  3
#define SV_PRINT 4
int
main(int argc, char **argv)
{
	FILE           *fp;
	char           *device = "/dev/console";
	char           *qmaildir, *ptr, *jobfile = 0, *print_cmd = 0, *jobdir;
	char            buffer[2048];
	int             flag, found, colonCount, fd;
	long            pos;
	static char    *initargs[3] = { "/sbin/init", "q", 0 };

	if (argc != 2)
	{
		fprintf(stderr, "usage: initsvc -on|-off|-status|-print\n");
		return (1);
	} else
	if (getuid())
	{
		fprintf(stderr, "initsvc: this program must be run as root\n");
		return (1);
	}
	if (!strncmp(argv[1], "-on", 4))
		flag = SV_ON;
	else
	if (!strncmp(argv[1], "-off", 5))
		flag = SV_OFF;
	else
	if (!strncmp(argv[1], "-status", 8))
		flag = SV_STAT;
	else
	if (!strncmp(argv[1], "-print", 7))
		flag = SV_PRINT;
	else
	{
		fprintf(stderr, "usage: initsvc -on|-off|-status|-print\n");
		return (1);
	}
	getEnvConfigStr(&qmaildir, "QMAILDIR", QMAILDIR);
	if (!access("/etc/init", F_OK)) /*- Upstart */
	{
		jobdir = "/etc/init";
		jobfile = "/etc/init/svscan.conf";
		print_cmd = "/bin/cat /etc/init/svscan.conf;/bin/ls -l /etc/init/svscan.conf";
	} else
	if (!access("/etc/event.d", F_OK)) /*- Upstart */
	{
		jobdir = "/etc/event.d";
		jobfile = "/etc/event.d/svscan";
		print_cmd = "/bin/cat /etc/event.d/svscan;/bin/ls -l /etc/event.d/svscan";
	} else
		jobdir = (char *) 0;
	if (jobdir)
	{
		/* Install upstart */
		if (access(jobfile, F_OK))
		{
			if (flag == SV_OFF)
				return (0);
			printf("Installing upstart service\n");
			if (chdir(QMAILDIR) || chdir("boot"))
			{
				fprintf(stderr, "chdir %s/boot: %s\n", QMAILDIR, strerror(errno));
				return(1);
			} else
			if (fappend("upstart", jobfile, "w", 0644, 0, getgid()))
			{
				fprintf(stderr, "fappend %s %s: %s\n", "upstart", jobfile, strerror(errno));
				return(1);
			}
		} 
		switch(flag)
		{
			case SV_ON:
				execl("/sbin/initctl", "initctl", "start", "svscan", (char *) 0);
				perror("/sbin/initctl");
				break;
			case SV_OFF:
				execl("/sbin/initctl", "initctl", "stop", "svscan", (char *) 0);
				perror("/sbin/initctl");
				break;
			case SV_STAT:
				execl("/sbin/initctl", "initctl",  "status", "svscan", (char *) 0);
				perror("/sbin/initctl");
				break;
			case SV_PRINT:
				if (print_cmd)
					execl("/bin/sh", "sh", "-c",  print_cmd, (char *) 0);
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
			if (flag == SV_OFF)
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
			case SV_ON:
				execl("/bin/launchctl", "launchctl", "load", "/System/Library/LaunchDaemons/indimail.plist", (char *) 0);
				perror("/bin/launchctl");
				break;
			case SV_OFF:
				execl("/bin/launchctl", "launchctl", "unload", "/System/Library/LaunchDaemons/indimail.plist", (char *) 0);
				perror("/bin/launchctl");
				break;
			case SV_STAT:
				execl("/bin/sh", "sh", "-c",
				"/bin/launchctl list indimail || /bin/cat /System/Library/LaunchDaemons/indimail.plist;\
				/bin/ls -l /System/Library/LaunchDaemons/indimail.plist", (char *) 0);
				perror("/bin/launchctl");
				break;
			case SV_PRINT:
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
	 * SV:345:respawn:/var/qmail/bin/svscanboot /service <>/dev/console 2<>/dev/console
	 */
	for (found = 0;;)
	{
		if (!fgets(buffer, sizeof(buffer) - 2, fp))
			break;
		if ((ptr = strstr(buffer, "SV:345:")) && strstr(buffer, "svscanboot"))
		{
			if(flag == SV_STAT)
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
			if(flag == SV_PRINT)
			{
				printf("%s", buffer);
				fclose(fp);
				return(0);
			}
			found = 1;
			break;
		}
	}
	if(flag == SV_STAT)
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
	if ((fd = open("/dev/console", O_RDWR)) == -1)
	{
		if (errno == EPERM) /*- some virtual servers have this problem */
			device = "/dev/null";
	} else
		close(fd);
	/*
	 * SV:345:respawn:/var/qmail/bin/svscanboot <>/dev/console 2<>/dev/console
	 */
	snprintf(buffer, sizeof(buffer), "SV:345:%s:%s/bin/svscanboot <>%s 2<>%s",
			 flag == SV_ON ? "respawn" : "off", qmaildir, device, device);
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
