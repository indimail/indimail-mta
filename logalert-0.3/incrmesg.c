/*
 * $Log: incrmesg.c,v $
 * Revision 1.5  2013-05-15 00:19:32+05:30  Cprogrammer
 * fixed warnings
 *
 * Revision 1.4  2013-03-03 23:36:12+05:30  Cprogrammer
 * create the directory with owner of incrmesg process
 *
 * Revision 1.3  2012-12-13 08:36:45+05:30  Cprogrammer
 * use SEEKDIR env variable
 *
 * Revision 1.2  2012-09-18 17:39:28+05:30  Cprogrammer
 * changed seek directory to /var/tmp
 *
 * Revision 1.1  2012-09-18 13:26:10+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif
#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif
#ifdef SOLARIS
#include <sys/systeminfo.h>
#endif
#include "common.h"

#define MAXBUF  8192
#define SEEKDIR "/var/tmp/incrmesg"

#ifndef	lint
static char     sccsid[] = "$Id: incrmesg.c,v 1.5 2013-05-15 00:19:32+05:30 Cprogrammer Exp mbhangui $";
#endif

struct msgtable
{
	char            fname[MAXBUF];
	int             fd;
	FILE           *fp;
	int             seekfd;
	long            machcnt;
};

struct msgtable *msghd;

#ifdef __STDC__
int             main(int, char **);
int             incrmesg(char **);
static int      IOplex();
static int      checkfiles(char *, FILE *, long *, int);
#else
int             main();
int             incrmesg();
static int      IOplex();
static int      checkfiles();
#endif

int
main(argc, argv)
	int             argc;
	char          **argv;
{
	char           *ptr;

	if ((ptr = strrchr(argv[0], '/')))
		ptr++;
	else
		ptr = argv[0];
	if (argc < 2)
	{
		fprintf(stderr, "USAGE: %s logfile(s)\n", ptr);
		return (1);
	}
	if (!(msghd = (struct msgtable *) malloc(sizeof(struct msgtable) * argc)))
	{
		perror("malloc");
		return (1);
	}
	return (incrmesg(argv + 1));
}

int
incrmesg(fname)
	char         **fname;
{
	char            seekfile[MAXBUF];
	long            seekval[2];
	struct msgtable *msgptr;
	char           *ptr, *seekdir;
	char          **fptr;

	if (!(seekdir = getenv("SEEKDIR")))
		seekdir = SEEKDIR;
	if (access(seekdir, F_OK) && r_mkdir(seekdir, 0700, getuid(), getgid()))
	{
		perror(seekdir);
		return (1);
	}
	for(fptr = fname, msgptr = msghd;*fptr;msgptr++, fptr++)
	{
		if ((ptr = strrchr(*fptr, '/')))
			ptr++;
		else
			ptr = *fptr;
		(void) snprintf(seekfile, MAXBUF, "%s/%s.seek", seekdir, ptr);
		(void) scopy(msgptr->fname, *fptr, MAXBUF);
		if ((msgptr->fd = open(msgptr->fname, O_RDONLY)) == -1)
		{
			perror(msgptr->fname);
			return (1);
		} else
		if (!(msgptr->fp = fdopen(msgptr->fd, "r")))
		{
			perror(msgptr->fname);
			return (1);
		}
		if (!access(seekfile, R_OK))
		{
			if ((msgptr->seekfd = open(seekfile, O_RDWR)) == -1)
			{
				perror(seekfile);
				return (1);
			}
			if (read(msgptr->seekfd, seekval, sizeof(seekval)) == -1)
				return (1);
			fseek(msgptr->fp, seekval[0], SEEK_SET);
			msgptr->machcnt = seekval[1];
		} else
		if ((msgptr->seekfd = open(seekfile, O_CREAT | O_RDWR, 0644)) == -1)
		{
			perror(seekfile);
			return (1);
		} else
		{
			seekval[0] = 0l;
			seekval[1] = msgptr->machcnt = 0l;
		}
	}
	*(msgptr->fname) = 0;
	msgptr->fd = -1;
	msgptr->seekfd = -1;
#ifdef DEBUG
	for(msgptr = msghd;msgptr->fd != -1;msgptr++)
	{
		printf("%s\n", msgptr->fname);
		printf("%d\n", msgptr->fd);
		printf("%d\n", msgptr->seekfd);
	}
#endif
	return (IOplex());
}

/* function for I/O multiplexing */
static int
IOplex()
{
	char            lhost[MAXHOSTNAMELEN];
	char            Buffer[MAXBUF];
	long            seekval[2], startsrno;
	int             dflag;
	struct msgtable *msgptr;

#ifdef SOLARIS
	(void) sysinfo(SI_HOSTNAME, lhost, MAXHOSTNAMELEN);
#else
	(void) gethostname(lhost, MAXHOSTNAMELEN);
#endif
	for(msgptr = msghd;msgptr->fd != -1;msgptr++)
	{
#ifdef DEBUG
		printf("Selecting file[%s] fd[%d] seekfd[%d]\n", msgptr->fname, msgptr->fd, msgptr->seekfd);
#endif
		for (dflag = 0, startsrno = msgptr->machcnt;;)
		{
			(void) fgets(Buffer, MAXBUF - 2, msgptr->fp);
			seekval[0] = ftell(msgptr->fp);
			if (feof(msgptr->fp))
			{
				/*
				 * If message file has been moved than update
				 * seekfile
				 */
				if (checkfiles(msgptr->fname, msgptr->fp, seekval, msgptr->seekfd))
				{
					(void) clearerr(msgptr->fp);
					(void) fflush(stdout);
					continue;
				}
				if (dflag)
				{
					printf("=======================================\n");
					printf("Message        File : %s@%s\n", msgptr->fname, lhost);
					printf("Message       Count : %ld\n", msgptr->machcnt - startsrno);
					printf("Start Serial Number : %ld\n", startsrno);
					printf("End   Serial Number : %ld\n", msgptr->machcnt - 1);
					printf("=======================================\n");
					fflush(stdout);
				}
				break;
			}
			else
			{
				if (!dflag++)
				{
					printf("=======================================================\n");
					printf("Filename %-25s\n", msgptr->fname);
					printf("=======================================================\n");
				}
				if (printf("%s %ld %s", lhost, (msgptr->machcnt)++, Buffer) == -1)
				{
						perror("printf");
						return (-1);
				}
				if (fflush(stdout) == EOF)
				{
					perror("fflush");
					return (-1);
				}
				seekval[1] = msgptr->machcnt;
				(void) lseek(msgptr->seekfd, 0, SEEK_SET);
				(void) write(msgptr->seekfd, seekval, sizeof(seekval));
			}
		} /* end of for(dflag = 0;;) */
	} /* end of for(msgptr = msghd;;) */
	return (0);
}

static int
checkfiles(fname, msgfp, seekval, seekfd)
    char           *fname;
    FILE           *msgfp;
	long            *seekval;
	int             seekfd;
{
	int             fd, msgfd, count;
	long            tmpseekval;

	msgfd = fileno(msgfp);
	for (count = 0;; count++)
	{
		if ((fd = open(fname, O_RDONLY)) == -1)
		{
			sleep(5);
			continue;
		} else
		if (count)	/* new message file has been created */
		{
			(void) close(msgfd);
			(void) dup2(fd, msgfd);
			if (fd != msgfd)
				(void) close(fd);
			rewind(msgfp);
			(void) lseek(seekfd, 0l, SEEK_SET);
			seekval[0] = 0l;
			(void) write(seekfd, seekval, sizeof(seekval));
			return (1);
		} else
			break;
	}
	tmpseekval = lseek(fd, 0l, SEEK_END);
	if (tmpseekval == seekval[0])/* Just an EOF on message file */
	{
		(void) close(fd);
		return (0);
	} else
	if (tmpseekval > seekval[0]) /* update happened after EOF */
	{
		close(fd);
		return (1);
	} else	/* new message file has been created */
	{
		(void) close(msgfd);
		(void) dup2(fd, msgfd);
		if (fd != msgfd)
			(void) close(fd);
		rewind(msgfp);
		(void) lseek(seekfd, 0l, SEEK_SET);
		seekval[0] = 0l;
		(void) write(seekfd, &seekval, sizeof(seekval));
		return (1);
	}
}

void
getversion_incrmesg_c()
{
	printf("%s\n", sccsid);
}
