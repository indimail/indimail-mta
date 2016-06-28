/*
 * $Log: vgroup.c,v $
 * Revision 2.23  2016-01-28 15:06:07+05:30  Cprogrammer
 * maildirquota spec for quota
 *
 * Revision 2.22  2011-11-09 19:46:13+05:30  Cprogrammer
 * removed getversion
 *
 * Revision 2.21  2010-08-11 18:36:51+05:30  Cprogrammer
 * added checks for group member syntax
 *
 * Revision 2.20  2010-08-08 20:17:47+05:30  Cprogrammer
 * use configurable users per level
 *
 * Revision 2.19  2010-05-01 13:50:26+05:30  Cprogrammer
 * close all database connections before exit
 *
 * Revision 2.18  2010-04-14 18:13:03+05:30  Cprogrammer
 * corrected usage description
 *
 * Revision 2.17  2010-04-14 15:46:27+05:30  Cprogrammer
 * added hostid argument.
 * changed -m option to -I
 *
 * Revision 2.16  2009-11-25 12:54:41+05:30  Cprogrammer
 * do not allow empty email addresses
 *
 * Revision 2.15  2009-10-14 20:47:41+05:30  Cprogrammer
 * use strtoll() instead of atol()
 *
 * Revision 2.14  2009-09-13 12:49:07+05:30  Cprogrammer
 * added 'm' option to getopt for previous ignore option
 *
 * Revision 2.13  2009-09-13 12:47:25+05:30  Cprogrammer
 * added option to ignore requirement of destination group address to be local
 *
 * Revision 2.12  2008-08-02 09:10:22+05:30  Cprogrammer
 * use new function error_stack
 *
 * Revision 2.11  2008-07-13 19:50:18+05:30  Cprogrammer
 * removed compilation warning on FC 9
 *
 * Revision 2.10  2008-06-13 10:46:15+05:30  Cprogrammer
 * conditional compilation of vgroup if VALIAS is defined
 * fixed compilation warnings if CLUSTERED_SITE was not defined
 *
 * Revision 2.9  2008-06-03 19:48:43+05:30  Cprogrammer
 * added mdahost argument to vadduser()
 *
 * Revision 2.8  2007-12-22 00:31:54+05:30  Cprogrammer
 * added option to add quota
 *
 * Revision 2.7  2005-12-21 09:16:39+05:30  Cprogrammer
 * segmentation fault fixed
 *
 * Revision 2.6  2004-09-21 23:45:03+05:30  Cprogrammer
 * change for actFlag argument to vauth_adduser()
 *
 * Revision 2.5  2004-07-03 23:55:29+05:30  Cprogrammer
 * check return status of parse_email
 *
 * Revision 2.4  2004-07-03 23:03:21+05:30  Cprogrammer
 * use vauthOpen() to automatically connect to the mda
 *
 * Revision 2.3  2004-07-02 18:13:29+05:30  Cprogrammer
 * renamed IndiGroup to MailGroup
 *
 * Revision 2.2  2003-05-31 12:11:41+05:30  Cprogrammer
 * formatting change for usage
 *
 * Revision 2.1  2003-05-30 01:58:22+05:30  Cprogrammer
 * Program to add groups
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: vgroup.c,v 2.23 2016-01-28 15:06:07+05:30 Cprogrammer Exp mbhangui $";
#endif

#ifdef VALIAS
#include <unistd.h>
#define XOPEN_SOURCE = 600
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define ADDNEW_GROUP  0
#define INSERT_MEMBER 1
#define DELETE_MEMBER 2
#define UPDATE_MEMBER 3

static int      get_options(int, char **, int *, char **, char **, char **, char **,
					char **, char **, char **, char **, int *);
static void     usage();
int             addGroup(char *, char *, char *, char *, char *, char *);

int
main(int argc, char **argv)
{
	char            User[MAX_BUFF], Domain[MAX_BUFF], alias_line[MAX_BUFF], old_alias[MAX_BUFF],
					quotaVal[QUOTA_BUFLEN];
	char           *group, *gecos, *member, *old_member, *passwd, *hostid, 
				   *mdahost, *Quota, *real_domain;
#ifdef CLUSTERED_SITE
	char           *ptr;
#endif
	int             option, ignore = 0, ret = -1;

	if (get_options(argc, argv, &option, &group, &gecos, &member, &old_member, &passwd,
		&hostid, &mdahost, &Quota, &ignore))
		return (1);
	if (parse_email(group, User, Domain, MAX_BUFF))
	{
		fprintf(stderr, "%s: Email too long\n", group);
		return (1);
	}
	if (option != ADDNEW_GROUP)
	{
#ifdef CLUSTERED_SITE
		if (vauthOpen_user(group, 0))
#else
		if (vauth_open((char *) 0))
#endif
			return(1);
	}
	/* Do this so that users do not get added in a alias domain */
	real_domain = (char *) 0;
	if (*Domain && !(real_domain = vget_real_domain(Domain)))
	{
		fprintf(stderr, "%s: No such domain\n", Domain);
		vclose();
		return(1);
	}
	switch (option)
	{
		case ADDNEW_GROUP:
			if (Quota && *Quota) {
				if (strncmp(Quota, "NOQUOTA", 8)) {
					snprintf(quotaVal, sizeof(quotaVal), "%"PRId64"", parse_quota(Quota, 0));
					if (!(ptr = strchr(quotaVal, ','))) {
						if ((ptr = strchr(Quota, ',')))
							scat(quotaVal, ptr, sizeof(quotaVal));
					}
				}
			} else
				*quotaVal = 0;
#ifdef CLUSTERED_SITE
			if (!mdahost && hostid)
			{
				if (!(ptr = vauth_getipaddr(hostid)))
				{
					error_stack(stderr, "Failed to obtain mdahost for host %s domain %s\n", hostid, real_domain);
					vclose();
					return(1);
				} else
					mdahost = ptr;
			}
			/* add the user */
			if (mdahost)
			{
				vclose();
				if (!(ptr = SqlServer(mdahost, real_domain)))
				{
					fprintf(stderr, "Failed to obtain sqlserver for host %s domain %s\n", mdahost, real_domain);
					return(1);
				} else
				if (vauth_open(ptr))
				{
					fprintf(stderr, "Failed to connect to %s\n", ptr);
					return(1);
				}
				if (verbose)
					printf("Connected to MDAhost %s SqlServer %s\n", mdahost, ptr);
			} 
#endif
			ret = addGroup(User, real_domain, mdahost, gecos, passwd, quotaVal);
			break;
		case INSERT_MEMBER:
			if (*member == '.')
			{
				fprintf(stderr, "Illegal member %s\n", member);
				vclose();
				return(1);
			}
			if (*member != '&' && *member != '/' && *member != '|')
				snprintf(alias_line, sizeof(alias_line),  "&%s",  member);
			else
				strncpy(alias_line, member, sizeof(alias_line));
			ret = valias_insert(User, real_domain, alias_line, ignore);
			break;
		case DELETE_MEMBER:
			if (*member == '.')
			{
				fprintf(stderr, "Illegal member %s\n", member);
				vclose();
				return(1);
			}
			if (*member != '&' && *member != '/' && *member != '|')
				snprintf(alias_line, sizeof(alias_line),  "&%s",  member);
			else
				strncpy(alias_line, member, sizeof(alias_line));
			ret = valias_delete(User, real_domain, alias_line);
			break;
		case UPDATE_MEMBER:
			if (*member == '.')
			{
				fprintf(stderr, "Illegal member %s\n", member);
				vclose();
				return(1);
			} else
			if (*old_member == '.')
			{
				fprintf(stderr, "Illegal member %s\n", old_member);
				vclose();
				return(1);
			}
			if (*member != '&' && *member != '/' && *member != '|')
				snprintf(alias_line, sizeof(alias_line),  "&%s",  member);
			else
				strncpy(alias_line, member, sizeof(alias_line));
			if (*old_member != '&' && *old_member != '/' && *old_member != '|')
				snprintf(old_alias, sizeof(old_alias),  "&%s",  old_member);
			else
				strncpy(old_alias, old_member, sizeof(old_alias));
			ret = valias_update(User, real_domain, old_alias, alias_line);
			break;
	}
	vclose();
	return(ret);
}

int
addGroup(char *user, char *domain, char *mdahost, char *gecos, char *passwd, char *quota)
{
	char            Gecos[MAX_BUFF];
	struct passwd  *pw;
	int             i;

	if (!passwd || !*passwd)
		passwd = vgetpasswd("Passwd");
	if (gecos)
		snprintf(Gecos, sizeof(Gecos), "MailGroup %s", gecos);
	else
		snprintf(Gecos, sizeof(Gecos), "MailGroup %s", user);
	if ((i = vadduser(user, domain, mdahost, passwd, Gecos, quota, 0, USE_POP, 1)) < 0)
	{
		error_stack(stderr, 0);
		return(i);
	}
	if (!(pw = vauth_getpw(user, domain)))
	{
		fprintf(stderr, "%s@%s: vauth_getpw failed: %s\n", user, domain, strerror(errno));
		return(1);
	}
	return(0);
}

static int
get_options(int argc, char **argv, int *option, char **group, char **gecos, char **member, char **old_member, char **passwd,
	char **hostid, char **mdahost, char **quota, int *ignore)
{
	int             c;

	*group = *gecos = *member = *old_member = *passwd = *hostid = *mdahost = *quota = 0;
	*option = -1;
	*ignore = 0;
	while ((c = getopt(argc, argv, "vaIc:i:d:o:u:h:m:q:")) != -1)
	{
		switch (c)
		{
		case 'v':
			verbose = 1;
			break;
		case 'a':
			switch (*option)
			{
			case ADDNEW_GROUP:
			case INSERT_MEMBER:
			case DELETE_MEMBER:
			case UPDATE_MEMBER:
				usage();
				return(1);
			}
			*option = ADDNEW_GROUP;
			break;
		case 'I':
			*ignore = 1;
			break;
		case 'c':
			*gecos = optarg;
			break;
		case 'q':
			*quota = optarg;
			break;
		case 'i':
			if (*option == ADDNEW_GROUP)
			{
				usage();
				return(1);
			}
			if (!*optarg)
			{
				fprintf(stderr, "You cannot have an empty email address\n");
				usage();
				return(1);
			}
			*option = INSERT_MEMBER;
			*member = optarg;
			break;
		case 'd':
			if (*option == ADDNEW_GROUP)
			{
				usage();
				return(1);
			}
			*option = DELETE_MEMBER;
			*member = optarg;
			break;
		case 'u':
			if (*option == ADDNEW_GROUP)
			{
				usage();
				return(1);
			}
			if (!*optarg)
			{
				fprintf(stderr, "You cannot have an empty email address\n");
				usage();
				return(1);
			}
			*option = UPDATE_MEMBER;
			*member = optarg;
			break;
		case 'o':
			if (*option == ADDNEW_GROUP)
			{
				usage();
				return(1);
			}
			if (!*optarg)
			{
				fprintf(stderr, "You cannot have an empty email address\n");
				usage();
				return(1);
			}
			*option = UPDATE_MEMBER;
			*old_member = optarg;
			break;
#ifdef CLUSTERED_SITE
		case 'h':
			*hostid = optarg;
			break;
		case 'm':
			*mdahost = optarg;
			break;
#endif
		default:
			usage();
		}
	}
	if (optind < argc)
		*group = argv[optind++];
	else
	{
		usage();
		return (1);
	}
	if (optind < argc)
		*passwd = argv[optind++];
	if (*option == UPDATE_MEMBER && (!*member || !*old_member))
	{
		usage();
		return (1);
	}
	if (*option == -1)
	{
		usage();
		return(1);
	}
	return (0);
}

void
usage()
{
	fprintf(stderr, "usage1: vgroup -a [-cqvVhmI] groupAddress [password]\n");
	fprintf(stderr, "usage2: vgroup    [-iduovV]  groupAddress\n\n");
	fprintf(stderr, "options: -V (print version number)\n");
	fprintf(stderr, "         -v (verbose)\n");
	fprintf(stderr, "         -a (Add new group)\n");
#ifdef CLUSTERED_SITE
	fprintf(stderr, "         -h hostid   (host on which the group needs to be created - specify hostid)\n");
	fprintf(stderr, "         -m mdahost  (host on which the group needs to be created - specify mdahost)\n");
	fprintf(stderr, "         -I Ignore requirement of of groupAddress to be local\n");
#endif
	fprintf(stderr, "         -c comment (sets the gecos comment field)\n");
	fprintf(stderr, "         -q quota_in_bytes (sets the users quota)\n");
	fprintf(stderr, "         -i member_email_address (insert member to group)\n");
	fprintf(stderr, "         -d member_email_address (delete member from group)\n");
	fprintf(stderr, "         -u newMemberEmail -o oldMemberEmail (update member with a new address)\n");
}
#else
int
main()
{
	printf("IndiMail not configured with --enable-valias=y\n");
	return(0);
}
#endif

void
getversion_vgroup_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
