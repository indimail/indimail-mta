/*
** Copyright 1998 - 2009 Double Precision, Inc.
** See COPYING for distribution information.
*/

#include	"config.h"
#include	"maildirquota.h"
#include	"maildircreate.h"
#include	"maildirmisc.h"
#include	"quotawarnmsg.h"
#include	<stdio.h>
#include	<string.h>
#include	<stdlib.h>
#include	<sys/types.h>
#include	<errno.h>
#if HAVE_SYS_STAT_H
#include	<sys/stat.h>
#endif
#if HAVE_UNISTD_H
#include	<unistd.h>
#endif
#if	HAVE_FCNTL_H
#include	<fcntl.h>
#endif
#include	<time.h>
#if	HAVE_SYSEXITS_H
#include	<sysexits.h>
#endif
#include	"rfc822/rfc822.h"
#ifndef	BUFSIZE
#define	BUFSIZE	8192
#endif

#ifndef	EX_OSERR
#define	EX_OSERR	71
#endif

#ifndef	EX_IOERR
#define	EX_IOERR	74
#endif

#ifndef	EX_TEMPFAIL
#define EX_TEMPFAIL	75
#endif

#ifndef	EX_NOPERM
#define	EX_NOPERM	77
#endif


static long deliver(int fdin, const char *dir, long s,
		    int auto_create, int quota_warn_percent, const char *pfix,
		    const char *newquota,
		    const char *quota_warn_msg)
{
	struct maildir_tmpcreate_info createInfo;
	char	buf[BUFSIZ];
	int	n;
	long	ss=0;
	int	fd;

	maildir_tmpcreate_init(&createInfo);
	createInfo.openmode=0666;
	createInfo.maildir=dir;
	createInfo.uniq=pfix;
	createInfo.msgsize=s;
	createInfo.doordie=1;

	while ((fd=maildir_tmpcreate_fd(&createInfo)) < 0)
	{
		if (errno == ENOENT && auto_create && maildir_mkdir(dir) == 0)
		{
			auto_create=0;
			continue;
		}

		perror(dir);
		exit(EX_TEMPFAIL);
	}

	while ((n=read(fdin, buf, sizeof(buf))) > 0)
	{
	char	*p=buf;

		ss += n;
		while (n)
		{
		int	l;

			if ((l=write(fd, p, n)) < 0)
			{
				close(fd);
				unlink(createInfo.tmpname);
				perror(createInfo.tmpname);
				exit(EX_IOERR);
			}
			p += l;
			n -= l;
		}
	}
	close(fd);
	if (n < 0)
	{
		unlink(createInfo.tmpname);
		perror(createInfo.tmpname);
		exit(EX_IOERR);
	}

	if (s != ss)
	{
		char	*qq;
		struct maildirsize info;

		if (s)	*strrchr(createInfo.newname, ',')=0;
		/* Zap incorrect size */
		qq=malloc(strlen(createInfo.newname)+100);
		if (!qq)
		{
			unlink(createInfo.tmpname);
			perror(createInfo.tmpname);
			exit(EX_OSERR);
		}
		sprintf(qq, "%s,S=%ld", createInfo.newname, ss-s);
		free(createInfo.newname);
		createInfo.newname=qq;

		if (maildirquota_countfolder(dir))
		{
			if (maildir_quota_add_start(dir, &info, ss-s, 1,
						    newquota))
			{
				unlink(createInfo.tmpname);
				printf("Mail quota exceeded.\n");
#if HAVE_COURIER
				exit(EX_TEMPFAIL);
#else
				exit(EX_NOPERM);
#endif
			}
			maildir_quota_add_end(&info, ss-s, 1);
		}
	}

	if (maildir_movetmpnew(createInfo.tmpname, createInfo.newname))
	{
		unlink(createInfo.tmpname);
		perror(createInfo.tmpname);
		exit(EX_IOERR);
	}
	maildir_tmpcreate_free(&createInfo);

	if (quota_warn_percent >= 0)
		maildir_deliver_quota_warning(dir, quota_warn_percent,
					      quota_warn_msg);

	return (ss);
}

int main(int argc, char **argv)
{
	const char *dir;
	struct	stat	stat_buf;
	int	auto_create = 0;
	int	quota_warn_percent = -1;
	int i;
	const char *quota=NULL;
	const   char *quota_warn_msg=0;

	for (i=1; i<argc; i++)
	{
		if (strcmp(argv[i], "-c") == 0)
		{
			auto_create = 1;
			continue;
		}

		if (strcmp(argv[i], "-w") == 0 && argc - i > 1)
		{
			quota_warn_percent = atoi(argv[i+1]);
			++i;
			continue;
		}

		if (strcmp(argv[i], "-W") == 0 && argc - i > 1)
		{
			quota_warn_msg = argv[i+1];
			++i;
			continue;
		}

		break;
	}
	if (i >= argc || quota_warn_percent < -1 || quota_warn_percent > 100)
	{
		fprintf(stderr, "Usage: %s [-c] [-w percent] maildir\n",
			argv[0]);
		exit(73);
	}

	dir=argv[i];

	++i;
	if (i < argc)
		quota=argv[i];

	if (fstat(0, &stat_buf) == 0 && S_ISREG(stat_buf.st_mode) &&
		stat_buf.st_size > 0)
	{
		struct maildirsize info;
		int doquota=maildirquota_countfolder(dir);

		if (doquota &&
		    maildir_quota_add_start(dir, &info, stat_buf.st_size, 1,
					    quota))
		{
			if (quota_warn_percent >= 0)
				maildir_deliver_quota_warning(dir, quota_warn_percent,
							      quota_warn_msg);
			printf("Mail quota exceeded.\n");
			exit(77);
		}
		deliver(0, dir, stat_buf.st_size,
			auto_create, quota_warn_percent, NULL, quota,
			quota_warn_msg);

		if (doquota)
			maildir_quota_add_end(&info, stat_buf.st_size, 1);
		exit(0);
	}
	deliver(0, dir, 0, auto_create, quota_warn_percent, NULL, quota,
		quota_warn_msg);
	exit(0);
}
