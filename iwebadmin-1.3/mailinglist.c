/*
 * $Id: mailinglist.c,v 1.6 2017-04-03 13:03:31+05:30 Cprogrammer Exp mbhangui $
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
 * 
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
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <pwd.h>
#include <dirent.h>
#include <errno.h>
#include "cgi.h"
#include "limits.h"
#include "mailinglist.h"
#include "printh.h"
#include "iwebadmin.h"
#include "iwebadminx.h"
#include "show.h"
#include "template.h"
#include "util.h"

char            dotqmail_name[MAX_FILE_NAME];
char            replyto_addr[256];
int             replyto;
int             dotnum;
int             checkopt[256];	/* used to display mailing list options */

#define REPLYTO_SENDER 1
#define REPLYTO_LIST 2
#define REPLYTO_ADDRESS 3
#define GROUP_SUBSCRIBER 0
#define GROUP_MODERATOR 1
#define GROUP_DIGEST 2

void            set_options();
void            default_options();

void
show_mailing_lists(char *user, char *dom, time_t mytime)
{
	if (AdminType != DOMAIN_ADMIN) {
		snprintf(StatusMessage, sizeof (StatusMessage), "%s", html_text[142]);
		vclose();
		exit(0);
	}

	/*- see if there's anything to display */
	count_mailinglists();
	if (CurMailingLists == 0) {
		snprintf(StatusMessage, sizeof (StatusMessage), "%s", html_text[231]);
		show_menu(Username, Domain, Mytime);
		vclose();
		exit(0);
	}

	if (MaxMailingLists == 0) {
		return;
	}
	send_template("show_mailinglist.html");
}

void
show_mailing_list_line(char *user, char *dom, time_t mytime, char *dir)
{
	DIR            *mydir;
	struct dirent  *mydirent;
	FILE           *fs;
	char           *addr;
	char            testfn[MAX_FILE_NAME];
	int             i;

	if (AdminType != DOMAIN_ADMIN) {
		snprintf(StatusMessage, sizeof (StatusMessage), "%s", html_text[142]);
		vclose();
		exit(0);
	}

	if (MaxMailingLists == 0) {
		return;
	}

	if ((mydir = opendir(".")) == NULL) {
		printf("<tr><td>%s %d</tr><td>", html_text[143], 1);
		return;
	}

	/*- First display the title row */
	printf("<tr bgcolor=\"#cccccc\">");
	printf("<th align=center><font size=2>%s</font></th>", html_text[72]);
#ifdef EZMLMIDX
	printf("<th align=center><font size=2>%s</font></th>", html_text[71]);
#endif
	printf("<th align=center><font size=2>%s</font></th>", html_text[81]);
	printf("<th align=center><font size=2>%s</font></th>", html_text[83]);
	printf("<th align=center><font size=2>%s</font></th>", html_text[84]);
	printf("<th align=center><font size=2>%s</font></th>", html_text[85]);
#ifdef EZMLMIDX
	printf("<th align=center><font size=2>%s</font></th>", html_text[86]);
	printf("<th align=center><font size=2>%s</font></th>", html_text[87]);
	printf("<th align=center><font size=2>%s</font></th>", html_text[88]);
	printf("<th align=center><font size=2>%s</font></th>", html_text[237]);
	printf("<th align=center><font size=2>%s</font></th>", html_text[238]);
	printf("<th align=center><font size=2>%s</font></th>", html_text[239]);
#endif
	printf("</tr>\n");

	sort_init();

	/*- Now, display each list */
	while ((mydirent = readdir(mydir)) != NULL) {
		if (strncmp(".qmail-", mydirent->d_name, 7) == 0) {
			if ((fs = fopen(mydirent->d_name, "r")) == NULL) {
#ifdef EZMLMIDX
				printf("<tr><td colspan=12>%s %s</td></tr>\n", html_text[144], mydirent->d_name);
#else
				printf("<tr><td colspan=5>%s %s</td></tr>\n", html_text[144], mydirent->d_name);
#endif
				continue;
			}
			(void) fgets(TmpBuf2, sizeof (TmpBuf2), fs);
			(void) fclose(fs);
			if (strstr(TmpBuf2, "ezmlm-reject") != 0) {
				sort_add_entry(&mydirent->d_name[7], 0);
			}
		}
	}
	closedir(mydir);
	sort_dosort();

	for (i = 0; (addr = (char *) sort_get_entry(i)); ++i) {
		sprintf(testfn, ".qmail-%s-digest-owner", addr);
		/*- convert ':' in addr to '.' */
		str_replace(addr, ':', '.');

		printf("<tr>");
		qmail_button(addr, "delmailinglist", user, dom, mytime, "trash.png");

#ifdef EZMLMIDX
		qmail_button(addr, "modmailinglist", user, dom, mytime, "modify.png");
#endif
		printh("<td align=left>%H</td>\n", addr);

		qmail_button(addr, "addlistuser", user, dom, mytime, "delete.png");
		qmail_button(addr, "dellistuser", user, dom, mytime, "delete.png");
		qmail_button(addr, "showlistusers", user, dom, mytime, "delete.png");

#ifdef EZMLMIDX
		qmail_button(addr, "addlistmod", user, dom, mytime, "delete.png");
		qmail_button(addr, "dellistmod", user, dom, mytime, "delete.png");
		qmail_button(addr, "showlistmod", user, dom, mytime, "delete.png");

		/*- Is it a digest list?  */
		if ((fs = fopen(testfn, "r")) == NULL) {
			/*- not a digest list */
			printf("<TD COLSPAN=3> </TD>");
		} else {
			qmail_button(addr, "addlistdig", user, dom, mytime, "delete.png");
			qmail_button(addr, "dellistdig", user, dom, mytime, "delete.png");
			qmail_button(addr, "showlistdig", user, dom, mytime, "delete.png");
			fclose(fs);
		}
#endif
		printf("</tr>\n");
	}
	sort_cleanup();
}

int
ismailing_list(FILE * fs)
{
	while (!feof(fs)) {
		(void) fgets(TmpBuf2, sizeof (TmpBuf2), fs);
		if (strstr(TmpBuf2, "ezmlm-reject") != 0 || strstr(TmpBuf2, "ezmlm-send") != 0)
			return -1;
	}
	return 0;
}

/*
 * mailing list lines on the add user page 
 */
void
show_mailing_list_line2(char *user, char *dom, time_t mytime, char *dir)
{
	DIR            *mydir;
	struct dirent  *mydirent;
	FILE           *fs;
	char           *addr;
	int             i;
	int             listcount;

	if (AdminType != DOMAIN_ADMIN) {
		snprintf(StatusMessage, sizeof (StatusMessage), "%s", html_text[142]);
		vclose();
		exit(0);
	}

	if (*EZMLMDIR == 'n' || MaxMailingLists == 0) {
		return;
	}

	if ((mydir = opendir(".")) == NULL) {
		printf("%s %d<BR>\n", html_text[143], 1);
		return;
	}

	listcount = 0;
	sort_init();

	while ((mydirent = readdir(mydir)) != NULL) {
		if (strncmp(".qmail-", mydirent->d_name, 7) == 0) {
			if ((fs = fopen(mydirent->d_name, "r")) == NULL) {
				printf("%s %s<br>\n", html_text[144], mydirent->d_name);
				continue;
			}
			(void) fgets(TmpBuf2, sizeof (TmpBuf2), fs);
			fclose(fs);
			if (strstr(TmpBuf2, "ezmlm-reject") != 0) {
				sort_add_entry(&mydirent->d_name[7], 0);
				listcount++;
			}
		}
	}
	closedir(mydir);

	/*- if there aren't any lists, don't display anything */
	if (listcount == 0) {
		sort_cleanup();
		return;
	}

	printf("<hr><table width=100%% cellpadding=1 cellspacing=0 border=0");
	printf(" align=center bgcolor=\"#000000\"><tr><td>");
	printf("<table width=100%% cellpadding=0 cellspacing=0 border=0 bgcolor=\"#e6e6e6\">");
	printf("<tr><th bgcolor=\"#000000\" colspan=2>");
	printf("<font color=\"#ffffff\">%s</font></th>\n", html_text[95]);

	sort_dosort();

	printf("<INPUT NAME=number_of_mailinglist TYPE=hidden VALUE=%d>\n", listcount);
	for (i = 0; i < listcount; ++i) {
		addr = (char *) sort_get_entry(i);
		str_replace(addr, ':', '.');
		printh("<TR><TD ALIGN=RIGHT><INPUT NAME=\"subscribe%d\" TYPE=checkbox VALUE=\"%H\"></TD>", i, addr);
		printh("<TD align=LEFT>%H@%H</TD></TR>", addr, Domain);
	}
	printf("</table></td></tr></table>\n");
	sort_cleanup();
}

void
addmailinglist()
{
	if (AdminType != DOMAIN_ADMIN) {
		snprintf(StatusMessage, sizeof (StatusMessage), "%s", html_text[142]);
		vclose();
		exit(0);
	}

	count_mailinglists();
	load_limits();
	if (MaxMailingLists != -1 && CurMailingLists >= MaxMailingLists) {
		printf("%s %d\n", html_text[184], MaxMailingLists);
		show_menu(Username, Domain, Mytime);
		vclose();
		exit(0);
	}

	/*- set up default options for new list */
	default_options();

#ifdef EZMLMIDX
	send_template("add_mailinglist-idx.html");
#else
	send_template("add_mailinglist-no-idx.html");
#endif

}

void
delmailinglist()
{
	if (AdminType != DOMAIN_ADMIN) {
		snprintf(StatusMessage, sizeof (StatusMessage), "%s", html_text[142]);
		vclose();
		exit(0);
	}

	send_template("del_mailinglist_confirm.html");
}

void
delmailinglistnow()
{
	DIR            *mydir;
	struct dirent  *mydirent;

	if (AdminType != DOMAIN_ADMIN) {
		snprintf(StatusMessage, sizeof (StatusMessage), "%s", html_text[142]);
		vclose();
		exit(0);
	}

	if (fixup_local_name(ActionUser)) {
		/*- invalid address given, abort */
		vclose();
		exit(0);
	}

	if ((mydir = opendir(".")) == NULL) {
		printf("%s %d<BR>\n", html_text[143], 1);
		printf("</table>");
		return;
	}
#ifdef ONCHANGE_SCRIPT
	snprintf(onchange_buf, MAX_BUFF, "%s@%s", ActionUser, Domain);
	call_onchange("delmailinglist");
#endif

	/*- make dotqmail name */
	strcpy(dotqmail_name, ActionUser);
	for (dotnum = 0; dotqmail_name[dotnum] != '\0'; dotnum++) {
		if (dotqmail_name[dotnum] == '.')
			dotqmail_name[dotnum] = ':';
	}

	sprintf(TmpBuf2, ".qmail-%s", dotqmail_name);
	sprintf(TmpBuf3, ".qmail-%s-", dotqmail_name);
	while ((mydirent = readdir(mydir)) != NULL) {

		/*- delete the main .qmail-"list" file */
		if (strcmp(TmpBuf2, mydirent->d_name) == 0) {
			if (unlink(mydirent->d_name) != 0) {
				ack("185", TmpBuf2);
			}

		} else /*- delete secondary .qmail-"list"-* files */
		if (strncmp(TmpBuf3, mydirent->d_name, strlen(TmpBuf3)) == 0) {
			if (unlink(mydirent->d_name) != 0)
				ack("185", TmpBuf2);
		}
	}
	closedir(mydir);


	sprintf(TmpBuf2, "%s/%s", RealDir, ActionUser);
	vdelfiles(TmpBuf2, ActionUser, Domain);

	count_mailinglists();
	snprinth(StatusMessage, sizeof (StatusMessage), "%s %H\n", html_text[186], ActionUser);
	if (CurMailingLists == 0) {
		show_menu(Username, Domain, Mytime);
	} else {
		show_mailing_lists(Username, Domain, Mytime);
	}

}

/*-
 * sets the Reply-To header in header* files based on form fields
 * designed to be called by ezmlm_make() (after calling ezmlm-make)
 * Replaces the "Reply-To" line in <filename> with <newtext>.
 */
void
ezmlm_setreplyto(char *filename, char *newtext)
{
	FILE           *headerfile, *temp;
	char            realfn[256];
	char            tempfn[256];
	char            buf[256];

	sprintf(realfn, "%s/%s/%s", RealDir, ActionUser, filename);
	sprintf(tempfn, "%s.tmp", realfn);

	headerfile = fopen(realfn, "r");
	if (!headerfile)
		return;
	temp = fopen(tempfn, "w");
	if (!temp) {
		fclose(headerfile);
		return;
	}

	/*- copy contents to new file, except for Reply-To header */
	while (fgets(buf, sizeof (buf), headerfile) != NULL) {
		if (strncasecmp("Reply-To", buf, 8) != 0) {
			fputs(buf, temp);
		}
	}

	fputs(newtext, temp);

	fclose(headerfile);
	fclose(temp);
	unlink(realfn);
	rename(tempfn, realfn);
}

void
ezmlm_make(int newlist)
{
	FILE           *file;
	int             pid;

#ifdef EZMLMIDX
	char            list_owner[MAX_BUFF];
	char            owneremail[MAX_BUFF + 5];
	int             i;
#endif
	char            options[MAX_BUFF];
	char           *arguments[MAX_BUFF];
	int             argc;
	char            tmp[MAX_BUFF];
	char           *tmpstr;
	char            loop_ch[64];
	int             loop;

/*-
 * Initialize listopt to be a string of the characters A-Z, with each one
 * set to the correct case (e.g., A or a) to match the expected behavior
 * of not checking any checkboxes.  Leave other letters blank.
 * NOTE: Leave F out, since we handle it manually.
 */
	char            listopt[] = "A  D   hIj L N pQRST      ";

	if (AdminType != DOMAIN_ADMIN) {
		snprintf(StatusMessage, sizeof (StatusMessage), "%s", html_text[142]);
		vclose();
		exit(0);
	}

	if (fixup_local_name(ActionUser)) {
		snprinth(StatusMessage, sizeof (StatusMessage), "%s %H\n", html_text[188], ActionUser);
		addmailinglist();
		vclose();
		exit(0);
	}

	/*- update listopt based on user selections */
	for (loop = 0; loop < 20; loop++) {
		sprintf(tmp, "opt%d=", loop);
		GetValue(TmpCGI, loop_ch, tmp, sizeof (loop_ch));
		for (tmpstr = loop_ch; *tmpstr; tmpstr++) {
			if ((*tmpstr >= 'A') && (*tmpstr <= 'Z')) {
				listopt[*tmpstr - 'A'] = *tmpstr;
			} else if ((*tmpstr >= 'a') && (*tmpstr <= 'z')) {
				listopt[*tmpstr - 'a'] = *tmpstr;
			}
		}
	}

	/*- don't allow option c, force option e if modifying existing list */
	listopt[2] = ' ';
	listopt[4] = newlist ? ' ' : 'e';

	argc = 0;
	arguments[argc++] = "ezmlm-make";

#ifdef EZMLMIDX
	/*- check the list owner entry */
	GetValue(TmpCGI, list_owner, "listowner=", sizeof (list_owner));		/*- Get the listowner -*/
	if (strlen(list_owner) > 0) {
		sprintf(owneremail, "-5%s", list_owner);
		arguments[argc++] = owneremail;
	}
#endif

	/*- build the option string */
	tmpstr = options;
	arguments[argc++] = tmpstr;
	*tmpstr++ = '-';
#ifndef EZMLMIDX
	/*- non idx list, only allows options A and P */
	*tmpstr++ = listopt[0];		/* a or A */
	*tmpstr++ = listopt['p' - 'a'];	/* p or P */
	*tmpstr++ = 0;				/* add NULL terminator */
#else
	/*- ignore options v-z, but test a-u */
	for (i = 0; i <= ('u' - 'a'); i++) {
		if (listopt[i] != ' ') {
			*tmpstr++ = listopt[i];
		}
	}
	*tmpstr++ = 0;				/* add NULL terminator */

	/*- check for sql support */
	GetValue(TmpCGI, tmp, "sqlsupport=", sizeof (tmp));
	if (strlen(tmp) > 0) {
		arguments[argc++] = tmpstr;
		tmpstr += sprintf(tmpstr, "%s", tmp) + 1;
		arguments[argc++] = tmpstr;
		for (loop = 1; loop <= NUM_SQL_OPTIONS; loop++) {
			sprintf(tmp, "sql%d=", loop);
			GetValue(TmpCGI, loop_ch, tmp, sizeof (loop_ch));
			tmpstr += sprintf(tmpstr, "%s:", loop_ch);
		}
		/*- remove trailing : */
		tmpstr--;
		*tmpstr++ = 0;
	}
#endif

	/*- make dotqmail name */
	strcpy(dotqmail_name, ActionUser);
	for (dotnum = 0; dotqmail_name[dotnum] != '\0'; dotnum++) {
		if (dotqmail_name[dotnum] == '.')
			dotqmail_name[dotnum] = ':';
	}
	pid = fork();
	if (pid == 0) {
		sprintf(TmpBuf1, "%s/ezmlm-make", EZMLMDIR);
		sprintf(TmpBuf2, "%s/%s", RealDir, ActionUser);
		sprintf(TmpBuf3, "%s/.qmail-%s", RealDir, dotqmail_name);

		arguments[argc++] = TmpBuf2;
		arguments[argc++] = TmpBuf3;
		arguments[argc++] = ActionUser;
		arguments[argc++] = Domain;
		arguments[argc] = (char *) NULL;

		execv(TmpBuf1, arguments);
		exit(127);
	} else {
		wait(&pid);
	}

	/*- Check for prefix setting */
	GetValue(TmpCGI, tmp, "prefix=", sizeof (tmp));

	/*- strip leading '[' and trailing ']' from tmp */
	tmpstr = strchr(tmp, ']');
	if (tmpstr != NULL)
		*tmpstr = '\0';
	tmpstr = tmp;
	while (*tmpstr == '[')
		tmpstr++;

	/*- Create (or delete) the file as appropriate */
	sprintf(TmpBuf, "%s/%s/prefix", RealDir, ActionUser);
	if (strlen(tmp) > 0) {
		file = fopen(TmpBuf, "w");
		if (file) {
			fprintf(file, "[%s]", tmpstr);
			fclose(file);
		}
	} else {
		unlink(TmpBuf);
	}

	/*- set Reply-To header */
	GetValue(TmpCGI, TmpBuf, "replyto=", sizeof (TmpBuf));
	replyto = atoi(TmpBuf);
	if (replyto == REPLYTO_SENDER) {
		/*- ezmlm shouldn't remove/add Reply-To header */
		ezmlm_setreplyto("headeradd", "");
		ezmlm_setreplyto("headerremove", "");
	} else {
		if (replyto == REPLYTO_ADDRESS) {
			GetValue(TmpCGI, replyto_addr, "replyaddr=", sizeof (replyto_addr));
			sprintf(TmpBuf, "Reply-To: %s\n", replyto_addr);
		} else {				/* REPLYTO_LIST */
			strcpy(TmpBuf, "Reply-To: <#l#>@<#h#>\n");
		}
		ezmlm_setreplyto("headeradd", TmpBuf);
		ezmlm_setreplyto("headerremove", "Reply-To");
	}

	/*- update inlocal file */
	sprintf(TmpBuf, "%s/%s/inlocal", RealDir, ActionUser);
	if ((file = fopen(TmpBuf, "w")) != NULL) {
		fprintf(file, "%s-%s", Domain, ActionUser);
		fclose(file);
	}
#ifdef EZMLMIDX
	/*- if this is a new list, add owner as subscriber */
	if (newlist && (*list_owner != '\0')) {
		ezmlm_sub(GROUP_SUBSCRIBER, list_owner);
		if (listopt['M' - 'A'] == 'm') {	/* moderation on */
			/*- add owner as moderator/remote admin as well */
			ezmlm_sub(GROUP_MODERATOR, list_owner);
		}
	}
#endif
}

void
addmailinglistnow()
{
	count_mailinglists();
	load_limits();
	if (MaxMailingLists != -1 && CurMailingLists >= MaxMailingLists) {
		printf("%s %d\n", html_text[184], MaxMailingLists);
		show_menu(Username, Domain, Mytime);
		vclose();
		exit(0);
	}

	if (check_local_user(ActionUser)) {
		snprinth(StatusMessage, sizeof (StatusMessage), "%s %H\n", html_text[175], ActionUser);
		addmailinglist();
		vclose();
		exit(0);
	}

	ezmlm_make(1);

#ifdef ONCHANGE_SCRIPT
	snprintf(onchange_buf, MAX_BUFF, "%s@%s", ActionUser, Domain);
	call_onchange("addmailinglist");
#endif

	snprinth(StatusMessage, sizeof (StatusMessage), "%s %H@%H\n", html_text[187], ActionUser, Domain);
	show_mailing_lists(Username, Domain, Mytime);
}

/*-
 * mod = 0 for subscribers, 1 for moderators, 2 for digest users 
 */
void
show_list_group_now(int mod)
{
	FILE           *fs;
	int             handles[2], pid, z = 0, subuser_count = 0;
	char            buf[256];
	char           *addr;

	if (AdminType != DOMAIN_ADMIN) {
		snprintf(StatusMessage, sizeof (StatusMessage), "%s", html_text[142]);
		vclose();
		exit(0);
	}

	lowerit(ActionUser);
	if (pipe(handles)) {
		vclose();
		exit(0);
	}

	pid = fork();
	if (pid == 0) {
		(void) close(handles[0]);
		(void) dup2(handles[1], fileno(stdout));
		sprintf(TmpBuf1, "%s/ezmlm-list", EZMLMDIR);
		sprintf(TmpBuf2, "%s/%s", RealDir, ActionUser);
		if (mod == GROUP_MODERATOR) {
			execl(TmpBuf1, "ezmlm-list", TmpBuf2, "mod", (char *) NULL);
		} else if (mod == GROUP_DIGEST) {
			execl(TmpBuf1, "ezmlm-list", TmpBuf2, "digest", (char *) NULL);
		} else {
			execl(TmpBuf1, "ezmlm-list", TmpBuf2, (char *) NULL);
		}
		exit(127);
	} else {
		close(handles[1]);
		fs = fdopen(handles[0], "r");

		/*- Load subscriber/moderator list */

		sort_init();
		while ((fgets(buf, sizeof (buf), fs) != NULL)) {
			sort_add_entry(buf, '\n');	/* don't copy newline */
			subuser_count++;
		}

		sort_dosort();

		/*- Display subscriber/moderator/digest list, along with delete button */
		if (mod == 1) {
			strcpy(TmpBuf, "228");
			strcpy(TmpBuf1, "220");
			/*- strcpy(TmpBuf2, "087"); */
		} else if (mod == 2) {
			strcpy(TmpBuf, "244");
			strcpy(TmpBuf1, "246");
			/*- strcpy(TmpBuf2, "245"); */
		} else {
			strcpy(TmpBuf, "230");
			strcpy(TmpBuf1, "222");
			/*- strcpy(TmpBuf2, "084"); */
		}
		strcpy(TmpBuf2, "072");
		printf("<TABLE border=0 width=\"100%%\">\n");
		printf(" <TR>\n");
		printf("  <TH align=left COLSPAN=4><B>%s</B> %d<BR><BR></TH>\n", get_html_text(TmpBuf), subuser_count);
		printf(" </TR>\n");
		printf(" <TR align=center bgcolor=%s>\n", get_color_text("002"));
		printf("  <TH align=center><b><font size=2>%s</font></b></TH>\n", get_html_text(TmpBuf2));
		printf("  <TH align=center><b><font size=2>%s</font></b></TH>\n", get_html_text(TmpBuf1));
		printf("  <TH align=center><b><font size=2>%s</font></b></TH>\n", get_html_text(TmpBuf2));
		printf("  <TH align=center><b><font size=2>%s</font></b></TH>\n", get_html_text(TmpBuf1));
		printf(" </TR>\n");

		if (mod == 1) {
			strcpy(TmpBuf, "dellistmodnow");
		} else if (mod == 2) {
			strcpy(TmpBuf, "dellistdignow");
		} else {
			strcpy(TmpBuf, "dellistusernow");
		}
		for (z = 0; (addr = (char *) sort_get_entry(z)); ++z) {
			printf(" <TR align=center>");
			printh("  <TD align=right><A href=\"%s&modu=%C&newu=%C\"><IMG src=\"%s/trash.png\" border=0></A></TD>\n",
				   cgiurl(TmpBuf), ActionUser, addr, IMAGEURL);
			printh("  <TD align=left>%H</TD>\n", addr);
			++z;
			if ((addr = (char *) sort_get_entry(z))) {
				printh("  <TD align=right><A href=\"%s&modu=%C&newu=%C\"><IMG src=\"%s/trash.png\" border=0></A></TD>\n",
					   cgiurl(TmpBuf), ActionUser, addr, IMAGEURL);
				printh("  <TD align=left>%H</TD>\n", addr);
			} else {
				printf("  <TD COLSPAN=2> </TD>");
			}
			printf(" </TR>");
		}

		sort_cleanup();

		printf("</TABLE>");
		fclose(fs);
		close(handles[0]);
		wait(&pid);
		snprintf(StatusMessage, sizeof (StatusMessage), "%s\n", html_text[190]);
	}
}

void
show_list_group(char *template)
{
	if (AdminType != DOMAIN_ADMIN) {
		snprintf(StatusMessage, sizeof (StatusMessage), "%s", html_text[142]);
		vclose();
		exit(0);
	}

	if (MaxMailingLists == 0) {
		return;
	}

	send_template(template);
}

void
addlistgroup(char *template)
{
	if (AdminType != DOMAIN_ADMIN) {
		snprintf(StatusMessage, sizeof (StatusMessage), "%s", html_text[142]);
		vclose();
		exit(0);
	}
	send_template(template);
}

void
addlistuser()
{
	addlistgroup("add_listuser.html");
}

void
addlistmod()
{
	addlistgroup("add_listmod.html");
}

void
addlistdig()
{
	addlistgroup("add_listdig.html");
}

/*-
 * returns 0 for success 
 */
int
ezmlm_sub(int mod, char *email)
{
	int             pid;
	char            subpath[MAX_BUFF];
	char            listpath[MAX_BUFF];

	pid = fork();
	if (pid == 0) {
		snprintf(subpath, sizeof (subpath), "%s/ezmlm-sub", EZMLMDIR);
		snprintf(listpath, sizeof (listpath), "%s/%s", RealDir, ActionUser);
		if (mod == GROUP_MODERATOR)
			execl(subpath, "ezmlm-sub", listpath, "mod", email, (char *) NULL);
		else if (mod == GROUP_DIGEST)
			execl(subpath, "ezmlm-sub", listpath, "digest", email, (char *) NULL);
		else
			execl(subpath, "ezmlm-sub", listpath, email, (char *) NULL);
		exit(127);
	} else
		wait(&pid);
	/*- need to check exit code for failure somehow */
	return 0;
}

/*- mod = 0 for subscribers, 1 for moderators, 2 for digest subscribers */
void
addlistgroupnow(int mod)
{

	if (AdminType != DOMAIN_ADMIN) {
		snprintf(StatusMessage, sizeof (StatusMessage), "%s", html_text[142]);
		vclose();
		exit(0);
	}

	lowerit(ActionUser);

	if (check_email_addr(Newu)) {
		snprinth(StatusMessage, sizeof (StatusMessage), "%s %H\n", html_text[148], Newu);
		if (mod == GROUP_MODERATOR)
			addlistmod();
		else if (mod == GROUP_DIGEST)
			addlistdig();
		else
			addlistuser();
		vclose();
		exit(0);
	}

	ezmlm_sub(mod, Newu);
	if (mod == GROUP_MODERATOR) {
		snprinth(StatusMessage, sizeof (StatusMessage), "%H %s %H@%H\n", Newu, html_text[194], ActionUser, Domain);
		send_template("add_listmod.html");
	} else if (mod == GROUP_DIGEST) {
		snprinth(StatusMessage, sizeof (StatusMessage), "%H %s %H@%H\n", Newu, html_text[240], ActionUser, Domain);
		send_template("add_listdig.html");
	} else {
		snprinth(StatusMessage, sizeof (StatusMessage), "%H %s %H@%H\n", Newu, html_text[193], ActionUser, Domain);
		send_template("add_listuser.html");
	}
	vclose();
	exit(0);
}

void
dellistgroup(char *template)
{
	if (AdminType != DOMAIN_ADMIN) {
		snprintf(StatusMessage, sizeof (StatusMessage), "%s", html_text[142]);
		vclose();
		exit(0);
	}

	send_template(template);
}

void
dellistgroupnow(int mod)
{
	int             pid;
	char           *p;

	if (AdminType != DOMAIN_ADMIN) {
		snprintf(StatusMessage, sizeof (StatusMessage), "%s", html_text[142]);
		vclose();
		exit(0);
	}

	lowerit(Newu);

	/*-
	 * for dealing with AOL spam complaints, if address doesn't contain @,
	 * but does contain '=', change the '=' to '@'.
	 */
	if (!strchr(Newu, '@')) {
		if ((p = strchr(Newu, '=')))
			*p = '@';
	}

	pid = fork();
	if (pid == 0) {
		sprintf(TmpBuf1, "%s/ezmlm-unsub", EZMLMDIR);
		sprintf(TmpBuf2, "%s/%s", RealDir, ActionUser);
		if (mod == GROUP_MODERATOR)
			execl(TmpBuf1, "ezmlm-unsub", TmpBuf2, "mod", Newu, (char *) NULL);
		else if (mod == GROUP_DIGEST)
			execl(TmpBuf1, "ezmlm-unsub", TmpBuf2, "digest", Newu, (char *) NULL);
		else
			execl(TmpBuf1, "ezmlm-unsub", TmpBuf2, Newu, (char *) NULL);
		execl(TmpBuf1, "ezmlm-unsub", TmpBuf2, Newu, NULL);
		exit(127);
	} else
		wait(&pid);

	if (mod == GROUP_MODERATOR) {
		snprinth(StatusMessage, sizeof (StatusMessage), "%H %s %H@%H\n", Newu, html_text[197], ActionUser, Domain);
	} else if (mod == GROUP_DIGEST) {
		snprinth(StatusMessage, sizeof (StatusMessage), "%H %s %H@%H\n", Newu, html_text[242], ActionUser, Domain);
	} else {
		snprinth(StatusMessage, sizeof (StatusMessage), "%H %s %H@%H\n", Newu, html_text[203], ActionUser, Domain);
	}
	show_mailing_lists(Username, Domain, Mytime);
	vclose();
	exit(0);
}

void
count_mailinglists()
{
	DIR            *mydir;
	struct dirent  *mydirent;
	FILE           *fs;

	if ((mydir = opendir(".")) == NULL) {
		printf("%s %d<BR>\n", html_text[143], 1);
		printf("</table>");
		return;
	}


	CurMailingLists = 0;
	while ((mydirent = readdir(mydir)) != NULL) {
		if (strncmp(".qmail-", mydirent->d_name, 7) == 0) {
			if ((fs = fopen(mydirent->d_name, "r")) == NULL) {
				printf(html_text[144], mydirent->d_name);
				continue;
			}
			if (!fgets(TmpBuf2, sizeof (TmpBuf2), fs)) {
				printf(html_text[144], mydirent->d_name);
				continue;
			}
			if (strstr(TmpBuf2, "ezmlm-reject") != 0) {
				++CurMailingLists;
			}
			fclose(fs);
		}
	}
	closedir(mydir);

}

/*-
 * name of list to modify is stored in ActionUser 
 */
void
modmailinglist()
{
	int             i;
	FILE           *fs;

	if (AdminType != DOMAIN_ADMIN) {
		snprintf(StatusMessage, sizeof (StatusMessage), "%s", html_text[142]);
		vclose();
		exit(0);
	}

	strcpy(Alias, "");			/* initialize Alias (list owner) to empty string */

	/*- get the current listowner and copy it to Alias */
	strcpy(dotqmail_name, ActionUser);
	str_replace(dotqmail_name, '.', ':');
	sprintf(TmpBuf, ".qmail-%s-owner", dotqmail_name);
	if ((fs = fopen(TmpBuf, "r")) != NULL) {
		while (fgets(TmpBuf2, sizeof (TmpBuf2), fs)) {
			if (strstr(TmpBuf2, "@") != NULL) {
			/*
			 * strip leading & if present 
			 */
				sprintf(Alias, "%s", (*TmpBuf2 == '&' ? (TmpBuf2 + 1) : TmpBuf2));
				i = strlen(Alias);
				--i;
				Alias[i] = '\0';	/* strip newline */
			}
		}
		fclose(fs);
	}

	/*- set default to "replies go to original sender" */
	replyto = REPLYTO_SENDER;	/* default */
	*replyto_addr = '\0';
	sprintf(TmpBuf, "%s/headeradd", ActionUser);
	/*- get the Reply-To setting for the list */
	if ((fs = fopen(TmpBuf, "r")) != NULL) {
		while (fgets(TmpBuf2, sizeof (TmpBuf2), fs)) {
			if (strncasecmp("Reply-To: ", TmpBuf2, 10) == 0) {
				i = strlen(TmpBuf2);
				--i;
				TmpBuf2[i] = '\0';	/* strip newline */
				if (strcmp("<#l#>@<#h#>", TmpBuf2 + 10) == 0) {
					replyto = REPLYTO_LIST;
				} else {
					replyto = REPLYTO_ADDRESS;
					strcpy(replyto_addr, TmpBuf2 + 10);
				}
			}
		}
		fclose(fs);
	}

	/*- read in options for the current list */
	set_options();

#ifdef EZMLMIDX
	send_template("mod_mailinglist-idx.html");
#else
	send_template("show_mailinglists.html");
#endif

}

void
modmailinglistnow()
{
	ezmlm_make(0);

	snprinth(StatusMessage, sizeof (StatusMessage), "%s %H@%H\n", html_text[226], ActionUser, Domain);
	show_mailing_lists(Username, Domain, Mytime);
}

void
build_list_value(char *param, char *color, char *opt1, int desc1, char *opt2, int desc2, int checked)
{
	printf("<tr bgcolor=%s>\n", get_color_text(color));
	printf("  <td>\n");
	printf("    <input type=radio name=%s value=%s%s></td>\n", param, opt1, checked ? "" : " CHECKED");
	printf("  <td>%s</td>\n", html_text[desc1]);
	printf("  <td>\n");
	printf("    <input type=radio name=%s value=%s%s></td>\n", param, opt2, checked ? " CHECKED" : "");
	printf("  <td>%s</td>\n", html_text[desc2]);
	printf("</tr>\n");
}

void
build_option_str(char *type, char *param, char *options, char *description)
{
	int             selected;
	char           *optptr;

	selected = 1;
	for (optptr = options; *optptr; optptr++) {
		selected = selected && checkopt[(int) *optptr];
	}
	/*- selected is now true if all options for this radio button are true */
	printh("<INPUT TYPE=%s NAME=\"%H\" VALUE=\"%H\"%s> %s\n", type, param, options, selected ? " CHECKED" : "", description);
}

int
file_exists(char *filename)
{
	FILE           *fs;
	if ((fs = fopen(filename, "r")) != NULL) {
		fclose(fs);
		return 1;
	} else {
		return 0;
	}
}

int
get_ezmlmidx_line_arguments(char *line, char *program, char argument)
{
	char           *begin;
	char           *end;
	char           *arg;

	/*- does line contain program name? */
	if ((strstr(line, program)) != NULL) {
	/*- find the options */
		begin = strchr(line, ' ');
		begin++;
		if (*begin == '-') {
			end = strchr(begin, ' ');
			arg = strchr(begin, argument);
			/*- if arg is found && it's in the options (before the trailing space), return 1 */
			if (arg && (arg < end))
				return 1;
		}
	}
	return 0;
}

void
set_options()
{
	char            c;
	FILE           *fs;

/*-
 * Note that with ezmlm-idx it might be possible to replace most
 * of this code by reading the config file in the list's directory.
 */

	/*- make dotqmail name (ActionUser with '.' replaced by ':') */
	strcpy(dotqmail_name, ActionUser);
	for (dotnum = 0; dotqmail_name[dotnum] != '\0'; dotnum++) {
		if (dotqmail_name[dotnum] == '.')
			dotqmail_name[dotnum] = ':';
	}

	/*- default to false for lowercase letters */
	for (c = 'a'; c <= 'z'; checkopt[(int) c++] = 0);

/*- ------ newer configuration reads -*/

	/*- -s: Subscription moderation. touching dir/modsub */
	sprintf(TmpBuf, "%s/modsub", ActionUser);
	checkopt['s'] = file_exists(TmpBuf);
	/*- -h: Help  subscription. Don't require confirmation. Not recommented! -*/
	sprintf(TmpBuf, "%s/nosubconfirm", ActionUser);
	checkopt['h'] = file_exists(TmpBuf);
	/*- -j Jump off. Unsubscribe  does not require confirmation. -*/
	sprintf(TmpBuf, "%s/nounsubconfirm", ActionUser);
	checkopt['j'] = file_exists(TmpBuf);

	/*- -m: Message  moderation. touch dir/modpost -*/
	sprintf(TmpBuf, "%s/modpost", ActionUser);		/*- valid for newer ezmlm-versions -*/
	checkopt['m'] = file_exists(TmpBuf);
	/*- -o: Reject others than; applicable to message moderated lists only -*/
	sprintf(TmpBuf, "%s/modpostonly", ActionUser);
	checkopt['o'] = file_exists(TmpBuf);
	/*- -u: User posts only. subscribers, digest-subscribers and dir/allow -*/
	sprintf(TmpBuf, "%s/subpostonly", ActionUser);
	checkopt['u'] = file_exists(TmpBuf);

	/*- -f: Subject Prefix. outgoing subject will be pre-fixed with the list name -*/
	sprintf(TmpBuf, "%s/prefix", ActionUser);
	checkopt['f'] = file_exists(TmpBuf);
	/*- -t: Message Trailer. create dir/text/trailer -*/
	sprintf(TmpBuf, "%s/addtrailer", ActionUser);
	checkopt['t'] = file_exists(TmpBuf);

	/*- -a: Archived: touch dir/archived and dir/indexed -*/
	sprintf(TmpBuf, "%s/archived", ActionUser);
	checkopt['a'] = file_exists(TmpBuf);
	/*- -i: indexed for WWW archive access -*/
	sprintf(TmpBuf, "%s/threaded", ActionUser);
	checkopt['i'] = file_exists(TmpBuf);
	/*- -p: Public archive. touch dir/public -*/
	sprintf(TmpBuf, "%s/public", ActionUser);
	checkopt['p'] = file_exists(TmpBuf);
	/*- -g: Guard archive. Access requests from unrecognized SENDERs will be rejected. -*/
	sprintf(TmpBuf, "%s/subgetonly", ActionUser);
	checkopt['g'] = file_exists(TmpBuf);
	/*- -b: Block archive. Only moderators are allowed to access the archive. -*/
	sprintf(TmpBuf, "%s/modgetonly", ActionUser);
	checkopt['b'] = file_exists(TmpBuf);

	/*- -d: Digest -*/
	sprintf(TmpBuf, "%s/digested", ActionUser);
	checkopt['d'] = file_exists(TmpBuf);

	/*- -r: Remote admin. touching dir/remote -*/
	sprintf(TmpBuf, "%s/remote", ActionUser);
	checkopt['r'] = file_exists(TmpBuf);
	/*- -l List subscribers. administrators can request a subscriber -*/
	sprintf(TmpBuf, "%s/modcanlist", ActionUser);
	checkopt['l'] = file_exists(TmpBuf);
	/*- -n New text file. administrators may edit texts -*/
	sprintf(TmpBuf, "%s/modcanedit", ActionUser);
	checkopt['n'] = file_exists(TmpBuf);

	/*- ------ end of newer configuration reads -*/

	/*- ------ read in old ezmlm's values -*/
	/*- figure out some options in the -default file; -*/
	sprintf(TmpBuf, ".qmail-%s-default", dotqmail_name);
	if ((fs = fopen(TmpBuf, "r")) != NULL) {
		while (fgets(TmpBuf2, sizeof (TmpBuf2), fs)) {
			/*- -b: Block archive. Only moderators are allowed to access the archive. -*/
			if ((get_ezmlmidx_line_arguments(TmpBuf2, "ezmlm-get", 'P')) > 0) {
				checkopt['b'] = 1;
			}
			/*- -g: Guard archive. Access requests from unrecognized SENDERs will be rejected. -*/
			if ((get_ezmlmidx_line_arguments(TmpBuf2, "ezmlm-get", 's')) > 0) {
				checkopt['g'] = 1;
			}
			/*- -h: Help  subscription. Don't require confirmation. Not recommented! -*/
			if ((get_ezmlmidx_line_arguments(TmpBuf2, "ezmlm-manage", 'S')) > 0) {
				checkopt['h'] = 1;
			}
			/*- -j Jump off. Unsubscribe  does not require confirmation. -*/
			if ((get_ezmlmidx_line_arguments(TmpBuf2, "ezmlm-manage", 'U')) > 0) {
				checkopt['j'] = 1;
			}
			/*- -l List subscribers. administrators can request a subscriber -*/
			if ((get_ezmlmidx_line_arguments(TmpBuf2, "ezmlm-manage", 'l')) > 0) {
				checkopt['l'] = 1;
			}
			/*- -n New text file. administrators may edit texts -*/
			if ((get_ezmlmidx_line_arguments(TmpBuf2, "ezmlm-manage", 'e')) > 0) {
				checkopt['n'] = 1;
			}
		}
		fclose(fs);
	}
	/*- figure out some options in the qmail file -*/
	sprintf(TmpBuf, ".qmail-%s", dotqmail_name);
	if ((fs = fopen(TmpBuf, "r")) != NULL) {
		while (fgets(TmpBuf2, sizeof (TmpBuf2), fs)) {
			/*- -o: Reject others than; applicable to message moderated lists only -*/
			if (((get_ezmlmidx_line_arguments(TmpBuf2, "ezmlm-store", 'P')) > 0)) {
				checkopt['o'] = 1;
			}
		}
		fclose(fs);
	}
	/*- -t: Message Trailer. create dir/text/trailer -*/
	sprintf(TmpBuf, "%s/text/trailer", ActionUser);
	if (file_exists(TmpBuf)) {
		checkopt['t'] = 1;
	}
	/*- ------ end of read in old ezmlm's values -*/

	/*- update the uppercase option letters (just the opposite of the lowercase) */
	for (c = 'A'; c <= 'Z'; c++) {
		checkopt[(int) c] = !checkopt[(int) (c - 'A' + 'a')];
	}
}

void
default_options()
{
	char            c;

	*dotqmail_name = '\0';
	replyto = REPLYTO_SENDER;
	*replyto_addr = '\0';

/*-
 * These are currently set to defaults for a good, generic list.
 * Basically, make it safe/friendly and don't turn anything extra on.
 */

	/*- for the options below, use 1 for "on" or "yes" */
	checkopt['a'] = 1;			/* Archive */
	checkopt['b'] = 1;			/* Moderator-only access to archive */
	checkopt['c'] = 0;			/* ignored */
	checkopt['d'] = 0;			/* Digest */
	checkopt['e'] = 0;			/* ignored */
	checkopt['f'] = 1;			/* Prefix */
	checkopt['g'] = 1;			/* Guard Archive */
	checkopt['h'] = 0;			/* Subscribe doesn't require conf */
	checkopt['i'] = 0;			/* Indexed */
	checkopt['j'] = 0;			/* Unsubscribe doesn't require conf */
	checkopt['k'] = 0;			/* Create a blocked sender list */
	checkopt['l'] = 0;			/* Remote admins can access subscriber list */
	checkopt['m'] = 0;			/* Moderated */
	checkopt['n'] = 0;			/* Remote admins can edit text files */
	checkopt['o'] = 0;			/* Others rejected (for Moderated lists only */
	checkopt['p'] = 1;			/* Public */
	checkopt['q'] = 1;			/* Service listname-request, no longer supported */
	checkopt['r'] = 0;			/* Remote Administration */
	checkopt['s'] = 0;			/* Subscriptions are moderated */
	checkopt['t'] = 0;			/* Add Trailer to outgoing messages */
	checkopt['u'] = 1;			/* Only subscribers can post */
	checkopt['v'] = 0;			/* ignored */
	checkopt['w'] = 0;			/* special ezmlm-warn handling (ignored) */
	checkopt['x'] = 0;			/* enable some extras (ignored) */
	checkopt['y'] = 0;			/* ignored */
	checkopt['z'] = 0;			/* ignored */

	/*- update the uppercase option letters (just the opposite of the lowercase) */
	for (c = 'A'; c <= 'Z'; c++) {
		checkopt[(int) c] = !checkopt[(int) (c - 'A' + 'a')];
	}
}

void
show_current_list_values()
{
	FILE           *fs;
	int             sqlfileok = 0;
	int             usesql = 0;
	int             i = 0, j;
	char            checked1[MAX_BUFF] = "";
	char            listname[128];
	int             checked;

/*-
 * Note that we do not support the following list options:
 * k - posts from addresses in listname/deny are rejected
 * x - strip annoying MIME parts (spreadsheets, rtf, html, etc.)
 * 0 - make the list a sublist of another list
 * 3 - replace the From: header with another address
 * 4 - digest options (limits related to sending digest out)
 * 7, 8, 9 - break moderators up into message, subscription and admin
 */

/*-
 * IMPORTANT: If you change the behavior of the checkboxes, you need
 * to update the default settings in modmailinglistnow so iwebadmin
 * will use the proper settings when a checkbox isn't checked.
 */

	if (*dotqmail_name) {		/* modifying an existing list */
		snprinth(listname, sizeof (listname), "%H", dotqmail_name);
		str_replace(listname, ':', '.');
	} else {
		sprintf(listname, "<I>%s</I>", html_text[261]);
	}

	/*- Posting Messages */
	printf("<P><B><U>%s</U></B><BR>\n", html_text[262]);
	build_option_str("RADIO", "opt1", "MU", html_text[263]);
	printf("<BR>\n");
	build_option_str("RADIO", "opt1", "Mu", html_text[264]);
	printf("<BR>\n");
	build_option_str("RADIO", "opt1", "mu", html_text[265]);
	printf("<BR>\n");
	build_option_str("RADIO", "opt1", "mUo", html_text[266]);
	printf("<BR>\n");
	build_option_str("RADIO", "opt1", "mUO", html_text[267]);
	printf("</P>\n");

	/*- List Options */
	printf("<P><B><U>%s</U></B><BR>\n", html_text[268]);
	printf("<TABLE><TR><TD ROWSPAN=3 VALIGN=TOP>%s</TD>", html_text[310]);
	printf("<TD><INPUT TYPE=RADIO NAME=\"replyto\" VALUE=\"%d\"%s>%s</TD></TR>\n", REPLYTO_SENDER,
		   (replyto == REPLYTO_SENDER) ? " CHECKED" : "", html_text[311]);
	printf("<TR><TD><INPUT TYPE=RADIO NAME=\"replyto\" VALUE=\"%d\"%s>%s</TD></TR>\n", REPLYTO_LIST,
		   (replyto == REPLYTO_LIST) ? " CHECKED" : "", html_text[312]);
	printf("<TR><TD><INPUT TYPE=RADIO NAME=\"replyto\" VALUE=\"%d\"%s>%s ", REPLYTO_ADDRESS,
		   (replyto == REPLYTO_ADDRESS) ? " CHECKED" : "", html_text[313]);
	printh("<INPUT TYPE=TEXT NAME=\"replyaddr\" VALUE=\"%H\" SIZE=30></TD></TR>\n", replyto_addr);
	printf("</TABLE><BR>\n");
	build_option_str("CHECKBOX", "opt4", "t", html_text[270]);
	printf("<BR>\n");
	build_option_str("CHECKBOX", "opt5", "d", html_text[271]);
	sprintf(TmpBuf, html_text[272], listname);
	printf("<SMALL>(%s)</SMALL>", TmpBuf);
	printf("</P>");

/*
 * Remote Administration 
 */
	printf("<P><B><U>%s</U></B><BR>\n", html_text[275]);
	build_option_str("CHECKBOX", "opt7", "r", html_text[276]);
	printf("<BR>\n");
	build_option_str("CHECKBOX", "opt8", "P", html_text[277]);
	printf("<SMALL>(%s)</SMALL><BR>", html_text[278]);
	printf("<TABLE><TR><TD ROWSPAN=2 VALIGN=TOP>%s</TD>", html_text[279]);
	printf("<TD>");
	build_option_str("CHECKBOX", "opt9", "l", html_text[280]);
	printf("</TD>\n</TR><TR>\n<TD>");
	build_option_str("CHECKBOX", "opt10", "n", html_text[281]);
	printf("<SMALL>(%s)</SMALL>.</TD>\n", html_text[282]);
	printf("</TR></TABLE>\n</P>\n");

	printf("<P><B><U>%s</U></B><BR>\n", html_text[283]);
	printf("%s<BR>\n&nbsp; &nbsp; ", html_text[284]);
	build_option_str("CHECKBOX", "opt11", "H", html_text[285]);
	printf("<BR>\n&nbsp; &nbsp; ");
	build_option_str("CHECKBOX", "opt12", "s", html_text[286]);
	printf("<BR>\n%s<BR>\n&nbsp; &nbsp; ", html_text[287]);
	build_option_str("CHECKBOX", "opt13", "J", html_text[285]);
	printf("<BR>\n");
	printf("<SMALL>%s</SMALL>\n</P>\n", html_text[288]);

	printf("<P><B><U>%s</U></B><BR>\n", html_text[289]);
	build_option_str("CHECKBOX", "opt14", "a", html_text[290]);
	printf(" &nbsp; %s\n<SELECT NAME=\"opt15\">", html_text[292]);
	printf("<OPTION VALUE=\"BG\"%s>%s\n", checkopt['B'] && checkopt['G'] ? " SELECTED" : "", html_text[293]);
	printf("<OPTION VALUE=\"Bg\"%s>%s\n", checkopt['B'] && checkopt['g'] ? " SELECTED" : "", html_text[294]);
	printf("<OPTION VALUE=\"b\"%s>%s\n", checkopt['b'] ? " SELECTED" : "", html_text[295]);
	printf("</SELECT>.");
	printf("<BR>\n");
	/*-
	 * note that if user doesn't have ezmlm-cgi installed, it might be
	 * a good idea to default to having option i off. 
	 */
	build_option_str("CHECKBOX", "opt16", "i", html_text[291]);
	printf("</P>\n");

	/*- begin MySQL options */

	/*- See if sql is turned on */
	checked = 0;
	sprintf(TmpBuf, "%s/sql", ActionUser);
	if ((fs = fopen(TmpBuf, "r")) != NULL) {
		checked = 1;
		while (fgets(TmpBuf2, sizeof (TmpBuf2), fs)) {
			strcpy(TmpBuf1, TmpBuf2);
			i = strlen(TmpBuf1);
			--i;
			TmpBuf1[i] = 0;		/* take off newline */
			if ((strstr(TmpBuf1, ":")) != NULL) {
				sqlfileok = 1;
			}
		}
		usesql = 1;
		fclose(fs);
	}
#ifdef ENABLE_MYSQL
	printf("<P><B><U>%s</U></B><BR>\n", html_text[99]);
	printf("<input type=checkbox name=\"sqlsupport\" value=\"-6\"%s> %s", checked ? " CHECKED" : "", html_text[53]);

	/*- parse dir/sql file for SQL settings */
	printf("    <table cellpadding=0 cellspacing=2 border=0>\n");
#else
	if (checked)
		printf("<INPUT TYPE=HIDDEN NAME=sqlsupport VALUE=\"-6\">\n");
#endif

	/*- get hostname */
	strcpy(checked1, "localhost");
	if (usesql == 1 && sqlfileok == 1) {
		strncpy(TmpBuf3, TmpBuf1, 1);
		if ((strstr(TmpBuf3, ":")) == NULL) {
			for (i = 0, j = 0; TmpBuf1[i] != ':' && TmpBuf1[i] != '\0'; ++j, ++i)
				checked1[j] = TmpBuf1[i];
			checked1[j] = '\0';
		}
	}
#ifdef ENABLE_MYSQL
	printf("      <tr>\n");
	printf("        <td ALIGN=RIGHT>%s:\n", html_text[54]);
	printf("          </td><td>\n");
	printh("          <input type=text name=sql1 value=\"%H\"></td>\n", checked1);
#else
	printh("<INPUT TYPE=HIDDEN NAME=sql1 VALUE=\"%H\">\n", checked1);
#endif

	/*- get port */
	strcpy(checked1, "3306");
	if (usesql == 1 && sqlfileok == 1) {
		strncpy(TmpBuf3, &TmpBuf1[++i], 1);
		if ((strstr(TmpBuf3, ":")) == NULL) {
			for (j = 0; TmpBuf1[i] != ':' && TmpBuf1[i] != '\0'; ++j, ++i)
				checked1[j] = TmpBuf1[i];
			checked1[j] = '\0';
		}
	}
#ifdef ENABLE_MYSQL
	printf("        <td ALIGN=RIGHT>%s:\n", html_text[55]);
	printf("          </td><td>\n");
	printh("          <input type=text size=7 name=sql2 value=\"%H\"></td>\n", checked1);
	printf("      </tr>\n");
#else
	printh("<INPUT TYPE=HIDDEN NAME=sql2 VALUE=\"%H\">\n", checked1);
#endif

	/*- get user */
	strcpy(checked1, "");
	if (usesql == 1 && sqlfileok == 1) {
		strncpy(TmpBuf3, &TmpBuf1[++i], 1);
		if ((strstr(TmpBuf3, ":")) == NULL) {
			for (j = 0; TmpBuf1[i] != ':' && TmpBuf1[i] != '\0'; ++j, ++i)
				checked1[j] = TmpBuf1[i];
			checked1[j] = '\0';
		}
	}
#ifdef ENABLE_MYSQL
	printf("      <tr>\n");
	printf("        <td ALIGN=RIGHT>%s:\n", html_text[56]);
	printf("          </td><td>\n");
	printh("          <input type=text name=sql3 value=\"%H\"></td>\n", checked1);
#else
	printh("<INPUT TYPE=HIDDEN NAME=sql3 VALUE=\"%H\">\n", checked1);
#endif

	/*- get password */
	strcpy(checked1, "");
	if (usesql == 1 && sqlfileok == 1) {
		strncpy(TmpBuf3, &TmpBuf1[++i], 1);
		if ((strstr(TmpBuf3, ":")) == NULL) {
			for (j = 0; TmpBuf1[i] != ':' && TmpBuf1[i] != '\0'; ++j, ++i)
				checked1[j] = TmpBuf1[i];
			checked1[j] = '\0';
		}
	}
#ifdef ENABLE_MYSQL
	printf("        <td ALIGN=RIGHT>%s:\n", html_text[57]);
	printf("          </td><td>\n");
	printh("          <input type=text name=sql4 value=\"%H\"></td>\n", checked1);
	printf("      </tr>\n");
#else
	printh("<INPUT TYPE=HIDDEN NAME=sql4 VALUE=\"%H\">\n", checked1);
#endif

	/*- get database name */
	strcpy(checked1, "");
	if (usesql == 1 && sqlfileok == 1) {
		strncpy(TmpBuf3, &TmpBuf1[++i], 1);
		if ((strstr(TmpBuf3, ":")) == NULL) {
			for (j = 0; TmpBuf1[i] != ':' && TmpBuf1[i] != '\0'; ++j, ++i)
				checked1[j] = TmpBuf1[i];
			checked1[j] = '\0';
		}
	}
#ifdef ENABLE_MYSQL
	printf("      <tr>\n");
	printf("        <td ALIGN=RIGHT>%s:\n", html_text[58]);
	printf("          </td><td>\n");
	printh("          <input type=text name=sql5 value=\"%H\"></td>\n", checked1);
#else
	printh("<INPUT TYPE=HIDDEN NAME=sql5 VALUE=\"%H\">\n", checked1);
#endif

	/*- get table name */
	strcpy(checked1, "ezmlm");
	if (usesql == 1 && sqlfileok == 1) {
		++i;
		if (strlen(TmpBuf1) != i) {
			for (j = 0; TmpBuf1[i] != ':' && TmpBuf1[i] != '\0'; ++j, ++i)
				checked1[j] = TmpBuf1[i];
			checked1[j] = '\0';
		}
	}
#ifdef ENABLE_MYSQL
	printf("        <td ALIGN=RIGHT>%s:\n", html_text[59]);
	printf("          </td><td>\n");
	printh("          <input type=text name=\"sql6\" value=\"%H\"></td>\n", checked1);
	printf("      </tr>\n");
	printf("    </table>\n");
#else
	printh("<INPUT TYPE=HIDDEN NAME=sql6 VALUE=\"%H\">\n", checked1);
#endif

}

int
get_mailinglist_prefix(char *prefix)
{
	char            buffer[MAX_BUFF];
	char           *b, *p;
	FILE           *file;

	sprintf(buffer, "%s/%s/prefix", RealDir, ActionUser);
	file = fopen(buffer, "r");

	if (file) {
		(void) fgets(buffer, sizeof (buffer), file);
		(void) fclose(file);
		b = buffer;
		p = prefix;
		while (*b == '[')
			b++;
		while ((*b != ']') && (*b != '\n') && (*b != '\0'))
			*p++ = *b++;
		*p++ = '\0';
	} else {
		return 1;
	}
	return 0;
}

void
getversion_qamailinglist_c()
{
	printf("%s\n", sccsidh);
}
