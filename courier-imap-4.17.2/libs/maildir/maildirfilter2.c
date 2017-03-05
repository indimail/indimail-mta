/*
** Copyright 2000-2007 Double Precision, Inc.
** See COPYING for distribution information.
*/

#include	"maildirfilter.h"
#include	"maildirfilterconfig.h"
#include	"maildircreate.h"
#include	<stdlib.h>
#include	<string.h>
#include	<stdio.h>
#include	<ctype.h>
#include	<errno.h>
#if	HAVE_SYS_STAT_H
#include	<sys/stat.h>
#endif
#if	HAVE_UNISTD_H
#include	<unistd.h>
#endif


static const char *maildir_filter_config(const char *maildir,
	const char *varname)
{
char *p=malloc(strlen(maildir)+sizeof("/maildirfilterconfig"));
FILE	*f;
static char configbuf[256];

	if (!p)	return (0);

	strcat(strcpy(p, maildir), "/maildirfilterconfig");
	f=fopen(p, "r");
	free(p);

	if (!f)	f=fopen(MAILDIRFILTERCONFIG, "r");
	if (!f)	return (0);

	while ((p=fgets(configbuf, sizeof(configbuf), f)) != 0)
	{
		if ((p=strchr(configbuf, '\n')) != 0)	*p=0;
		p=strchr(configbuf, '=');
		if (!p)	continue;
		*p++=0;
		if (strcmp(configbuf, varname) == 0)
		{
			fclose(f);
			return (p);
		}
	}
	fclose(f);
	return ("");
}

static char *maildir_filter_config_maildirfilter(const char *maildir)
{
const char *p=maildir_filter_config(maildir, "MAILDIRFILTER");
char *q;

	if (!p)	return (0);
	if (*p == 0)
	{
		errno=ENOENT;
		return (0);
	}

	q=malloc(strlen(maildir)+strlen(p)+2);
	if (!q)
		return NULL;

	*q=0;
	if (*p != '/')
		strcat(strcpy(q, maildir), "/");
	strcat(q, p);
	return (q);
}

int maildir_filter_importmaildirfilter(const char *maildir)
{
	const char *p=maildir_filter_config(maildir, "MAILDIRFILTER");
	char *maildirfilter;
	FILE *i, *o;
	struct maildir_tmpcreate_info createInfo;

	if (!p)	return (-1);

	if (!*p)
	{
		errno=ENOENT;
		return (-1);
	}

	maildirfilter=maildir_filter_config_maildirfilter(maildir);
	if (!maildirfilter)	return (-1);

	maildir_tmpcreate_init(&createInfo);

	createInfo.maildir=maildir;
	createInfo.uniq="maildirfilter-tmp";
	createInfo.doordie=1;

	if ((o=maildir_tmpcreate_fp(&createInfo)) == NULL)
	{
		free(maildirfilter);
		return (-1);
	}

	strcat(strcpy(createInfo.newname, maildir),
	       "/maildirfilter.tmp"); /* We enough we have enough mem: .uniq */

	if ((i=fopen(maildirfilter, "r")) == 0)
	{
	struct	maildirfilter mf;

		if (errno != ENOENT)
		{
			fclose(o);
			unlink(createInfo.tmpname);
			maildir_tmpcreate_free(&createInfo);
			free(maildirfilter);
			return (-1);
		}

		memset(&mf, 0, sizeof(mf));
		fclose(o);
		unlink(createInfo.tmpname);
		unlink(createInfo.newname);
		maildir_filter_savemaildirfilter(&mf, maildir, "");
		/* write out a blank one */
	}
	else
	{
		char	buf[BUFSIZ];
		int	n;

		while ((n=fread(buf, 1, sizeof(buf), i)) > 0)
			if (fwrite(buf, 1, n, o) != n)
			{
				fclose(o);
				fclose(i);
				unlink(createInfo.tmpname);
				maildir_tmpcreate_free(&createInfo);
				free(maildirfilter);
				return (-1);
			}
		if (fflush(o))
		{
			fclose(o);
			fclose(i);
			unlink(createInfo.tmpname);
			maildir_tmpcreate_free(&createInfo);
			free(maildirfilter);
			return (-1);
		}
		fclose(o);
		fclose(i);
		if (chmod(createInfo.tmpname, 0600)
		    || rename(createInfo.tmpname, createInfo.newname))
		{
			unlink(createInfo.tmpname);
			maildir_tmpcreate_free(&createInfo);
			free(maildirfilter);
			return (-1);
		}
	}

	maildir_tmpcreate_free(&createInfo);
	free(maildirfilter);
	return (0);
}

int maildir_filter_loadmaildirfilter(struct maildirfilter *mf, const char *maildir)
{
char *newname=malloc(strlen(maildir)+sizeof("/maildirfilter.tmp"));
int	rc;

	if (!newname)	return (-1);
	strcat(strcpy(newname, maildir), "/maildirfilter.tmp");

	rc=maildir_filter_loadrules(mf, newname);
	free(newname);
	if (rc && rc != MF_LOADNOTFOUND)
		rc= -1;
	else
		rc=0;
	return (rc);
}


int maildir_filter_savemaildirfilter(struct maildirfilter *mf, const char *maildir,
			 const char *from)
{
	const char *maildirpath=maildir_filter_config(maildir, "MAILDIR");
	struct maildir_tmpcreate_info createInfo;
	int fd, rc;

	if (!maildirpath || !*maildirpath)
	{
		errno=EINVAL;
		return (-1);
	}

	maildir_tmpcreate_init(&createInfo);
	createInfo.maildir=maildir;
	createInfo.uniq="maildirfilter-tmp";
	createInfo.doordie=1;

	if ((fd=maildir_tmpcreate_fd(&createInfo)) < 0)
		return -1;

	close(fd);
	unlink(createInfo.tmpname);

	strcat(strcpy(createInfo.newname, maildir), "/maildirfilter.tmp");

	rc=maildir_filter_saverules(mf, createInfo.tmpname,
				    maildir, maildirpath, from);
	if (rc == 0 && rename(createInfo.tmpname, createInfo.newname))
		rc= -1;
	maildir_tmpcreate_free(&createInfo);
	return (rc);
}

int maildir_filter_exportmaildirfilter(const char *maildir)
{
char *maildirfilter=maildir_filter_config_maildirfilter(maildir);
char *newname;
int	rc;

	if (!maildirfilter)	return (-1);

	newname=malloc(strlen(maildir)+sizeof("/maildirfilter.tmp"));
	if (!newname)
	{
		free(maildirfilter);
		return (-1);
	}

	strcat(strcpy(newname, maildir), "/maildirfilter.tmp");
	rc=rename(newname, maildirfilter);
	free(maildirfilter);
	free(newname);
	return (rc);
}

int maildir_filter_hasmaildirfilter(const char *maildir)
{
const char *p=maildir_filter_config(maildir, "MAILDIR");

	if (!p || !*p)	return (-1);

	p=maildir_filter_config(maildir, "MAILDIRFILTER");
	if (!p || !*p)	return (-1);
	return (0);
}

void maildir_filter_endmaildirfilter(const char *maildir)
{
char *maildirfilter=maildir_filter_config_maildirfilter(maildir);
char *newname;

	if (!maildirfilter)	return;

	newname=malloc(strlen(maildir)+sizeof("/maildirfilter.tmp"));
	if (!newname)
	{
		free(maildirfilter);
		return;
	}

	strcat(strcpy(newname, maildir), "/maildirfilter.tmp");
	unlink(newname);
	free(maildirfilter);
	free(newname);
}
