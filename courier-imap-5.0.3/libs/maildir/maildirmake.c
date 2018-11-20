/*
** Copyright 1998 - 2018 Double Precision, Inc.
** See COPYING for distribution information.
*/


#if	HAVE_CONFIG_H
#include	"config.h"
#endif
#include	<stdio.h>
#include	<string.h>
#include	<stdlib.h>
#if	HAVE_FCNTL_H
#include	<fcntl.h>
#endif
#if	HAVE_UNISTD_H
#include	<unistd.h>
#endif
#include	<sys/types.h>
#if	HAVE_SYS_STAT_H
#include	<sys/stat.h>
#endif
#include	<ctype.h>
#include	<errno.h>

#include	"maildircreate.h"
#include	"maildirmisc.h"
#include	"maildirinfo.h"
#include	"maildirsharedrc.h"
#include	"maildirquota.h"
#include	"maildirfilter.h"
#include	"unicode/courier-unicode.h"

static void usage()
{
	printf("Usage: maildirmake [ options ] maildir\n");
	exit(1);
}

extern FILE *maildir_shared_fopen(const char *, const char *);
extern void maildir_shared_fparse(char *, char **, char **);

static void add(const char *dir, const char *name)
{
char	*c=malloc(strlen(name)+1);
char	*s;
FILE	*fp;
char	buf[BUFSIZ];
char	*ptr, *dirp;

	if (!c)
	{
		perror("malloc");
		exit(1);
	}
	strcpy(c, name);

	if ((s=strchr(c, '=')) == 0)
		usage();
	*s++=0;
	if (*s != '/')
		usage();
	if (access(s, R_OK))
	{
		perror(s);
		exit(1);
	}
	if (strchr(c, '.') || strchr(c, '/'))
	{
		fprintf(stderr, "%s: invalid name\n", c);
		exit(1);
	}

	if (chdir(dir))
	{
		perror(dir);
		exit(1);
	}

	if ((fp=fopen(MAILDIRSHAREDRC, "r")) != 0)
	{
		while ((ptr=fgets(buf, sizeof(buf), fp)) != 0)
		{
		char	*namep;

			maildir_shared_fparse(buf, &namep, &dirp);
			if (!namep)	continue;
			if (strcmp(namep, c) == 0)
			{
				fclose(fp);
				fprintf(stderr,
		"%s: already defined as a sharable maildir in %s.\n",
					namep, MAILDIRSHAREDRC);
				exit(2);
			}
		}
		fclose(fp);
	}

	if ((fp=maildir_shared_fopen(".", "a+")) == 0)
	{
		perror(dir);
		exit(1);
	}

	if (fseek(fp, 0L, SEEK_SET) < 0)
	{
		perror(dir);
		exit(1);
	}

	while ((ptr=fgets(buf, sizeof(buf), fp)) != 0)
	{
	char	*namep;

		maildir_shared_fparse(buf, &namep, &dirp);
		if (!namep)	continue;
		if (strcmp(namep, c) == 0)
		{
			fclose(fp);
			fprintf(stderr, "%s: already defined as a sharable maildir.\n",
				namep);
			exit(2);
		}
	}
	fprintf(fp, "%s\t%s\n", c, s);
	if (fflush(fp) || ferror(fp) || fclose(fp))
	{
		perror("dir");
		exit(1);
	}
	exit(0);
}

static void del(const char *dir, const char *n)
{
FILE	*fp;
char	buf[BUFSIZ];
char	buf2[BUFSIZ];

FILE	*fp2;
int	found;
 struct maildir_tmpcreate_info createInfo;

	if (chdir(dir))
	{
		perror(dir);
		exit(1);
	}
	if ((fp=maildir_shared_fopen(".", "r")) == 0)
	{
		perror(dir);
		exit(1);
	}

	maildir_tmpcreate_init(&createInfo);

	createInfo.uniq="shared";
	createInfo.msgsize=0;
	createInfo.doordie=1;

	if ((fp2=maildir_tmpcreate_fp(&createInfo)) == NULL)
	{
		perror(dir);
		exit(1);
	}

	found=0;
	while (fgets(buf, sizeof(buf), fp))
	{
	char	*name, *dirp;

		strcpy(buf2, buf);
		maildir_shared_fparse(buf2, &name, &dirp);
		if (name && strcmp(name, n) == 0)
		{
			found=1;
			continue;
		}
		fprintf(fp2, "%s", buf);
	}
	fclose(fp);
	if (fflush(fp2) || ferror(fp2) || fclose(fp2)
	    || rename(createInfo.tmpname, "shared-maildirs"))
	{
		perror(createInfo.tmpname);
		unlink(createInfo.tmpname);
		exit(1);
	}
	maildir_tmpcreate_free(&createInfo);
	if (!found)
	{
		fprintf(stderr, "%s: not found.\n", n);
		exit(1);
	}
	exit(0);
}

/*****************************************************************************

Convert modified-UTF7 folder names to UTF-8 (sort-of).

*****************************************************************************/

struct convertutf8_list {
	struct convertutf8_list *next;
	char *rename_from;
	char *rename_to;
};

struct convertutf8_status {
	struct convertutf8_list *list;
	int status;
};

/* Find folders that need to change */

static void convertutf8_build_list(const char *inbox_name,
				   void *arg)
{
	struct convertutf8_status *status=
		(struct convertutf8_status *)arg;
	struct convertutf8_list *list;

	char *converted=imap_foldername_to_filename(0, inbox_name);

	if (!converted)
	{
		fprintf(stderr,
			"Error: %s: does not appear to be valid"
			" modified-UTF7\n",
			inbox_name);
		status->status=1;
		return;
	}

	if (strcmp(converted, inbox_name) == 0)
	{
		free(converted);
		return;
	}
	list=(struct convertutf8_list *)malloc(sizeof(struct convertutf8_list));

	if (!list || !(list->rename_from=strdup(inbox_name)))
	{
		perror("malloc");
		exit(1);
	}

	list->rename_to=converted;
	list->next=status->list;
	status->list=list;
}

void convertutf8(const char *maildir, const char *mailfilter, int doit)
{
	struct convertutf8_status status;
	struct convertutf8_list *list;
	char *courierimapsubscribed;
	char *courierimapsubscribed_new;
	FILE *courierimapsubscribed_fp;
	struct maildirfilter mf;
	int mf_status;
	char *mailfilter_newname=0;

	memset(&status, 0, sizeof(status));
	memset(&mf, 0, sizeof(mf));

	printf("Checking %s:\n", maildir);

	maildir_list(maildir, convertutf8_build_list, &status);

	if (status.status)
		exit(status.status);

	if (mailfilter)
	{
		struct maildirfilterrule *r;

		/*
		** Try to convert folder references from mailfilter
		*/

		mailfilter_newname=malloc(strlen(mailfilter)+10);

		strcat(strcpy(mailfilter_newname, mailfilter), ".new");
		mf_status=maildir_filter_loadrules(&mf, mailfilter);

		if (mf_status != MF_LOADOK && mf_status != MF_LOADNOTFOUND)
		{
			fprintf(stderr, "Error: cannot load %s\n",
				mailfilter);
		}

		for (r=mf.first; r; r=r->next)
		{
			char *converted;

			/* Look for deliveries to a folder */

			if (strncmp(r->tofolder, "INBOX.", 6))
				continue;

			converted=imap_foldername_to_filename(0, r->tofolder);

			if (!converted)
			{
				fprintf(stderr, "Error: %s: "
					"%s: does not appear to be valid "
					"modified-UTF7\n",
					mailfilter,
					r->tofolder);
				status.status=1;
			}

			if (strcmp(converted, r->tofolder) == 0)
			{
				free(converted);
				continue;
			}

			printf("Mail filter to %s updated to %s\n",
			       r->tofolder, converted);

			free(r->tofolder);
			r->tofolder=converted;
		}

		if (mf_status == MF_LOADOK)
		{
			FILE *fp=fopen(mailfilter, "r");
			char detected_from[1024];
			char detected_tomaildir[1024];
			char buffer[1024];
			struct stat st_buf;

			/*
			** We need to know the FROM address and the MAILDIR
			** reference. Attempt to ham-fistedly parse the
			** current filter file.
			*/

			detected_from[0]=0;
			detected_tomaildir[0]=0;

			if (!fp)
			{
				perror(mailfilter);
				exit(1);
			}

			while (fgets(buffer, sizeof(buffer), fp))
			{
				char *p, *q;

				if (strncmp(buffer, "FROM='", 6) == 0)
				{
					char *p=strchr(buffer+6, '\'');

					if (!p)
					{
						fprintf(stderr,
							"Cannot parse %s\n",
							mailfilter);
						status.status=1;
					}
					else
					{
						*p=0;
						strcpy(detected_from, buffer+6);

						/*
						  Unescape, because saverules()
						  escapes it.
						*/

						for (q=p=detected_from; *p; p++)
						{
							if (*p == '\\' && p[1])
								++p;
							*q=*p;
							++q;
						}
						*q=0;
					}
				}

				if (strncmp(buffer, "to \"", 4) == 0)
				{
					p=strstr(buffer+4, "/.\"");

					if (!p)
					{
						fprintf(stderr,
							"Cannot parse %s\n",
							mailfilter);
						status.status=1;
					}
					else
					{
						*p=0;
					}
					strcpy(detected_tomaildir, buffer+4);
				}
			}
			fclose(fp);
			if (detected_from[0] == 0 ||
			    detected_tomaildir[0] == 0)
			{
				fprintf(stderr,
					"Failed to parse %s\n",
					mailfilter);
				status.status=1;
			}
			maildir_filter_saverules(&mf, mailfilter_newname,
						 detected_tomaildir,
						 detected_from);

			if (stat(mailfilter, &st_buf) ||
			    chmod(mailfilter_newname, st_buf.st_mode))
			{
				perror(mailfilter);
				exit(1);
			}
			/*
			** If we're root, preserve the ownership and permission
			*/
			if (geteuid() == 0)
			{
				if (chown(mailfilter_newname, st_buf.st_uid,
					  st_buf.st_gid))
				{
					perror(mailfilter_newname);
					exit(1);
				}
			}
		}
	}
	else
	{
		mf_status=MF_LOADNOTFOUND;
	}

	courierimapsubscribed=malloc(strlen(maildir)+100);
	courierimapsubscribed_new=malloc(strlen(maildir)+100);

	strcat(strcpy(courierimapsubscribed, maildir),
	       "/courierimapsubscribed");
	strcat(strcpy(courierimapsubscribed_new, maildir),
	       "/courierimapsubscribed.new");

	/*
	** Update folder references in the IMAP subscription file.
	*/

	if ((courierimapsubscribed_fp=fopen(courierimapsubscribed, "r"))
	    != NULL)
	{
		char buffer[2048];
		FILE *new_fp=fopen(courierimapsubscribed_new, "w");
		char *converted;
		struct stat st_buf;

		while (fgets(buffer, sizeof(buffer), courierimapsubscribed_fp))
		{
			char *p=strchr(buffer, '\n');

			if (!p)
			{
				fprintf(stderr, "Error: courierimapsubscribed: "
					"folder name too long\n");
				status.status=1;
				continue;
			}

			*p=0;

			converted=imap_foldername_to_filename(0, buffer);

			if (!converted)
			{
				fprintf(stderr, "Error: courierimapsubscribed: "
					"%s: does not appear to be valid "
					"modified-UTF7\n",
					buffer);
				status.status=1;
				continue;
			}
			fprintf(new_fp, "%s\n", converted);

			if (strcmp(buffer, converted))
			{
				printf("Subscription to %s changed to %s\n",
				       buffer, converted);
			}
			free(converted);
		}
		if (fflush(new_fp) || fclose(new_fp))
		{
			exit(1);
		}
		fclose(courierimapsubscribed_fp);

		/*
		** If we're root, preserve the ownership and permission
		*/
		if (stat(courierimapsubscribed, &st_buf) ||
		    chmod(courierimapsubscribed_new, st_buf.st_mode))
		{
			perror(courierimapsubscribed);
			exit(1);
		}

		if (geteuid() == 0)
		{
			if (chown(courierimapsubscribed_new, st_buf.st_uid,
				  st_buf.st_gid))
			{
				perror(courierimapsubscribed_new);
				exit(1);
			}
		}
	}

	if (status.status)
	{
		unlink(courierimapsubscribed_new);
		if (mf_status == MF_LOADOK)
			unlink(mailfilter_newname);
		exit(status.status);
	}
	for (list=status.list; list; list=list->next)
	{
		char *frompath, *topath;

		printf("Rename %s to %s\n", list->rename_from, list->rename_to);

		frompath=malloc(strlen(maildir)+strlen(list->rename_from));
		topath=malloc(strlen(maildir)+strlen(list->rename_to));

		if (!frompath || !topath)
		{
			perror("malloc");
			exit(1);
		}

		strcat(strcpy(frompath, maildir), "/");
		strcat(strcpy(topath, maildir), "/");

		/* They all have the INBOX. prefix, strip it off */
		strcat(frompath, strchr(list->rename_from, '.'));
		strcat(topath, strchr(list->rename_to, '.'));

		if (doit)
		{
			if (rename(frompath, topath))
			{
				fprintf(stderr,
					"FATAL ERROR RENAMING %s to %s: %s\n",
					frompath, topath, strerror(errno));
				status.status=1;
			}
		}
		free(frompath);
		free(topath);
	}

	if (doit)
	{
		if (courierimapsubscribed_fp)
		{
			printf("Updating %s\n", courierimapsubscribed);

			if (rename(courierimapsubscribed_new,
				   courierimapsubscribed))
			{
				fprintf(stderr,
					"FATAL ERROR RENAMING %s to %s: %s\n",
					courierimapsubscribed_new,
					courierimapsubscribed, strerror(errno));
				status.status=1;
			}
		}

		if (mf_status == MF_LOADOK)
		{
			printf("Updating %s\n", mailfilter);

			if (rename(mailfilter_newname, mailfilter))
			{
				fprintf(stderr,
					"FATAL ERROR RENAMING %s to %s: %s\n",
					mailfilter_newname, mailfilter,
					strerror(errno));
				status.status=1;
			}
		}
	}
	else
	{
		if (courierimapsubscribed_fp)
		{
			printf("Verified %s\n", courierimapsubscribed);
		}

		if (mf_status == MF_LOADOK)
		{
			printf("Verified %s\n", mailfilter);
		}
	}
	exit(status.status);
}

int main(int argc, char *argv[])
{
const char *maildir, *folder=0;
char *p;
int	argn;
int	perm=0700;
int	musthavefolder=0;
int	subdirperm;
char	*addshared=0, *delshared=0;
const char *quota=0;
char	*tmp=0;

	for (argn=1; argn<argc; argn++)
	{
		if (argv[argn][0] != '-')	break;
		if (strcmp(argv[argn], "-") == 0)	break;
		if (strncmp(argv[argn], "-f", 2) == 0)
		{
			folder=argv[argn]+2;
			if (*folder == 0 && argn+1 < argc)
				folder=argv[++argn];
			continue;
		}
		if (strncmp(argv[argn], "-F", 2) == 0)
		{
			int converr;

			const char *p=argv[argn]+2;

			if (*p == 0 && argn+1 < argc)
				p=argv[++argn];

			folder=unicode_convert_tobuf(p,
						       unicode_default_chset(),
						       unicode_x_smap_modutf8,
						       &converr);

			if (converr || !folder)
			{
				fprintf(stderr, "Cannot convert %s to maildir encoding\n",
					p);
				exit(1);
			}
			continue;
		}
		if (strcmp(argv[argn], "-S") == 0)
		{
			perm=0755;
			continue;
		}

		if (strncmp(argv[argn], "-s", 2) == 0)
		{
		const char *p=argv[argn]+2;

			if (*p == 0 && argn+1 < argc)
				p=argv[++argn];

			perm=0755;
			for (; *p; ++p)
			{
				if (isspace((int)(unsigned char)*p) ||
					*p == ',')
					continue;
				if (*p == 'r')
					perm=0755;
				else if (*p == 'w')
					perm=0777 | S_ISVTX;
				else if (*p == 'g')
					perm &= ~0007;

				while (*p && !isspace((int)(unsigned char)*p)
					&& *p != ',')
					++p;
				if (!*p)	break;
			}
			musthavefolder=1;
			continue;
		}

		if (strncmp(argv[argn], "-q", 2) == 0)
		{
			const char *p=argv[argn]+2;

			if (*p == 0 && argn+1 < argc)
				p=argv[++argn];

			quota=p;
			continue;
		}

		if (strcmp(argv[argn], "--add") == 0 && argc-argn > 1)
		{
			addshared=argv[++argn];
			continue;
		}

		if (strcmp(argv[argn], "--del") == 0 && argc-argn > 1)
		{
			delshared=argv[++argn];
			continue;
		}

		if (strcmp(argv[argn], "--convutf8") == 0 && argc-argn > 1)
		{
			char *maildir=argv[++argn];
			char *mailfilter=argn < argc ? argv[++argn]:0;
			convertutf8(maildir, mailfilter, 1);
			exit(0);
		}

		if (strcmp(argv[argn], "--checkutf8") == 0 && argc-argn > 1)
		{
			char *maildir=argv[++argn];
			char *mailfilter=argn < argc ? argv[++argn]:0;
			convertutf8(maildir, mailfilter, 0);
			exit(0);
		}
		usage();
	}

	if (argn == argc)	usage();
	maildir=argv[argn];

	if (addshared)
	{
		add(maildir, addshared);
		exit (0);
	}

	if (delshared)
	{
		del(maildir, delshared);
		exit (0);
	}

	if (folder && *folder == '.')
	{
		printf("Invalid folder name: %s\n", folder);
		exit(1);
	}

	if (folder)
	{
		if ((p=(char *)malloc(strlen(maildir)+strlen(folder)+4)) == 0)
		{
			perror("maildirmake malloc");
			exit(1);
		}
		strcat(strcat(strcpy(p, maildir), "/."), folder);
		maildir=p;
	}
	else	if (musthavefolder)
		usage();

	if (quota)
	{
		struct stat stat_buf;

		if (stat(maildir, &stat_buf) < 0 && errno == ENOENT)
		{
			if (maildir_make(maildir, perm & ~0022,
					 (perm & ~0022),
					 0) < 0)
			{
				perror(maildir);
				exit(1);
			}
		}

		maildir_quota_set(maildir, quota);
		exit(0);
	}
	subdirperm=perm;
	if (!folder)	subdirperm=0700;
	umask(0);
	if (maildir_make(maildir, perm & ~0022, subdirperm, folder ? 1:0) < 0)
	{
		tmp=(char *)malloc(strlen(maildir)+14);
		if (!tmp) {
			perror("maildirmake malloc");
			exit(1);
		}
		snprintf(tmp, 14+strlen(maildir), "maildirmake: %s", maildir);
		perror(tmp);
		exit(1);
	}
	exit(0);
	return (0);
}
