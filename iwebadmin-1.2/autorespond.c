/*
 * $Id: autorespond.c,v 1.6 2013-09-20 14:44:22+05:30 Cprogrammer Exp mbhangui $
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

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <dirent.h>
#include "autorespond.h"
#include "limits.h"
#include "printh.h"
#include "iwebadmin.h"
#include "iwebadminx.h"
#include "show.h"
#include "template.h"
#include "util.h"

void
show_autoresponders(user, dom, mytime)
	char           *user;
	char           *dom;
	time_t          mytime;
{
	if (MaxAutoResponders == 0)
		return;
	count_autoresponders();
	if (CurAutoResponders == 0) {
		snprintf(StatusMessage, sizeof (StatusMessage), "%s", html_text[233]);
		show_menu(Username, Domain, Mytime);
	} else {
		send_template("show_autorespond.html");
	}
}

void
show_autorespond_line(char *user, char *dom, time_t mytime, char *dir)
{
	char           *addr, *domptr;
	int             i;
	DIR            *mydir;
	struct dirent  *mydirent;
	struct passwd  *vpw;

	sort_init();
	sprintf(TmpBuf, "%s/vacation", RealDir);
	if ((mydir = opendir(TmpBuf)) == NULL) {
		printf("<tr><td>%s %d</tr><td>", html_text[143], 1);
		return;
	}
	while ((mydirent = readdir(mydir)) != NULL) {
		if (mydirent->d_name[0] == '.')
			continue;
		sort_add_entry(mydirent->d_name, 0);
	}
	if ((domptr = strrchr(RealDir, '/')))
		domptr++;
	else
		domptr = dom;
	sort_dosort();
	for (i = 0; (addr = (char *) sort_get_entry(i)); ++i) {
		printf("<tr>");
		printf("<td align=\"center\">");
		printh("<a href=\"%s&modu=%C\">", cgiurl("delautorespond"), addr);
		printf("<img src=\"%s/trash.png\" border=\"0\"></a>", IMAGEURL);
		printf("</td>");
		printf("<td align=\"center\">");
		if ((vpw = vauth_getpw(addr, domptr)))
			printh("<a href=\"%s&moduser=%C\">", cgiurl("moduser"), addr);
		else
			printh("<a href=\"%s&modu=%C\">", cgiurl("modautorespond"), addr);
		printf("<img src=\"%s/modify.png\" border=\"0\"></a>", IMAGEURL);
		printf("</td>");
		printh("<td align=\"left\">%H@%H</td>", addr, Domain);
		printf("</tr>\n");
	}
	sort_cleanup();
}

void
addautorespond()
{

	if (AdminType != DOMAIN_ADMIN) {
		snprintf(StatusMessage, sizeof (StatusMessage), "%s", html_text[142]);
		vclose();
		exit(0);
	}
	count_autoresponders();
	load_limits();
	if (MaxAutoResponders != -1 && CurAutoResponders >= MaxAutoResponders) {
		printf("%s %d\n", html_text[158], MaxAutoResponders);
		show_menu(Username, Domain, Mytime);
		vclose();
		exit(0);
	}
	send_template("add_autorespond.html");

}

void
addautorespondnow()
{
	FILE           *fs;

	if (AdminType != DOMAIN_ADMIN) {
		snprintf(StatusMessage, sizeof (StatusMessage), "%s", html_text[142]);
		vclose();
		exit(0);
	}
	count_autoresponders();
	load_limits();
	if (MaxAutoResponders != -1 && CurAutoResponders >= MaxAutoResponders) {
		printf("%s %d\n", html_text[158], MaxAutoResponders);
		show_menu(Username, Domain, Mytime);
		vclose();
		exit(0);
	}
	*StatusMessage = '\0';
	if (fixup_local_name(ActionUser))
		snprinth(StatusMessage, sizeof (StatusMessage), "%s %H\n", html_text[174], ActionUser);
	else
	if (check_local_user(ActionUser))
		snprinth(StatusMessage, sizeof (StatusMessage), "%s %H\n", html_text[175], ActionUser);
	else
	if (strlen(ActionUser) == 0)
		snprintf(StatusMessage, sizeof (StatusMessage), "%s\n", html_text[176]);
	else
	if (strlen(Newu) > 0 && check_email_addr(Newu))
		snprinth(StatusMessage, sizeof (StatusMessage), "%s %H\n", html_text[177], Newu);
	else
	if (strlen(Alias) <= 1)
		snprinth(StatusMessage, sizeof (StatusMessage), "%s %H\n", html_text[178], ActionUser);
	else
	if (strlen(Message) <= 1)
		snprinth(StatusMessage, sizeof (StatusMessage), "%s %H\n", html_text[179], ActionUser);
	/*- if there was an error, go back to the add screen */
	if (*StatusMessage != '\0') {
		addautorespond();
		vclose();
		exit(0);
	}
	/*- Make the autoresponder directory */
	sprintf(TmpBuf, "%s/vacation/%s", RealDir, ActionUser);
	r_mkdir(TmpBuf, 0750, Uid, Gid);
	/*- Make the autoresponder message file */
	sprintf(TmpBuf, "%s/vacation/%s/.vacation.msg", RealDir, ActionUser);
	if ((fs = fopen(TmpBuf, "w")) == NULL)
		ack("150", TmpBuf);
	fprintf(fs, "Subject: This is an autoresponse From: %s@%s Re: %s\n", 
		ActionUser, Domain, Alias);
	fprintf(fs, "\n%s\n", Message);
	fclose(fs);
	/*- Make the autoresponder .qmail file */
	valias_delete(ActionUser, Domain, 0);
	if (strlen(Newu) > 0) {
		sprintf(TmpBuf, "&%s", Newu);
		valias_insert(ActionUser, Domain, TmpBuf, 1);
	}
	sprintf(TmpBuf, "%s/content-type", RealDir);
	if (access(TmpBuf, R_OK))
		sprintf(TmpBuf, "|%s/bin/autoresponder -q %s/vacation/%s/.vacation.msg %s/vacation/%s",
			INDIMAILDIR, RealDir, ActionUser, RealDir, ActionUser);
	else
		sprintf(TmpBuf, "|%s/bin/autoresponder -q -T %s/content-type %s/vacation/%s/.vacation.msg %s/vacation/%s",
			INDIMAILDIR, RealDir, RealDir, ActionUser, RealDir, ActionUser);
	valias_insert(ActionUser, Domain, TmpBuf, 1);
	/*- Report success */
	snprinth(StatusMessage, sizeof (StatusMessage), "%s %H@%H\n", html_text[180], ActionUser, Domain);
	show_autoresponders(Username, Domain, Mytime);
}

void
delautorespond()
{
	if (AdminType != DOMAIN_ADMIN) {
		snprintf(StatusMessage, sizeof (StatusMessage), "%s", html_text[142]);
		vclose();
		exit(0);
	}
	send_template("del_autorespond_confirm.html");
}

void
delautorespondnow()
{
	if (AdminType != DOMAIN_ADMIN) {
		snprintf(StatusMessage, sizeof (StatusMessage), "%s", html_text[142]);
		vclose();
		exit(0);
	}
	/*- delete the alias */
	valias_delete(ActionUser, Domain, 0);
	/*- delete the autoresponder directory */
	sprintf(TmpBuf, "%s/vacation/%s", RealDir, ActionUser);
	vdelfiles(TmpBuf, ActionUser, Domain);
	snprinth(StatusMessage, sizeof (StatusMessage), "%s %H\n", html_text[182], ActionUser);
	count_autoresponders();
	if (CurAutoResponders == 0) {
		show_menu(Username, Domain, Mytime);
	} else {
		send_template("show_autorespond.html");
	}
}

void
modautorespond()
{
	if (AdminType != DOMAIN_ADMIN) {
		snprintf(StatusMessage, sizeof (StatusMessage), "%s", html_text[142]);
		vclose();
		exit(0);
	}
	/*- send_template("show_forwards.html"); -*/
	send_template("mod_autorespond.html");
}


/*
 * addautorespondnow and modautorespondnow should be merged into a single function 
 */
void
modautorespondnow()
{
	FILE           *fs;

	if (AdminType != DOMAIN_ADMIN) {
		snprintf(StatusMessage, sizeof (StatusMessage), "%s", html_text[142]);
		vclose();
		exit(0);
	}
	*StatusMessage = '\0';
	if (fixup_local_name(ActionUser))
		snprinth(StatusMessage, sizeof (StatusMessage), "%s %H\n", html_text[174], ActionUser);
	else
	if (strlen(Newu) > 0 && check_email_addr(Newu))
		snprinth(StatusMessage, sizeof (StatusMessage), "%s %H\n", html_text[177], Newu);
	else
	if (strlen(Alias) <= 1)
		snprinth(StatusMessage, sizeof (StatusMessage), "%s %H\n", html_text[178], ActionUser);
	else
	if (strlen(Message) <= 1)
		snprinth(StatusMessage, sizeof (StatusMessage), "%s %H\n", html_text[179], ActionUser);
	/*- exit on errors */
	if (*StatusMessage != '\0') {
		modautorespond();
		vclose();
		exit(0);
	}
	/*- Make the autoresponder directory */
	sprintf(TmpBuf, "%s/vacation/%s", RealDir, ActionUser);
	r_mkdir(TmpBuf, 0750, Uid, Gid);
	/*- Make the autoresponder message file */
	sprintf(TmpBuf, "%s/vacation/%s/.vacation.msg", RealDir, ActionUser);
	if ((fs = fopen(TmpBuf, "w")) == NULL)
		ack("150", TmpBuf);
	fprintf(fs, "Subject: This is an autoresponse From: %s@%s Re: %s\n",
		ActionUser, Domain, Alias);
	fprintf(fs, "\n%s\n", Message);
	fclose(fs);
	/*- Make the autoresponder .qmail file */
	valias_delete(ActionUser, Domain, 0);
	if (strlen(Newu) > 0) {
		sprintf(TmpBuf, "&%s", Newu);
		valias_insert(ActionUser, Domain, TmpBuf, 1);
	}
	sprintf(TmpBuf, "%s/content-type", RealDir);
	if (access(TmpBuf, R_OK))
		sprintf(TmpBuf, "|%s/bin/autoresponder -q %s/vacation/%s/.vacation.msg %s/vacation/%s",
			INDIMAILDIR, RealDir, ActionUser, RealDir, ActionUser);
	else
		sprintf(TmpBuf, "|%s/bin/autoresponder -q -T %s/content-type %s/vacation/%s/.vacation.msg %s/vacation/%s",
			INDIMAILDIR, RealDir, RealDir, ActionUser, RealDir, ActionUser);
	valias_insert(ActionUser, Domain, TmpBuf, 1);
	/*- Report success */
	snprinth(StatusMessage, sizeof (StatusMessage), "%s %H@%H\n", html_text[183], ActionUser, Domain);
	show_autoresponders(Username, Domain, Mytime);
}

void
count_autoresponders()
{
#if 0
	char            alias_name[MAX_FILE_NAME];
	char           *alias_line;
#endif
	DIR            *mydir;
	struct dirent  *mydirent;

	CurAutoResponders = 0;
#if 0
	for (;;)
	{
		if (!(alias_line = valias_select_all(alias_name, Domain, MAX_BUFF)))
			break;
		if (strstr(alias_line, "/autoresponder ") != 0)
			CurAutoResponders++;
	}
#endif
	sprintf(TmpBuf, "%s/vacation", RealDir);
	if ((mydir = opendir(TmpBuf)) == NULL) {
		printf("<tr><td>%s %d</tr><td>", html_text[143], 1);
		return;
	}
	while ((mydirent = readdir(mydir)) != NULL) {
		if (mydirent->d_name[0] == '.')
			continue;
		CurAutoResponders++;
	}
}

void
getversion_qaautorespond_c()
{
	printf("%s\n", sccsidh);
}
