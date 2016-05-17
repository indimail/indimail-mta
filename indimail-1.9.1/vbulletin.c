/*
 * $Log: vbulletin.c,v $
 * Revision 2.17  2016-05-17 14:57:02+05:30  Cprogrammer
 * use control directory defined by configure
 *
 * Revision 2.16  2014-04-17 12:19:10+05:30  Cprogrammer
 * use setuser_privilege() to set uid, gid and supplementary group ids.
 *
 * Revision 2.15  2014-04-17 11:42:27+05:30  Cprogrammer
 * set supplementary group ids for indimail
 *
 * Revision 2.14  2009-12-30 13:11:18+05:30  Cprogrammer
 * run vbulletin with uid,gid of domain
 *
 * Revision 2.13  2009-02-18 21:35:04+05:30  Cprogrammer
 * check return value of fscanf, fwrite
 *
 * Revision 2.12  2008-09-14 19:50:56+05:30  Cprogrammer
 * do setuid / setgid only if not running with indimail perms
 *
 * Revision 2.11  2008-09-12 22:42:45+05:30  Cprogrammer
 * setuid, setgid to domains uid, gid
 *
 * Revision 2.10  2008-07-13 19:51:22+05:30  Cprogrammer
 * removed compilation warning on FC 9
 *
 * Revision 2.9  2008-06-25 15:40:07+05:30  Cprogrammer
 * removed sstrcmp and sstrncmp
 *
 * Revision 2.8  2008-05-28 16:40:03+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 2.7  2004-07-03 23:54:23+05:30  Cprogrammer
 * check return status of parse_email
 *
 * Revision 2.6  2004-05-17 14:02:21+05:30  Cprogrammer
 * changed VPOPMAIL to INDIMAIL
 *
 * Revision 2.5  2003-06-25 12:19:38+05:30  Cprogrammer
 * added option for domain wide bulk bulletin
 *
 * Revision 2.4  2003-02-27 23:56:24+05:30  Cprogrammer
 * major bug fixes
 *
 * Revision 2.3  2002-05-11 15:55:56+05:30  Cprogrammer
 * removed comments
 *
 * Revision 2.2  2002-04-12 15:52:06+05:30  Cprogrammer
 * replaced insert_bulletin() with bulletin()
 *
 * Revision 2.1  2002-04-11 23:25:09+05:30  Cprogrammer
 * added mysql bulletin option
 *
 * Revision 1.4  2001-12-08 17:46:15+05:30  Cprogrammer
 * usage message change
 *
 * Revision 1.3  2001-11-24 12:22:14+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.2  2001-11-20 11:01:34+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.1  2001-10-24 18:15:43+05:30  Cprogrammer
 * Initial revision
 */
#include "indimail.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <pwd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>

#ifndef	lint
static char     sccsid[] = "$Id: vbulletin.c,v 2.17 2016-05-17 14:57:02+05:30 Cprogrammer Exp mbhangui $";
#endif

#define COPY_IT          0
#define HARD_LINK_IT     1
#define SYMBOLIC_LINK_IT 2
#define BULK_BULLETIN    3
#define USER_BULLETIN    4
#define DOMAIN_BULLETIN  5

static char     CurDir[MAX_BUFF];
static int      Verbose;
static int      DoNothing;
static int      DeliveryMethod = COPY_IT;

int             process_domain(char *, char *, char *);
int             copy_email(char *, FILE *, char *, char *, struct passwd *);
int             in_exclude_list(FILE *, char *, char *);
static int      spost(char *, char *, int);
static int      get_options(int, char **, char *, char *, char *, char *, char *);
void            usage();

int
main(argc, argv)
	int             argc;
	char           *argv[];
{
	DIR            *entry;
	struct dirent  *dp;
	char            Domain[MAX_BUFF], Email[MAX_BUFF], Subscriber_list[MSG_BUF_SIZE];
	char            EmailFile[MAX_BUFF], ExcludeFile[MAX_BUFF];

	if (get_options(argc, argv, Email, Domain, EmailFile, ExcludeFile, Subscriber_list))
		return(1);
	if (!getcwd(CurDir, MAX_BUFF))
	{
		fprintf(stderr, "getcwd (size %d): %s", MAX_BUFF, strerror(errno));
		return(1);
	}
	if(DeliveryMethod == DOMAIN_BULLETIN)
	{
		if(DoNothing == 0)
			return(spost(Domain, EmailFile, 1));
	} else
	if(DeliveryMethod == BULK_BULLETIN)
	{
		if(DoNothing == 0)
			return(bulletin(EmailFile, Subscriber_list) == -1 ? 1 : 0);
	}
	if(DeliveryMethod == USER_BULLETIN)
	{
		if(DoNothing == 0)
			return(spost(Email, EmailFile, 0));
	}
	if (*Domain)
		return(process_domain(EmailFile, ExcludeFile, Domain));
	else
	{
		if (chdir(INDIMAILDIR) != 0)
		{
			printf("can not change dir %s\n", INDIMAILDIR);
			return(1);
		}
		if (chdir("domains") != 0)
		{
			printf("could change dir into domains dir\n");
			return(1);
		}
		if(!(entry = opendir(".")))
		{
			perror("vmysqlconv: opendir failed");
			return (1);
		}
		while ((dp = readdir(entry)) != NULL)
		{
			if (strncmp(dp->d_name, ".", 1) == 0)
				continue;
			if(process_domain(EmailFile, ExcludeFile, dp->d_name))
				fprintf(stderr, "%s: Bulletin failed\n", dp->d_name);
		}
		closedir(entry);
	}
	vclose();
	return (0);
}

static int
get_options(int argc, char **argv, char *Email, char *Domain, char *EmailFile, char *ExcludeFile, char *Subscriber_list)
{
	int             c;

	*Email = *Domain = *EmailFile = *ExcludeFile = *Subscriber_list = 0;
	Verbose = 0;
	DoNothing = 0;
	while ((c = getopt(argc, argv, "VcshnaS:f:e:")) != -1)
	{
		switch (c)
		{
		case 'V':
			Verbose = 1;
			break;
		case 'c':
			if(DeliveryMethod != BULK_BULLETIN && DeliveryMethod != DOMAIN_BULLETIN)
				DeliveryMethod = COPY_IT;
			break;
		case 's':
			if(DeliveryMethod != BULK_BULLETIN && DeliveryMethod != DOMAIN_BULLETIN)
				DeliveryMethod = SYMBOLIC_LINK_IT;
			break;
		case 'h':
			if(DeliveryMethod != BULK_BULLETIN && DeliveryMethod != DOMAIN_BULLETIN)
				DeliveryMethod = HARD_LINK_IT;
			break;
		case 'n':
			DoNothing = 1;
			break;
		case 'f':
			scopy(EmailFile, optarg, MAX_BUFF);
			break;
		case 'a':
			DeliveryMethod = DOMAIN_BULLETIN;
			break;
		case 'S':
			DeliveryMethod = BULK_BULLETIN;
			if(!*optarg)
			{
				usage();
				return(1);
			}
			if(access(optarg, F_OK))
			{
				perror(optarg);
				return(1);
			}
			scopy(Subscriber_list, optarg, MAX_BUFF);
			break;
		case 'e':
			if(!*optarg)
			{
				usage();
				return(1);
			}
			if(access(optarg, F_OK))
			{
				perror(optarg);
				return(1);
			}
			scopy(ExcludeFile, optarg, MAX_BUFF);
			break;
		default:
			usage();
			return(1);
		}
	}
	if (optind < argc)
	{
		if(strchr(argv[optind], '@'))
		{
			DeliveryMethod = USER_BULLETIN;
			scopy(Email, argv[optind++], MAX_BUFF);
		} else
			scopy(Domain, argv[optind++], MAX_BUFF);
	}
	if(DeliveryMethod == BULK_BULLETIN || DeliveryMethod == DOMAIN_BULLETIN)
	{
		if(*ExcludeFile || *Email)
		{
			fprintf(stderr, "options -e or email cannot be used with bulk bulletin\n");
			usage();
			return(1);
		}
	}
	if(DeliveryMethod == USER_BULLETIN)
	{
		if(*ExcludeFile)
		{
			fprintf(stderr, "option -e cannot be used with user bulletin\n");
			usage();
			return(1);
		}
	}
	if(!*Email && !*Domain)
	{
		usage();
		return(1);
	}
	return(0);
}

int
process_domain(EmailFile, ExcludeFile, domain)
	char           *EmailFile, *ExcludeFile, *domain;
{
	char            filename[MAX_BUFF], hostname[MAX_BUFF], Dir[MAX_BUFF];
	static FILE    *fsi, *fsx;
	static struct passwd *pwent;
	time_t          tm;
	int             pid, first = 1;
	uid_t           uid, myuid;
	gid_t           gid;

	if (!fsi && !(fsi = fopen(EmailFile, "r")))
	{
		perror(EmailFile);
		return(1);
	}
	if (*ExcludeFile && !fsx && !(fsx = fopen(ExcludeFile, "r")))
	{
		perror(ExcludeFile);
		return(1);
	}
	gethostname(hostname, sizeof(hostname));
	pid = getpid();
	time(&tm);
	if(!vget_assign(domain, Dir, MAX_BUFF, &uid, &gid))
	{
		fprintf(stderr, "No such Domain %s\n", domain);
		return (-1);
	}
	myuid = getuid();
	if (myuid != 0 && myuid != uid)
	{
		error_stack(stderr, "you must be root or domain user (uid=%d) to run this program\n", uid);
		return(1);
	}
	if (setuser_privileges(uid, gid, "indimail"))
	{
		error_stack(stderr, "setuser_privileges: (%d/%d): %s", uid, gid, strerror(errno));
		return (1);
	}
	snprintf(filename, MAX_BUFF, "%lu.%d.%s", tm, pid, hostname);
	first = 1;
	while ((pwent = vauth_getall(domain, first, 0)) != NULL)
	{
		first = 0;
		if (!in_exclude_list(fsx, domain, pwent->pw_name))
		{
			if (Verbose == 1)
				printf("%s@%s\n", pwent->pw_name, domain);
			if (DoNothing == 0)
				copy_email(EmailFile, fsi, filename, domain, pwent);
		}
	}
	if (chdir(".."));
	return (0);
}

int
copy_email(EmailFile, fs_file, name, domain, pwent)
	char           *EmailFile;
	FILE           *fs_file;
	char           *name;
	char           *domain;
	struct passwd  *pwent;
{
	static char     tmpbuf[MAX_BUFF];
	static char     tmpbuf1[MAX_BUFF];
	static char     MsgBuf[MSG_BUF_SIZE];
	FILE           *fs;
	int             count;

	snprintf(tmpbuf, MAX_BUFF, "%s/Maildir/new/%s", pwent->pw_dir, name);
	if (DeliveryMethod == COPY_IT)
	{
		rewind(fs_file);
		if ((fs = fopen(tmpbuf, "w+")) == NULL)
			return (1);
		fprintf(fs, "To: %s@%s\n", pwent->pw_name, domain);
		while ((count = fread(MsgBuf, sizeof(char), MSG_BUF_SIZE, fs_file)) != 0)
		{
			if (fwrite(MsgBuf, sizeof(char), count, fs) != count)
			{
				fprintf(stderr, "fwrite: %d: %s\n", count, strerror(errno));
				fclose(fs);
				return (1);
			}
		}
		fclose(fs);
	} else
	if (DeliveryMethod == HARD_LINK_IT)
	{
		if(*EmailFile == '/')
			snprintf(tmpbuf1, MAX_BUFF, "%s", EmailFile);
		else
			snprintf(tmpbuf1, MAX_BUFF, "%s/%s", CurDir, EmailFile);
		if (link(tmpbuf1, tmpbuf) < 0)
			fprintf(stderr, "link: %s->%s: %s\n", tmpbuf1, tmpbuf, strerror(errno));
	} else
	if (DeliveryMethod == SYMBOLIC_LINK_IT)
	{
		if(*EmailFile == '/')
			snprintf(tmpbuf1, MAX_BUFF, "%s", EmailFile);
		else
			snprintf(tmpbuf1, MAX_BUFF, "%s/%s", CurDir, EmailFile);
		if (symlink(tmpbuf1, tmpbuf) < 0)
			perror(tmpbuf);
	} else
		printf("no delivery method set\n");
	return (0);
}

int
in_exclude_list(FILE * fsx, char *domain, char *user)
{
	static char     tmpbuf[512];
	static char     emailaddr[512];
	int             i;

	if (fsx == NULL)
		return (0);
	rewind(fsx);
	snprintf(emailaddr, 512, "%s@%s", user, domain);
	while (fgets(tmpbuf, 512, fsx) != NULL)
	{
		for (i = 0; tmpbuf[i] != 0; ++i)
			if (tmpbuf[i] == '\n')
				tmpbuf[i] = 0;
		if (strcmp(tmpbuf, emailaddr) == 0)
			return (1);
	}
	return (0);
}

static int
spost(char *EmailOrDomain, char *Filename, int bulk)
{
	char            bulkdir[MAX_BUFF], TmpBuf[MAX_BUFF], User[MAX_BUFF], Domain[MAX_BUFF];
	char           *ptr1, *ptr2, *domain = 0;
	int             copy_method;
	uid_t           uid;
	gid_t           gid;
	struct stat     statbuf;
	struct passwd  *mypw;

	if (!bulk)
	{
		if (parse_email(EmailOrDomain, User, Domain, MAX_BUFF))
		{
			fprintf(stderr, "%s: Email too long\n", EmailOrDomain);
			return(1);
		}
		printf("Email %s User %s Domain %s\n", EmailOrDomain, User, Domain);
		if(!*User || !*Domain)
		{
			fprintf(stderr, "Invalid Email Address\n");
			return(1);
		}
	} else
		scopy(Domain, EmailOrDomain, MAX_BUFF);
	if (!(domain = vget_real_domain(Domain)))
	{
		fprintf(stderr, "%s: No such domain\n", Domain);
		return (1);
	}
	snprintf(bulkdir, MAX_BUFF, "%s/%s/%s", CONTROLDIR, domain, (ptr1 = getenv("BULK_MAILDIR")) ? ptr1 : BULK_MAILDIR);
	if (access(bulkdir, F_OK))
	{
		fprintf(stderr, "spost: access: %s: %s\n", bulkdir, strerror(errno));
		return (1);
	}
	if ((ptr2 = strrchr(Filename, '/')))
		ptr2++;
	else
		ptr2 = Filename;
	if(bulk)
		snprintf(TmpBuf, MAX_BUFF, "%s/%s,all", bulkdir, ptr2);
	else
		snprintf(TmpBuf, MAX_BUFF, "%s/%s", bulkdir, ptr2);
	if (access(Filename, F_OK))
	{
		fprintf(stderr, "spost: access: %s: %s\n", Filename, strerror(errno));
		return (1);
	} else
	if (!access(TmpBuf, F_OK))
	{
		errno = EEXIST;
		perror(TmpBuf);
		return (-1);
	}
	if(!vget_assign(domain, NULL, 0, &uid, &gid))
	{
		fprintf(stderr, "Domain %s exists\n", domain);
		return (1);
	}
	if (fappend(Filename, TmpBuf, "w", INDIMAIL_QMAIL_MODE, uid, gid))
	{
		fprintf(stderr, "fappend: %s->%s: %s\n", Filename, TmpBuf, strerror(errno));
		return (-1);
	}
	if (stat(TmpBuf, &statbuf))
	{
		fprintf(stderr, "spost: stat: %s: %s\n", TmpBuf, strerror(errno));
		return (1);
	}
	if (bulk)
		return(0);
	if (setuser_privileges(uid, gid, "indimail"))
	{
		error_stack(stderr, "setuser_privilege: (%d/%d): %s", uid, gid, strerror(errno));
		fprintf(stderr, "setuid/setgid (%d/%d): %s\n", uid, gid, strerror(errno));
		return (-1);
	}
	if (vauth_open((char *) 0))
		return (1);
	if (!(mypw = vauth_getpw(User, domain)))
	{
		if(userNotFound)
			fprintf(stderr, "%s: No such User\n", EmailOrDomain);
		return(1);
	}
	if(DeliveryMethod == SYMBOLIC_LINK_IT)
		copy_method = 1;
	else
		copy_method = 0;
	if(CopyEmailFile(mypw->pw_dir, TmpBuf, EmailOrDomain, EmailOrDomain, 0, 0, 1, copy_method, statbuf.st_size))
	{
		vclose();
		return(1);
	}
	vclose();
	return (0);
}

void
usage()
{
	printf("usage: vbulletin -f email_file [-e exclude_email_addr_file]\n");
	printf("                               [-v (verbose)]\n");
	printf("                               [-n (don't mail)]\n");
	printf("                               [-c (default, copy file)]\n");
	printf("                               [-h (use hard links)]\n");
	printf("                               [-s (use symbolic links)]\n");
	printf("                               [-S Subsriber_list_file (use Instant Mysql Bulletin)]\n");
	printf("                               [-a (Instant Bulletin for entire domain)]\n");
	printf("                               [ virtual_domain... | Email] \n");
}

void
getversion_vbulletin_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
