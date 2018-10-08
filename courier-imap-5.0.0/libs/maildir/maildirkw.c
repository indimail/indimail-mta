/*
** Copyright 2003-2004 Double Precision, Inc.
** See COPYING for distribution information.
*/


#if	HAVE_CONFIG_H
#include	"config.h"
#endif
#include	<stdio.h>
#include	<string.h>
#include	<stdlib.h>
#include	<errno.h>

#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <sys/stat.h>
#include	"maildirkeywords.h"
#include	"maildirwatch.h"

static void usage()
{
	printf("Usage: maildirkw [ options ] maildir [+/-]flag [+/-]flag...\n");
	exit(1);
}

static int doit_locked(const char *maildir,
		       const char *filename,
		       int lockflag, int plusminus,
		       char **argv,
		       int optind, int argc)
{
	char *tmpname, *newname;

	if (!plusminus)
	{
		struct libmail_kwHashtable kwh;
		struct libmail_kwMessage *kwm;

		struct libmail_kwGeneric g;
		int rc;

		libmail_kwgInit(&g);

		/* Make sure courierimapkeywords directory exists */

		libmail_kwEnabled=0;
		rc=libmail_kwgReadMaildir(&g, maildir);
		libmail_kwEnabled=1;
		libmail_kwgDestroy(&g);

		if (rc)
			return -1;


		libmail_kwhInit(&kwh);

		if (!(kwm=libmail_kwmCreate()))
		{
			perror("libmail_kwmCreate");
			return -1;
		}

		while (optind < argc)
			if (libmail_kwmSetName(&kwh, kwm,
					       argv[optind++]) < 0)
			{
				libmail_kwmDestroy(kwm);
				perror("libmail_kwmSetName");
				return -1;
			}

		if (maildir_kwSave(maildir, filename, kwm,
				   &tmpname, &newname, 0) < 0)
		{
			perror(maildir);
			libmail_kwmDestroy(kwm);
			return -1;
		}
		libmail_kwmDestroy(kwm);

		if (rename(tmpname, newname) < 0)
		{
			perror(newname);
			free(tmpname);
			free(newname);
			return -1;
		}
		free(tmpname);
		free(newname);
		return 0;
	}
	else
	{
		struct libmail_kwGeneric g;
		int rc;
		struct libmail_kwGenericEntry *e;
		struct libmail_kwMessage *kwm, *kwm_alloced;

		libmail_kwgInit(&g);

		rc=libmail_kwgReadMaildir(&g, maildir);

		if (rc != 0)
			return rc;

		e=libmail_kwgFindByName(&g, filename);

		kwm_alloced=NULL;

		if (e && e->keywords)
			kwm=e->keywords;
		else
		{
			if ((kwm=kwm_alloced=libmail_kwmCreate()) == NULL)
			{
				perror("libmail_kwmCreate");
				libmail_kwgDestroy(&g);
				return -1;
			}
		}

		while (optind < argc)
		{
			const char *f=argv[optind++];

			if ( plusminus == '+')
			{
				if (libmail_kwmSetName(&g.kwHashTable,
						       kwm, f) < 0)
				{
					perror("libmail_kwmSetName");
					if (kwm_alloced)
						libmail_kwmDestroy(kwm_alloced
								   );
					libmail_kwgDestroy(&g);
					return -1;
				}
			} else
			{
				struct libmail_keywordEntry *kwe=
					libmail_kweFind(&g.kwHashTable,
							f, 0);

				if (kwe)
					libmail_kwmClear(kwm, kwe);
			}
		}

		rc=maildir_kwSave(maildir, filename, kwm,
				  &tmpname, &newname, 1);

		if (rc == 0)
		{
			if (link(tmpname, newname) == 0)
			{
				struct stat stat_buf;

				if (stat(tmpname, &stat_buf) == 0 &&
				    stat_buf.st_nlink == 2)
					unlink(tmpname);
				else
					rc=1; /* What's up? */
			}
			else
			{
				if (errno == EEXIST)
					rc=1;
				else
					rc= -1;
				unlink(tmpname);
			}
		}

		if (kwm_alloced)
			libmail_kwmDestroy(kwm_alloced);
		libmail_kwgDestroy(&g);

		return rc;
	}
}

static int list_locked(const char *maildir)
{
	struct libmail_kwGeneric g;
	int rc;
	size_t n;

	libmail_kwgInit(&g);

	rc=libmail_kwgReadMaildir(&g, maildir);

	if (rc)
		return rc;

	for (n=0; n<g.nMessages; n++)
	{
		struct libmail_kwGenericEntry *e=
			libmail_kwgFindByIndex(&g, n);
		struct libmail_kwMessageEntry *k;

		if (!e)
			continue;

		printf("%s", e->filename);

		for (k=e->keywords ? e->keywords->firstEntry:NULL; k;
		     k=k->next)
			printf(" %s", keywordName(k->libmail_keywordEntryPtr));
		printf("\n");
	}
	return 0;
}

static int doit(const char *maildir, const char *filename, int lockflag,
		int plusminus,
		char **argv, int optind, int argc)
{
	if (lockflag)
	{
		struct maildirwatch *w=maildirwatch_alloc(maildir);
		int tryAnyway;
		char *lockname;
		int rc;

		if (!w)
		{
			perror(maildir);
			return -1;
		}

		lockname=maildir_lock(maildir, w, &tryAnyway);

		if (!lockname)
		{
			perror(maildir);
			if (!tryAnyway)
			{
				maildirwatch_free(w);
				maildirwatch_cleanup();
				return -1;
			}
		}

		rc=doit_locked(maildir, filename, 1, plusminus,
			       argv, optind, argc);
		if (lockname)
		{
			unlink(lockname);
			free(lockname);
		}
		maildirwatch_free(w);
		maildirwatch_cleanup();
		return rc;
	}

	return doit_locked(maildir, filename, 0, plusminus,
			   argv, optind, argc);
}

static int dolist(const char *maildir, int lockflag)
{
	if (lockflag)
	{
		struct maildirwatch *w=maildirwatch_alloc(maildir);
		int tryAnyway;
		char *lockname;
		int rc;

		if (!w)
		{
			perror(maildir);
			return -1;
		}

		lockname=maildir_lock(maildir, w, &tryAnyway);

		if (!lockname)
		{
			perror(maildir);
			if (!tryAnyway)
			{
				maildirwatch_free(w);
				maildirwatch_cleanup();
				return -1;
			}
		}

		rc=list_locked(maildir);
		if (lockname)
		{
			unlink(lockname);
			free(lockname);
		}
		maildirwatch_free(w);
		maildirwatch_cleanup();
		return rc;
	}

	return list_locked(maildir);
}

int main(int argc, char *argv[])
{
	int lockflag=0;
	int optc;
	const char *maildir;
	const char *filename;
	int list=0;
	int plusminus=0;
	int n;

	libmail_kwCaseSensitive=0;

	while ((optc=getopt(argc, argv, "arLlhc")) != -1)
		switch (optc) {
		case 'c':
			libmail_kwCaseSensitive=1;
			break;
		case 'l':
			lockflag=1;
			break;
		case 'L':
			list=1;
			break;
		case 'a':
			plusminus='+';
			break;
		case 'r':
			plusminus='-';
			break;
		default:
			usage();
		}

	if (optind >= argc)
		usage();

	maildir=argv[optind++];

	if (list)
	{
		exit (dolist(maildir, lockflag));
	}

	if (optind >= argc)
		usage();

	filename=argv[optind++];

	while ((n=doit(maildir, filename, lockflag, plusminus,
		       argv, optind, argc)) > 0)
		;

	exit(-n);
	return (0);
}
