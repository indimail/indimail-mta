/*
 * $Log: vmoddomain.c,v $
 * Revision 2.2  2009-03-08 11:42:38+05:30  Cprogrammer
 * made vmoddomain setuid
 *
 * Revision 2.1  2009-01-28 11:25:17+05:30  Cprogrammer
 * program to modify .qmail-default
 *
 */
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: vmoddomain.c,v 2.2 2009-03-08 11:42:38+05:30 Cprogrammer Exp mbhangui $";
#endif

static void     usage();
static int      get_options(int argc, char **argv, int *, char **, char **);
int
main(int argc, char **argv)
{

	char           *domain = 0, *handler = 0;
	char            TheDir[AUTH_SIZE], tmpbuf[MAX_BUFF];
	int             fd, use_vfilter;
#ifdef FILE_LOCKING
	int             lockfd;
#endif
	uid_t           uid, myuid;
	gid_t           gid;

	if (get_options(argc, argv, &use_vfilter, &handler, &domain))
		return (1);
	if (!vget_assign(domain, TheDir, AUTH_SIZE, &uid, &gid))
	{
		error_stack(stderr, "%s: domain does not exist\n", domain);
		return (1);
	}
	if (indimailuid == -1 || indimailgid == -1)
		GetIndiId(&indimailuid, &indimailgid);
	myuid = getuid();
	if (myuid != 0 && myuid != indimailuid)
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
	snprintf(tmpbuf, MAX_BUFF, "%s/.qMail-default", TheDir);
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
		error_stack(stderr, "chown: %s: %s\n", tmpbuf, strerror(errno));
		close(fd);
		unlink(tmpbuf);
#ifdef FILE_LOCKING
		delDbLock(lockfd, tmpbuf, 1);
#endif
		return (1);
	}
#ifdef VFILTER
	if(use_vfilter)
		filewrt(fd, "| %s/sbin/vfilter '' %s\n", INDIMAILDIR, handler);
	else
#endif
		filewrt(fd, "| %s/sbin/vdelivermail '' %s\n", INDIMAILDIR, handler);
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

static void
usage()
{
	fprintf(stderr, "usage: vmoddomain [options] domain\n");
	fprintf(stderr, "options: -V         (print version number)\n");
	fprintf(stderr, "         -v         (verbose)\n");
	fprintf(stderr, "         -f         (Sets the Domain with VFILTER capability)\n");
	fprintf(stderr, "         -h handler (can be one of the following\n");
 	fprintf(stderr, "                    (1 %s)\n", DELETE_ALL);
 	fprintf(stderr, "                    (2 %s)\n", BOUNCE_ALL);
	fprintf(stderr, "                    (3 Maildir)\n");
	fprintf(stderr, "                    (4 Email Addres)\n");
	fprintf(stderr, "                    (5 IP Address - SMTPROUTE spec)\n");
	return;
}

static int
get_options(int argc, char **argv, int *use_vfilter, char **handler, char **domain)
{
	int             c;

	*use_vfilter = 0;
	*handler = *domain = 0;
	while ((c = getopt(argc, argv, "aVvfh:")) != -1) 
	{
		switch (c)
		{
		case 'V':
			getversion(sccsid);
			break;
		case 'v':
			verbose = 1;
			break;
		case 'f':
			*use_vfilter = 1;
			break;
		case 'h':
			*handler = optarg;
			break;
		default:
			usage();
			return (1);
		}
	}
	if (!*handler)
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
	/* Last case: the last parameter is a Maildir, an email address, ipaddress or hostname */
	if (!strncmp(*handler, BOUNCE_ALL, AUTH_SIZE) || !strncmp(*handler, DELETE_ALL, AUTH_SIZE))
		return (0);
	if (**handler == '/')
	{
		if (chdir (*handler))
		{
			error_stack(stderr, "chdir: %s: %s\n", *handler, strerror(errno));
			return (1);
		}
		if (access("new", F_OK) || access("cur", F_OK) || access("tmp", F_OK))
		{
			error_stack(stderr, "%s not a Maildir\n", *handler);
			return (1);
		}
	} else
	{
		if (!strchr(*handler, '@')) /*- IP Address */
		{
			char           *ptr;
			int             i;
			for (i = 0, ptr = *handler;*ptr;ptr++)
			{
				if (*ptr == ':')
					i++;
			}
			if (i != 2)
			{
				error_stack(stderr, "Invalid SMTPROUTE Specification [%s]\n", *handler);
				return (1);
			}
		}
	}
	return(0);
}

void
getversion_vmoddomain_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
