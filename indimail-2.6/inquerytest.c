/*
 * $Log: inquerytest.c,v $
 * Revision 2.28  2019-03-24 20:53:08+05:30  Cprogrammer
 * force ipaddr for relay query
 *
 * Revision 2.27  2018-09-11 10:35:16+05:30  Cprogrammer
 * fixed compiler warnings
 *
 * Revision 2.26  2017-03-27 08:54:04+05:30  Cprogrammer
 * added FIFODIR variable for location of infifo
 *
 * Revision 2.25  2017-03-13 14:02:17+05:30  Cprogrammer
 * replaced qmaildir with sysconfdir
 *
 * Revision 2.24  2016-05-17 17:09:39+05:30  Cprogrammer
 * use control directory set by configure
 *
 * Revision 2.23  2016-01-12 23:45:56+05:30  Cprogrammer
 * removed leftover sleep() call
 *
 * Revision 2.22  2016-01-12 14:28:01+05:30  Cprogrammer
 * fix for fifo balancing not happening when infifo=""
 *
 * Revision 2.21  2013-11-22 16:41:39+05:30  Cprogrammer
 * added getopt style arguments for options
 *
 * Revision 2.20  2013-11-15 19:13:15+05:30  Cprogrammer
 * define INFIFO environment variable to select a specific fifo
 *
 * Revision 2.19  2011-04-04 23:04:21+05:30  Cprogrammer
 * added instance number to ProcessInFifo()
 *
 * Revision 2.18  2011-04-02 13:59:24+05:30  Cprogrammer
 * fix for 64 bit data type
 *
 * Revision 2.17  2010-04-11 22:21:52+05:30  Cprogrammer
 * replaced LPWD_QUERY with LIMIT_QUERY for domain limits
 *
 * Revision 2.16  2008-05-28 21:55:29+05:30  Cprogrammer
 * removed LDAP Query
 *
 * Revision 2.15  2005-12-29 22:41:24+05:30  Cprogrammer
 * use getEnvConfigStr to set variables from environment variables
 *
 * Revision 2.14  2004-02-18 14:23:59+05:30  Cprogrammer
 * added domain query
 *
 * Revision 2.13  2002-10-18 14:54:39+05:30  Cprogrammer
 * set verbose
 * incorrect opening of fifo corrected
 *
 * Revision 2.12  2002-09-04 11:31:48+05:30  Cprogrammer
 * added checks to display correct usage correctly
 *
 * Revision 2.11  2002-08-30 00:17:08+05:30  Cprogrammer
 * initialized default_table and inactive_table
 *
 * Revision 2.10  2002-08-25 22:31:01+05:30  Cprogrammer
 * made control dir configurable
 * check if fifo is already open by another process
 *
 * Revision 2.9  2002-08-03 00:36:11+05:30  Cprogrammer
 * added Local Passwd Query test
 * added display of inactive status
 *
 * Revision 2.8  2002-07-22 20:02:20+05:30  Cprogrammer
 * if argv[2] is a null byte, do not fork
 * use infifo from environment variable if argv[2] is a null byte
 *
 * Revision 2.7  2002-07-15 19:46:49+05:30  Cprogrammer
 * added LDAP_QUERY
 *
 * Revision 2.6  2002-07-07 20:53:44+05:30  Cprogrammer
 * added printing of errno.
 * corrected wrong value of argc being used for relay query type
 *
 * Revision 2.5  2002-07-05 03:59:14+05:30  Cprogrammer
 * changed SIGKILL to SIGTERM
 *
 * Revision 2.4  2002-07-05 03:55:04+05:30  Cprogrammer
 * use existing default fifo if fifo argument is null
 *
 * Revision 2.3  2002-07-05 00:37:39+05:30  Cprogrammer
 * typecast to prevent compilation warnings
 *
 * Revision 2.2  2002-07-04 00:33:32+05:30  Cprogrammer
 * valias functionality incorporated
 *
 * Revision 2.1  2002-07-03 22:50:26+05:30  Cprogrammer
 * program to test inquery() function
 *
 */
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: inquerytest.c,v 2.28 2019-03-24 20:53:08+05:30 Cprogrammer Exp mbhangui $";
#endif

void            print_limits(struct vlimits *);
void            SigChild();

void usage(char *ptr)
{
	(void) fprintf(stderr, "%s: [-v] -q query_type -i infifo email [ipaddr]\n", ptr);
	(void) fprintf(stderr, "1   - User         Status Query\n");
	(void) fprintf(stderr, "2   - Relay        Status Query\n");
	(void) fprintf(stderr, "3   - Passwd       Query\n");
	(void) fprintf(stderr, "4   - SmtpRoute    Query\n");
	(void) fprintf(stderr, "5   - Valias       Query\n");
	(void) fprintf(stderr, "6   - Domain Limit Query\n");
	(void) fprintf(stderr, "7   - Domain       Query\n");
	exit(1);
}

int
main(int argc, char **argv)
{
	struct passwd  *pw;
#ifdef ENABLE_DOMAIN_LIMITS
	struct vlimits *lmt;
#endif
	void           *dbptr;
	char           *ptr, *infifo = 0, *infifo_dir, *controldir, *email = 0, *ipaddr = 0;
	char            InFifo[MAX_BUFF], InFifoEnv[MAX_BUFF + 28];
	int             c, query_type = -1, fd = -1, status;
	pid_t           pid;

	if ((ptr = strrchr(argv[0], '/')))
		ptr++;
	else
		ptr = argv[0];
	while ((c = getopt(argc, argv, "vq:i:")) != EOF)
	{
		switch (c)
		{
		case 'v':
			verbose = 1;
			break;
		case 'q':
			query_type = *optarg - '0';
			break;
		case 'i':
			infifo = optarg;
			break;
		default:
			usage(ptr);
		}
	}
	if (query_type == -1)
		usage(ptr);
	if (optind < argc) {
		email = argv[optind++];
	}
	if (optind < argc)
		ipaddr = argv[optind++];
	switch (query_type)
	{
	case USER_QUERY:
	case RELAY_QUERY:
		if (!ipaddr) {
			fprintf(stderr, "%s: ipaddr must be specified for RELAY query\n", ptr);
			usage(ptr);
		}
	case PWD_QUERY:
#ifdef CLUSTERED_SITE
	case HOST_QUERY:
#endif
	case ALIAS_QUERY:
	case LIMIT_QUERY:
	case DOMAIN_QUERY:
		break;
	default:
		fprintf(stderr, "%s: Invalid query type %d\n", ptr, query_type);
		usage(ptr);
	}
	if (infifo && *infifo)
	{
		if (*infifo == '/' || *infifo == '.')
			snprintf(InFifo, MAX_BUFF, "%s", infifo);
		else
		{
			getEnvConfigStr(&controldir, "CONTROLDIR", CONTROLDIR);
			getEnvConfigStr(&infifo_dir, "FIFODIR", INDIMAILDIR"/inquery");
			if (*infifo_dir == '/') {
				if (indimailuid == -1 || indimailgid == -1)
					GetIndiId(&indimailuid, &indimailgid);
				r_mkdir(infifo_dir, 0775, indimailuid, indimailgid);
				snprintf(InFifo, MAX_BUFF, "%s/%s", infifo_dir, infifo);
			} else
				snprintf(InFifo, MAX_BUFF, INDIMAILDIR"%s/%s", infifo_dir, infifo);
		}
		if (access(InFifo, F_OK) || (fd = open(InFifo, O_WRONLY|O_NONBLOCK)) == -1)
		{
			if (FifoCreate(InFifo) == -1)
			{
				fprintf(stderr, "%s: FifoCreate: %s: %s\n", ptr, InFifo, strerror(errno));
				return (1);
			}
			snprintf(InFifoEnv, sizeof(InFifoEnv), "INFIFO=%s", InFifo);
			putenv(InFifoEnv);
			switch (pid = fork())
			{
			case -1:
				perror("fork");
				return (1);
			case 0:
				return (ProcessInFifo(0));
			default:
				signal(SIGCHLD, SigChild);
				break;
			}
		} else /*- Fifo is present and open by inlookup */
		{
			snprintf(InFifoEnv, sizeof(InFifoEnv), "INFIFO=%s", InFifo);
			putenv(InFifoEnv);
			pid = -1;
			close(fd);
		}
	} else
	{
		getEnvConfigStr(&infifo, "INFIFO", INFIFO);
		if (*infifo == '/' || *infifo == '.')
			snprintf(InFifo, MAX_BUFF, "%s", infifo);
		else
		{
			getEnvConfigStr(&controldir, "CONTROLDIR", CONTROLDIR);
			getEnvConfigStr(&infifo_dir, "FIFODIR", INDIMAILDIR"/inquery");
			if (*controldir == '/')
				snprintf(InFifo, MAX_BUFF, "%s/%s", infifo_dir, infifo);
			else
				snprintf(InFifo, MAX_BUFF, INDIMAILDIR"%s/%s", infifo_dir, infifo);
		}
		pid = -1;
	}
	getEnvConfigStr(&default_table, "MYSQL_TABLE", MYSQL_DEFAULT_TABLE);
	getEnvConfigStr(&inactive_table, "MYSQL_INACTIVE_TABLE", MYSQL_INACTIVE_TABLE);
	if (!(dbptr = inquery(query_type, email, ipaddr)))
	{
		if (userNotFound)
			printf("%s: No such user\n", email);
		else
		{
			if (errno)
				printf("%s: inquery failure: %s\n", email, strerror(errno));
			else
				printf("%s: inquery failure\n", email);
		}
		signal(SIGCHLD, SIG_IGN);
		if (pid != -1)
		{
			kill(pid, SIGTERM);
			unlink(infifo);
		}
		if (fd != -1)
			close(fd);
		return (1);
	}
	if (fd != -1)
		close(fd);
	if (pid != -1)
		unlink(infifo);
	switch (query_type)
	{
	case USER_QUERY:
		switch ((int) *((int *) dbptr))
		{
		case 0:
			fprintf(stdout, "%s has all services enabled \n", email);
			break;
		case 1:
			fprintf(stdout, "%s is Absent on this domain (#5.1.1)\n", email);
			break;
		case 2:
			fprintf(stdout, "%s is Inactive on this domain (#5.2.1)\n", email);
			break;
		case 3:
			fprintf(stdout, "%s is Overquota on this domain (#5.2.2)\n", email);
			break;
		case 4:
			fprintf(stdout, "%s is an alias\n", email);
			break;
		default:
			fprintf(stdout, "%s has unknown status %d\n", email, (int) *((int *) dbptr));
			break;
		}
		break;
	case RELAY_QUERY:
		fprintf(stdout, "%s is %s for IP %s\n", email,
			(int) *((int *) dbptr) == 1 ? "authenticated" : "not authenticated", ipaddr);
		break;
	case PWD_QUERY:
		pw = (struct passwd *) dbptr;
		printf("pw_name  : %s\n", pw->pw_name);
		printf("pw_passwd: %s\n", pw->pw_passwd);
		printf("pw_uid   : %d\n", (int) pw->pw_uid);
		printf("pw_gid   : %d\n", (int) pw->pw_gid);
		printf("pw_gecos : %s\n", pw->pw_gecos);
		printf("pw_dir   : %s\n", pw->pw_dir);
		printf("pw_shell : %s\n", pw->pw_shell);
		printf("Table    : %s\n", is_inactive ? inactive_table : default_table);
		break;
	case LIMIT_QUERY:
		lmt = (struct vlimits *) dbptr;
		print_limits(lmt);
		break;
#ifdef CLUSTERED_SITE
	case HOST_QUERY:
		fprintf(stdout, "%s: SMTPROUTE is %s\n", email, (char *) dbptr);
		break;
#endif
	case ALIAS_QUERY:
		if (!*((char *) dbptr))
		{
			fprintf(stderr, "%s: No aliases\n", email);
			break;
		}
		if (!(ptr = strtok((char *) dbptr, "\n")))
		{
			fprintf(stderr, "%s: Invalid Packet\n", email);
			break;
		}
		printf("Alias List for %s\n", email);
		printf("%s\n", ptr);
		for (;(ptr = strtok(0, "\n"));)
			printf("%s\n", ptr);
		break;
	case DOMAIN_QUERY:
		fprintf(stdout, "%s: Real Domain is %s\n", email, (char *) dbptr);
		break;
	default:
		fprintf(stderr, "%s query_type email [ipaddr]\n", ptr);
		signal(SIGCHLD, SIG_IGN);
		if (pid != -1)
			kill(pid, SIGTERM);
		return (1);
	}
	signal(SIGCHLD, SIG_IGN);
	if (pid != -1)
		kill(pid, SIGTERM);
	wait(&status);
	return (0);
}

void
print_limits(struct vlimits *limits)
{
	printf("Domain Expiry Date   : %s", limits->domain_expiry == -1 ? "Never Expires\n" : ctime(&limits->domain_expiry));
	printf("Password Expiry Date : %s", limits->passwd_expiry == -1 ? "Never Expires\n" : ctime(&limits->passwd_expiry));
	printf("Max Domain Quota     : %"PRId64"\n", limits->diskquota);
	printf("Max Domain Messages  : %"PRId64"\n", limits->maxmsgcount);
	printf("Default User Quota   : %"PRId64"\n", limits->defaultquota);
	printf("Default User Messages: %"PRId64"\n", limits->defaultmaxmsgcount);
	printf("Max Pop Accounts     : %d\n", limits->maxpopaccounts);
	printf("Max Aliases          : %d\n", limits->maxaliases);
	printf("Max Forwards         : %d\n", limits->maxforwards);
	printf("Max Autoresponders   : %d\n", limits->maxautoresponders);
	printf("Max Mailinglists     : %d\n", limits->maxmailinglists);
	printf("GID Flags:\n");
	if (limits->disable_imap != 0)
		printf("  NO_IMAP\n");
	if (limits->disable_smtp != 0)
		printf("  NO_SMTP\n");
	if (limits->disable_pop != 0)
		printf("  NO_POP\n");
	if (limits->disable_webmail != 0)
		printf("  NO_WEBMAIL\n");
	if (limits->disable_passwordchanging != 0)
		printf("  NO_PASSWD_CHNG\n");
	if (limits->disable_relay != 0)
		printf("  NO_RELAY\n");
	if (limits->disable_dialup != 0)
		printf("  NO_DIALUP\n");
	printf("Flags for non postmaster accounts:\n");
	printf("  pop account           : ");
	printf((limits->perm_account & VLIMIT_DISABLE_CREATE ? "DENY_CREATE  " : "ALLOW_CREATE "));
	printf((limits->perm_account & VLIMIT_DISABLE_MODIFY ? "DENY_MODIFY  " : "ALLOW_MODIFY "));
	printf((limits->perm_account & VLIMIT_DISABLE_DELETE ? "DENY_DELETE  " : "ALLOW_DELETE "));
	printf("\n");
	printf("  alias                 : ");
	printf((limits->perm_alias & VLIMIT_DISABLE_CREATE ? "DENY_CREATE  " : "ALLOW_CREATE "));
	printf((limits->perm_alias & VLIMIT_DISABLE_MODIFY ? "DENY_MODIFY  " : "ALLOW_MODIFY "));
	printf((limits->perm_alias & VLIMIT_DISABLE_DELETE ? "DENY_DELETE  " : "ALLOW_DELETE "));
	printf("\n");
	printf("  forward               : ");
	printf((limits->perm_forward & VLIMIT_DISABLE_CREATE ? "DENY_CREATE  " : "ALLOW_CREATE "));
	printf((limits->perm_forward & VLIMIT_DISABLE_MODIFY ? "DENY_MODIFY  " : "ALLOW_MODIFY "));
	printf((limits->perm_forward & VLIMIT_DISABLE_DELETE ? "DENY_DELETE  " : "ALLOW_DELETE "));
	printf("\n");
	printf("  autoresponder         : ");
	printf((limits->perm_autoresponder & VLIMIT_DISABLE_CREATE ? "DENY_CREATE  " : "ALLOW_CREATE "));
	printf((limits->perm_autoresponder & VLIMIT_DISABLE_MODIFY ? "DENY_MODIFY  " : "ALLOW_MODIFY "));
	printf((limits->perm_autoresponder & VLIMIT_DISABLE_DELETE ? "DENY_DELETE  " : "ALLOW_DELETE "));
	printf("\n");
	printf("  mailinglist           : ");
	printf((limits->perm_maillist & VLIMIT_DISABLE_CREATE ? "DENY_CREATE  " : "ALLOW_CREATE "));
	printf((limits->perm_maillist & VLIMIT_DISABLE_MODIFY ? "DENY_MODIFY  " : "ALLOW_MODIFY "));
	printf((limits->perm_maillist & VLIMIT_DISABLE_DELETE ? "DENY_DELETE  " : "ALLOW_DELETE "));
	printf("\n");
	printf("  mailinglist users     : ");
	printf((limits->perm_maillist_users & VLIMIT_DISABLE_CREATE ? "DENY_CREATE  " : "ALLOW_CREATE "));
	printf((limits->perm_maillist_users & VLIMIT_DISABLE_MODIFY ? "DENY_MODIFY  " : "ALLOW_MODIFY "));
	printf((limits->perm_maillist_users & VLIMIT_DISABLE_DELETE ? "DENY_DELETE  " : "ALLOW_DELETE "));
	printf("\n");
	printf("  mailinglist moderators: ");
	printf((limits->perm_maillist_moderators & VLIMIT_DISABLE_CREATE ? "DENY_CREATE  " : "ALLOW_CREATE "));
	printf((limits->perm_maillist_moderators & VLIMIT_DISABLE_MODIFY ? "DENY_MODIFY  " : "ALLOW_MODIFY "));
	printf((limits->perm_maillist_moderators & VLIMIT_DISABLE_DELETE ? "DENY_DELETE  " : "ALLOW_DELETE "));
	printf("\n");
	printf("  domain quota          : ");
	printf((limits->perm_quota & VLIMIT_DISABLE_CREATE ? "DENY_CREATE  " : "ALLOW_CREATE "));
	printf((limits->perm_quota & VLIMIT_DISABLE_MODIFY ? "DENY_MODIFY  " : "ALLOW_MODIFY "));
	printf((limits->perm_quota & VLIMIT_DISABLE_DELETE ? "DENY_DELETE  " : "ALLOW_DELETE "));
	printf("\n");
	printf("  default quota         : ");
	printf((limits->perm_defaultquota & VLIMIT_DISABLE_CREATE ? "DENY_CREATE  " : "ALLOW_CREATE "));
	printf((limits->perm_defaultquota & VLIMIT_DISABLE_MODIFY ? "DENY_MODIFY  " : "ALLOW_MODIFY "));
	printf("\n");
	return;
}

void
SigChild()
{
	int             status;

	fprintf(stderr, "InLookup died\n");
	wait(&status);
	exit(1);
}

void
getversion_inquerytest_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
