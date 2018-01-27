/*
 * $Log: vatrn.c,v $
 * Revision 2.6  2011-11-09 19:45:54+05:30  Cprogrammer
 * removed getversion
 *
 * Revision 2.5  2009-02-26 20:26:17+05:30  Cprogrammer
 * show all atrn access if email and domain are not specfied
 *
 * Revision 2.4  2008-05-28 16:38:38+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 2.3  2004-07-03 23:54:13+05:30  Cprogrammer
 * check return status of parse_email
 *
 * Revision 2.2  2003-10-06 00:02:28+05:30  Cprogrammer
 * added option to select maps by specifying atrn domain on command line
 *
 * Revision 2.1  2003-07-04 11:33:45+05:30  Cprogrammer
 * program for managing atrn maps
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: vatrn.c,v 2.6 2011-11-09 19:45:54+05:30 Cprogrammer Stab mbhangui $";
#endif

#include <unistd.h>
#include <string.h>

void            usage();
int             get_options(int, char **, int *, char *, char *, char *, char *, char *);

#define PRINT_IT  0
#define ADD_IT    1
#define DELETE_IT 2
#define UPDATE_IT 3

int
main(argc, argv)
	int             argc;
	char           *argv[];
{
	int             result, Action;
	char            Email[MAX_BUFF], User[MAX_BUFF], Domain[MAX_BUFF], domain_list[MAX_BUFF], old_domain[MAX_BUFF];
	char           *ptr, *user, *domain;

	if (get_options(argc, argv, &Action, Email, User, Domain, domain_list, old_domain))
		return(1);
	switch (Action)
	{
	case ADD_IT:
		result = vadd_atrn_map(User, Domain, domain_list);
		break;
	case DELETE_IT:
		result = vdel_atrn_map(User, Domain, domain_list);
		break;
	case UPDATE_IT:
		result = vupd_atrn_map(User, Domain, old_domain, domain_list);
		break;
	case PRINT_IT:
		domain = Domain;
		for(result = 1;;)
		{
			if (*User)
				user = User;
			else
				user = (char *) 0;
			if (!(ptr = vshow_atrn_map(&user, &domain)))
				break;
			result = 0;
			printf("%-20s %-20s %s\n", user, domain, ptr);
		}
		if (result && verbose)
			printf("No ATRN Maps present\n");
		break;
	default:
		usage();
		return(1);
		break;
	}
	return(result);
}

void
usage()
{
	printf("usage: vatrn [options] email|ATRNdomain\n");
	printf("options: -d ATRNdomain (delete mapping)\n");
	printf("         -i ATRNdomain (add    mapping)\n");
	printf("         -u ATRNdomain -i newATRNDomain (update mapping)\n");
	printf("         -s print mapping\n");
	printf("         -V print version number\n");
	printf("         -v verbose\n");

}

int
get_options(int argc, char **argv, int *Action, char *emailid, char *user, char *domain, char *domain_list, char *old_domain)
{
	int             c;

	/*- Action = PRINT_IT; -*/
	*Action = -1;
	*emailid = *user = *domain = *domain_list = 0;
	*old_domain = 0;
	while ((c = getopt(argc, argv, "vsd:i:u:n:")) != -1)
	{
		switch (c)
		{
		case 'v':
			verbose = 1;
			break;
		case 's':
			*Action = PRINT_IT;
			break;
		case 'i':
			if (*Action != UPDATE_IT)
				*Action = ADD_IT;
			else
				*Action = UPDATE_IT;
			scopy(domain_list, optarg, MAX_BUFF);
			break;
		case 'd':
			*Action = DELETE_IT;
			scopy(domain_list, optarg, MAX_BUFF);
			break;
		case 'u':
			*Action = UPDATE_IT;
			scopy(old_domain, optarg, MAX_BUFF);
			break;
		default:
			usage();
			return(1);
		}
	}
	if (*Action == -1)
	{
		fprintf(stderr, "vatrn: must specify one of -s, -i, -u and -i, -d options\n");
		usage();
		return(1);
	}
	if (optind < argc)
	{
		scopy(emailid, argv[optind++], MAX_BUFF);
		if (strchr(emailid, '@'))
		{
			if (parse_email(emailid, user, domain, MAX_BUFF))
			{
				fprintf(stderr, "%s: Email too long\n", emailid);
				return(1);
			}
			if (!*user || !*domain)
			{
				fprintf(stderr, "vatrn: Invalid Email address %s\n", emailid);
				usage();
				return(1);
			}
		} else
			scopy(domain, emailid, MAX_BUFF);
	}
	if (*Action != PRINT_IT && !*emailid)
	{
		fprintf(stderr, "must supply email address or domain\n");
		usage();
		return(1);
	}
	if ((*Action == ADD_IT || *Action == DELETE_IT || *Action == UPDATE_IT) && !*domain_list)
	{
		fprintf(stderr, "must supply argument for domain\n");
		usage();
		return(1);
	}
	return(0);
}

void
getversion_vatrn_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
