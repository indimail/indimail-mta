/*
 * $Log: vcfilter.c,v $
 * Revision 2.32  2017-05-01 20:15:54+05:30  Cprogrammer
 * removed mailing list feature from vfilter
 *
 * Revision 2.31  2016-06-09 15:32:32+05:30  Cprogrammer
 * run if indimail gid is present in process supplementary groups
 *
 * Revision 2.30  2016-06-09 14:22:32+05:30  Cprogrammer
 * allow privilege to process running with indimail gid
 *
 * Revision 2.29  2014-04-17 11:42:30+05:30  Cprogrammer
 * set supplementary group ids for indimail
 *
 * Revision 2.28  2013-10-11 01:14:40+05:30  Cprogrammer
 * BUG - fixed not able to create filters for prefilt and postfilt
 *
 * Revision 2.27  2011-06-20 21:27:47+05:30  Cprogrammer
 * added description on |program
 *
 * Revision 2.26  2010-08-16 21:14:25+05:30  Cprogrammer
 * skip creating Maildir/vfilter file for prefilt & postfilt users
 *
 * Revision 2.25  2010-05-01 14:14:39+05:30  Cprogrammer
 * added -C option to connect to cluster
 *
 * Revision 2.24  2009-12-30 13:12:11+05:30  Cprogrammer
 * run vcfilter with uid,gid of domain
 *
 * Revision 2.23  2009-02-18 09:08:30+05:30  Cprogrammer
 * check chown error
 *
 * Revision 2.22  2009-02-06 11:40:50+05:30  Cprogrammer
 * ignore return value of chown
 *
 * Revision 2.21  2008-05-28 16:40:09+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 2.20  2004-07-03 23:54:43+05:30  Cprogrammer
 * use parse_email
 *
 * Revision 2.19  2004-07-02 18:11:43+05:30  Cprogrammer
 * renamed .vfilter to vfilter
 *
 * Revision 2.18  2004-04-08 13:06:34+05:30  Cprogrammer
 * remove white spaces for numerical comparisions
 *
 * Revision 2.17  2003-12-30 00:31:02+05:30  Cprogrammer
 * use headerlist() to get standard mail header list
 *
 * Revision 2.16  2003-03-26 11:01:56+05:30  Cprogrammer
 * added option to allow programs to be called as the forwarding address
 *
 * Revision 2.15  2002-12-06 21:20:48+05:30  Cprogrammer
 * bug fix - Copy folder is /NoDeliver instead of ./NoDeliver
 *
 * Revision 2.14  2002-12-05 17:44:38+05:30  Cprogrammer
 * forwarding address was not set
 *
 * Revision 2.13  2002-11-24 15:33:37+05:30  Cprogrammer
 * folder was not being initialized correctly
 *
 * Revision 2.12  2002-11-24 12:46:07+05:30  Cprogrammer
 * changed position of filter_name item in usage
 *
 * Revision 2.11  2002-11-18 12:42:20+05:30  Cprogrammer
 * added option to display the result in raw format
 *
 * Revision 2.10  2002-11-13 13:38:10+05:30  Cprogrammer
 * added filter name
 *
 * Revision 2.9  2002-10-18 01:18:42+05:30  Cprogrammer
 * use the array rfc_ids[] to check for mandatory RFC821 ids
 *
 * Revision 2.8  2002-10-16 20:02:38+05:30  Cprogrammer
 * added option to bounce and forward at the same time
 * added -f /NoDeliver option for junking mail
 * return value of mlist_filterno() now correctly interpreted
 *
 * Revision 2.7  2002-10-15 11:42:54+05:30  Cprogrammer
 * allow changing of mailing list option based on comparision rather than on filter_no
 *
 * Revision 2.6  2002-10-14 20:59:51+05:30  Cprogrammer
 * extensive rewrite
 * added option to forward mails on filter match
 * added option MLIST_OPTION to alter match for mailing lists
 *
 * Revision 2.5  2002-10-13 16:36:26+05:30  Cprogrammer
 * added bounds checks on header_name and comparision values
 *
 * Revision 2.4  2002-10-12 02:38:07+05:30  Cprogrammer
 * corrected vfilter_header going out of bounds
 *
 * Revision 2.3  2002-10-11 20:03:32+05:30  Cprogrammer
 * extensive code change for mailing list functionality
 * display of filters moved to function vfilter_display()
 *
 * Revision 2.2  2002-10-11 00:05:55+05:30  Cprogrammer
 * increased size of comparision display by 1 byte
 *
 * Revision 2.1  2002-10-10 03:34:48+05:30  Cprogrammer
 * filter administration utility
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: vcfilter.c,v 2.32 2017-05-01 20:15:54+05:30 Cprogrammer Exp mbhangui $";
#endif

#ifdef VFILTER
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#define FILTER_SELECT 0
#define FILTER_INSERT 1
#define FILTER_DELETE 2
#define FILTER_UPDATE 3

int             header_name;
int             comparision;
int             bounce_action;
int             filter_no;
char            filter_name[MAX_BUFF];
char            keyword[MAX_BUFF];
char            folder[MAX_BUFF];
char            emailid[AUTH_SIZE];
char            faddr[AUTH_SIZE];
char          **header_list;
char           *ptr1, *ptr2;
int             FilterAction;

void            usage();
int             get_options(int argc, char **argv, int *, int *);

int
main(int argc, char **argv)
{
	int             i, status = -1, raw = 0, cluster_conn = 0;
	uid_t           uid, uidtmp;
	gid_t           gid, gidtmp;
	struct passwd  *pw;
	char           *real_domain;
	char            user[AUTH_SIZE], domain[AUTH_SIZE], vfilter_file[MAX_BUFF],
					buffer[AUTH_SIZE];

	if (get_options(argc, argv, &raw, &cluster_conn))
		return(1);
	if (parse_email(emailid, user, domain, AUTH_SIZE))
	{
		fprintf(stderr, "%s: Email too long\n", emailid);
		return (1);
	}
	if (!(real_domain = vget_real_domain(domain)))
		real_domain = domain;
	if (!vget_assign(real_domain, buffer, AUTH_SIZE, &uid, &gid))
	{
		error_stack(stderr, "%s: domain does not exist\n", real_domain);
		return (1);
	}
	if (cluster_conn && vauthOpen_user(emailid, 0) == -1)
	{
		if (userNotFound)
		{
			fprintf(stderr, "%s: No such user\n", emailid);
			return(1);
		} else
		{
			fprintf(stderr, "Temporary database Errror\n");
			return(1);
		}
	}
	if (FilterAction != FILTER_SELECT)
	{
		uidtmp = getuid();
		gidtmp = getgid();
		if (uidtmp != 0 && uidtmp != uid && gidtmp != gid && check_group(gid) != 1)
		{
			error_stack(stderr, "you must be root or domain user (uid=%d/gid=%d) to run this program\n", uid, gid);
			if (cluster_conn)
				vclose();
			return(1);
		}
		if (setuser_privileges(uid, gid, "indimail"))
		{
			error_stack(stderr, "setuser_privileges: (%d/%d): %s", uid, gid, strerror(errno));
			if (cluster_conn)
				vclose();
			return(1);
		}
	}
	if (FilterAction == FILTER_INSERT || FilterAction == FILTER_DELETE)
	{
		if (!(pw = vauth_getpw(user, real_domain)))
		{
			if (userNotFound)
				fprintf(stderr, "%s@%s: No such user\n", user, real_domain);
			else
				fprintf(stderr, "Temporary database Errror\n");
			if (cluster_conn)
				vclose();
			return(1);
		}
		snprintf(vfilter_file, sizeof(vfilter_file), "%s/Maildir/vfilter", pw->pw_dir);
	}
	switch (FilterAction)
	{
	case FILTER_SELECT:
		status = vfilter_display(emailid, raw, &filter_no, filter_name, &header_name, &comparision, keyword, folder, &bounce_action, faddr);
		break;
	case FILTER_INSERT:
		status = vfilter_insert(emailid, filter_name, header_name, comparision, keyword, folder, bounce_action, faddr);
		if (!status && strncmp(emailid, "prefilt@", 8) && strncmp(emailid, "postfilt@", 9)
			&& access(vfilter_file, F_OK))
		{
			if ((i = open(vfilter_file, O_CREAT | O_TRUNC, 0644)) == -1)
			{
				fprintf(stderr, "open: %s: %s\n", vfilter_file, strerror(errno));
				status = -1;
			} else
			{
				close(i);
				if (chown(vfilter_file, uid, gid))
				{
					fprintf(stderr, "chown: %s: %s\n", vfilter_file, strerror(errno));
					status = -1;
				}
			}
		}
		break;
	case FILTER_DELETE:
		if (!(status = vfilter_delete(emailid, filter_no)))
		{
			if (vfilter_select(emailid, &i, filter_name, &header_name, &comparision, keyword, folder, &bounce_action, faddr) == -2)
				unlink(vfilter_file);
		}
		break;
	case FILTER_UPDATE:
		status = vfilter_update(emailid, filter_no, header_name, comparision, keyword, folder, bounce_action, faddr);
		break;
	}
#ifdef DEBUG
	printf("action %d, header %d keyword [%s] comparision %d folder [%s] bounce_action %d Forward %s email [%s]\n",
			FilterAction, header_name, keyword, comparision, folder, bounce_action, bounce_action == 2 ? faddr : "N/A", emailid);
#endif
	if (cluster_conn)
		vclose();
	return(status);
}

int
get_options(int argc, char **argv, int *raw, int *cluster_conn)
{
	int             i, c, max_header, max_comparision;
	char           *ptr;

	filter_no = header_name = comparision = bounce_action = -1;
	memset(filter_name, 0, MAX_BUFF);
	memset(keyword, 0, MAX_BUFF);
	memset(folder, 0, MAX_BUFF);
	FilterAction = -1;
	
	if (!(header_list = headerList()))
		header_list = vfilter_header;
	for (max_header = 0;header_list[max_header];max_header++);
	for (max_comparision = 0;vfilter_comparision[max_comparision];max_comparision++);
	max_header--;
	max_comparision--;
	*cluster_conn = 0;
	while ((c = getopt(argc, argv, "vCsird:u:h:c:b:k:f:t:")) != -1)
	{
		switch (c)
		{
		case 'v':
			verbose = 1;
			break;
#ifdef CLUSTERED_SITE
		case 'C':
			*cluster_conn = 1;
			break;
#endif
		case 's':
			FilterAction = FILTER_SELECT;
			break;
		case 'r':
			*raw = 1;
			break;
		case 'i':
			FilterAction = FILTER_INSERT;
			break;
		case 'd':
			FilterAction = FILTER_DELETE;
			if (isnum(optarg))
				filter_no = atoi(optarg);
			else
				filter_no = -1;
			break;
		case 'u':
			FilterAction = FILTER_UPDATE;
			if (isnum(optarg))
				filter_no = atoi(optarg);
			else
				filter_no = -1;
			break;
		case 'h':
			if (isnum(optarg))
			{
				header_name = atoi(optarg);
				if (header_name < 0 || header_name > max_header)
				{
					fprintf(stderr, "Header value %d out of range\n", header_name);
					usage();
					return(1);
				}
			} else
				header_name = -1;
			break;
		case 'c':
			if (isnum(optarg))
			{
				comparision = atoi(optarg);
				if (comparision < 0 || comparision > max_comparision)
				{
					fprintf(stderr, "Comparision value %d out of range\n", comparision);
					usage();
					return(1);
				}
			} else
				comparision = -1;
			break;
		case 'b':
			if (isdigit((int) *optarg))
			{
				bounce_action = *optarg - '0';
				if (bounce_action != 0 && bounce_action != 1 && bounce_action != 2 && bounce_action != 3)
				{
					fprintf(stderr, "Invalid Bounce action %d\n", bounce_action);
					usage();
					return(1);
				} else
				if (bounce_action == 2 || bounce_action == 3)
				{
					if ((optarg[1] && optarg[1] != '&' && optarg[1] != '|') || !optarg[1] || !optarg[2])
					{
						fprintf(stderr, "forwarding address incorrect or not specified\n");
						usage();
						return(1);
					}
					scopy(faddr, optarg + 1, AUTH_SIZE);
				}
			} else
				bounce_action = -1;
			break;
		case 't':
			scopy(filter_name, optarg, MAX_BUFF);
			break;
		case 'k':
			scopy(keyword, optarg, MAX_BUFF);
			if (comparision == 7)
			{
				for (ptr1 = keyword, ptr2 = keyword; *ptr1; ptr1++)
				{
					if (!isspace((int) *ptr1))
						*ptr2++ = *ptr1;
				}
				*ptr2 = 0;
				/* Should call evaluate() here and test */
			}
			break;
		case 'f':
			scopy(folder, optarg, MAX_BUFF);
			if (*folder == '.')
			{
				fprintf(stderr, "%s: Folder Name cannot start with '%c'\n", folder, *folder);
				*folder = 0;
				return(1);
			} else
			if (strncasecmp(folder, "/NoDeliver", 10))
			{
				for (ptr = folder;*ptr;ptr++)
				{
					if (*ptr == '/')
					{
						fprintf(stderr, "%s: Invalid Char %c\n", folder, *ptr);
						*folder = 0;
						return(1);
					}
				}
			}
			if (strncasecmp(folder, "/NoDeliver", 11))
				snprintf(folder, MAX_BUFF, ".%s", optarg);
			else
				scopy(folder, "/NoDeliver", 11);
			break;
		default:
			usage();
			return(1);
		}
	} /*- while ((c = getopt(argc, argv, "vCsirm:d:u:h:c:b:k:f:o:DU")) != -1) */
	if (FilterAction == -1)
	{
		fprintf(stderr, "Must specify one of -s -i, -u, -d, -m option\n");
		usage();
		return(1);
	}
	if (optind < argc)
		scopy(emailid, argv[optind++], AUTH_SIZE);
	if (!*emailid)
	{
		fprintf(stderr, "vcfilter: must supply emailid\n");
		usage();
		return(1);
	} else
	{
		for (i = 0;rfc_ids[i];i++)
		{
			if (!strncmp(emailid, "prefilt", 7) || !strncmp(emailid, "postfilt", 8))
				continue;
			if (!strncmp(emailid, rfc_ids[i], slen(rfc_ids[i])))
			{
				fprintf(stderr, "vcfilter: email %s not allowed for filtering\n", emailid);
				usage();
				return(1);
			}
		}
	}
	if (comparision == 5 || comparision == 6)
	{
		header_name = -1;
		*keyword = 0;
	}
	switch(FilterAction)
	{
	case FILTER_DELETE:
	case FILTER_UPDATE:
	case FILTER_INSERT:
		if (FilterAction == FILTER_INSERT && !*filter_name)
		{
			fprintf(stderr, "filter name not specified\n");
			usage();
			return(1);
		}
		if ((FilterAction == FILTER_UPDATE || FilterAction == FILTER_DELETE) && filter_no == -1)
		{
			fprintf(stderr, "filter no not specified or invalid\n");
			usage();
			return(1);
		}
		if (FilterAction == FILTER_DELETE)
			break;
		if (comparision == -1 || bounce_action == -1 || !*folder)
		{
			if (comparision == -1)
				fprintf(stderr, "-c option not specified\n");
			if (bounce_action == -1)
				fprintf(stderr, "-b option not specified\n");
			if (!*folder)
				fprintf(stderr, "-f option not specified\n");
			usage();
			return(1);
		}
		if (comparision != 5 && comparision != 6)
		{
			if (header_name == -1 || !*keyword)
			{
				if (header_name == -1)
					fprintf(stderr, "-h option not specified\n");
				if (!*keyword)
					fprintf(stderr, "-k option not specified\n");
				usage();
				return(1);
			}
		}
		break;
	} /*- switch(FilterAction) */
	return(0);
}

void
usage()
{
	int             i;

	fprintf(stderr, "usage: vcfilter [options] emailid\n");
	fprintf(stderr, "options: -v verbose\n");
	fprintf(stderr, "         -C connect to Cluster\n");
	fprintf(stderr, "         -r raw display (for -s option)\n");
	fprintf(stderr, "         -s show filter(s)\n");
	fprintf(stderr, "         -i add filter\n");
	fprintf(stderr, "         -d filter_no delete filter(s)\n");
	fprintf(stderr, "         -u filter_no update filter\n");
	fprintf(stderr, "         -t Filter Name (textual description of filter)\n");
	fprintf(stderr, "         -h header value\n");
	fprintf(stderr, "            %3d - %-25s\n", -1, "If comparision (-c option) is 5 or 6");
	for (i = 0;header_list[i];)
	{
		if (header_list[i + 1])
		{
			fprintf(stderr, "            %3d - %-25s          %3d - %-25s\n", i, header_list[i], i + 1, header_list[i + 1]);
			i += 2;
		}
		else
		{
			fprintf(stderr, "            %3d - %-25s\n", i, header_list[i]);
			i++;
		}
	}
	fprintf(stderr, "         -c comparision\n");
	for (i = 0;vfilter_comparision[i];)
	{
		if (vfilter_comparision[i + 1])
		{
			fprintf(stderr, "            %3d - %-25s          %3d - %-25s\n", i, vfilter_comparision[i],
				i + 1, vfilter_comparision[i + 1]);
			i += 2;
		}
		else
		{
			fprintf(stderr, "            %3d - %-25s\n", i, vfilter_comparision[i]);
			i++;
		}
	}
	fprintf(stderr, "         -k keyword [\"\" string if comparision (-c option) is 5 or 6]\n");
	fprintf(stderr, "         -f folder [Specify /NoDeliver for delivery to be junked]\n");
	fprintf(stderr, "         -b bounce action\n");
	fprintf(stderr, "              0               - Do not Bounce to sender\n");
	fprintf(stderr, "              1               - Bounce to sender\n");
	fprintf(stderr, "              2'&user@domain' - Forward to another id\n");
	fprintf(stderr, "              2'|program'     - Feed mail to another program\n");
	fprintf(stderr, "              3'&user@domain' - Forward to another id and Bounce\n");
	fprintf(stderr, "              3'|program'     - Feed mail to another program and Bounce\n");
}
#else
int
main()
{
	printf("IndiMail not configured with --enable-vfilter=y\n");
	return(0);
}
#endif

void
getversion_vcfilter_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
