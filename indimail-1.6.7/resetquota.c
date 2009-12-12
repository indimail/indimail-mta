/*
 * $Log: resetquota.c,v $
 * Revision 2.2  2008-06-24 21:54:40+05:30  Cprogrammer
 * porting for 64 bit
 *
 * Revision 2.1  2004-07-08 13:58:56+05:30  Cprogrammer
 * program to set maildir quota
 *
 */
#include "indimail.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <errno.h>

#ifndef	lint
static char     sccsid[] = "$Id: resetquota.c,v 2.2 2008-06-24 21:54:40+05:30 Cprogrammer Stab mbhangui $";
#endif

int get_options(int, char **, char **, char **, char **, char **);

int
main(int argc, char **argv)
{
	mdir_t          mailcount, mailsize;
	int             i, fd, status;
	char           *ptr, *user, *group, *perm, *quota;
	char            tmpbuf[MAX_BUFF], filename[MAX_BUFF];
	uid_t           uid = -1;
	gid_t           gid = -1;
	int             perms = 0600;
	struct passwd  *pw;
	struct group   *gr;
	FILE           *fp;

	if (get_options(argc, argv, &user, &group, &perm, &quota))
	{
		fprintf(stderr, "USAGE: resetquota [options] directory(s)\n");
		fprintf(stderr, "       -v            ( verbose )\n");
		fprintf(stderr, "       -u user       ( username )\n");
		fprintf(stderr, "       -g group      ( group )\n");
		fprintf(stderr, "       -p perm       ( permission )\n");
		fprintf(stderr, "       -q quota_spec ( maildirquota quota_spec )\n");
		return (1);
	}

	if (user)
	{
		if (!(pw = getpwnam(user)))
		{
			fprintf(stderr, "%s: failed to get user info\n", user);
			return(1);
		}
		uid = pw->pw_uid;
		gid = pw->pw_gid;
	}
	if (group)
	{
		if (!(gr = getgrnam(group)))
		{
			fprintf(stderr, "%s: failed to get group info\n", group);
			return(1);
		}
		gid = gr->gr_gid;
	}
	if (perm)
		sscanf(perm, "%o", &perms);
	for (i = optind, status = 0; i < argc; i++)
	{
		snprintf(filename, sizeof(filename), "%s/maildirsize", argv[i]);
#ifdef FILE_LOCKING
		if ((fd = getDbLock(filename, 1)) == -1)
		{
			fprintf(stderr, "get_write_lock: %s: %s", filename, strerror(errno));
			status = -1;
			continue;
		}
#endif
		if (!quota)
		{
			if (!(fp = fopen(filename, "r")))
			{
				perror(filename);
#ifdef FILE_LOCKING
				delDbLock(fd, filename, 1);
#endif
				status = -1;
				continue;
			}
			if (!fgets(tmpbuf, sizeof(tmpbuf) - 2, fp))
			{
				fprintf(stderr, "%s/maildirsize: EOF\n", argv[i]);
#ifdef FILE_LOCKING
				delDbLock(fd, filename, 1);
#endif
				fclose(fp);
				status = -1;
				continue;
			}
			fclose(fp);
			if ((ptr = strchr(tmpbuf, '\n')))
				*ptr = 0;
		}
		if ((mailsize = count_dir(argv[i], &mailcount)) == -1)
		{
			status = -1;
			continue;
		}
		if (!(fp = fopen(filename, "w")))
		{
			perror(filename);
#ifdef FILE_LOCKING
			delDbLock(fd, filename, 1);
#endif
			status = -1;
			continue;
		}
		if (uid != -1 && gid != -1)
		{
			if (fchown(fileno(fp), uid, gid))
			{
				fprintf(stderr, "fchown: %s: %s\n", filename, strerror(errno));
#ifdef FILE_LOCKING
				delDbLock(fd, filename, 1);
#endif
				status = -1;
				continue;
			}
		}
		if (fchmod(fileno(fp), perms))
		{
			fprintf(stderr, "fchmod: %s: %s\n", filename, strerror(errno));
#ifdef FILE_LOCKING
			delDbLock(fd, filename, 1);
#endif
			status = -1;
			continue;
		}
		if (!quota)
			quota = tmpbuf;
		fprintf(fp, "%s\n%"PRIu64" %"PRIu64"\n", quota, mailsize, mailcount);
		fclose(fp);
		fprintf(stdout, "%s\n%"PRIu64" %"PRIu64"\n", quota, mailsize, mailcount);
		delDbLock(fd, filename, 1);
	}
	return (status);
}

int
get_options(int argc, char **argv, char **user, char **group, char **perm, char **quota)
{
	int             c;

	*user = *group = *perm = *quota = 0;
	verbose = 0;
	while ((c = getopt(argc, argv, "vu:g:p:q:")) != -1)
	{
		switch (c)
		{
		case 'v':
			verbose = 1;
			break;
		case 'u':
			*user = optarg;
			break;
		case 'g':
			*group = optarg;
			break;
		case 'p':
			*perm = optarg;
			break;
		case 'q':
			*quota = optarg;
			break;
		default:
			return(1);
		}
	}
	if (optind < argc)
		return (0);
	return (1);
}

void
getversion_resetquota_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
