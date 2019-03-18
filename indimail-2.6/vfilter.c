/*-
 * $Log: vfilter.c,v $
 * Revision 2.52  2018-09-11 15:08:18+05:30  Cprogrammer
 * fixed compiler warnings
 *
 * Revision 2.51  2017-05-01 20:17:34+05:30  Cprogrammer
 * removed mailing list feature from vfilter
 *
 * Revision 2.50  2017-03-13 14:13:16+05:30  Cprogrammer
 * replaced INDIMAILDIR with PREFIX
 *
 * Revision 2.49  2015-12-17 17:36:46+05:30  Cprogrammer
 * fixed issue of indimail.org string in X-Filter header
 *
 * Revision 2.48  2011-12-24 09:02:30+05:30  Cprogrammer
 * set PWSTRUCT for non-existent user
 *
 * Revision 2.47  2011-11-09 19:46:07+05:30  Cprogrammer
 * removed getversion
 *
 * Revision 2.46  2011-06-20 21:30:25+05:30  Cprogrammer
 * fixed Mail BlackHoled message
 *
 * Revision 2.45  2010-10-09 13:48:15+05:30  Cprogrammer
 * display the mda used in errors
 *
 * Revision 2.44  2010-04-30 14:43:58+05:30  Cprogrammer
 * free pointer returned by replacestr()
 *
 * Revision 2.43  2008-07-13 19:49:45+05:30  Cprogrammer
 * use ERESTART only if available
 *
 * Revision 2.42  2008-06-13 10:44:14+05:30  Cprogrammer
 * fixed compilation error if VFILTER not defined
 *
 * Revision 2.41  2008-06-05 16:22:12+05:30  Cprogrammer
 * moved vdelivermail to sbin
 *
 * Revision 2.40  2008-05-28 17:40:49+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 2.39  2007-12-22 00:42:32+05:30  Cprogrammer
 * BUG - corrected 'Ends with' case
 *
 * Revision 2.38  2006-02-03 10:25:10+05:30  Cprogrammer
 * Removed 'Mails Blackholed' message for successful folder delivery
 *
 * Revision 2.37  2005-12-21 09:51:47+05:30  Cprogrammer
 * make gcc 4 happy
 *
 * Revision 2.36  2005-03-01 19:30:14+05:30  Cprogrammer
 * made MDA configurable
 *
 * Revision 2.35  2005-02-09 22:45:44+05:30  Cprogrammer
 * added missing new line in display of message
 *
 * Revision 2.34  2005-01-27 13:10:15+05:30  Cprogrammer
 * Indicate blackholed mails
 *
 * Revision 2.33  2004-10-27 17:22:23+05:30  Cprogrammer
 * BUG - mailing list variable was not initialized
 *
 * Revision 2.32  2004-10-08 10:17:23+05:30  Cprogrammer
 * bounds check for headerlist
 *
 * Revision 2.31  2004-08-16 18:33:21+05:30  Cprogrammer
 * BUG - header_list was being initialized only under certain conditions
 *
 * Revision 2.30  2004-07-17 14:35:51+05:30  Cprogrammer
 * qqeh argument to qmail-local
 *
 * Revision 2.29  2004-07-03 23:55:21+05:30  Cprogrammer
 * use parse_email()
 *
 * Revision 2.28  2004-07-02 18:12:42+05:30  Cprogrammer
 * renamed .noprefilt to noprefilt, .nopostfilt to nopostfilt, .vfilter to vfilter
 *
 * Revision 2.27  2004-06-19 00:22:42+05:30  Cprogrammer
 * renamed vfilter id to prefilt
 *
 * Revision 2.26  2004-05-10 18:25:10+05:30  Cprogrammer
 * added postfilter
 * added option to disable global and postfilters
 *
 * Revision 2.25  2004-04-08 13:07:26+05:30  Cprogrammer
 * code overhaul. Moved major chunk of code as a function process_filter()
 * added code for numerical comparisions
 *
 * Revision 2.24  2003-12-30 00:31:45+05:30  Cprogrammer
 * use headerlist() to get mail headers
 *
 * Revision 2.23  2003-10-01 13:00:50+05:30  Cprogrammer
 * changed id for global filter to vfilter
 * prevent core dump if DestFolder is null
 *
 * Revision 2.22  2003-09-16 12:35:05+05:30  Cprogrammer
 * use postmaster's filter as a global filter for all users
 *
 * Revision 2.21  2003-06-03 00:12:24+05:30  Cprogrammer
 * removed extra setbuf
 *
 * Revision 2.20  2003-04-24 18:09:43+05:30  Cprogrammer
 * corrected bug with comparision of addresses with emailid
 *
 * Revision 2.19  2003-03-27 16:30:02+05:30  Cprogrammer
 * bug fix - comparasion of Bcc was not correct
 * allow match if To, Cc, Bcc is addressed to an alias domain
 *
 * Revision 2.18  2003-01-17 01:07:36+05:30  Cprogrammer
 * made matches case insensitve
 *
 * Revision 2.17  2003-01-08 20:03:18+05:30  Cprogrammer
 * prevent status getting clobbered by longjmp or vfork
 *
 * Revision 2.16  2002-12-13 19:11:21+05:30  Cprogrammer
 * added environment variable MAKE_SEEKABLE to turn on/off seekable stdin
 *
 * Revision 2.15  2002-12-11 10:29:14+05:30  Cprogrammer
 * added ERESTART check for errno
 *
 * Revision 2.14  2002-12-06 23:41:24+05:30  Cprogrammer
 * replaced printf() with format_filter_display()
 *
 * Revision 2.13  2002-11-30 09:38:33+05:30  Cprogrammer
 * bounce only if userNotFound is set
 *
 * Revision 2.12  2002-11-24 15:34:38+05:30  Cprogrammer
 * setting of destination folder corrected
 *
 * Revision 2.11  2002-11-22 01:17:00+05:30  Cprogrammer
 * incorrect usage of sizeof() corrected
 *
 * Revision 2.10  2002-11-21 00:59:14+05:30  Cprogrammer
 * added functionality to handle local users
 *
 * Revision 2.9  2002-11-13 13:38:16+05:30  Cprogrammer
 * added filter name
 *
 * Revision 2.8  2002-10-20 22:18:58+05:30  Cprogrammer
 * correction for solaris compilation
 *
 * Revision 2.7  2002-10-18 23:53:32+05:30  Cprogrammer
 * conditional compilation for Regular Expression Matching
 *
 * Revision 2.6  2002-10-18 01:18:58+05:30  Cprogrammer
 * removed global variables
 * changed get_options() to accomodate variables emailid and Maildir
 *
 * Revision 2.5  2002-10-16 20:04:18+05:30  Cprogrammer
 * added missing break statement in switch()
 * extensive rewrite of myExit() function (bugfix and option to junk mail if folder is /NoDeliver)
 * changed printBounce() function to deliberately send misleading information that user does not exist (for SPAM)
 *
 * Revision 2.4  2002-10-15 11:43:35+05:30  Cprogrammer
 * added forwarding functionality in filters
 *
 * Revision 2.3  2002-10-13 02:01:06+05:30  Cprogrammer
 * corrected core dump when argc == 1
 *
 * Revision 2.2  2002-10-12 02:40:11+05:30  Cprogrammer
 * added PWSTRUCT variable for vdelivermail optimization
 * added X-Filter Header
 *
 * Revision 2.1  2002-10-12 01:49:35+05:30  Cprogrammer
 * vfilter routine for IndiMail
 *
 */

#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: vfilter.c,v 2.52 2018-09-11 15:08:18+05:30 Cprogrammer Exp mbhangui $";
#endif

#ifdef VFILTER
#include "eps.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#ifdef HAVE_FNMATCH_H
#define _GNU_SOURCE
#include <fnmatch.h>
#endif
#include "evaluate.h"

static void     usage();
static int      get_options(int argc, char **argv, char *, char *, char *, char *, char *);
static int      myExit(int, char **, int, int, char *, char *);
static void     printBounce(char *);
static void     process_filter(int, char **, struct header **, char *, int *, char *, int *, int *, char *, char *, int *, char *);
int             numerical_compare(char *, char *);

int             interactive;

int
main(int argc, char **argv)
{
	unsigned char  *l = NULL;
	char           *str;
	char          **tmp_ptr;
	struct header **hptr, **ptr;
	struct eps_t   *eps = NULL;
	struct header_t *h = NULL;
	int             ret = 0, fd = 0;
	int             header_name, comparision, bounce_action, filter_no, MsgSize;
	char            emailid[1025], Maildir[MAX_BUFF], filter_name[MAX_BUFF];
	char            keyword[MAX_BUFF], folder[MAX_BUFF], filterid[MAX_BUFF];
	char            bounce[AUTH_SIZE], forward[AUTH_SIZE], user[AUTH_SIZE];
	char            tmpFlag[MAX_BUFF + 28], domain[AUTH_SIZE];

	*emailid = *user = *domain = *Maildir = 0;
	setbuf(stdout, 0);
	if (get_options(argc, argv, bounce, emailid, user, domain, Maildir))
		myExit(argc, argv, -1, 0, 0, 0);
#ifdef MAKE_SEEKABLE
	if ((str = getenv("MAKE_SEEKABLE")) && *str != '0' && makeseekable(stdin))
		myExit(argc, argv, -1, 0, 0, 0);
#endif
	if (!(MsgSize = get_message_size()))
	{
		printf("Discarding 0 size message\n");
		_exit(0);
	}
	if (!(eps = eps_begin(INTERFACE_STREAM, (int *) &fd)))
	{
		fprintf(stderr, "eps_begin: Error");
		myExit(argc, argv, -1, 0, 0, 0);
	}
	for (h = eps_next_header(eps), hptr = (struct header **) 0; h; h = eps_next_header(eps))
	{
		if ((h->name) && h->data)
		{
			storeHeader(&hptr, h);
			if (interactive && verbose)
				printf("%s: %s\n", h->name, h->data);
		}
		eps_header_free(eps);
	}
	if (interactive && verbose)
		printf("\n");
	for (l = eps_next_line(eps); l; l = eps_next_line(eps))
	{
		if (interactive && verbose)
			printf("%s\n", l);
	}
	if (interactive && verbose)
		printf("\n");
	while ((!(eps->u->b->eof)) && (eps->content_type & CON_MULTI))
	{
		if (!(ret = mime_init_stream(eps)))
			break;
		for (h = mime_next_header(eps); h; h = mime_next_header(eps))
		{
			if ((h->name) && (h->data))
			{
				storeHeader(&hptr, h);
				if (interactive && verbose)
					printf("%s: %s\n", h->name, h->data);
			}
			header_kill(h);
		}
		if (interactive && verbose)
			printf("\n");
		for (l = mime_next_line(eps); l; l = mime_next_line(eps))
		{
			if (interactive && verbose)
				printf("%s\n", l);
		}
	}
	eps_end(eps);
	/*- Filter Engine */
	for (ptr = hptr; ptr && *ptr; ptr++)
	{
		if (verbose && interactive)
			printf("%-25s\n", (*ptr)->name);
		for (tmp_ptr = (*ptr)->data; tmp_ptr && *tmp_ptr; tmp_ptr++)
		{
			lowerit(*tmp_ptr);
			if (verbose && interactive)
				printf("                          -> %s\n", *tmp_ptr);
		}
	}
	/*- Global prefilt ID Filter */
	if ((str = strrchr(emailid, '@')))
		snprintf(filterid, sizeof(filterid), "prefilt%s", str);
	else
		strncpy(filterid, "prefilt", sizeof(filterid));
	snprintf(tmpFlag, sizeof(tmpFlag) - 1, "%snoprefilt", Maildir);
	if (access(tmpFlag, F_OK))
		process_filter(argc, argv, hptr, filterid, &filter_no, filter_name, &header_name,
			&comparision, keyword, folder, &bounce_action, forward);

	/*- Process User Filter */
	snprintf(tmpFlag, sizeof(tmpFlag) - 1, "%svfilter", Maildir);
	if (access(tmpFlag, F_OK))
	{
		if (interactive)
			return (0);
		myExit(argc, argv, 0, 0, 0, 0);
	}
	process_filter(argc, argv, hptr, emailid, &filter_no, filter_name, &header_name, &comparision,
		keyword, folder, &bounce_action, forward);

	/*- Global postfilt ID Filter */
	if ((str = strrchr(emailid, '@')))
		snprintf(filterid, sizeof(filterid) - 1, "postfilt%s", str);
	else
		strncpy(filterid, "postfilt", sizeof(filterid) - 1);
	snprintf(tmpFlag, sizeof(tmpFlag) - 1, "%snopostfilt", Maildir);
	if (access(tmpFlag, F_OK))
		process_filter(argc, argv, hptr, filterid, &filter_no, filter_name, &header_name,
			&comparision, keyword, folder, &bounce_action, forward);
	if (interactive && verbose)
		printf("Passed All Filters\n");
	myExit(argc, argv, 0, 0, 0, 0);
	return (0);/*- Not reached */
}

static void
process_filter(int argc, char **argv, struct header **hptr, char *filterid, int *filter_no, char *filter_name, int *header_name,
			   int *comparision, char *keyword, char *folder, int *bounce_action, char *forward)
{
	int             i, j, filter_opt, ret = 0, global_filter = 0, max_header_value;
	char           *str, *real_domain;
	char          **tmp_ptr, **address_list;
	static char   **header_list;
	struct header **ptr;

	if (interactive && verbose)
		printf("Processing Filter %s\n", filterid);
	if (!strncmp(filterid, "prefilt@", 8) || !strncmp(filterid, "postfilt@", 9))
		global_filter = 1;
	if (!header_list && !(header_list = headerList()))
		header_list = vfilter_header;
	for (max_header_value = 0; header_list[max_header_value]; max_header_value++);
	max_header_value--;
	for (j = 0;;)
	{
		i = vfilter_select(filterid, filter_no, filter_name, header_name, comparision,
			keyword, folder, bounce_action, forward);
		if (i == -1)
		{
			fprintf(stderr, "vfilter_select: failure\n");
			break;
		} else
		if (i == -2)
			break;
		if (interactive && verbose && !j++)
		{
			if (global_filter)
				printf("No  global Filter                 FilterName Header          Comparision                Keyword         Folder          Action\n");
			else
				printf("No  EmailId                       FilterName Header          Comparision                Keyword         Folder          Action\n");
			printf("-----------------------------------------------------------------------------------------------------------------------------------------\n");
		}
		if (interactive && verbose)
			format_filter_display(0, *filter_no, filterid, filter_name, *header_name, *comparision,
				keyword, folder, forward, *bounce_action);
		/*
		 * comparision 5 - Sender not in Address Book
		 * comparision 6 - ID not in To, Cc, Bcc
		 */
		if (!global_filter && (*comparision == 5 || *comparision == 6))
		{
			address_list = getAddressBook(filterid);
			if (interactive && verbose)
				printf
					("-----------------------------------------------------------------------------------------------------------------------------------------\n");
			for (ret = 0, ptr = hptr; ptr && *ptr && !ret; ptr++)
			{
				if (!strncasecmp((*ptr)->name, "From", 4) || !strncasecmp((*ptr)->name, "Return-Path", 11))
					filter_opt = 1;
				else
				if (!strncasecmp((*ptr)->name, "To", 2) || !strncasecmp((*ptr)->name, "Cc", 2) ||
					!strncasecmp((*ptr)->name, "Bcc", 3))
					filter_opt = 2;
				else
					continue;
				for (tmp_ptr = (*ptr)->data; tmp_ptr && *tmp_ptr && !ret; tmp_ptr++)
				{
					/*- Sender not in Address Book */
					if (*comparision == 5 && filter_opt == 1)
					{
						for (rewindAddrToken();;)
						{
							if (!(str = addressToken(*tmp_ptr)))
								break;
							if (!strncasecmp(str, filterid, MAX_PW_NAME + MAX_DOMAINNAME + 2))
							{
								/*- Allow the user to send himself mail */
								ret = 1;
								break;
							}
							for (i = 0; address_list && address_list[i]; i++)
							{
								if (!address_list[i] || !*address_list[i])
									continue;
								if (!strncasecmp(str, address_list[i], MAX_PW_NAME + MAX_DOMAINNAME + 2))
								{
									ret = 1;
									break;
								}
							}
						}
					}
					/*- ID not in To, Cc, Bcc */
					if (*comparision == 6 && filter_opt == 2)
					{
						char           *tmpstr, *sptr, *cptr;
						char            tmpUser[MAX_BUFF];
						int             tmplen;

						for (rewindAddrToken();;)
						{
							if (!(str = addressToken(*tmp_ptr)))
								break;
							for (sptr = str, cptr = tmpUser, tmplen = 0; *sptr && *sptr != '@'; *cptr++ = *sptr++, tmplen++);
							*cptr++ = '@';
							*cptr = 0;
							if (!(real_domain = vget_real_domain(sptr + 1)))
								tmpstr = str;
							else
							{
								strncat(tmpUser, real_domain, MAX_BUFF - tmplen - 1);
								tmpstr = tmpUser;
							}
							if (!strncasecmp(tmpstr, filterid, MAX_PW_NAME + MAX_DOMAINNAME + 2))
							{
								ret = 1;
								break;
							}
							if (ret)
								break;
						}
					}
				} /*- for(tmp_ptr = (*ptr)->data;tmp_ptr && *tmp_ptr && !ret;tmp_ptr++) */
			} /*- for(ret = 0, ptr = hptr;ptr && *ptr && !ret;ptr++) */
			if (!ret)
			{
				if (interactive && verbose)
					printf("Matched Filter No %d Comparision %s\n", *filter_no, vfilter_comparision[*comparision]);
				myExit(argc, argv, 1, *bounce_action, folder, forward);
			}
		} else
		if (*comparision < 5 || *comparision == 7 || *comparision == 8)
		{
			/*
			 * 0 - Equals
			 * 1 - Contains
			 * 2 - Does not contain
			 * 3 - Starts with
			 * 4 - Ends with
			 * 7 - Numerical logical expression
			 * 8 - Regular expression
			 */
			if (*header_name > max_header_value) /*- Invalid header value in vfilter table */
				continue;
			lowerit(keyword);
			for (ptr = hptr; ptr && *ptr; ptr++)
			{
				if (!strncasecmp((*ptr)->name, header_list[*header_name], MAX_LINE_LENGTH))
				{
					switch (*comparision)
					{
					case 0:	/*- Equals */
						for (tmp_ptr = (*ptr)->data; tmp_ptr && *tmp_ptr; tmp_ptr++)
						{
							if (!strncasecmp(*tmp_ptr, keyword, MAX_LINE_LENGTH))
							{
								if (interactive && verbose)
									printf("Matched Filter No %d Data %s Keyword %s\n", *filter_no, *tmp_ptr, keyword);
								myExit(argc, argv, 1, *bounce_action, folder, forward);
							}
						}
						break;
					case 1:	/*- Contains */
						for (tmp_ptr = (*ptr)->data; tmp_ptr && *tmp_ptr; tmp_ptr++)
						{
							if (strstr(*tmp_ptr, keyword))
							{
								if (interactive && verbose)
									printf("Matched Filter No %d Data %s Keyword %s\n", *filter_no, *tmp_ptr, keyword);
								myExit(argc, argv, 1, *bounce_action, folder, forward);
							}
						}
						break;
					case 2:	/*- Does not contain */
						for (ret = 0, tmp_ptr = (*ptr)->data; tmp_ptr && *tmp_ptr; tmp_ptr++)
						{
							if (strstr(*tmp_ptr, keyword))
							{
								ret = 1;
								break;
							}
						}
						if (!ret)
						{
							if (interactive && verbose)
								printf("Matched Filter No %d Data %s Keyword %s\n", *filter_no, *tmp_ptr, keyword);
							myExit(argc, argv, 1, *bounce_action, folder, forward);
						}
						break;
					case 3:	/*- Starts with */
						for (tmp_ptr = (*ptr)->data; tmp_ptr && *tmp_ptr; tmp_ptr++)
						{
							if (!memcmp(*tmp_ptr, keyword, slen(keyword)))
							{
								if (interactive && verbose)
									printf("Matched Filter No %d Data %s Keyword %s\n", *filter_no, *tmp_ptr, keyword);
								myExit(argc, argv, 1, *bounce_action, folder, forward);
							}
						}
						break;
					case 4:	/*- Ends with */
						for (tmp_ptr = (*ptr)->data; tmp_ptr && *tmp_ptr; tmp_ptr++)
						{
							if ((str = strstr(*tmp_ptr, keyword)) && !strncasecmp(str, keyword, MAX_LINE_LENGTH))
							{
								if (interactive && verbose)
									printf("Matched Filter No %d Comparision %s Data %s Keyword %s\n", *filter_no,
										   vfilter_comparision[*comparision], *tmp_ptr, keyword);
								myExit(argc, argv, 1, *bounce_action, folder, forward);
							}
						}
						break;
					case 7:	/*- Float */
						/*
						 * e.g. tmp_ptr = 0.7, keyword = %p < 0.4
						 */
						for (tmp_ptr = (*ptr)->data; tmp_ptr && *tmp_ptr; tmp_ptr++)
						{
							if (numerical_compare(*tmp_ptr, keyword))
							{
								if (interactive && verbose)
									printf("Matched Filter No %d Data %s Keyword %s Folder %s\n", *filter_no, *tmp_ptr, keyword,
										   folder);
								myExit(argc, argv, 1, *bounce_action, folder, forward);
							}
						}
						break;
#ifdef HAVE_FNMATCH
					case 8:	/*- Regular Expressions */
						for (tmp_ptr = (*ptr)->data; tmp_ptr && *tmp_ptr; tmp_ptr++)
						{
#ifdef FNM_CASEFOLD
							if (!fnmatch(keyword, *tmp_ptr, FNM_PATHNAME | FNM_CASEFOLD))
#else
							if (!fnmatch(keyword, *tmp_ptr, FNM_PATHNAME))
#endif
							{
								if (interactive && verbose)
									printf("Matched Filter No %d Comparision %s Keyword %s\n", *filter_no,
										   vfilter_comparision[*comparision], keyword);
								myExit(argc, argv, 1, *bounce_action, folder, forward);
							}
						}
						break;
#endif
					} /*- switch(comparision) */
				} /*- if(!strncasecmp((*ptr)->name, header_list[header_name], MAX_LINE_LENGTH)) */
			} /*- for(ptr = hptr;ptr && *ptr;ptr++) */
		} /*- if(comparision < 5 || comparision == 7 || comparision == 8) */
	} /*- for (j = 0;;) */
	return;
}

int
execMda(char **argptr, char **mda)
{
	char           *x;
	char          **argv;
	char           *(vdelargs[]) = { PREFIX"/sbin/vdelivermail", "''", BOUNCE_ALL, 0};

	*mda = *vdelargs;
	if ((x = getenv("MDA")))
	{
		if (!(argv = MakeArgs(x)))
		{
			fprintf(stderr, "vfilter: MakeArgs: %s\n", strerror(errno));
			return(1);
		}
		*mda = *argv;
		if (*argv[0] != '/' && *argv[0] != '.')
			execvp(*argv, argv);
		else
			execv(*argv, argv);
		fprintf(stderr, "vfilter: execv: %s: %s\n", *argv, strerror(errno));
	} else
	{
		execv(*vdelargs, argptr);
		fprintf(stderr, "vfilter: execv: %s: %s\n", *vdelargs, strerror(errno));
	}
	return(1);
}

int
numerical_compare(char *data, char *expression)
{
	char           *ptr;
	struct val      result;
	struct vartable *vt;

	/*
	 * replace all occurences of %p in expression
	 * with the value of data
	 */
	if (!(vt = create_vartable()))
		return (0);
	if (!(ptr = replacestr(expression, "%p", data)))
		return (-1);
	switch (evaluate(ptr, &result, vt))
	{
	case ERROR_SYNTAX:
		if (interactive && verbose)
			printf("syntax error\n");
		if (ptr != expression)
			free(ptr);
		return (-1);
	case ERROR_VARNOTFOUND:
		if (interactive && verbose)
			printf("variable not found\n");
		if (ptr != expression)
			free(ptr);
		return (-1);
	case ERROR_NOMEM:
		if (interactive && verbose)
			printf("not enough memory\n");
		if (ptr != expression)
			free(ptr);
		return (-1);
	case ERROR_DIV0:
		if (interactive && verbose)
			printf("division by zero\n");
		if (ptr != expression)
			free(ptr);
		return (-1);
	case RESULT_OK:
		if (result.type == T_INT)
		{
			if (interactive && verbose)
				printf("result = %ld\n", result.ival);
			if (ptr != expression)
				free(ptr);
			return (result.ival);
		} else
		{
			if (interactive && verbose)
				printf("result = %g\n", result.rval);
			if (ptr != expression)
				free(ptr);
			return (0);
		}
	}
	if (ptr != expression)
		free(ptr);
	free_vartable(vt);
	return (0);
}

static int
get_options(int argc, char **argv, char *bounce, char *emailid, char *user, char *domain, char *Maildir)
{
	int             c, local;
	char           *tmpstr, *real_domain = 0;
	char            pwstruct[MAX_BUFF + 28];
	struct passwd  *pw;

	local = 0;
	if (argc == 3 && strncmp(argv[1], "-v", 2) && strncmp(argv[1], "-V", 2))
	{
		scopy(bounce, argv[2], MAX_BUFF);
		/*- get the last parameter in the .qmail-default file */
		if (!(tmpstr = getenv("EXT")))
		{
			fprintf(stderr, "No EXT environment variable\n");
			myExit(argc, argv, 100, 1, 0, 0);
		} else
		{
			if (*tmpstr)
				scopy(user, tmpstr, AUTH_SIZE);
			else
			if (!(tmpstr = getenv("LOCAL")))
			{
				fprintf(stderr, "No LOCAL environment variable\n");
				myExit(argc, argv, 100, 1, 0, 0);
			} else
			{
				scopy(user, tmpstr, AUTH_SIZE);
				local = 1;
			}
		}
		if (local)
			scopy(domain, ((tmpstr = (char *) getenv("DEFAULT_DOMAIN")) ? tmpstr : DEFAULT_DOMAIN), AUTH_SIZE);
		else
		if (!(tmpstr = getenv("HOST")))
		{
			fprintf(stderr, "No HOST environment variable\n");
			myExit(argc, argv, 100, 1, 0, 0);
		} else
			scopy(domain, tmpstr, AUTH_SIZE);
		if (remove_quotes(user))
		{
			fprintf(stderr, "Invalid user [%s]\n", user);
			myExit(argc, argv, 100, 1, 0, 0);
		}
		lowerit(user);
		lowerit(domain);
		if (!(real_domain = vget_real_domain(domain)))
		{
			if (userNotFound)
				myExit(argc, argv, 100, 1, 0, 0);
			myExit(argc, argv, 111, 1, 0, 0);
		}
		snprintf(emailid, 1024, "%s@%s", user, real_domain);
	} else
	if (argc == 11)	/*- qmail-local */
	{
		scopy(user, argv[6], AUTH_SIZE);
		scopy(domain, argv[7], AUTH_SIZE);
		lowerit(user);
		lowerit(domain);
		if (!(real_domain = vget_real_domain(domain)))
			real_domain = domain;
		snprintf(emailid, 1024, "%s@%s", user, real_domain);
	} else
	if (argc == 2 || (argc > 1 && (!strncmp(argv[1], "-v", 2) || !strncmp(argv[1], "-V", 2))))
	{
		/*
		 * Procedure for manually testing vfilter
		 *
		 * cat full_path_of_an_existing_email_file | vfilter -v user@domain.com
		 * cat full_path_of_an_existing_email_file | vfilter user@domain.com
		 *
		 */
		interactive = 1;
		while ((c = getopt(argc, argv, "v")) != -1)
		{
			switch (c)
			{
			case 'v':
				verbose = 1;
				break;
			default:
				usage();
				return (1);
			}
		}
		if (optind < argc)
			scopy(emailid, argv[optind++], MAX_BUFF);
		if (!*emailid)
		{
			fprintf(stderr, "vfilter: must supply emailid\n");
			usage();
			return (1);
		}
		*user = *domain = 0;
		if (parse_email(emailid, user, domain, AUTH_SIZE))
		{
			fprintf(stderr, "%s: Email too long\n", emailid);
			return (1);
		}
		if (!(real_domain = vget_real_domain(domain)))
			real_domain = domain;
	} else
	{
		fprintf(stderr, "vfilter: Invalid number of arguments\n");
		myExit(argc, argv, 111, 0, 0, 0);
	}
	if (!(pw = (local ? getpwnam(user) : vauth_getpw(user, real_domain))))
	{
		if (userNotFound)
		{
			if (interactive)
			{
				fprintf(stderr, "vfilter: %s@%s: No such user\n", user, real_domain);
				return (1);
			}
			snprintf(pwstruct, sizeof(pwstruct), "PWSTRUCT=No such user %s@%s",
				user, real_domain);
			if (putenv(pwstruct) == -1)
			{
				fprintf(stderr, "vfilter: putenv: %s\n", strerror(ENOMEM));
				if (interactive)
					return (1);
				_exit(111);
			}
			myExit(argc, argv, -1, 0, 0, 0);
		} else
		{
			fprintf(stderr, "vfilter: Temporary database Errror\n");
			if (interactive)
				return (1);
			myExit(argc, argv, 111, 0, 0, 0);
		}
	}
	snprintf(pwstruct, sizeof(pwstruct), "PWSTRUCT=%s@%s:%s:%d:%d:%s:%s:%s:%d",
		pw->pw_name, real_domain, pw->pw_passwd, pw->pw_uid,
		local ? 0 : pw->pw_gid, pw->pw_gecos &&
		*(pw->pw_gecos) ? pw->pw_gecos : pw->pw_name, pw->pw_dir,
		local ? "NOQUOTA" : pw->pw_shell, is_inactive);
	if (putenv(pwstruct) == -1)
	{
		fprintf(stderr, "vfilter: putenv: %s\n", strerror(ENOMEM));
		if (interactive)
			return (1);
		_exit(111);
	}
	snprintf(Maildir, MAX_BUFF, "%s/Maildir/", pw->pw_dir);
	return (0);
}

/*
 * status value :
 *  -1 : Filter failure - pass it to vdelivermail
 *   0 : Passed through the filter
 * 111 : Temporary Error
 *   n : Message has been filtered
 */
static int
myExit(int argc, char **argv, int status, int bounce, char *DestFolder, char *forward)
{
	char           *revision = "$Revision: 2.52 $";
	char           *ptr, *mda;
	char            MaildirFolder[MAX_BUFF], XFilter[MAX_BUFF];
	pid_t           pid;
	int             tmp_stat, wait_status;
	static int      _status;

	_status = status;
	if (interactive)
		_exit(_status ? 1 : 0);
	snprintf(XFilter, sizeof(XFilter) - 26, "XFILTER=X-Filter: xFilter/IndiMail Revision %s", revision + 11);
	if ((ptr = strrchr(XFilter, '$')))
		*ptr = 0;
	strncat(XFilter, "(http://www.indimail.org)", 26);
	if (putenv(XFilter) == -1)
	{
		fprintf(stderr, "vfilter: putenv: %s\n", strerror(ENOMEM));
		_exit(111);
	}
	if (_status == 111)
		_exit(_status);
	else /*- Mail has been matched by filter */
	if (_status > 0)
	{
		if (DestFolder && *DestFolder && strncasecmp(DestFolder, "/NoDeliver", 10))
		{
			switch (pid = vfork())
			{
			case -1:
				_status = -1;
				break;
			case 0:
				if (strncasecmp(DestFolder, "Inbox", 5))
				{
					snprintf(MaildirFolder, sizeof(MaildirFolder), "MAILDIRFOLDER=%s", DestFolder);
					if (putenv(MaildirFolder) == -1)
					{
						fprintf(stderr, "vfilter: putenv: %s\n", strerror(ENOMEM));
						_exit(111);
					}
				}
				execMda(argv, &mda);
				_exit(111);
			default: /*- parent */
				for (;;)
				{
					pid = wait(&wait_status);
#ifdef ERESTART
					if (pid == -1 && (errno == EINTR || errno == ERESTART))
#else
					if (pid == -1 && errno == EINTR)
#endif
						continue;
					else
					if (pid == -1)
					{
						fprintf(stderr, "vfilter: %s. indimail bug\n", mda);
						_status = -1;
					}
					break;
				}
				if (_status == -1)
					break;
				if (WIFSTOPPED(wait_status) || WIFSIGNALED(wait_status))
				{
					fprintf(stderr, "vfilter: %s crashed.\n", mda);
					_status = -1;
				} else
				if (WIFEXITED(wait_status))
				{
					switch ((tmp_stat = WEXITSTATUS(wait_status)))
					{
					case 0:
						if (!bounce)
							_exit(0);
						break;
					case 100:
					case 111:
						_exit(tmp_stat);
					default:
						fprintf(stderr, "vfilter: Unable to run %s.\n", mda);
						_status = -1;
						break;
					}
				}
				break;
			} /*- switch (pid = vfork()) */
		} /*- if(strncasecmp(DestFolder, "/NoDeliver", 10)) */
		if (_status > 0)
		{
			if (bounce > 0)
			{
				if (bounce == 2 || bounce == 3)
				{
					if (interactive && verbose)
						printf("Delivering to %s\n", forward);
					deliver_mail(forward, 0, "NOQUOTA", 0, 0, DEFAULT_DOMAIN, 0, 0);
				}
				if (bounce == 1 || bounce == 3)
				{
					printBounce(argv[2]);
					_exit(100);
				}
			}
			if (DestFolder && *DestFolder && !strncasecmp(DestFolder, "/NoDeliver", 10))
				printf("Mail BlackHoled\n");
			_exit(0);
		}
	} /*- if(_status > 0) */
	/*- Mail has passed through the filter or filter failure */
	execMda(argv, &mda);
	_exit(111);
	return (0);/*- Not reached */
}

static void
printBounce(char *bounce)
{
	char           *ptr;
	char            user[AUTH_SIZE], domain[AUTH_SIZE];

	if (strncmp(bounce, BOUNCE_ALL, strlen(BOUNCE_ALL) + 1))
	{
		printf("Hi. This is the IndiMail MDA for %s\n", (ptr = (char *) getenv("HOST")) ? ptr : DEFAULT_DOMAIN);
		printf("I'm afraid I cannot accept your message as my Spam Filter has decided\n");
		printf("to reject this mail\n");
		printf("Please refrain from sending such mail in future\n");
	} else
	{
		/*- get the last parameter in the .qmail-default file */
		if (!(ptr = getenv("EXT")))
			scopy(user, "???????", AUTH_SIZE);
		else
			scopy(user, ptr, AUTH_SIZE);
		if (!(ptr = getenv("HOST")))
			scopy(domain, ((ptr = (char *) getenv("DEFAULT_DOMAIN")) ? ptr : DEFAULT_DOMAIN), AUTH_SIZE);
		else
			scopy(domain, ptr, AUTH_SIZE);
		printf("No Account %s@%s here by that name. indimail (#5.1.5)", user, domain);
	}
	return;
}

static void
usage()
{
	fprintf(stderr, "usage: vfilter [options] emailid]\n");
	fprintf(stderr, "options: -V ( print version number )\n");
	fprintf(stderr, "         -v ( verbose )\n");
}
#else
int
main(int argc, char **argv)
{
	fprintf(stderr, "IndiMail not configured with --enable-vfilter=y\n");
	return (1);
}
#endif /*- #ifdef VFILTER */

void
getversion_vfilter_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
#ifdef VFILTER
	printf("%s\n", sccsidevalh);
#endif
}
