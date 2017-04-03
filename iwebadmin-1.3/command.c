/*
 * $Id: command.c,v 1.6 2017-04-03 14:35:30+05:30 Cprogrammer Exp mbhangui $
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <dirent.h>
#include "alias.h"
#include "autorespond.h"
#include "cgi.h"
#include "command.h"
#include "forward.h"
#include "mailinglist.h"
#include "printh.h"
#include "iwebadmin.h"
#include "iwebadminx.h"
#include "show.h"
#include "user.h"
#include "util.h"

void
process_commands()
{
	if (strcmp(TmpBuf2, "showmenu") == 0) {
		show_menu(Username, Domain, Mytime);
	} else
	if (strcmp(TmpBuf2, "quick") == 0) {
		/*
		 * This feature sponsored by PinkRoccade Public Sector, Sept 2004 
		 */
		char            moddel[20];
		/*
		 * we use global ActionUser here because the functions that
		 * quickAction calls expect the username in that global.
		 */
		GetValue(TmpCGI, ActionUser, "modu=", sizeof (ActionUser));
		lowerit(ActionUser);	/* convert username to lower case */
		GetValue(TmpCGI, moddel, "MODIFY=", sizeof (moddel));
		if (*moddel) {
			quickAction(ActionUser, ACTION_MODIFY);
		} else {
			GetValue(TmpCGI, moddel, "DELETE=", sizeof (moddel));
			if (*moddel) {
				quickAction(ActionUser, ACTION_DELETE);
			} else {
			/*
			 * malformed request -- missing fields 
			 */
				show_menu(Username, Domain, Mytime);
				vclose();
				exit(0);
			}
		}
	} else
	if (strcmp(TmpBuf2, "showusers") == 0) {
		GetValue(TmpCGI, Pagenumber, "page=", sizeof (Pagenumber));
		GetValue(TmpCGI, SearchUser, "searchuser=", sizeof (SearchUser));
		show_users(Username, Domain, Mytime);
	} else
	if (strcmp(TmpBuf2, "showaliases") == 0) {
		GetValue(TmpCGI, Pagenumber, "page=", sizeof (Pagenumber));
		show_aliases();
	} else
	if (strcmp(TmpBuf2, "showforwards") == 0) {
		GetValue(TmpCGI, Pagenumber, "page=", sizeof (Pagenumber));
		GetValue(TmpCGI, SearchUser, "searchuser=", sizeof (SearchUser));
		show_forwards(Username, Domain, Mytime);
	} else
	if (strcmp(TmpBuf2, "showmailinglists") == 0) {
		show_mailing_lists(Username, Domain, Mytime);
	} else
	if (strcmp(TmpBuf2, "showautoresponders") == 0) {
		show_autoresponders(Username, Domain, Mytime);
	} else
	if (strcmp(TmpBuf2, "adduser") == 0) {
		adduser();
	} else
	if (strcmp(TmpBuf2, "addusernow") == 0) {
		addusernow();
#ifdef CATCHALL_ENABLED
	} else
	if (strcmp(TmpBuf2, "setdefault") == 0) {
		GetValue(TmpCGI, ActionUser, "deluser=", sizeof (ActionUser));
		GetValue(TmpCGI, Pagenumber, "page=", sizeof (Pagenumber));
		setdefaultaccount();
	} else
	if (strcmp(TmpBuf2, "bounceall") == 0) {
		bounceall();
	} else
	if (strcmp(TmpBuf2, "deleteall") == 0) {
		deleteall();
	} else
	if (strcmp(TmpBuf2, "setremotecatchall") == 0) {
		setremotecatchall();
	} else
	if (strcmp(TmpBuf2, "setremotecatchallnow") == 0) {
		setremotecatchallnow();
#endif
	} else
	if (strcmp(TmpBuf2, "addlistmodnow") == 0) {
		GetValue(TmpCGI, ActionUser, "modu=", sizeof (ActionUser));
		GetValue(TmpCGI, Newu, "newu=", sizeof (Newu));
		addlistgroupnow(1);
	} else
	if (strcmp(TmpBuf2, "dellistmod") == 0) {
		GetValue(TmpCGI, ActionUser, "modu=", sizeof (ActionUser));
		dellistgroup("del_listmod.html");
	} else
	if (strcmp(TmpBuf2, "dellistmodnow") == 0) {
		GetValue(TmpCGI, ActionUser, "modu=", sizeof (ActionUser));
		GetValue(TmpCGI, Newu, "newu=", sizeof (Newu));
		dellistgroupnow(1);
	} else
	if (strcmp(TmpBuf2, "addlistmod") == 0) {
		GetValue(TmpCGI, ActionUser, "modu=", sizeof (ActionUser));
		addlistgroup("add_listmod.html");
	} else
	if (strcmp(TmpBuf2, "showlistmod") == 0) {
		GetValue(TmpCGI, ActionUser, "modu=", sizeof (ActionUser));
		show_list_group("show_moderators.html");
	} else
	if (strcmp(TmpBuf2, "addlistdig") == 0) {
		GetValue(TmpCGI, ActionUser, "modu=", sizeof (ActionUser));
		addlistgroup("add_listdig.html");
	} else
	if (strcmp(TmpBuf2, "addlistdignow") == 0) {
		GetValue(TmpCGI, ActionUser, "modu=", sizeof (ActionUser));
		GetValue(TmpCGI, Newu, "newu=", sizeof (Newu));
		addlistgroupnow(2);
	} else
	if (strcmp(TmpBuf2, "dellistdig") == 0) {
		GetValue(TmpCGI, ActionUser, "modu=", sizeof (ActionUser));
		dellistgroup("del_listdig.html");
	} else
	if (strcmp(TmpBuf2, "dellistdignow") == 0) {
		GetValue(TmpCGI, ActionUser, "modu=", sizeof (ActionUser));
		GetValue(TmpCGI, Newu, "newu=", sizeof (Newu));
		dellistgroupnow(2);
	} else
	if (strcmp(TmpBuf2, "showlistdig") == 0) {
		GetValue(TmpCGI, ActionUser, "modu=", sizeof (ActionUser));
		show_list_group("show_digest_subscribers.html");
	} else
	if (strcmp(TmpBuf2, "moduser") == 0) {
		GetValue(TmpCGI, ActionUser, "moduser=", sizeof (ActionUser));
		moduser();
	} else
	if (strcmp(TmpBuf2, "modusernow") == 0) {
		GetValue(TmpCGI, ActionUser, "modu=", sizeof (ActionUser));
		GetValue(TmpCGI, Password1, "password1=", sizeof (Password1));
		GetValue(TmpCGI, Password2, "password2=", sizeof (Password2));
		GetValue(TmpCGI, Gecos, "gecos=", sizeof (Gecos));
		modusergo();
	} else
	if (strcmp(TmpBuf2, "deluser") == 0) {
		GetValue(TmpCGI, ActionUser, "deluser=", sizeof (ActionUser));
		deluser();
	} else
	if (strcmp(TmpBuf2, "delusernow") == 0) {
		GetValue(TmpCGI, ActionUser, "deluser=", sizeof (ActionUser));
		delusergo();
	} else
	if (strcmp(TmpBuf2, "moddotqmail") == 0) {
		GetValue(TmpCGI, ActionUser, "modu=", sizeof (ActionUser));
		moddotqmail();
	} else
	if (strcmp(TmpBuf2, "moddotqmailnow") == 0) {
		GetValue(TmpCGI, ActionUser, "modu=", sizeof (ActionUser));
		GetValue(TmpCGI, Newu, "newu=", sizeof (Newu));
		GetValue(TmpCGI, LineData, "linedata=", sizeof (LineData));
		GetValue(TmpCGI, Action, "action=", sizeof (Action));
		moddotqmailnow();
	} else
	if (strcmp(TmpBuf2, "deldotqmail") == 0) {
		GetValue(TmpCGI, ActionUser, "modu=", sizeof (ActionUser));
		deldotqmail();
	} else
	if (strcmp(TmpBuf2, "deldotqmailnow") == 0) {
		GetValue(TmpCGI, ActionUser, "modu=", sizeof (ActionUser));
		deldotqmailnow();
	} else
	if (strcmp(TmpBuf2, "adddotqmail") == 0) {
		adddotqmail();
	} else
	if (strcmp(TmpBuf2, "adddotqmailnow") == 0) {
		GetValue(TmpCGI, ActionUser, "newu=", sizeof (ActionUser));
		GetValue(TmpCGI, Alias, "alias=", sizeof (Alias));
		adddotqmailnow();
	} else
	if (strcmp(TmpBuf2, "addmailinglist") == 0) {
		addmailinglist();
	} else
	if (strcmp(TmpBuf2, "delmailinglist") == 0) {
		GetValue(TmpCGI, ActionUser, "modu=", sizeof (ActionUser));
		delmailinglist();
	} else
	if (strcmp(TmpBuf2, "delmailinglistnow") == 0) {
		GetValue(TmpCGI, ActionUser, "modu=", sizeof (ActionUser));
		delmailinglistnow();
	} else
	if (strcmp(TmpBuf2, "addlistusernow") == 0) {
		GetValue(TmpCGI, ActionUser, "modu=", sizeof (ActionUser));
		GetValue(TmpCGI, Newu, "newu=", sizeof (Newu));
		addlistgroupnow(0);
	} else
	if (strcmp(TmpBuf2, "dellistuser") == 0) {
		GetValue(TmpCGI, ActionUser, "modu=", sizeof (ActionUser));
		dellistgroup("del_listuser.html");
	} else
	if (strcmp(TmpBuf2, "dellistusernow") == 0) {
		GetValue(TmpCGI, ActionUser, "modu=", sizeof (ActionUser));
		GetValue(TmpCGI, Newu, "newu=", sizeof (Newu));
		dellistgroupnow(0);
	} else
	if (strcmp(TmpBuf2, "addlistuser") == 0) {
		GetValue(TmpCGI, ActionUser, "modu=", sizeof (ActionUser));
		addlistgroup("add_listuser.html");
	} else
	if (strcmp(TmpBuf2, "addmailinglistnow") == 0) {
		GetValue(TmpCGI, ActionUser, "newu=", sizeof (ActionUser));
		addmailinglistnow();
	} else
	if (strcmp(TmpBuf2, "modmailinglist") == 0) {
		GetValue(TmpCGI, ActionUser, "modu=", sizeof (ActionUser));
		modmailinglist();
	} else
	if (strcmp(TmpBuf2, "modmailinglistnow") == 0) {
		GetValue(TmpCGI, ActionUser, "newu=", sizeof (ActionUser));
		modmailinglistnow();
	} else
	if (strcmp(TmpBuf2, "modautorespond") == 0) {
		GetValue(TmpCGI, ActionUser, "modu=", sizeof (ActionUser));
		modautorespond();
	} else
	if (strcmp(TmpBuf2, "addautorespond") == 0) {
		addautorespond();
	} else
	if (strcmp(TmpBuf2, "addautorespondnow") == 0) {
		GetValue(TmpCGI, ActionUser, "newu=", sizeof (ActionUser));
		GetValue(TmpCGI, Alias, "alias=", sizeof (Alias)); /* subject */
		GetValue(TmpCGI, Message, "message=", sizeof (Message));
		GetValue(TmpCGI, Newu, "owner=", sizeof (Newu));
		addautorespondnow();
	} else
	if (strcmp(TmpBuf2, "modautorespondnow") == 0) {
		GetValue(TmpCGI, ActionUser, "newu=", sizeof (ActionUser));
		GetValue(TmpCGI, Alias, "alias=", sizeof (Alias)); /* subject */
		GetValue(TmpCGI, Message, "message=", sizeof (Message));
		GetValue(TmpCGI, Newu, "owner=", sizeof (Newu));
		modautorespondnow();
	} else
	if (strcmp(TmpBuf2, "showlistusers") == 0) {
		GetValue(TmpCGI, ActionUser, "modu=", sizeof (ActionUser));
		show_list_group("show_subscribers.html");
	} else
	if (strcmp(TmpBuf2, "delautorespond") == 0) {
		GetValue(TmpCGI, ActionUser, "modu=", sizeof (ActionUser));
		delautorespond();
	} else
	if (strcmp(TmpBuf2, "delautorespondnow") == 0) {
		GetValue(TmpCGI, ActionUser, "modu=", sizeof (ActionUser));
		delautorespondnow();
	} else
	if (strcmp(TmpBuf2, "logout") == 0) {
		sprintf(TmpBuf, "%s/%s/" MAILDIR, RealDir, Username);
		del_id_files(TmpBuf);
		show_login();
	} else
	if (strcmp(TmpBuf2, "showcounts") == 0) {
		show_counts();
	}
	vclose();
	exit(0);
}

void
setdefaultaccount()
{
	struct passwd  *pw;
	FILE           *fs;
	int             use_vfilter = 0;
	char            buffer[MAX_BUFF];

	if ((fs = fopen(".qmail-default", "r"))) {
		fscanf(fs, "%s", buffer);
		use_vfilter = strstr(buffer, "vfilter") ? 1: 0;
		fclose(fs);
	}

	if ((pw = vauth_getpw(ActionUser, Domain)) == NULL) {
		snprinth(StatusMessage, sizeof (StatusMessage), "%s %H@%H", html_text[223], ActionUser, Domain);
	} else {
		if ((fs = fopen(".qmail-default", "w")) == NULL) {
			snprintf(StatusMessage, sizeof (StatusMessage), "%s", html_text[82]);
		} else {
			fprintf(fs, "| %s/sbin/%s '' %s@%s\n", INDIMAILDIR, 
				use_vfilter ? "vfilter" : "vdelivermail", ActionUser, Domain);
			fclose(fs);
		}
	}
	show_users(Username, Domain, Mytime);
	vclose();
	exit(0);
}

void
getversion_qacommand_c()
{
	printf("%s\n", sccsidh);
}
