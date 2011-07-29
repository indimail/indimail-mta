/*
 * $Log: etrn.c,v $
 * Revision 1.9  2011-07-29 09:28:21+05:30  Cprogrammer
 * fixed gcc 4.6 warnings
 *
 * Revision 1.8  2008-06-25 23:15:38+05:30  Cprogrammer
 * change for 64 bit port of indimail
 *
 * Revision 1.7  2007-12-20 12:43:59+05:30  Cprogrammer
 * removed compiler warning
 *
 * Revision 1.6  2004-10-22 20:24:55+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.5  2003-10-23 01:19:58+05:30  Cprogrammer
 * fixed compilation warnings
 *
 * Revision 1.4  2003-09-27 21:29:55+05:30  Cprogrammer
 * added code for reading morercpthosts
 *
 * Revision 1.3  2002-09-04 01:49:41+05:30  Cprogrammer
 * added function count_dir()
 *
 * Revision 1.2  2002-08-25 19:44:57+05:30  Cprogrammer
 * exitcodes logic enhanced
 *
 * Revision 1.1  2002-08-25 03:29:02+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "rcpthosts.h"
#include "etrn.h"
#include "case.h"
#include "sig.h"
#include "stralloc.h"
#include "constmap.h"
#include "control.h"
#include "str.h"
#include "fmt.h"
#include "auto_qmail.h"
#include "wait.h"
#include <ctype.h>
#include <sys/types.h>
#include <unistd.h>

int             err_child();
void            die_nomem();
void            die_control();

static char    *binetrnargs[3] = { 0, 0, 0 };

#include "hasindimail.h"
#ifndef INDIMAIL
#include <stdio.h>
#include <errno.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>

size_t count_dir(char *dir_name, size_t *mailcount)
{
	DIR            *entry;
	struct dirent  *dp;
	struct stat     statbuf;
	size_t          file_size, len, flen;
	size_t          count;
	int             is_trash;
	char           *tmpstr, *ptr;

	if (!dir_name || !*dir_name)
		return (0);
	if (!(entry = opendir(dir_name)))
		return (0);
	if (strstr(dir_name, "/Maildir/.Trash"))
		is_trash = 1;
	else
		is_trash = 0;
	if (mailcount)
		*mailcount = 0;
	for (file_size = 0, tmpstr = 0;;)
	{
		if (!(dp = readdir(entry)))
			break;
		if (!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, ".."))
			continue;
		else
		if (!strcmp(dp->d_name, ".Trash"))
			continue;
		else
		if (!strcmp(dp->d_name, ".current_size"))
			continue;
		else
		if (!strcmp(dp->d_name, "maildirsize"))
			continue;
		else
		if (!strcmp(dp->d_name, "maildirfolder"))
			continue;
		else
		if (!strcmp(dp->d_name, "core"))
			continue;
		else
		if (!strncmp(dp->d_name, "sqwebmail", 9))
			continue;
		else
		if (!strncmp(dp->d_name, "courier", 7))
			continue;
		else
		if (!strcmp(dp->d_name, ".domain"))
			continue;
		else
		if (!strcmp(dp->d_name, ".QuotaWarn"))
			continue;
		else
		if (!strcmp(dp->d_name, ".BulkMail"))
			continue;
		flen = strlen(dp->d_name);
		len = flen + strlen(dir_name) + 2;
		if (!(tmpstr = (char *) realloc(tmpstr, (len * sizeof(char)))))
		{
			fprintf(stderr, "realloc: %d bytes: %s\n", (int) len, strerror(errno));
			closedir(entry);
			return(-1);
		}
		snprintf(tmpstr, len, "%s/%s", dir_name, dp->d_name);
		if ((ptr = strstr(tmpstr, ",S=")) != NULL)
		{
			ptr += 3;
			file_size += atoi(ptr);
			if (mailcount)
				(*mailcount)++;
		} else
		if (!stat(tmpstr, &statbuf))
		{
			if (S_ISDIR(statbuf.st_mode))
			{
				file_size += count_dir(tmpstr, &count);
				if (mailcount)
					*mailcount += count;
			}
			else
			{
				if (*(dp->d_name + flen - 1) == 'T' || is_trash)
					continue;
				if (mailcount)
					(*mailcount)++;
				file_size += statbuf.st_size;
			}
		}
	}
	closedir(entry);
	free(tmpstr);
	return (file_size);
}
#endif

int
etrn_queue(char *arg, char *remoteip)
{
	int             child, r, flagetrn, len, exitcode, wstat;
#ifndef INDIMAIL
	size_t          mailcount;
#else
	mdir_t          mailcount;
#endif
	stralloc        etrn = { 0 };
	char            maildir1[1024], maildir2[1024];
	struct constmap mapetrn;
	static int      flagrcpt = 1;

	if (flagrcpt)
		flagrcpt = rcpthosts_init();
	if ((flagetrn = control_readfile(&etrn, "etrnhosts", 0)) == -1)
		die_control();
	if (flagrcpt || !flagetrn)
		return(-2);
	if (!constmap_init(&mapetrn, etrn.s, etrn.len, 0))
		die_nomem();
	case_lowerb(arg, len = str_len(arg)); /*- convert into lower case */
	if (!constmap(&mapetrn, arg, len))
		return(-2);
	if (rcpthosts(arg, len, 1) != 1)
		return(-2);
	if ((r = fmt_strn(maildir1, auto_qmail, 1024)) > 128)
		return(-1);
	r += fmt_str(maildir1 + r, "/autoturn/");
	r += fmt_strn(maildir1 + r, arg, 119);
	if (r > 256)
		return(-1);
	r += fmt_str(maildir1 + r, "/Maildir/");
	maildir1[r] = 0;
	if ((r = fmt_strn(maildir2, auto_qmail, 1024)) > 128)
		return(-1);
	r += fmt_str(maildir2 + r, "/autoturn/");
	r += fmt_strn(maildir2 + r, remoteip, 119);
	if (r > 256)
		return(-1);
	r += fmt_str(maildir2 + r, "/Maildir/");
	maildir2[r] = 0;

	mailcount = 0;
	if (!access(maildir1, F_OK))
		count_dir(maildir1, &mailcount);
	else
	if (!access(maildir2, F_OK))
		count_dir(maildir2, &mailcount);
	if (!mailcount)
		return(-3);
	switch (child = fork())
	{
	case -1:
		return(-1);
	case 0:
		sig_pipedefault();
		close(1);
		dup2(2, 1);
		binetrnargs[0] = "bin/etrn";
		binetrnargs[1] = arg;
		binetrnargs[2] = remoteip;
		execvp(*binetrnargs, binetrnargs);
		_exit(1);
	}
	if (wait_pid(&wstat, child) == -1)
		return err_child();
	if (wait_crashed(wstat))
		return err_child();
	if ((exitcode = wait_exitcode(wstat)))
	{
		if (exitcode == 4)
			return(mailcount ? mailcount : -4);
		exitcode = 0 - exitcode;
		return(exitcode); /*- no */
	}
	return (0);
}

int
valid_hostname(char *name)
{
	const char     *cp;
	int             label_length = 0;
	int             label_count = 0;
	int             ch;

	if (!name || !*name)
		return (0);
	/*
	 * Find bad characters or label lengths. Find adjacent delimiters.
	 */
	for (cp = name; (ch = *(unsigned char *) cp) != 0; cp++)
	{
		if (isalnum(ch) || ch == '_')
		{
			if (label_length == 0)
				label_count++;
			label_length++;
			if (label_length > VALID_LABEL_LEN)
				return (0);
		} else
		if (ch == '.')
		{
			if (label_length == 0 || cp[1] == 0)
				return (0);
			label_length = 0;
		} else
		if (ch == '-')
		{
			label_length++;
			if (label_length == 1 || cp[1] == 0 || cp[1] == '.')
				return (0);
		} else
			return (0);
	}
	if (cp - name > VALID_HOSTNAME_LEN)
		return (0);
	return (1);
}

void
getversion_etrn_c()
{
	static char    *x = "$Id: etrn.c,v 1.9 2011-07-29 09:28:21+05:30 Cprogrammer Stab mbhangui $";

#ifdef INDIMAIL
	if (x)
		x = sccsidh;
#else
	if (x)
		x++;
#endif
}
