/*
 * $Log: vmoddomain.c,v $
 * Revision 2.10  2017-03-13 14:13:51+05:30  Cprogrammer
 * replaced INDIMAILDIR with PREFIX
 *
 * Revision 2.9  2016-06-09 15:32:32+05:30  Cprogrammer
 * run if indimail gid is present in process supplementary groups
 *
 * Revision 2.8  2016-06-09 14:22:47+05:30  Cprogrammer
 * allow privilege to process running with indimail gid
 *
 * Revision 2.7  2011-12-24 08:49:17+05:30  Cprogrammer
 * display a more helpful usage
 *
 * Revision 2.6  2011-11-09 19:46:21+05:30  Cprogrammer
 * removed getversion
 *
 * Revision 2.5  2010-08-16 21:12:35+05:30  Cprogrammer
 * fixed usage & setting of handler
 *
 * Revision 2.4  2010-04-24 09:31:01+05:30  Cprogrammer
 * description now specifies SMTPROUTE/QMTPROUTE
 *
 * Revision 2.3  2009-12-02 11:05:33+05:30  Cprogrammer
 * added option to turn on and off domain limits
 *
 * Revision 2.2  2009-03-08 11:42:38+05:30  Cprogrammer
 * made vmoddomain setuid
 *
 * Revision 2.1  2009-01-28 11:25:17+05:30  Cprogrammer
 * program to modify .qmail-default
 *
 */
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: vmoddomain.c,v 2.10 2017-03-13 14:13:51+05:30 Cprogrammer Exp mbhangui $";
#endif

static void     usage();
static int      get_options(int argc, char **argv, int *, int *, char **, char **);
int             set_handler(char *, char *, uid_t, gid_t, int);
int             set_domain_limits(char *, uid_t, gid_t, int);

int
main(int argc, char **argv)
{

	char           *domain = 0, *handler = 0;
	char            TheDir[AUTH_SIZE];
	int             use_vfilter, domain_limits;
	uid_t           uid, myuid;
	gid_t           gid, mygid;

	if (get_options(argc, argv, &use_vfilter, &domain_limits, &handler, &domain))
		return (1);
	if (!vget_assign(domain, TheDir, AUTH_SIZE, &uid, &gid))
	{
		error_stack(stderr, "%s: domain does not exist\n", domain);
		return (1);
	}
	if (indimailuid == -1 || indimailgid == -1)
		GetIndiId(&indimailuid, &indimailgid);
	myuid = getuid();
	mygid = getgid();
	if (myuid != 0 && myuid != indimailuid && mygid != indimailgid && check_group(indimailgid) != 1)
	{
		error_stack(stderr, "you must be root or indimail to run this program\n");
		return(1);
	}
	if (myuid & setuid(0))
	{
		error_stack(stderr, "setuid: %s\n", strerror(errno));
		return(1);
	}
	if (access(TheDir, F_OK))
	{
		if (r_mkdir(TheDir, INDIMAIL_DIR_MODE, uid, gid))
		{
			error_stack(stderr, "r_mkdir: %s: %s\n", TheDir, strerror(errno));
			return (1);
		}
	}
	if (chdir(TheDir))
	{
		error_stack(stderr, "chdir: %s: %s\n", TheDir, strerror(errno));
		return (1);
	}
	umask(INDIMAIL_UMASK);
	if (handler && set_handler(TheDir, handler, uid, gid, use_vfilter))
		return (1);
	if ((domain_limits == 0 || domain_limits == 1) && 
		set_domain_limits(TheDir, uid, gid, domain_limits))
		return (1);
	return (0);
}

int
set_handler(char *dir, char *handler, uid_t uid, gid_t gid, int use_vfilter)
{
	char            tmpbuf[MAX_BUFF];
	int             fd;
#ifdef FILE_LOCKING
	int             lockfd;
#endif

	if (!handler)
		return (0);
	/* Last case: the last parameter is a Maildir, an email address, ipaddress or hostname */
	if (strncmp(handler, BOUNCE_ALL, AUTH_SIZE) && strncmp(handler, DELETE_ALL, AUTH_SIZE))
	{
		if (*handler == '/')
		{
			if (chdir (handler))
			{
				error_stack(stderr, "chdir: %s: %s\n", handler, strerror(errno));
				return (1);
			}
			if (access("new", F_OK) || access("cur", F_OK) || access("tmp", F_OK))
			{
				error_stack(stderr, "%s not a Maildir\n", handler);
				return (1);
			}
		} else
		{
			if (!strchr(handler, '@')) /*- IP Address */
			{
				char           *ptr;
				int             i;
				for (i = 0, ptr = handler;*ptr;ptr++)
				{
					if (*ptr == ':')
						i++;
				}
				if (i != 2)
				{
					error_stack(stderr, "Invalid SMTPROUTE/QMTPROUTE Specification [%s]\n", handler);
					return (1);
				}
			}
		}
	}
	snprintf(tmpbuf, MAX_BUFF, "%s/.qMail-default", dir);
#ifdef FILE_LOCKING
	if ((lockfd = getDbLock(tmpbuf, 1)) == -1)
	{
		error_stack(stderr, "get_write_lock: %s: %s\n", tmpbuf, strerror(errno));
		return (1);
	}
#endif
	if ((fd = open(tmpbuf, O_CREAT|O_TRUNC|O_WRONLY, 0400)) == -1)
	{
		error_stack(stderr, "open: %s: %s\n", tmpbuf, strerror(errno));
#ifdef FILE_LOCKING
		delDbLock(lockfd, tmpbuf, 1);
#endif
		return (1);
	}
	if (fchown(fd, uid, gid))
	{
		error_stack(stderr, "chown: %s: (uid %d: gid %d): %s\n", tmpbuf, uid, gid,
			strerror(errno));
		close(fd);
		unlink(tmpbuf);
#ifdef FILE_LOCKING
		delDbLock(lockfd, tmpbuf, 1);
#endif
		return (1);
	}
#ifdef VFILTER
	if(use_vfilter == 1)
		filewrt(fd, "| %s/sbin/vfilter '' %s\n", PREFIX, handler);
	else
#endif
		filewrt(fd, "| %s/sbin/vdelivermail '' %s\n", PREFIX, handler);
	close(fd);
	if (rename(tmpbuf, ".qmail-default"))
	{
		error_stack(stderr, "rename: %s->.qmail-default: %s\n", tmpbuf, strerror(errno));
		unlink(tmpbuf);
#ifdef FILE_LOCKING
		delDbLock(lockfd, tmpbuf, 1);
#endif
		return (1);
	}
#ifdef FILE_LOCKING
	delDbLock(lockfd, tmpbuf, 1);
#endif
	return(0);
}

int
set_domain_limits(char *dir, uid_t uid, gid_t gid, int domain_limits)
{
	char            tmpbuf[MAX_BUFF];
	int             fd;

	snprintf(tmpbuf, MAX_BUFF, "%s/.domain_limits", dir);
	if (!domain_limits)
	{
		if (unlink(tmpbuf))
		{
			error_stack(stderr, "unlink: %s: %s\n", tmpbuf, strerror(errno));
			return (1);
		}
		return (0);
	}
	if ((fd = open(tmpbuf, O_CREAT|O_TRUNC|O_WRONLY, 0400)) == -1)
	{
		error_stack(stderr, "open: %s: %s\n", tmpbuf, strerror(errno));
		return (1);
	}
	if (fchown(fd, uid, gid))
	{
		error_stack(stderr, "chown: %s: (uid %d: gid %d): %s\n", tmpbuf, uid, gid,
			strerror(errno));
		close(fd);
		unlink(tmpbuf);
		return (1);
	}
	close(fd);
	return(0);
}

static void
usage()
{
	fprintf(stderr, "usage: vmoddomain [options] domain\n");
	fprintf(stderr, "options: -V         (print version number)\n");
	fprintf(stderr, "         -v         (verbose)\n");
	fprintf(stderr, "         -l 0|1     Enable Domain Limits\n");
	fprintf(stderr, "         -f 0|1     Enable VFILTER capability\n");
	fprintf(stderr, "         -h handler (can be one of the following\n");
 	fprintf(stderr, "                    %s\n", DELETE_ALL);
 	fprintf(stderr, "                    %s\n", BOUNCE_ALL);
	fprintf(stderr, "                    Maildir    - Maildir Path\n");
	fprintf(stderr, "                    email      - Email Addres\n");
	fprintf(stderr, "                    IP Address - SMTPROUTE/QMTPROUTE spec)\n");
	fprintf(stderr, "you need to specify handler and vfilter option or domain limits\n");
	return;
}

static int
get_options(int argc, char **argv, int *use_vfilter, int *domain_limits, 
	char **handler, char **domain)
{
	int             c;

	*use_vfilter = -1;
	*domain_limits = -1;
	*handler = *domain = 0;
	while ((c = getopt(argc, argv, "avf:h:l:")) != -1) 
	{
		switch (c)
		{
		case 'v':
			verbose = 1;
			break;
		case 'l':
			*domain_limits = atoi(optarg);
			break;
		case 'f':
			*use_vfilter = atoi(optarg);
			break;
		case 'h':
			*handler = optarg;
			break;
		default:
			usage();
			return (1);
		}
	}
	if ((*use_vfilter != -1 && !*handler) || (*handler && *use_vfilter == -1) ||
		(!*handler && *domain_limits == -1 && *use_vfilter == -1))
	{
		usage();
		return (1);
	} else
	if (optind < argc)
		*domain = argv[optind++];
	else
	{
		usage();
		return (1);
	}
	return(0);
}

void
getversion_vmoddomain_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
