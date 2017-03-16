/*
 * $Log: vdominfo.c,v $
 * Revision 2.22  2017-03-13 14:12:59+05:30  Cprogrammer
 * replaced qmaildir with sysconfdir
 *
 * Revision 2.21  2016-06-09 15:32:32+05:30  Cprogrammer
 * run if indimail gid is present in process supplementary groups
 *
 * Revision 2.20  2016-06-09 14:22:43+05:30  Cprogrammer
 * allow privilege to process running with indimail gid
 *
 * Revision 2.19  2016-05-18 11:47:29+05:30  Cprogrammer
 * use ASSIGNDIR for users/assign
 *
 * Revision 2.18  2016-05-17 17:09:39+05:30  Cprogrammer
 * use control directory set by configure
 *
 * Revision 2.17  2016-01-21 16:57:57+05:30  Cprogrammer
 * removed compiler warning for fscanf()
 *
 * Revision 2.16  2016-01-12 14:27:15+05:30  Cprogrammer
 * use AF_INET for get_local_ip()
 *
 * Revision 2.15  2014-04-17 11:42:59+05:30  Cprogrammer
 * set supplementary group ids for indimail
 *
 * Revision 2.14  2011-11-09 19:46:05+05:30  Cprogrammer
 * removed getversion
 *
 * Revision 2.13  2010-08-15 15:54:59+05:30  Cprogrammer
 * display max users per level and vlimits status
 *
 * Revision 2.12  2010-05-25 13:14:18+05:30  Cprogrammer
 * added option -b to display user's base home directory
 *
 * Revision 2.11  2010-05-17 18:54:15+05:30  Cprogrammer
 * display base dir
 *
 * Revision 2.10  2010-04-20 13:17:40+05:30  Cprogrammer
 * skip cluster code if host.cntrl is missing
 *
 * Revision 2.9  2009-12-30 13:15:03+05:30  Cprogrammer
 * run vdominfo with uid, gid of domain specfified or with indimail uid
 *
 * Revision 2.8  2009-02-18 09:08:41+05:30  Cprogrammer
 * display domain only if isvirtualdomain() does not return error
 *
 * Revision 2.7  2008-09-17 19:30:19+05:30  Cprogrammer
 * setuid to indimail or domain uid to be able to read .filesystems
 *
 * Revision 2.6  2005-12-29 22:52:42+05:30  Cprogrammer
 * use getEnvConfigStr to set variables from environment variables
 *
 * Revision 2.5  2004-05-19 20:06:41+05:30  Cprogrammer
 * new logic from indimail (tom collins)
 *
 * Revision 2.4  2003-05-26 12:59:09+05:30  Cprogrammer
 * added option to display alias domains
 *
 * Revision 2.3  2003-01-14 12:43:58+05:30  Cprogrammer
 * changes for silent option to print_control()
 *
 * Revision 2.2  2002-12-29 19:01:24+05:30  Cprogrammer
 * minor modication - skip copying into Domain is line is not correct
 *
 * Revision 2.1  2002-08-11 00:57:19+05:30  Cprogrammer
 * improved formatting
 *
 * Revision 1.15  2002-03-29 23:51:34+05:30  Cprogrammer
 * added display of hostid and ip address
 *
 * Revision 1.14  2002-02-24 22:46:53+05:30  Cprogrammer
 * added getversion
 *
 * Revision 1.13  2002-02-17 00:18:28+05:30  Cprogrammer
 * removed hardcoding of constant 156
 *
 * Revision 1.12  2001-12-27 01:30:26+05:30  Cprogrammer
 * version and verbose switch update
 *
 * Revision 1.11  2001-12-23 00:48:42+05:30  Cprogrammer
 * removed vclose inside loop and moved it to main()
 * removed putting of blank line
 *
 * Revision 1.10  2001-12-14 15:38:27+05:30  Cprogrammer
 * Formatted display of ports
 *
 * Revision 1.9  2001-12-11 11:34:54+05:30  Cprogrammer
 * put a blank new line when no ports are defined
 *
 * Revision 1.8  2001-12-10 00:51:52+05:30  Cprogrammer
 * display of domain in the printf statement
 *
 * Revision 1.7  2001-12-10 00:44:52+05:30  Cprogrammer
 * added option to display smtp ports
 *
 * Revision 1.6  2001-12-08 17:45:51+05:30  Cprogrammer
 * usage message change
 *
 * Revision 1.5  2001-12-02 18:50:48+05:30  Cprogrammer
 * replaced dir_control code with function print_control()
 *
 * Revision 1.4  2001-11-28 23:10:05+05:30  Cprogrammer
 * getversion() function call added
 *
 * Revision 1.3  2001-11-24 12:21:51+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.2  2001-11-20 11:00:39+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.1  2001-10-24 18:15:36+05:30  Cprogrammer
 * Initial revision
 *
 * vdominfo
 *
 * prints domain information
 *
 * part of the indimail package
 *
 * Copyright (C) 2001 Inter7 Internet Technologies, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */
#include "indimail.h"
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <memory.h>
#include <sys/socket.h>

#ifndef	lint
static char     sccsid[] = "$Id: vdominfo.c,v 2.22 2017-03-13 14:12:59+05:30 Cprogrammer Exp mbhangui $";
#endif

char            Domain[MAX_BUFF];
char            Dir[MAX_BUFF];
char            TmpBuf[MAX_BUFF];
uid_t           Uid;
gid_t           Gid;

int             DisplayName;
int             DisplayUid;
int             DisplayGid;
int             DisplayDir;
int             DisplayBaseDir;
#ifdef CLUSTERED_SITE
int             DisplayPort;
#endif
int             DisplayAll;
int             DisplayTotalUsers;
int             DisplayAliasDomains;

void            usage();
int             get_options(int, char **);
void            display_domain(char *, char *, uid_t, gid_t);
void            display_all_domains();

#define VDOMTOKENS ":\n"

extern vdir_type vdir;

int
main(argc, argv)
	int             argc;
	char           *argv[];
{
	uid_t           myuid;
	gid_t           mygid;

	if (get_options(argc, argv))
		return(1);
	myuid = getuid();
	mygid = getgid();
	if (*Domain)
	{
		if (vget_assign(Domain, Dir, MAX_BUFF, &Uid, &Gid) == NULL)
		{
			error_stack(stderr, "domain %s does not exist\n", Domain);
			return(1);
		}
		if (myuid != Uid && mygid != Gid && myuid != 0 && check_group(Gid) != 1)
		{
			error_stack(stderr, "you must be root / domain user (uid=%d) / (gid=%d) to run this program\n", Uid, Gid);
			return(1);
		}
		if (setuser_privileges(Uid, Gid, "indimail"))
		{
			error_stack(stderr, "setuser_privilege: (%d/%d): %s\n", Uid, Gid, strerror(errno));
			return (1);
		}
		if (isvirtualdomain(Domain) == 1)
			display_domain(Domain, Dir, Uid, Gid);
		else
			error_stack(stderr, "domain %s does not exist\n", Domain);
	} else
	{
		if (indimailuid == -1 || indimailgid == -1)
			GetIndiId(&indimailuid, &indimailgid);
		if (setuser_privileges(0, indimailgid, "indimail"))
		{
			error_stack(stderr, "setuser_privilege: (0/%d): %s\n", indimailgid, strerror(errno));
			return (1);
		}
		display_all_domains();
	}
	vclose();
	return(0);
}

void
usage()
{
	printf("usage: vdominfo [options] [domain]\n");
	printf("options: -V (print version number)\n");
	printf("         -a (display all fields, this is the default)\n");
	printf("         -n (display domain name)\n");
	printf("         -u (display uid field)\n");
	printf("         -g (display gid field)\n");
	printf("         -d (display domain directory)\n");
	printf("         -b (display user's base directory)\n");
	printf("         -t (display total users)\n");
	printf("         -l (display alias domains)\n");
#ifdef CLUSTERED_SITE
	printf("         -p (display smtp ports)\n");
#endif
}

int
get_options(int argc, char **argv)
{
	int             c;
	int             errflag;
	extern char    *optarg;
	extern int      optind;

	DisplayName = 0;
	DisplayUid = 0;
	DisplayGid = 0;
	DisplayDir = 0;
	DisplayBaseDir = 0;
#ifdef CLUSTERED_SITE
	DisplayPort = 0;
#endif
	DisplayTotalUsers = 0;
	DisplayAliasDomains = 0;
	DisplayAll = 1;
	memset(Domain, 0, MAX_BUFF);
	errflag = 0;
	while (!errflag && (c = getopt(argc, argv, "vanugdbtpl")) != -1)
	{
		switch (c)
		{
		case 'n':
			DisplayName = 1;
			DisplayAll = 0;
			break;
		case 'u':
			DisplayUid = 1;
			DisplayAll = 0;
			break;
		case 'g':
			DisplayGid = 1;
			DisplayAll = 0;
			break;
		case 'd':
			DisplayDir = 1;
			DisplayAll = 0;
			break;
		case 'b':
			DisplayBaseDir = 1;
			DisplayAll = 0;
			break;
		case 't':
			DisplayTotalUsers = 1;
			DisplayAll = 0;
			break;
		case 'l':
			DisplayAliasDomains = 1;
			DisplayAll = 0;
			break;
#ifdef CLUSTERED_SITE
		case 'p':
			DisplayPort = 1;
			DisplayAll = 0;
			break;
#endif
		case 'a':
			DisplayAll = 1;
			break;
		default:
			errflag = 1;
			break;
		}
	}
	if (errflag)
	{
		usage();
		return(1);
	}
	if (optind < argc)
		scopy(Domain, argv[optind++], MAX_BUFF);
	return(0);
}

void
display_domain(char *domain, char *dir, uid_t uid, gid_t gid)
{
	char           *real_domain, *base_path;
	char            tmpbuf[MAX_BUFF];
	FILE           *fp;
	unsigned long   total;
	int             users_per_level = 0;
#ifdef CLUSTERED_SITE
	char           *ptr, *hostid, *sysconfdir, *controldir;
	char            host_path[MAX_BUFF];
	int             Port, host_cntrl = 0;
#endif

	if (!(real_domain = vget_real_domain(domain)))
		return;
	if (DisplayAll)
	{
		printf("---- Domain %-25s -------------------------------\n", domain);
		if (!strcmp(real_domain, domain))
			printf("    domain: %s\n", domain);
		else
			printf("    domain: %s aliased to %s\n", domain, real_domain);
		printf("       uid: %lu\n", (long unsigned) uid);
		printf("       gid: %lu\n", (long unsigned) gid);
#ifdef CLUSTERED_SITE
		getEnvConfigStr(&controldir, "CONTROLDIR", CONTROLDIR);
		if (*controldir == '/') {
			if (snprintf(host_path, MAX_BUFF, "%s/host.cntrl", controldir) == -1)
				host_path[MAX_BUFF - 1] = 0;
		} else {
			getEnvConfigStr(&sysconfdir, "SYSCONFDIR", SYSCONFDIR);
			if (snprintf(host_path, MAX_BUFF, "%s/%s/host.cntrl", sysconfdir, controldir) == -1)
				host_path[MAX_BUFF - 1] = 0;
		}
		if ((host_cntrl = !access(host_path, F_OK)))
		{
			if ((hostid = get_local_hostid()))
				printf("   host ID: %s\n", hostid);
			else
				printf("   host ID: ???\n");
			if ((ptr = get_local_ip(PF_INET)))
				printf("   IP Addr: %s\n", ptr);
			else
				printf("   IP Addr: ??\n");
			for (total = 0;(ptr = vsmtp_select(domain, &Port)) != NULL;total++)
				printf("%10s: %35s@%-20s -> %d\n", total ? "     " : "Ports", ptr, domain, Port);
		}
#endif
		printf("Domain Dir: %s\n", dir);
		if (!strcmp(real_domain, domain))
		{
			snprintf(tmpbuf, MAX_BUFF, "%s/.base_path", dir);
			if ((fp = fopen(tmpbuf, "r"))) {
				if (fscanf(fp, "%s", tmpbuf) != 1) {
					error_stack(stderr, "invalid domain users per level\n");
					return;
				}
				printf("  Base Dir: %s\n", tmpbuf);
				fclose(fp);
			} else {
				getEnvConfigStr(&base_path, "BASE_PATH", BASE_PATH);
				printf("  Base Dir: %s\n", base_path);
			}
			snprintf(tmpbuf, MAX_BUFF, "%s/.users_per_level", dir);
			if ((fp = fopen(tmpbuf, "r")))
			{
				if (fscanf(fp, "%d", &users_per_level) != 1)
				{
					error_stack(stderr, "invalid domain users per level\n");
					return;
				}
				fclose(fp);
			} else
			if (errno != ENOENT)
			{
				error_stack(stderr, "%s: %s\n", tmpbuf, strerror(errno));
				return;
			}
			snprintf(tmpbuf, MAX_BUFF, "%s/.filesystems", dir);
			total = print_control(tmpbuf, domain, users_per_level, 0);
			printf("     Users: %ld\n", total);
			snprintf(tmpbuf, MAX_BUFF, "%s/.domain_limits", dir);
			printf("   vlimits: %s\n", access(tmpbuf, F_OK) ? "disabled" : "enabled");
			snprintf(tmpbuf, MAX_BUFF, "%s/.aliasdomains", dir);
			if ((fp = fopen(tmpbuf, "r")))
			{
				printf("AliasDomains:\n");
				for (;;)
				{
					if (!fgets(tmpbuf, sizeof(tmpbuf) - 2, fp))
						break;
					printf("%s", tmpbuf);
				}
				fclose(fp);
			}
		}
	} else
	{
		printf("---- Domain %-25s -------------------------------\n", domain);
		if (DisplayName)
		{
			if (!strcmp(real_domain, domain))
				printf("    domain: %s\n", domain);
			else
				printf("    domain: %s aliased to %s\n", domain, real_domain);
#ifdef CLUSTERED_SITE
			if (host_cntrl)
			{
				if ((hostid = get_local_hostid()))
					printf("   host ID: %s\n", hostid);
				else
					printf("   host ID: ??\n");
				if ((ptr = get_local_ip(AF_INET)))
					printf("   IP Addr: %s\n", ptr);
				else
					printf("   IP Addr: ??\n");
			}
#endif
		}
		if (DisplayUid)
			printf("       uid: %lu\n", (long unsigned) uid);
		if (DisplayGid)
			printf("       gid: %lu\n", (long unsigned) gid);
#ifdef CLUSTERED_SITE
		if (DisplayPort && host_cntrl)
		{
			for (total = 0;(ptr = vsmtp_select(domain, &Port)) != NULL;total++)
				printf("%10s: %s@%s -> %d\n", total ? "     " : "Ports", ptr, domain, Port);
		}
#endif
		if (DisplayDir)
			printf("Domain Dir: %s\n", dir);
		if (!strncmp(real_domain, domain, MAX_BUFF))
		{
			if (DisplayBaseDir) {
				snprintf(tmpbuf, MAX_BUFF, "%s/.base_path", dir);
				if ((fp = fopen(tmpbuf, "r"))) {
					if (fscanf(fp, "%s", tmpbuf) != 1) {
						error_stack(stderr, "invalid domain users per level\n");
						return;
					}
					printf("  Base Dir: %s\n", tmpbuf);
					fclose(fp);
				} else {
					getEnvConfigStr(&base_path, "BASE_PATH", BASE_PATH);
					printf("  Base Dir: %s\n", base_path);
				}
			}
			if (DisplayTotalUsers)
			{
				snprintf(tmpbuf, MAX_BUFF, "%s/.users_per_level", dir);
				if ((fp = fopen(tmpbuf, "r")))
				{
					if (fscanf(fp, "%d", &users_per_level) != 1)
					{
						error_stack(stderr, "invalid domain users per level\n");
						return;
					}
					fclose(fp);
				} else
				if (errno != ENOENT)
				{
					error_stack(stderr, "%s: %s\n", tmpbuf, strerror(errno));
					return;
				}
				snprintf(tmpbuf, MAX_BUFF, "%s/.filesystems", dir);
				total = print_control(tmpbuf, domain, users_per_level, 0);
				printf("     Users: %ld\n", total);
				snprintf(tmpbuf, MAX_BUFF, "%s/.domain_limits", dir);
				printf("   vlimits: %s\n", access(tmpbuf, F_OK) ? "disabled" : "enabled");
			}
			if (DisplayAliasDomains)
			{
				snprintf(tmpbuf, MAX_BUFF, "%s/.aliasdomains", dir);
				if ((fp = fopen(tmpbuf, "r")))
				{
					printf("AliasDomains:\n");
					for (;;)
					{
						if (!fgets(tmpbuf, sizeof(tmpbuf) - 2, fp))
							break;
						printf("%s", tmpbuf);
					}
					fclose(fp);
				}
			}
		}
	}
}

void
display_all_domains()
{
	FILE           *fs;
	char           *tmpstr, *assigndir;

	getEnvConfigStr(&assigndir, "ASSIGNDIR", ASSIGNDIR);
	snprintf(TmpBuf, MAX_BUFF, "%s/assign", assigndir);
	if ((fs = fopen(TmpBuf, "r")) == NULL)
	{
		perror(TmpBuf);
		return;
	}
	while (fgets(TmpBuf, MAX_BUFF, fs) != NULL)
	{
		if (*TmpBuf != '+') /*- ignore lines that do not start with '+' */
			continue;
		if ((tmpstr = strtok(TmpBuf, VDOMTOKENS)) == NULL)
			continue;
		if ((tmpstr = strtok(NULL, VDOMTOKENS)) == NULL)
			continue;
		if (!isvirtualdomain(tmpstr))
			continue;
		if (!strchr(tmpstr, '.')) /*- non-indimail entries */
			continue;
		scopy(Domain, tmpstr, MAX_BUFF);
		if ((tmpstr = strtok(NULL, VDOMTOKENS)) == NULL)
			continue;
		Uid = atol(tmpstr);
		if ((tmpstr = strtok(NULL, VDOMTOKENS)) == NULL)
			continue;
		Gid = atol(tmpstr);
		if ((tmpstr = strtok(NULL, VDOMTOKENS)) == NULL)
			continue;
		scopy(Dir, tmpstr, MAX_BUFF);
		display_domain(Domain, Dir, Uid, Gid);
	}
	fclose(fs);
}

void
getversion_vdominfo_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
