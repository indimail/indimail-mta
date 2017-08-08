/*
** Copyright 2003 Double Precision, Inc.
** See COPYING for distribution information.
*/

#include	"maildiraclt.h"
#include	"maildirmisc.h"
#include	"maildircreate.h"
#include	"maildirnewshared.h"
#include	"numlib/numlib.h"
#include	<time.h>
#if	HAVE_UNISTD_H
#include	<unistd.h>
#endif
#include	<string.h>
#include	<errno.h>
#include	<stdio.h>
#include	<stdlib.h>


int maildir_newshared_disabled=0;

int maildir_newshared_open(const char *indexfile,
			   struct maildir_newshared_enum_cb *info)
{
	info->indexfile=indexfile;
	if ((info->fp=fopen(maildir_newshared_disabled ?
			    "/dev/null":indexfile, "r")) == NULL)
		return -1;
	info->startingpos=0;
	info->linenum=0;
	return 0;
}

void maildir_newshared_close(struct maildir_newshared_enum_cb *info)
{
	if (info->fp)
		fclose(info->fp);
	info->fp=NULL;
}

int maildir_newshared_nextAt(struct maildir_newshared_enum_cb *info,
			     int *eof,
			     int (*cb_func)(struct maildir_newshared_enum_cb*),
			     void *cb_arg)
{
	if (fseek(info->fp, info->startingpos, SEEK_SET) < 0)
		return -1;
	info->linenum= -1;
	return maildir_newshared_next(info, eof, cb_func, cb_arg);
}

int maildir_newshared_next(struct maildir_newshared_enum_cb *info,
			   int *eof,
			   int (*cb_func)(struct maildir_newshared_enum_cb *),
			   void *cb_arg)
{
	char linebuf[BUFSIZ];
	int rc;
	char *p;
	const char *name;
	const char *homedir;
	const char *maildir;
	uid_t uid;
	gid_t gid;
	off_t nbytes;

#define CB_INIT(name_,homedir_,maildir_,uid_,gid_) \
	info->name=name_; info->homedir=homedir_; info->maildir=maildir_; \
	info->uid=uid_; info->gid=gid_; info->cb_arg=cb_arg;

	*eof=0;

	while (fgets(linebuf, sizeof(linebuf), info->fp) != NULL)
	{
		nbytes=strlen(linebuf);

		if (nbytes && linebuf[nbytes-1] == '\n')
			linebuf[nbytes-1]=0;

		p=strchr(linebuf, '#');
		if (p) *p=0;

		p=strchr(linebuf, '\t');
		++info->linenum;
		if (p)
		{
			name=linebuf;
			*p++=0;

			if (*p == '*')
			{
				p=strchr(p, '\t');
				if (p)
				{
					const char *q;
					size_t n;

					*p++=0;
					maildir=p;
					p=strchr(p, '\t');
					if (p) *p=0;

					q=strrchr(info->indexfile, '/');
					if (q)
						++q;
					else q=info->indexfile;

					n=strlen(info->indexfile)-strlen(q);

					p=malloc(n+strlen(maildir)+1);
					if (!p)
						return -1;

					if (n)
						memcpy(p, info->indexfile, n);
					strcpy(p+n, maildir);


					CB_INIT(name, NULL, p, 0, 0);

					info->cb_arg=cb_arg;
					rc= (*cb_func)(info);

					free(p);
					info->startingpos += nbytes;
					return rc;
				}
			}
			else
			{
				uid=libmail_atouid_t(p);
				p=strchr(p, '\t');
				if (uid && p)
				{
					*p++=0;
					gid=libmail_atogid_t(p);
					p=strchr(p, '\t');
					if (gid && p)
					{
						*p++=0;
						homedir=p;
						p=strchr(p, '\t');
						maildir="./Maildir";

						if (p)
						{
							*p++=0;
							if (*p && *p != '\t')
								maildir=p;
							p=strchr(p, '\t');
							if (p) *p=0;
						}

						CB_INIT(name, homedir,
							maildir,
							uid,
							gid);

						info->cb_arg=cb_arg;
						rc=(*cb_func)(info);
						info->startingpos += nbytes;
						return rc;
					}
				}
			}
		}

		if (linebuf[0])
		{
			fprintf(stderr, "ERR: %s(%d): syntax error.\n",
				info->indexfile, (int)info->linenum);
		}
		info->startingpos += nbytes;
	}
	*eof=1;
	return 0;
}

int maildir_newshared_enum(const char *indexfile,
			   int (*cb_func)(struct maildir_newshared_enum_cb *),
			   void *cb_arg)
{
	struct maildir_newshared_enum_cb cb;
	int eof;
	int rc;

	if (maildir_newshared_open(indexfile, &cb) < 0)
		return -1;

	while ((rc=maildir_newshared_next(&cb, &eof, cb_func, cb_arg)) == 0)
	{
		if (eof)
		{
			maildir_newshared_close(&cb);
			return 0;
		}
	}

	maildir_newshared_close(&cb);
	return rc;
}
