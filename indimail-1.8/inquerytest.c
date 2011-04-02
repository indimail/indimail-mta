/*
 * $Log: inquerytest.c,v $
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
static char     sccsid[] = "$Id: inquerytest.c,v 2.18 2011-04-02 13:59:24+05:30 Cprogrammer Stab mbhangui $";
#endif

void            print_limits(struct vlimits *);
void            SigChild();

int
main(int argc, char **argv)
{
	struct passwd  *pw;
#ifdef ENABLE_DOMAIN_LIMITS
	struct vlimits *lmt;
#endif
	void           *dbptr;
	char           *ptr, *infifo, *qmaildir, *controldir;
	char            InFifo[MAX_BUFF], InFifoEnv[MAX_BUFF];
	int             fd = -1, status;
	pid_t           pid;

	if ((ptr = strrchr(argv[0], '/')))
		ptr++;
	else
		ptr = argv[0];
	if (argc != 4 && argc != 5)
	{
		fprintf(stderr, "%s query_type infifo email [ipaddr]\n", ptr);
		fprintf(stderr, "1   - User         Status Query\n");
		fprintf(stderr, "2   - Relay        Status Query\n");
		fprintf(stderr, "3   - Passwd       Query\n");
		fprintf(stderr, "4   - SmtpRoute    Query\n");
		fprintf(stderr, "5   - Valias       Query\n");
		fprintf(stderr, "6   - Domain Limit Query\n");
		fprintf(stderr, "7   - Domain       Query\n");
		return (1);
	}
	switch (*argv[1] - '0')
	{
	case USER_QUERY:
	case RELAY_QUERY:
	case PWD_QUERY:
#ifdef CLUSTERED_SITE
	case HOST_QUERY:
#endif
	case ALIAS_QUERY:
	case LIMIT_QUERY:
	case DOMAIN_QUERY:
		break;
	default:
		fprintf(stderr, "%s: Invalid query type %d\n", ptr, *argv[1] - '0');
		fprintf(stderr, "%s query_type infifo email [ipaddr]\n", ptr);
		fprintf(stderr, "1   - User         Status Query\n");
		fprintf(stderr, "2   - Relay        Status Query\n");
		fprintf(stderr, "3   - Passwd       Query\n");
		fprintf(stderr, "4   - SmtpRoute    Query\n");
		fprintf(stderr, "5   - Valias       Query\n");
		fprintf(stderr, "6   - Domain Limit Query\n");
		fprintf(stderr, "7   - Domain       Query\n");
		return (1);
	}
	verbose = 1;
	if(*argv[2])
	{
		infifo = argv[2];
		if (*infifo == '/' || *infifo == '.')
			snprintf(InFifo, MAX_BUFF, "%s", infifo);
		else
		{
			getEnvConfigStr(&qmaildir, "QMAILDIR", QMAILDIR);
			getEnvConfigStr(&controldir, "CONTROLDIR", "control");
			snprintf(InFifo, MAX_BUFF, "%s/%s/inquery/%s", qmaildir, controldir, infifo);
		}
		if(access(InFifo, F_OK) || (fd = open(InFifo, O_WRONLY|O_NONBLOCK)) == -1)
		{
			if (FifoCreate(InFifo) == -1)
			{
				fprintf(stderr, "%s: FifoCreate: %s: %s\n", ptr, InFifo, strerror(errno));
				return (1);
			}
			switch (pid = fork())
			{
			case -1:
				perror("fork");
				return (1);
			case 0:
				snprintf(InFifoEnv, MAX_BUFF, "INFIFO=%s", InFifo);
				putenv(InFifoEnv);
				return (ProcessInFifo());
			default:
				signal(SIGCHLD, SigChild);
				break;
			}
		} else /*- Fifo is present and open by inlookup */
		{
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
			getEnvConfigStr(&qmaildir, "QMAILDIR", QMAILDIR);
			getEnvConfigStr(&controldir, "CONTROLDIR", "control");
			snprintf(InFifo, MAX_BUFF, "%s/%s/inquery/%s", qmaildir, controldir, infifo);
		}
		pid = -1;
	}
	getEnvConfigStr(&default_table, "MYSQL_TABLE", MYSQL_DEFAULT_TABLE);
	getEnvConfigStr(&inactive_table, "MYSQL_INACTIVE_TABLE", MYSQL_INACTIVE_TABLE);
	if (!(dbptr = inquery(*argv[1] - '0', argv[3], argc == 5 ? argv[4] : 0)))
	{
		if(userNotFound)
			printf("%s: No such user\n", argv[3]);
		else
		{
			if(errno)
				printf("%s: inquery failure: %s\n", argv[3], strerror(errno));
			else
				printf("%s: inquery failure\n", argv[3]);
		}
		signal(SIGCHLD, SIG_IGN);
		if(pid != -1)
		{
			kill(pid, SIGTERM);
			unlink(argv[2]);
		}
		if(fd != -1)
			close(fd);
		return (1);
	}
	if(fd != -1)
		close(fd);
	if(pid != -1)
		unlink(argv[2]);
	switch (*argv[1] - '0')
	{
	case USER_QUERY:
		switch ((int) *((int *) dbptr))
		{
		case 0:
			fprintf(stdout, "%s has all services enabled \n", argv[3]);
			break;
		case 1:
			fprintf(stdout, "%s is Absent on this domain (#5.1.1)\n", argv[3]);
			break;
		case 2:
			fprintf(stdout, "%s is Inactive on this domain (#5.2.1)\n", argv[3]);
			break;
		case 3:
			fprintf(stdout, "%s is Overquota on this domain (#5.2.2)\n", argv[3]);
			break;
		case 4:
			fprintf(stdout, "%s is an alias\n", argv[3]);
			break;
		default:
			fprintf(stdout, "%s has unknown status %d\n", argv[3], (int) *((int *) dbptr));
			break;
		}
		break;
	case RELAY_QUERY:
		fprintf(stdout, "%s is %s\n", argv[3], (int) *((int *) dbptr) == 1 ? "authenticated" : "not authenticated");
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
		fprintf(stdout, "%s: SMTPROUTE is %s\n", argv[3], (char *) dbptr);
		break;
#endif
	case ALIAS_QUERY:
		if(!*((char *) dbptr))
		{
			fprintf(stderr, "%s: No aliases\n", argv[3]);
			break;
		}
		if(!(ptr = strtok((char *) dbptr, "\n")))
		{
			fprintf(stderr, "%s: Invalid Packet\n", argv[3]);
			break;
		}
		printf("Alias List for %s\n", argv[3]);
		printf("%s\n", ptr);
		for(;(ptr = strtok(0, "\n"));)
			printf("%s\n", ptr);
		break;
	case DOMAIN_QUERY:
		fprintf(stdout, "%s: Real Domain is %s\n", argv[3], (char *) dbptr);
		break;
	default:
		fprintf(stderr, "%s query_type email [ipaddr]\n", ptr);
		signal(SIGCHLD, SIG_IGN);
		if(pid != -1)
			kill(pid, SIGTERM);
		return (1);
	}
	signal(SIGCHLD, SIG_IGN);
	if(pid != -1)
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
