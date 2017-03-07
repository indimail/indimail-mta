/*
 * $Id: util.c,v 1.4 2011-11-26 09:35:17+05:30 Cprogrammer Exp mbhangui $
 * Copyright (C) 1999-2004 Inter7 Internet Technologies, Inc. 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

#include <indimail_config.h>
#undef PACKAGE
#undef VERSION
#undef PACKAGE_NAME
#undef PACKAGE_STRING
#undef PACKAGE_TARNAME
#undef PACKAGE_VERSION
#undef PACKAGE_BUGREPORT
#undef PACKAGE_URL
#include <indimail.h>
#undef PACKAGE
#undef VERSION
#undef PACKAGE_NAME
#undef PACKAGE_STRING
#undef PACKAGE_TARNAME
#undef PACKAGE_VERSION
#undef PACKAGE_BUGREPORT
#undef PACKAGE_URL
#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <dirent.h>
#include <ctype.h>
#include "alias.h"
#include "autorespond.h"
#include "forward.h"
#include "mailinglist.h"
#include "iwebadmin.h"
#include "iwebadminx.h"
#include "printh.h"
#include "user.h"
#include "util.h"

extern FILE    *lang_fs;
extern FILE    *color_table;

#define SORT_TABLE_ENTRIES 100000

/*
 * pointer to array of pointers 
 */
unsigned char **sort_list;

unsigned char  *sort_block[200];	/* memory blocks for sort data */
int             memleft, memindex, sort_entry;
unsigned char  *sort_ptr;

int
sort_init()
{
	sort_entry = memindex = memleft = 0;
	sort_list = malloc(SORT_TABLE_ENTRIES * sizeof (char *));
	if (!sort_list) {
		return -1;
	}
	return 0;
}

/*
 * add entry to list for sorting, copies string until it gets to char 
 * 'end' 
 */
int
sort_add_entry(char *entry, char end)
{
	int             len;

	if (sort_entry == SORT_TABLE_ENTRIES) {
		return -2;				/* table is full */
	}
	if (memleft < 256) {
	/*
	 * allocate a 64K block of memory to store table entries 
	 */
		memleft = 65536;
		sort_ptr = sort_block[memindex++] = malloc(memleft);
	}
	if (!sort_ptr) {
		return -1;
	}
	sort_list[sort_entry++] = sort_ptr;
	len = 1;					/* at least a terminator */
	while (*entry && *entry != end) {
		*sort_ptr++ = *entry++;
		len++;
	}
	*sort_ptr++ = 0;			/* NULL terminator */
	memleft -= len;
	return 0;
}

unsigned char  *
sort_get_entry(int index)
{
	if ((index < 0) || (index >= sort_entry)) {
		return NULL;
	}
	return sort_list[index];
}

void
sort_cleanup()
{
	while (memindex) {
		free(sort_block[--memindex]);
	}
	if (sort_list) {
		free(sort_list);
		sort_list = NULL;
	}
}

/*
 * Comparison routine used in qsort for multiple functions 
 */
static int
sort_compare(const void *p1, const void *p2)
{
	return strcasecmp(*(char **) p1, *(char **) p2);
}

void
sort_dosort()
{
	qsort(sort_list, sort_entry, sizeof (char *), sort_compare);
}

void
str_replace(char *s, char orig, char repl)
{
	while (*s) {
		if (*s == orig) {
			*s = repl;
		}
		s++;
	}
}

void
qmail_button(char *modu, char *command, char *user, char *dom, time_t mytime, char *png)
{
	printf("<td align=center>");
	printh("<a href=\"%s&modu=%C\">", cgiurl(command), modu);
	printf("<img src=\"%s/%s\" border=0></a>", IMAGEURL, png);
	printf("</td>\n");
}

int
check_local_user(user)
	char           *user;
{
	struct stat     buf;
	int             i, j;

/*
 * check for aliases and autoresponders 
 */
	if (valias_select(user, Domain))
		return (-1);

/*
 * check for mailing list 
 */
	strcpy(TmpBuf, ".qmail-");
	for (i = 0, j = 7; user[i] != 0; ++i, ++j) {
		if (user[i] == '.')
			TmpBuf[j] = ':';
		else
			TmpBuf[j] = user[i];
	}
	TmpBuf[j] = 0;
	if (stat(TmpBuf, &buf) == 0)
		return (-1);

/*
 * check for POP/IMAP user 
 */
	if (vauth_getpw(user, Domain))
		return (-1);

	return (0);
}

void
show_counts()
{
	count_users();
	count_forwards();
	count_autoresponders();
	count_mailinglists();

	printf("%s = %d<BR>\n", html_text[61], CurPopAccounts);
	printf("%s = %d<BR>\n", html_text[74], CurForwards);
	printf("%s = %d<BR>\n", html_text[77], CurAutoResponders);
	printf("%s = %d<BR>\n", html_text[80], CurMailingLists);
}

/*
 * check_email_addr( char *addr )
 * *
 * * Make sure 'addr' is a valid email address.  Returns 1 if it's bad,
 * * 0 if it's good.
 */
int
check_email_addr(char *addr)
{
	char           *taddr = addr;
	char           *atpos = NULL;
	char           *dotpos = NULL;

	for (taddr = addr; *taddr != '\0'; ++taddr) {
		if (*taddr == '@') {
			if (atpos != NULL)
				return 1;		/* double @ */
			atpos = taddr;
		} else if (!isalnum(*taddr) && (strchr(".-+=_&", *taddr) == NULL)) {
			return 1;
		}
	}

/*
 * if just a user name with no @domain.com then bad 
 */
	if (atpos == NULL)
		return 1;

/*
 * Look for a sub domain 
 */
	dotpos = strchr(atpos, '.');

/*
 * no '.' in the domain part 
 */
	if (dotpos == NULL)
		return 1;

/*
 * once we know it's good, convert it to lowercase 
 */
	lowerit(addr);

	return 0;
}

int
fixup_local_name(addr)
	char           *addr;
{
	char           *taddr = addr;

	/*- don't allow zero length user names */
	if (strlen(taddr) <= 0)
		return (1);
	/*- force it to lower case */
	lowerit(addr);
	/*- check for valid email address */
	for (taddr = addr; *taddr != 0; ++taddr) {
		if (!isalnum(*taddr) && !ispunct(*taddr))
			return (1);
		if (isspace(*taddr))
			return (1);
		if (ispunct(*taddr) && (strchr(".-+=_&", *taddr) == NULL)) {
			return (1);
		}
	}
	/*- if we made it here, everything is okay */
	return (0);
}

void
ack(msg, extra)
	char           *msg;
	char           *extra;
{
	printf("%s %s\n", get_html_text(msg), extra);
	printf("</BODY></HTML>\n");
	vclose();
	exit(0);
}

void
upperit(instr)
	char           *instr;
{
	while (*instr != 0) {
		if (islower(*instr))
			*instr = toupper(*instr);
		++instr;
	}
}

char           *
safe_getenv(char *var)
{
	char           *s;

	s = getenv(var);
	if (s == NULL)
		return ("");
	return (s);
}

char           *
strstart(sstr, tstr)
	char           *sstr;
	char           *tstr;
{
	char           *ret_str;

	ret_str = sstr;
	if (sstr == NULL || tstr == NULL)
		return (NULL);

	while (*sstr != 0 && *tstr != 0) {
		if (*sstr != *tstr)
			return (NULL);
		++sstr;
		++tstr;
	}

	if (*tstr == 0)
		return (ret_str);
	return (NULL);

}

int
open_lang(char *lang)
{
	char            langfile[200];
	static char    *langpath = NULL;
	struct stat     mystat;

/*
 * do not read lang files with path control characters 
 */
	if (strchr(lang, '.') != NULL || strchr(lang, '/') != NULL)
		return (-1);

/*
 * convert to lower case (using lowerit() from libvpopmail) 
 */
	lowerit(lang);

/*
 * close previous language if still open 
 */
	if (lang_fs != NULL)
		fclose(lang_fs);

	if (langpath == NULL) {
		langpath = getenv(QMAILADMIN_TEMPLATEDIR);
		if (langpath == NULL)
			langpath = HTMLLIBDIR;
	}

	snprintf(langfile, sizeof (langfile), "%s/lang/%s", langpath, lang);

/*
 * do not open symbolic links 
 */
	if (lstat(langfile, &mystat) == -1 || S_ISLNK(mystat.st_mode))
		return (-1);

	if ((lang_fs = fopen(langfile, "r")) == NULL)
		return (-1);

	return (0);
}

char           *
get_html_text(char *index)
{
	return html_text[atoi(index)];
}

int
open_colortable()
{
	char            tmpbuf[200];
	char           *tmpstr;

	tmpstr = getenv(QMAILADMIN_TEMPLATEDIR);
	if (tmpstr == NULL)
		tmpstr = HTMLLIBDIR;

	snprintf(tmpbuf, sizeof (tmpbuf), "%s/html/colortable", tmpstr);
	if ((color_table = fopen(tmpbuf, "r")) == NULL)
		return (-1);
	return (0);
}

char           *
get_color_text(char *index)
{
	static char     tmpbuf[400];
	char           *tmpstr;

	if (color_table == NULL)
		return ("");

	rewind(color_table);
	while (fgets(tmpbuf, sizeof (tmpbuf), color_table) != NULL) {
		tmpstr = strtok(tmpbuf, " ");
		if (strcmp(tmpstr, index) == 0) {
			tmpstr = strtok(NULL, "\n");
			return (tmpstr);
		}
	}
	return ("");
}

/*
 * bk - use maildir++ quotas now
 * char *get_quota_used(char *dir) {
 * static char tmpbuff[40];
 * double size;
 * 
 * size = get_du(dir);
 * if (size > 0) {
 * size = size / 1048576; 
 * }
 * sprintf(tmpbuff, "%.2lf", size);
 * return tmpbuff;
 * }
 */
/*
 * quota_to_bytes: used to convert user entered quota (given in MB)
 * back to bytes for vpasswd file
 * return value: 0 for success, 1 for failure
 */
int
quota_to_bytes(char returnval[], char *quota)
{
	mdir_t          tmp;

	if (quota == NULL) {
		return 1;
	}
	if ((tmp = strtoll(quota, NULL, 10))) {
		tmp *= 1048576;
		sprintf(returnval, "%.0lf", (double) tmp);
		return 0;
	} else {
		strcpy(returnval, "");
		return 1;
	}
}

/*
 * quota_to_megabytes: used to convert vpasswd representation of quota
 * to number of megabytes.
 * return value: 0 for success, 1 for failure
 */
int
quota_to_megabytes(char *returnval, char *quota)
{
	mdir_t          tmp;
	int             i;

	if (quota == NULL) {
		return 1;
	}
	i = strlen(quota);
	if ((quota[i - 1] == 'M') || (quota[i - 1] == 'm')) {
		tmp = strtol(quota, NULL, 10);		/* already in megabytes */
	} else if ((quota[i - 1] == 'K') || (quota[i - 1] == 'k')) {
		tmp = strtol(quota, NULL, 10) * 1024;	/* convert kilobytes to megabytes */
	} else if ((tmp = strtol(quota, NULL, 10))) {
		tmp /= 1048576.0;
	} else {
		strcpy(returnval, "");
		return 1;
	}
	sprintf(returnval, "%.2lf", (double) tmp);
	return 0;
}

void
print_user_index(char *action, int colspan, char *user, char *dom, time_t mytime)
{
#ifdef USER_INDEX
	int             k;

	printf("<tr bgcolor=%s>", get_color_text("000"));
	printf("<td colspan=%d align=\"center\">", colspan);
	printf("<hr>");
	printf("<b>%s</b> &nbsp; ", html_text[133]);
	for (k = 0; k < 10; k++) {
		printh("<a href=\"%s&searchuser=%d\">%d</a>\n", cgiurl(action), k, k);
	}
	for (k = 'a'; k <= 'z'; k++) {
		printh("<a href=\"%s&searchuser=%c\">%c</a>\n", cgiurl(action), k, k);
	}
	printf("</td>");
	printf("</tr>\n");

	printf("<tr bgcolor=%s>", get_color_text("000"));
	printf("<td colspan=%d>", colspan);
	printf("<table border=0 cellpadding=3 cellspacing=0 width=\"100%%\"><tr><td align=\"center\"><br>");
	printf("<form method=\"get\" action=\"%s/com/%s\">", CGIPATH, action);
	printh("<input type=\"hidden\" name=\"user\" value=\"%H\">", user);
	printh("<input type=\"hidden\" name=\"dom\" value=\"%H\">", dom);
	printf("<input type=\"hidden\" name=\"time\" value=\"%u\">", (unsigned int) mytime);
	printh("<input type=\"text\" name=\"searchuser\" value=\"%H\">&nbsp;", SearchUser);
	printf("<input type=\"submit\" value=\"%s\">", html_text[204]);
	printf("</form>");
	printf("</td></tr></table>");
	printf("<hr>");
	printf("</td></tr>\n");

#endif
}

char           *
cgiurl(char *action)
{
	static char     url[256];

	snprinth(url, sizeof (url), "%s/com/%s?user=%C&dom=%C&time=%d", CGIPATH, action,
		Username, Domain, Mytime);
	return url;
}

void
getversion_qautil_c()
{
	printf("%s\n", sccsidh);
}
