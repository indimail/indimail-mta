/*
** Copyright 2003 Double Precision, Inc.
** See COPYING for distribution information.
*/

#if	HAVE_CONFIG_H
#include	"config.h"
#endif
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<errno.h>
#include	<ctype.h>
#include	<fcntl.h>
#if	HAVE_UNISTD_H
#include	<unistd.h>
#endif
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

#include	<sys/types.h>
#include	<sys/stat.h>

#include	"config.h"
#include	"maildirinfo.h"

#include	"maildirmisc.h"
#include	"maildirnewshared.h"
#include	"unicode/courier-unicode.h"

void maildir_info_destroy(struct maildir_info *info)
{
	if (info->homedir)
		free(info->homedir);
	if (info->maildir)
		free(info->maildir);
	if (info->owner)
		free(info->owner);
	info->homedir=NULL;
	info->maildir=NULL;
	info->owner=NULL;
}

struct imap_find_shared {
	struct maildir_info *info;
	const char *path;
	size_t path_l;
	char *homedir;
	char *maildir;
};

static int imap_find_cb(struct maildir_newshared_enum_cb *cb)
{
	struct imap_find_shared *ifs=(struct imap_find_shared *)cb->cb_arg;

	if (cb->homedir)
	{
		ifs->homedir=strdup(cb->homedir);
		if (!ifs->homedir)
			return -1;
	}

	if (cb->maildir)
	{
		ifs->maildir=strdup(cb->maildir);

		if (!ifs->maildir)
		{
			if (cb->homedir)
			{
				free(ifs->homedir);
				ifs->homedir=NULL;
			}
			return -1;
		}
	}

	return 0;
}

int maildir_info_imap_find(struct maildir_info *info, const char *path,
			   const char *myId)
{
	const char *p;
	struct imap_find_shared ifs;
	const char *indexfile;
	char *indexfile_cpy;
	struct maildir_shindex_cache *curcache;
	const char *subhierarchy;

	info->homedir=NULL;
	info->maildir=NULL;
	info->owner=NULL;

	if (strchr(path, '/'))
	{
		errno=EINVAL;
		return -1;
	}

	for (p=path; *p; p++)
		if (*p == '.' && p[1] == '.')
		{
			errno=EINVAL;
			return -1;
		}

	if (strncmp(path, SHARED, sizeof(SHARED)-1) == 0)
	{
		path += sizeof(SHARED)-1;

		info->homedir=strdup(".");
		if (!info->homedir)
			return -1;

		info->mailbox_type=MAILBOXTYPE_OLDSHARED;
		info->owner=strdup("anonymous");

		if (!info->owner)
		{
			maildir_info_destroy(info);
			return -1;
		}

		/* We need to specialcase "shared" and "shared.name".
		** maildir_shareddir will return NULL for these cases, because
		** it will insist on "name.folder", but we need to return a
		** non NULL value to indicate that this is a valid hierarchy
		** name.  We return a special value of an empty string, which
		** is checked for in situations where a valid folder is
		** required.
		*/

		if (*path && *path != '.')
		{
			maildir_info_destroy(info);
			errno=EINVAL;
			return -1;
		}

		return 0;
	}

	if (strncasecmp(path, INBOX, sizeof(INBOX)-1) == 0)
	{
		switch (path[sizeof(INBOX)-1]) {
		case 0:
		case '.':
			break;
		default:
			errno=EINVAL;
			return -1;
		}

		info->homedir=strdup(".");

		if (!info->homedir)
			return -1;

		info->maildir=strdup(path);
		if (!info->maildir)
		{
			maildir_info_destroy(info);
			return -1;
		}

		info->owner=malloc(strlen(myId)+sizeof("user="));

		if (!info->owner)
		{
			maildir_info_destroy(info);
			return -1;
		}

		info->mailbox_type=MAILBOXTYPE_INBOX;
		strcat(strcpy(info->owner, "user="), myId);
		return 0;
	}

	if (strncmp(path, NEWSHARED,
		    sizeof(NEWSHARED)-1) != 0)
	{
		errno=EINVAL;
		return -1;
	}

	ifs.info=info;
	ifs.path=path+sizeof(NEWSHARED)-1;

	info->mailbox_type=MAILBOXTYPE_NEWSHARED;
	info->homedir=NULL;
	info->maildir=NULL;
	info->owner=strdup("vendor=courier.internal");

	if (!info->owner)
		return -1;

	ifs.homedir=NULL;
	ifs.maildir=NULL;

	indexfile=NULL;
	indexfile_cpy=NULL;
	curcache=NULL;
	subhierarchy=NULL;

	while (*ifs.path)
	{
		int rc, eof;
		size_t i;

		curcache=maildir_shared_cache_read(curcache, indexfile,
						   subhierarchy);

		if (indexfile_cpy)
		{
			free(indexfile_cpy);
			indexfile_cpy=NULL;
		}

		if (!curcache)
			break;

		p=strchr(ifs.path, '.');

		if (p)
			ifs.path_l=p-ifs.path;
		else
			ifs.path_l=strlen(ifs.path);


		if (ifs.homedir)
			free(ifs.homedir);
		if (ifs.maildir)
			free(ifs.maildir);

		ifs.homedir=NULL;
		ifs.maildir=NULL;

		for (i=0; i < curcache->nrecords; i++)
		{
			char *n=maildir_info_imapmunge(curcache->
						       records[i].name);

			if (n == NULL)
			{
				i=curcache->nrecords;
				break;
			}

			if (strlen(n) == ifs.path_l &&
			    strncmp(n, ifs.path, ifs.path_l) == 0)
			{
				free(n);
				break;
			}
			free(n);
		}

		if (i >= curcache->nrecords)
			break;

		curcache->indexfile.startingpos=
			curcache->records[i].offset;
		rc=maildir_newshared_nextAt(&curcache->indexfile,
					    &eof,
					    imap_find_cb, &ifs);

		if (rc || eof)
		{
			fprintf(stderr, "ERR: Internal error -"
				" maildir_newshared_nextAt: %s\n",
				strerror(errno));
			fflush(stderr);
			break;
		}

		if (!ifs.homedir && !ifs.maildir)
			break;

		if (!ifs.homedir)
		{
			indexfile=indexfile_cpy=ifs.maildir;
			ifs.maildir=NULL;
			subhierarchy=curcache->records[i].name;

			ifs.path += ifs.path_l;
			if (*ifs.path)
				++ifs.path;
			continue;
		}

		info->homedir=maildir_location(ifs.homedir,
					     ifs.maildir);

		free(ifs.homedir);
		free(ifs.maildir);

		free(info->owner);

		if (!info->homedir)
		{
			info->maildir=NULL;
			info->owner=NULL;
			return -1;
		}

		if (!subhierarchy || !*subhierarchy)
		{
			info->owner=strdup("vendor=courier.internal");
			if (!info->owner)
			{
				free(info->homedir);
				info->homedir=NULL;
				info->maildir=NULL;
				return -1;
			}
		}
		else
		{
			char *owner_utf8;

			info->owner=malloc(strlen(subhierarchy)
					   +sizeof("user="));

			if (!info->owner)
			{
				free(info->homedir);
				info->homedir=NULL;
				info->maildir=NULL;
				info->owner=NULL;
				return -1;
			}
			strcpy(info->owner, "user=");
			strcat(info->owner, subhierarchy);

			/*
			** The folder path is in modified-UTF7.  The owner is
			** obtained from shared hierarchy, but in ACL2 the
			** identifiers are in UTF8.
			*/

			owner_utf8=
				unicode_convert_tobuf(info->owner,
							unicode_x_imap_modutf7,
							"utf-8", NULL);

			if (!owner_utf8)
			{
				free(info->homedir);
				info->homedir=NULL;
				return (0);
			}

			free(info->owner);
			info->owner=owner_utf8;
		}

		ifs.path += ifs.path_l;

		info->maildir=malloc(strlen(INBOX)+1+strlen(ifs.path));
		if (!info->maildir)
		{
			free(info->owner);
			free(info->homedir);
			info->owner=NULL;
			info->homedir=NULL;
			return -1;
		}
		strcat(strcpy(info->maildir, INBOX), ifs.path);

		if (maildir_info_suppress(info->homedir))
		{

			free(info->homedir);
			free(info->maildir);
			info->homedir=NULL;
			info->maildir=NULL;
			info->mailbox_type=MAILBOXTYPE_IGNORE;
			free(info->owner);
			info->owner=NULL;
			info->owner=strdup("vendor=courier.internal");
			if (!info->owner)
			{
				return -1;
			}
		}

		return 0;
	}

	if (indexfile_cpy)
		free(indexfile_cpy);
	if (ifs.homedir)
	{
		free(ifs.homedir);
		ifs.homedir=NULL;
	}

	if (ifs.maildir)
	{
		free(ifs.maildir);
		ifs.maildir=NULL;
	}
	return 0;
}

/***************************************************************************/

/*
** Maildir folders are named in IMAP-compatible modified-UTF8 encoding,
** with periods as hierarchy delimiters.  One exception: ".", "/", "~", and
** ":" are also encoded using modified-UTF8, making folder names that contain
** those characters incompatible with IMAP.
**
** smap_to_utf8 crates a modified-UTF8-encoded folder name from a vector
** of UTF-8 words.
**
** input:  "INBOX" "a" "b"
** output: "INBOX.a.b"
**
*/

static char *smap_to_utf8(char **ptr)
{
	char *f=NULL;
	char *n;

	while ((n=*ptr++) != NULL && *n)
	{
		char *p=unicode_convert_tobuf(n, "utf-8",
					      unicode_x_smap_modutf8,
					      NULL);

		if (!p)
		{
			if (f)
				free(f);
			return NULL;
		}

		n= f ? realloc(f, strlen(f)+strlen(p)+2):malloc(strlen(p)+1);

		if (!n)
		{
			free(p);
			if (f)
				free(f);
			return NULL;
		}
		if (f)
			f=strcat(strcat(n, "."), p);
		else
			f=strcpy(n, p);
		free(p);
	}

	if (!f)
		errno=EINVAL;
	return f;
}

/*
** Convert modified-UTF8 folder name into an array of UTF-8 words, that
** represent a folder name.
*/

char **maildir_smapfn_fromutf8(const char *modutf8)
{
	char *p=strdup(modutf8), *q;
	size_t n, i;
	char **fn;

	if (!p)
		return NULL;

	n=1;
	for (i=0; p[i]; i++)
		if (p[i] == '.' && p[i+1] && p[i+1] != '.')
		{
			++n;
		}

	fn=malloc((n+1)*sizeof(char *));

	if (!fn)
	{
		free(p);
		return NULL;
	}

	n=0;
	q=p;
	do
	{
		for (i=0; q[i]; i++)
			if (q[i] == '.' && q[i+1] && q[i+1] != '.')
			{
				q[i++]=0;
				break;
			}

		fn[n]=unicode_convert_tobuf(q,
					    unicode_x_smap_modutf8,
					    "utf-8", NULL);
		q += i;

		if (!fn[n])
		{
			while (n)
				free(fn[--n]);
			free(fn);
			free(p);
			return NULL;
		}
		n++;
	} while (*q);
	fn[n]=0;
	free(p);
	return fn;
}

void maildir_smapfn_free(char **fn)
{
	size_t i;

	for (i=0; fn[i]; i++)
		free(fn[i]);
	free(fn);
}

struct get_existing_folder_info {
	char **fn;
	char *pathname;
};

static void get_existing_callback(const char *f, void *vp)
{
	char **fn;

	struct get_existing_folder_info *gefi=
		(struct get_existing_folder_info *)vp;
	size_t i;
	size_t j;

	if (gefi->pathname)
		return;

	fn=maildir_smapfn_fromutf8(f);
	if (!fn)
	{
		perror(f);
		return;
	}

	for (i=0; gefi->fn[i]; i++)
		if (fn[i] == NULL || strcmp(fn[i], gefi->fn[i]))
		{
			maildir_smapfn_free(fn);
			return;
		}

	maildir_smapfn_free(fn);

	for (j=0; i && f[j]; j++)
		if (f[j] == '.' && f[j+1] && f[j+1] != '.')
		{
			--i;
			if (i == 0)
				break;
		}

	gefi->pathname=malloc(j+1);

	if (!gefi->pathname)
	{
		perror("malloc");
		return;
	}

	memcpy(gefi->pathname, f, j);
	gefi->pathname[j]=0;
}

static char *smap_path(const char *homedir,
		       char **words)  /* words[0] better be INBOX! */
{
	struct get_existing_folder_info gefi;
	char *n, *p;
	struct stat stat_buf;

	if ((n=smap_to_utf8(words)) == NULL)
		return NULL;

	p=maildir_name2dir(homedir, n);

	if (!p)
	{
		free(n);
		return NULL;
	}

	if (stat(p, &stat_buf) == 0)
	{
		free(p);
		return n;
	}

	gefi.fn=words;
	gefi.pathname=NULL;

	maildir_list(homedir ? homedir:".",
		     &get_existing_callback, &gefi);

	if (gefi.pathname)
	{
		free(n);
		free(p);

		return gefi.pathname;
	}

	free(p);
	return n;
}

int maildir_info_smap_find(struct maildir_info *info, char **folder,
			   const char *myId)
{
	char *p;
	size_t n;
	const char *indexfile;
	struct maildir_shindex_cache *curcache;
	const char *subhierarchy;
	struct imap_find_shared ifs;
	int rc, eof;
	char *indexfile_cpy=NULL;

	info->homedir=NULL;
	info->maildir=NULL;
	info->owner=NULL;
	info->mailbox_type=MAILBOXTYPE_IGNORE;

	if (folder[0] == NULL)
	{
		errno=EINVAL;
		return -1;
	}

	if (strcmp(folder[0], PUBLIC))
	{
		if (strcmp(folder[0], INBOX))
		{
			errno=EINVAL;
			return -1;
		}

		info->maildir=smap_path(NULL, folder);

		if (info->maildir == NULL)
			return -1;
		info->homedir=strdup(".");
		if (!info->homedir)
		{
			maildir_info_destroy(info);
			return -1;
		}

		info->mailbox_type=MAILBOXTYPE_INBOX;

		info->owner=malloc(strlen(myId)+sizeof("user="));

		if (!info->owner)
		{
			maildir_info_destroy(info);
			return -1;
		}

		strcat(strcpy(info->owner, "user="), myId);

		return 0;
	}

	indexfile=NULL;
	curcache=NULL;
	subhierarchy=NULL;
	n=1;
	ifs.homedir=NULL;
	ifs.maildir=NULL;

	while (folder[n])
	{
		size_t i;

		curcache=maildir_shared_cache_read(curcache, indexfile,
						   subhierarchy);

		if (!curcache)
			break;

		for (i=0; i<curcache->nrecords; i++)
			if (strcmp(curcache->records[i].name,
				   folder[n]) == 0)
				break;

		if (i >= curcache->nrecords)
			break;
		curcache->indexfile.startingpos=
			curcache->records[i].offset;

		if (ifs.homedir)
			free(ifs.homedir);
		if (ifs.maildir)
			free(ifs.maildir);
		ifs.homedir=NULL;
		ifs.maildir=NULL;

		rc=maildir_newshared_nextAt(&curcache->indexfile,
					    &eof,
					    imap_find_cb, &ifs);

		if (rc || eof)
		{
			fprintf(stderr, "ERR: Internal error -"
				" maildir_newshared_nextAt: %s\n",
				strerror(errno));
			fflush(stderr);
			break;
		}

		if (!ifs.homedir && !ifs.maildir)
			break;

		if (!ifs.homedir)
		{
			if (indexfile_cpy)
				free(indexfile_cpy);
			indexfile=indexfile_cpy=ifs.maildir;
			ifs.maildir=NULL;
			subhierarchy=curcache->records[i].name;
			++n;
			continue;
		}

		if (indexfile_cpy)
			free(indexfile_cpy);
		info->homedir=maildir_location(ifs.homedir,
					       ifs.maildir);
		free(ifs.homedir);
		free(ifs.maildir);

		info->maildir=NULL;

		if (maildir_info_suppress(info->homedir))
		{

			free(info->homedir);
			info->homedir=NULL;
			info->maildir=NULL;
			info->mailbox_type=MAILBOXTYPE_IGNORE;
			info->owner=NULL;
			info->owner=strdup("vendor=courier.internal");
			if (!info->owner)
			{
				maildir_info_destroy(info);
				return -1;
			}

			return 0;
		}


		if (!subhierarchy || !*subhierarchy)
		{
			info->owner=strdup("vendor=courier.internal");
			if (!info->owner)
			{
				maildir_info_destroy(info);
				return -1;
			}
		}
		else
		{
			info->owner=malloc(strlen(subhierarchy)
					   +sizeof("user="));

			if (!info->owner)
			{
				free(info->homedir);
				info->homedir=NULL;
				info->maildir=NULL;
				return -1;
			}
			strcpy(info->owner, "user=");
			strcat(info->owner, subhierarchy);
		}

		p=folder[n];
		folder[n]=INBOX;
		info->maildir=smap_path(info->homedir, folder+n);
		folder[n]=p;

		if (!info->maildir)
		{
			free(info->homedir);
			free(info->owner);
			info->homedir=NULL;
			info->maildir=NULL;
			info->owner=NULL;
			return -1;
		}

		info->mailbox_type=MAILBOXTYPE_NEWSHARED;
		return 0;
	}

	if (ifs.homedir)
		free(ifs.homedir);
	if (ifs.maildir)
		free(ifs.maildir);
	if (indexfile_cpy)
		free(indexfile_cpy);

	if (folder[n] == 0)
	{
		info->mailbox_type=MAILBOXTYPE_NEWSHARED;
		info->owner=strdup("vendor=courier.internal");
		if (!info->owner)
		{
			maildir_info_destroy(info);
			return -1;
		}

		/* Intermediate shared namespce */
		return 0;
	}

	return -1;
}

static int complex_flag;

void maildir_info_munge_complex(int f)
{
	complex_flag=f;
}

static size_t munge_complex(const char *, char *);

char *maildir_info_imapmunge(const char *name)
{
	char *n=unicode_convert_tobuf(name, "utf-8",
					unicode_x_imap_modutf7, NULL);
	char *p;
	size_t cnt;

	if (!n)
		return NULL;

	if (!complex_flag)
	{
		for (p=n; *p; p++)
		{
			if (*p == '.' || *p == '/')
				*p=' ';
		}

		return n;
	}

	cnt=munge_complex(n, NULL);
	p=malloc(cnt);
	if (!p)
	{
		free(n);
		return NULL;
	}

	munge_complex(n, p);

	free(n);
	return p;
}

static size_t munge_complex(const char *orig, char *n)
{
	size_t cnt=0;

	while (*orig)
	{
		switch (*orig) {
		case '.':
			if (n)
			{
				*n++='\\';
				*n++=':';
			}
			cnt += 2;
			break;
		case '/':
			if (n)
			{
				*n++='\\';
				*n++=';';
			}
			cnt += 2;
			break;
		case '\\':
			if (n)
			{
				*n++='\\';
				*n++='\\';
			}
			cnt += 2;
			break;
		default:
			if (n) *n++ = *orig;
			++cnt;
		}
		++orig;
	}

	if (n) *n=0;
	return cnt+1;
}
