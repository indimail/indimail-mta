/*
 * $Log: vserverinfo.c,v $
 * Revision 2.3  2011-11-09 19:46:35+05:30  Cprogrammer
 * removed getversion
 *
 * Revision 2.2  2010-03-07 09:28:42+05:30  Cprogrammer
 * check return value of is_distributed_domain for error
 *
 * Revision 2.1  2009-11-18 11:49:30+05:30  Cprogrammer
 * program to display server info of a node in IndiMail cluster
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: vserverinfo.c,v 2.3 2011-11-09 19:46:35+05:30 Cprogrammer Stab mbhangui $";
#endif

#include <string.h>
#include <unistd.h>
#include <stdlib.h>

static int      get_options(int, char **, char **, char **, char **, char **);

int				display_user, display_passwd, display_database, display_port,
				display_server, display_mdahost, display_all;
int
main(int argc, char **argv)
{
	DBINFO        **rhostsptr;
	int             total, found;
	char           *mdahost, *server, *domain, *email, *mailstore, *ptr, *real_domain;
	char            User[AUTH_SIZE], Domain[AUTH_SIZE];

	if (get_options(argc, argv, &mdahost, &server, &domain, &email))
		return (1);
	if (email)
	{
		server = (char *) 0;
		if (parse_email(email, User, Domain, AUTH_SIZE))
		{
			fprintf(stderr, "userverinfo: could not parse email [%s]\n", email);
			return (1);
		}
		if (!(real_domain = vget_real_domain(Domain)))
			real_domain = Domain;
		domain = real_domain;
		if ((found = is_distributed_domain(real_domain)) == -1)
		{
			fprintf(stderr, "%s: is_distributed_domain failed\n", real_domain);
			return (1);
		} else
		if (found)
		{
			if (!(mailstore = findhost(email, 2)))
			{
				if (userNotFound)
					fprintf(stderr, "email %s not found\n", email);
				else
					fprintf(stderr, "No mailstore for %s\n", email);
				return (1);
			}
			if ((ptr = strrchr(mailstore, ':')) != (char *) 0)
				*ptr = 0;
			for (;*mailstore && *mailstore != ':';mailstore++);
			mailstore++;
			mdahost = mailstore;
		} else
			fprintf(stderr, "%s: not a distributed domain\n", real_domain);
	} else
		real_domain = domain;
	if (!RelayHosts && !(RelayHosts = LoadDbInfo_TXT(&total)))
	{
		perror("LoadDbInfo_TXT");
		return (1);
	}
	for (found = 0, rhostsptr = RelayHosts;*rhostsptr;rhostsptr++)
	{
		if (!strncmp((*rhostsptr)->domain, domain, DBINFO_BUFF))
		{
			if ((mdahost && !strncmp((*rhostsptr)->mdahost, mdahost, DBINFO_BUFF))
					|| (server && !strncmp((*rhostsptr)->server, server, DBINFO_BUFF))
				)
			{
				if (server)
					printf("Record  : %03d\n", found + 1);
				if (display_server || display_all)
					printf("server  : %s\n", (*rhostsptr)->server);
				if (display_mdahost || display_all)
					printf("mdahost : %s\n", (*rhostsptr)->mdahost);
				if (display_user || display_all)
					printf("user    : %s\n", (*rhostsptr)->user);
				if (display_passwd || display_all)
					printf("password: %s\n", (*rhostsptr)->password);
				if (display_port || display_all)
					printf("port    : %d\n", (*rhostsptr)->port);
				if (display_database || display_all)
					printf("database: %s\n", (*rhostsptr)->database);
				found++;
			}
		}
	}
	if (!found)
		fprintf(stderr, "userinfo: could not locate server info for %s\n", real_domain);
	return (found ? 0 : 1);
}

static int
get_options(int argc, char **argv, char **mdahost, char **server, char **domain, char **email)
{
	int             c;

	*email = *mdahost = *server = *domain = (char *) 0;
	display_all = 0;
	display_user = display_passwd = display_server = display_mdahost = display_port = display_database = 0;
	while ((c = getopt(argc, argv, "vdsmupPD:M:S:a")) != -1)
	{
		switch (c)
		{
		case 'v':
			verbose = 1;
			break;
		case 'u':
			display_all = 0;
			display_user = 1;
			break;
		case 'p':
			display_all = 0;
			display_passwd = 1;
			break;
		case 'P':
			display_all = 0;
			display_port = 1;
			break;
		case 's':
			display_all = 0;
			display_server = 1;
			break;
		case 'm':
			display_all = 0;
			display_mdahost = 1;
			break;
		case 'd':
			display_all = 0;
			display_database = 1;
			break;
		case 'D':
			*domain = optarg;
			break;
		case 'M':
			*mdahost = optarg;
			break;
		case 'S':
			*server = optarg;
			break;
		case 'a':
			display_all = 1;
			break;
		default:
			fprintf(stderr, "USAGE: vserserverinfo [-upPsmd] [-D domain -M host | -S server] | [email]\n");
			return (1);
		}
	}
	if (!display_user && !display_passwd && !display_server && !display_mdahost
		&& !display_port && !display_database && !display_all)
		display_all = 1;
	if ((!*mdahost && !*server) || !*domain)
	{
		if (optind < argc)
			*email = argv[optind++];
		else
		{
			fprintf(stderr, "USAGE: vserserverinfo [-upPsmd] [-D domain -M host | -S server] | [email]\n");
			return (1);
		}
	}
	return (0);
}

void
getversion_userverinfo_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
