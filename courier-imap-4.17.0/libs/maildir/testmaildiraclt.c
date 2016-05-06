/*
** Copyright 2003 Double Precision, Inc.
** See COPYING for distribution information.
*/

#include	"maildiraclt.h"
#include	"maildirmisc.h"

#if	HAVE_UNISTD_H
#include	<unistd.h>
#endif
#include	<string.h>
#include	<errno.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	<fcntl.h>
#if	HAVE_UTIME_H
#include	<utime.h>
#endif
#if TIME_WITH_SYS_TIME
#include	<sys/time.h>
#include	<time.h>
#else
#if HAVE_SYS_TIME_H
#include	<sys/time.h>
#else
#include	<time.h>
#endif
#endif


#define CHK(x) if (!(x)) { printf("%s(%d): sanity check failed.\n", \
			__FILE__, __LINE__); exit(1);}

static int cb_enum(const char *identifier,
		   const maildir_aclt *acl,
		   void *b)
{
	char *cb=(char *)b;

	if (strlen(cb) > 7000)
		return -1;

	strcat(strcat(strcat(strcat(b, identifier), "."),
		      maildir_aclt_ascstr(acl)), ".");
	return 0;
}

int main()
{
	maildir_aclt a, b;
	maildir_aclt_list l;
	char buf[8192];
	int i;
	struct stat stat_buf;

	CHK(maildir_aclt_init(&a, "rw", NULL) == 0);
	CHK(maildir_aclt_init(&b, NULL, &a) == 0);

	CHK(strcmp(maildir_aclt_ascstr(&a), "rw") == 0);
	CHK(strcmp(maildir_aclt_ascstr(&b), "rw") == 0);

	maildir_aclt_destroy(&b);
	CHK(maildir_aclt_init(&b, "arx", NULL) == 0);
	CHK(maildir_aclt_add(&a, NULL, &b) == 0);
	CHK(strcmp(maildir_aclt_ascstr(&a), "arwx") == 0);
	CHK(maildir_aclt_add(&a, "bwz", NULL) == 0);
	CHK(strcmp(maildir_aclt_ascstr(&a), "abrwxz") == 0);

	maildir_aclt_destroy(&b);
	CHK(maildir_aclt_init(&b, "wry", NULL) == 0);
	CHK(maildir_aclt_del(&a, NULL, &b) == 0);
	CHK(strcmp(maildir_aclt_ascstr(&a), "abxz") == 0);
	CHK(maildir_aclt_del(&a, "abc", NULL) == 0);
	CHK(strcmp(maildir_aclt_ascstr(&a), "xz") == 0);

	maildir_aclt_list_init(&l);
	CHK(maildir_aclt_list_add(&l, "owner", NULL, &a) == 0);
	CHK(maildir_aclt_list_add(&l, "user1", NULL, &a) == 0);
	CHK(maildir_aclt_list_add(&l, "user2", NULL, &b) == 0);
	CHK(maildir_aclt_list_add(&l, "owner", NULL, &b) == 0);
	CHK(maildir_aclt_list_del(&l, "user1") == 0);
	CHK(maildir_aclt_list_del(&l, "user3") == 0);
	CHK(maildir_aclt_list_find(&l, "owner") != NULL &&
	    strcmp(maildir_aclt_ascstr(maildir_aclt_list_find(&l, "owner")),
		   "rwy") == 0);
	maildir_aclt_destroy(&a);
	maildir_aclt_destroy(&b);

	buf[0]=0;

	CHK(maildir_aclt_list_enum(&l, cb_enum, buf) == 0 &&
	    strcmp(buf, "owner.rwy.user2.rwy.") == 0);
	maildir_aclt_list_destroy(&l);

	CHK(maildir_del("confmdtest") == 0);
	CHK(maildir_make("confmdtest", 0700, 0700, 0) == 0);

	for (i=0; i<150; i++)
	{
		int fd;

		sprintf(buf, "confmdtest/new/msg%d", i);
		CHK((fd=open(buf, O_RDWR|O_CREAT|O_TRUNC, 0600)) >= 0);
		close(fd);
	}
	CHK(maildir_del("confmdtest") == 0);
	CHK(maildir_make("confmdtest", 0700, 0700, 0) == 0);
	CHK(maildir_make("confmdtest/.foo.bar", 0700, 0700, 0) == 0);
	CHK(maildir_acl_read(&l, "confmdtest", ".") == 0);

	buf[0]=0;
	CHK(maildir_aclt_list_enum(&l, cb_enum, buf) == 0 &&
	    strcmp(buf, "owner.aceilrstwx.administrators.aceilrstwx.") == 0);

	CHK(maildir_aclt_list_add(&l, "anyone", "lr", NULL) == 0);
	CHK(maildir_acl_write(&l, "confmdtest", ".", NULL, NULL) == 0);
	maildir_aclt_list_destroy(&l);
	CHK(maildir_acl_read(&l, "confmdtest", ".") == 0);

	buf[0]=0;
	CHK(maildir_aclt_list_enum(&l, cb_enum, buf) == 0 &&
	    strcmp(buf, "owner.aceilrstwx.administrators.aceilrstwx.anyone.lr.") == 0);

	maildir_aclt_list_destroy(&l);
	CHK(maildir_acl_read(&l, "confmdtest", ".foo.bar") == 0);

	buf[0]=0;
	CHK(maildir_aclt_list_enum(&l, cb_enum, buf) == 0 &&
	    strcmp(buf, "owner.aceilrstwx.administrators.aceilrstwx.anyone.lr.") == 0);

	maildir_aclt_list_destroy(&l);
	CHK(maildir_acl_read(&l, "confmdtest", ".foo") == 0);

	buf[0]=0;
	CHK(maildir_aclt_list_enum(&l, cb_enum, buf) == 0 &&
	    strcmp(buf, "owner.aceilrstwx.administrators.aceilrstwx.anyone.lr.") == 0);

	CHK(maildir_acl_write(&l, "confmdtest", ".foobar", NULL, NULL)==0);

	maildir_acl_reset("confmdtest");

	CHK(stat("confmdtest/" ACLHIERDIR "/foo", &stat_buf) == 0 &&
	    stat("confmdtest/" ACLHIERDIR "/foobar", &stat_buf) == 0);

#if	HAVE_UTIME

	{
		struct	utimbuf ub;

		ub.actime=ub.modtime=time(NULL)-60*60*2;
		utime("confmdtest/" ACLHIERDIR "/foobar", &ub);

		maildir_acl_reset("confmdtest");

		CHK(stat("confmdtest/" ACLHIERDIR "/foo", &stat_buf) == 0 &&
		    stat("confmdtest/" ACLHIERDIR "/foobar", &stat_buf) < 0 &&
		    errno == ENOENT);

	}
#else
#if	HAVE_UTIMES

	{
		struct	timeval	tv;

		tv.tv_sec=time(NULL)-60*60*2;
		tv.tv_usec=0;
		utimes("confmdtest/" ACLHIERDIR "/foobar", &tv);

		maildir_acl_reset("confmdtest");

		CHK(stat("confmdtest/" ACLHIERDIR "/foo", &stat_buf) == 0 &&
		    stat("confmdtest/" ACLHIERDIR "/foobar", &stat_buf) < 0 &&
		    errno == ENOENT);
	}
#endif
#endif

#if 1
	CHK(maildir_del("confmdtest") == 0);
#endif

	maildir_aclt_list_destroy(&l);
	CHK(maildir_aclt_list_add(&l, "owner", "swite", NULL) == 0);
	CHK(maildir_aclt_list_add(&l, "anyone", "lr", NULL) == 0);
	CHK(maildir_aclt_list_add(&l, "-user1", "r", NULL) == 0);

	{
		static const char *id[3];

		id[0]="owner";
		id[1]="user0";
		id[2]=NULL;

		CHK(maildir_acl_compute_array(&a, &l, id) == 0 &&
		    strcmp(maildir_aclt_ascstr(&a), "aeilrstw") == 0);
		maildir_aclt_destroy(&a);

		id[0]="user1";
		id[1]=NULL;
		CHK(maildir_acl_compute_array(&a, &l, id) == 0 &&
		    strcmp(maildir_aclt_ascstr(&a), "l") == 0);
		maildir_aclt_destroy(&a);
	}

	exit(0);
	return (0);
}
