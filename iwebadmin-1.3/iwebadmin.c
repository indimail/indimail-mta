/*
 * $Id: iwebadmin.c,v 1.10 2019-05-01 23:20:11+05:30 Cprogrammer Exp mbhangui $
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
#include <stdlib.h>
#include <string.h>
#define _USE_XOPEN
#define _XOPEN_SOURCE
#include <unistd.h>
#include <sys/stat.h>
#include <pwd.h>
#include <dirent.h>
#include "alias.h"
#include "auth.h"
#include "autorespond.h"
#include "cgi.h"
#include "command.h"
#include "limits.h"
#include "mailinglist.h"
#include "printh.h"
#include "iwebadmin.h"
#include "show.h"
#include "template.h"
#include "user.h"
#include "util.h"

char            Username[MAX_BUFF];
char            Domain[MAX_BUFF];
char            Password[MAX_BUFF];
char            Gecos[MAX_BUFF];
char            Quota[MAX_BUFF];
char            Time[MAX_BUFF];
char            ActionUser[MAX_BUFF];
char            Newu[MAX_BUFF];
char            Password1[MAX_BUFF];
char            Password2[MAX_BUFF];
char            Crypted[MAX_BUFF];
char            Alias[MAX_BUFF];
char            LineData[MAX_BUFF];
char            Action[MAX_BUFF];
char            Message[MAX_BIG_BUFF];
char            StatusMessage[MAX_BIG_BUFF];
int             num_of_mailinglist;
int             CGIValues[256];
char            Pagenumber[MAX_BUFF];
char            SearchUser[MAX_BUFF];
time_t          Mytime;
char           *TmpCGI = NULL;
char            TmpBuf[MAX_BIG_BUFF];
char            TmpBuf1[MAX_BUFF];
char            TmpBuf2[MAX_BUFF];
char            TmpBuf3[MAX_BUFF];
char            TempBuf[MAX_BUFF];
int             Compressed;
FILE           *actout;
FILE           *lang_fs;
FILE           *color_table;
char           *html_text[MAX_LANG_STR + 1];

struct vlimits  Limits;
int             AdminType;
int             MaxPopAccounts;
int             MaxAliases;
int             MaxForwards;
int             MaxAutoResponders;
int             MaxMailingLists;

int             CurPopAccounts;
int             CurForwards;
int             CurBlackholes;
int             CurAutoResponders;
int             CurMailingLists;
uid_t           Uid;
gid_t           Gid;
char            RealDir[156];
char            Lang[40];

extern char *crypt(const char *, const char *);

void
iwebadmin_suid(gid_t Gid, uid_t Uid)
{
#ifdef HAVE_SETREGID
	if (setregid(Gid, Gid) != 0)
#else
	if (setgid(Gid) != 0)
#endif
	{
		printf("<h2>%s</h2>\n", html_text[318]);
		perror("iwebadmin: setregid");
		vclose();
		exit(EXIT_FAILURE);
	}
#ifdef HAVE_SETREGID
	if (setreuid(Uid, Uid) != 0)
#else
	if (setuid(Uid) != 0)
#endif
	{
		printf("<h2>%s</h2>\n", html_text[319]);
		perror("iwebadmin: setreuid");
		vclose();
		exit(EXIT_FAILURE);
	}
}

int
auth_user(struct passwd *pw, char *password)
{
	if (!strcmp(pw->pw_passwd, (char *) crypt(password, pw->pw_passwd)))
		return (0);
	/*- 
	 * Authtenticate case where you have CRAM-MD5 but clients do
	 * not have cram-md5 authentication mechanism 
	 */
	if (!access(".trivial_passwords", F_OK) && !strcmp(pw->pw_passwd, password))
		return (0);
	snprintf(StatusMessage, sizeof (StatusMessage), "%s", html_text[198]);
	return (1);
}

int
main(argc, argv)
	int             argc;
	char           *argv[];
{
	const char     *ip_addr = getenv("REMOTE_ADDR");
	const char     *x_forward = getenv("HTTP_X_FORWARDED_FOR");
	char           *pi;
	char           *rm;
	char            returnhttp[MAX_BUFF];
	char            returntext[MAX_BUFF];
	int             i;
	extern int      encrypt_flag;
	struct passwd  *pw;

	init_globals();
	if (x_forward)
		ip_addr = x_forward;
	if (!ip_addr)
		ip_addr = "127.0.0.1";
	pi = getenv("PATH_INFO");
	if (pi)
		pi = strdup(pi);
	if (pi)
		snprintf(TmpBuf2, sizeof (TmpBuf2), "%s", pi + 5);
	rm = getenv("REQUEST_METHOD");
	rm = (rm == NULL ? "" : strdup(rm));
	if (strncmp(rm, "POST", 4) == 0) {
		get_cgi();
	} else {
		TmpCGI = getenv("QUERY_STRING");
		TmpCGI = (TmpCGI == NULL ? "" : strdup(TmpCGI));
	}
	if (pi && strncmp(pi, "/com/", 5) == 0) {
		GetValue(TmpCGI, Username, "user=", sizeof (Username));
		GetValue(TmpCGI, Domain, "dom=", sizeof (Domain));
		GetValue(TmpCGI, Time, "time=", sizeof (Time));
		Mytime = atoi(Time);
		pw = vauth_getpw(Username, Domain);
		if (pw == NULL) {
			snprintf(StatusMessage, sizeof (StatusMessage), "%s\n", html_text[198]);
			show_login();
			vclose();
			exit(0);
		}
		/*
		 * get the real uid and gid and change to that user 
		 */
		vget_assign(Domain, RealDir, sizeof (RealDir), &Uid, &Gid);
		iwebadmin_suid(Gid, Uid);
		if (chdir(RealDir) < 0) {
			printf("<h2>%s %s</h2>\n", html_text[171], RealDir);
		}
		load_limits();
		set_admin_type();
		if (AdminType == USER_ADMIN || AdminType == DOMAIN_ADMIN) {
			auth_user_domain(ip_addr, pw);
		} else {
			auth_system(ip_addr, pw);
		}
		process_commands();

	} else
	if (pi && strncmp(pi, "/passwd/", 7) == 0) {
		char            User[MAX_BUFF];
		char           *dom;

		GetValue(TmpCGI, Username, "address=", sizeof (Username));
		GetValue(TmpCGI, Password, "oldpass=", sizeof (Password));
		GetValue(TmpCGI, Password1, "newpass1=", sizeof (Password1));
		GetValue(TmpCGI, Password2, "newpass2=", sizeof (Password2));

		if (*Username && (*Password == '\0') && (*Password1 || *Password2)) {
			/*- username entered, but no password */
			snprintf(StatusMessage, sizeof (StatusMessage), "%s", html_text[198]);
			fprintf(stderr, "iwebadmin: %s: No password\n", Username);
		} else
		if (*Username && *Password) {
			/*- attempt to authenticate user */
			vget_assign(Domain, RealDir, sizeof (RealDir), &Uid, &Gid);
			iwebadmin_suid(Gid, Uid);

			strcpy(User, Username);
			if ((dom = strchr(User, '@')) != NULL) {
				strcpy(Domain, dom + 1);
				*dom = '\0';
			}

			if (*Domain == '\0') {
				snprintf(StatusMessage, sizeof (StatusMessage), "%s", html_text[198]);
				fprintf(stderr, "iwebadmin: %s: No domain given\n", Username);
			} else {
				(void) chdir(RealDir);
				load_limits();
				if (!access(".trivial_passwords", F_OK))
					encrypt_flag = 1;
				if (!(pw = vauth_getpw(User, Domain))) {
					snprintf(StatusMessage, sizeof (StatusMessage), "%s", html_text[198]);
					fprintf(stderr, "iwebadmin: %s: No such user\n", Username);
				} else
				if (pw->pw_gid & NO_PASSWD_CHNG) {
					strcpy(StatusMessage, "You don't have permission to change your password.");
				} else
				if (auth_user(pw, Password)) {
					snprintf(StatusMessage, sizeof (StatusMessage), "%s", html_text[198]);
					fprintf(stderr, "iwebadmin: %s: incorrect password\n", Username);
				} else
				if (strcmp(Password1, Password2) != 0) {
					snprintf(StatusMessage, sizeof (StatusMessage), "%s", html_text[200]);
				} else
				if (*Password1 == '\0') {
					snprintf(StatusMessage, sizeof (StatusMessage), "%s", html_text[234]);
				} else
				if ((i = vpasswd(User, Domain, Password1, USE_POP)) != 1) {
					snprintf(StatusMessage, sizeof (StatusMessage), "%s", html_text[140]);
#ifndef TRIVIAL_PASSWORD_ENABLED
				} else
				if (strstr(User, Password1) != NULL) {
					snprintf(StatusMessage, sizeof (StatusMessage), "%s\n", html_text[320]);
#endif
				} else {
					/* success */
					snprintf(StatusMessage, sizeof (StatusMessage), "%s", html_text[139]);
					*Password = '\0';
					send_template("change_password_success.html");
					fprintf(stderr, "iwebadmin: %s: password changed\n", Username);
					return 0;
				}
			}
		}
		send_template("change_password.html");
		return 0;
	} else
	if (*rm) {
		FILE           *fs;
		char           *dom;

		GetValue(TmpCGI, Username, "username=", sizeof (Username));
		GetValue(TmpCGI, Domain, "domain=", sizeof (Domain));
		GetValue(TmpCGI, Password, "password=", sizeof (Password));
		if ((dom = strchr(Username, '@')) != NULL) {
			strcpy(Domain, dom + 1);
			*dom = '\0';
		}
		vget_assign(Domain, RealDir, sizeof (RealDir), &Uid, &Gid);
		iwebadmin_suid(Gid, Uid);
		/*- Authenticate a user and domain admin */
		if (strlen(Domain) > 0) {
			(void) chdir(RealDir);
			load_limits();
			if (!(pw = vauth_getpw(Username, Domain))) {
				snprintf(StatusMessage, sizeof (StatusMessage), "%s\n", html_text[198]);
				fprintf(stderr, "iwebadmin: %s: No such user\n", Username);
				show_login();
				vclose();
				exit(0);
			}
			if (auth_user(pw, Password)) {
				snprintf(StatusMessage, sizeof (StatusMessage), "%s", html_text[198]);
				fprintf(stderr, "iwebadmin: %s: incorrect password\n", Username);
				show_login();
				vclose();
				exit(0);
			}
			snprintf(TmpBuf, sizeof (TmpBuf), "%s/" MAILDIR, pw->pw_dir);
			del_id_files(TmpBuf);
			Mytime = time(NULL);
			snprintf(TmpBuf, sizeof (TmpBuf), "%s/" MAILDIR "/%u.qw", pw->pw_dir, (unsigned int) Mytime);
			fs = fopen(TmpBuf, "w");
			if (fs == NULL) {
				printf("%s %s<br>\n", html_text[144], TmpBuf);
				vclose();
				exit(0);
			}
			memset(TmpBuf, 0, sizeof (TmpBuf));
			/*- set session vars */
			GetValue(TmpCGI, returntext, "returntext=", sizeof (returntext));
			GetValue(TmpCGI, returnhttp, "returnhttp=", sizeof (returnhttp));
			snprinth(TmpBuf, sizeof (TmpBuf), "ip_addr=%C&returntext=%C&returnhttp=%C\n", ip_addr, returntext, returnhttp);
			fputs(TmpBuf, fs);
			fclose(fs);
			vget_assign(Domain, TmpBuf1, sizeof (TmpBuf1), &Uid, &Gid);
			set_admin_type();
			/*
			 * show the main menu for domain admins, modify user page
			 * for regular users 
			 */
			if (AdminType == DOMAIN_ADMIN) {
				show_menu(Username, Domain, Mytime);
			} else {
				strcpy(ActionUser, Username);
				moduser();
			}
			vclose();
			exit(0);
		}
	}
	show_login();
	vclose();
	return 0;
}

void
load_lang(char *lang)
{
	long            lang_size;
	char           *lang_entries;
	char           *id;
	char           *p;

	if (open_lang(lang)) {
	/*-
	 * Rare error likely caused by improper installation, should probably be 
	 * handled by regular error system, but this is a quick band-aid.
	 */
		printf("Content-Type: text/html\r\n\r\n");
		printf("<html> <head>\r\n");
		printf("<title>Failed to open lang file:%s</title>\r\n", lang);
		printf("</head>\r\n<body>\r\n");
		printf("<h1>iwebadmin error</h1>\r\n");
		printf("<p>Failed to open lang file: %s. Please check your lang directory.\r\n", lang);
		printf("</body></html>\r\n");
		exit(-1);
	}
	fseek(lang_fs, 0, SEEK_END);
	lang_size = ftell(lang_fs);
	if (!(lang_entries = malloc(lang_size + 1)))
		return;
	rewind(lang_fs);
	fread(lang_entries, 1, lang_size, lang_fs);
	lang_entries[lang_size] = 0;
	/*
	 * error handling for incomplete reads? 
	 */
	id = strtok(lang_entries, " \t");
	while (id) {
		p = strtok(NULL, "\n");
		if (p == NULL)
			break;
		html_text[atoi(id)] = p;
		id = strtok(NULL, " \t");
	}
	/*
	 * Do not free lang_entries!  html_text points into it! 
	 */
}

void
init_globals()
{
	char           *accept_lang;
	char           *langptr, *qptr;
	char           *charset;
	int             lang_err;
	int             i;
	float           maxq, thisq;

	memset(CGIValues, 0, sizeof (CGIValues));
	CGIValues['0'] = 0;
	CGIValues['1'] = 1;
	CGIValues['2'] = 2;
	CGIValues['3'] = 3;
	CGIValues['4'] = 4;
	CGIValues['5'] = 5;
	CGIValues['6'] = 6;
	CGIValues['7'] = 7;
	CGIValues['8'] = 8;
	CGIValues['9'] = 9;
	CGIValues['A'] = 10;
	CGIValues['B'] = 11;
	CGIValues['C'] = 12;
	CGIValues['D'] = 13;
	CGIValues['E'] = 14;
	CGIValues['F'] = 15;
	CGIValues['a'] = 10;
	CGIValues['b'] = 11;
	CGIValues['c'] = 12;
	CGIValues['d'] = 13;
	CGIValues['e'] = 14;
	CGIValues['f'] = 15;

	actout = stdout;
	memset(Username, 0, sizeof (Username));
	memset(Domain, 0, sizeof (Domain));
	memset(Password, 0, sizeof (Password));
	memset(Quota, 0, sizeof (Quota));
	memset(Time, 0, sizeof (Time));
	memset(ActionUser, 0, sizeof (ActionUser));
	memset(Newu, 0, sizeof (Newu));
	memset(Password1, 0, sizeof (Password1));
	memset(Password2, 0, sizeof (Password2));
	memset(Crypted, 0, sizeof (Crypted));
	memset(Alias, 0, sizeof (Alias));
	memset(Message, 0, sizeof (Message));
	memset(TmpBuf, 0, sizeof (TmpBuf));
	memset(TmpBuf1, 0, sizeof (TmpBuf1));
	memset(TmpBuf2, 0, sizeof (TmpBuf2));
	memset(TmpBuf3, 0, sizeof (TmpBuf3));
	AdminType = NO_ADMIN;
	lang_fs = NULL;

	/*
	 * Parse HTTP_ACCEPT_LANGUAGE to find highest preferred language
	 * that we have a translation for.  Example setting:
	 * de-de, ja;q=0.25, en;q=0.50, de;q=0.75
	 * The q= lines determine which is most preferred, defaults to 1.
	 * Our routine starts with en at 0.0, and then would try de-de (1.00),
	 * de (1.00), ja (0.25), en (0.50), and then de (0.75).
	 */
	/*
	 * default to English at 0.00 preference 
	 */
	maxq = 0.0;
	strcpy(Lang, "en");
	/* read in preferred languages */
	langptr = getenv("HTTP_ACCEPT_LANGUAGE");
	if (langptr != NULL) {
		accept_lang = malloc(strlen(langptr) + 1);
		strcpy(accept_lang, langptr);
		langptr = strtok(accept_lang, " ,\n");
		while (langptr != NULL) {
			qptr = strstr(langptr, ";q=");
			if (qptr != NULL) {
				*qptr = '\0';	/* convert semicolon to NULL */
				thisq = (float) atof(qptr + 3);
			} else {
				thisq = 1.0;
			}
			/*
			 * if this is a better match than our previous best, try it 
			 */
			if (thisq > maxq) {
				lang_err = open_lang(langptr);

				/*
				 * Remove this next section for strict interpretation of
				 * HTTP_ACCEPT_LANGUAGE.  It will try language xx (with the
				 * same q value) if xx-yy fails.
				 */
				if ((lang_err == -1) && (langptr[2] == '-')) {
					langptr[2] = '\0';
					lang_err = open_lang(langptr);
				}

				if (lang_err == 0) {
					maxq = thisq;
					strcpy(Lang, langptr);
				}
			}
			langptr = strtok(NULL, " ,\n");
		}

		free(accept_lang);
	}
	/*
	 * best language choice is now in 'Lang' 
	 */

	/*
	 * build table of html_text entries 
	 */
	for (i = 0; i <= MAX_LANG_STR; i++)
		html_text[i] = "";

	/*
	 * read English first as defaults for incomplete language files 
	 */
	if (strcmp(Lang, "en") != 0)
		load_lang("en");

	/*
	 * load the preferred language 
	 */
	load_lang(Lang);

	/*
	 * open the color table 
	 */
	open_colortable();
	umask(INDIMAIL_UMASK);
	charset = html_text[0];
	printf("Content-Type: text/html; charset=%s\n", *charset == '\0' ? "iso-8859-1" : charset);
#ifdef NO_CACHE
	printf("Cache-Control: no-cache\n");
	printf("Cache-Control: no-store\n");
	printf("Pragma: no-cache\n");
	printf("Expires: Thu, 01 Dec 1994 16:00:00 GMT\n");
#endif
	printf("\n");
}

void
del_id_files(char *dirname)
{
	DIR            *mydir;
	struct dirent  *mydirent;

	mydir = opendir(dirname);
	if (mydir == NULL)
		return;

	while ((mydirent = readdir(mydir)) != NULL) {
		if (strstr(mydirent->d_name, ".qw") != 0) {
			snprintf(TmpBuf3, sizeof (TmpBuf3), "%s/%s", dirname, mydirent->d_name);
			unlink(TmpBuf3);
		}
	}
	closedir(mydir);
}

void
quickAction(char *username, int action)
{
	/*
	 * This feature sponsored by PinkRoccade Public Sector, Sept 2004 
	 */
	struct stat     fileinfo;
	char            dotqmailfn[MAX_BUFF];
	char           *space, *ar, *ez;
	char           *aliasline;

	/*
	 * Note that all of the functions called from quickAction() assume
	 * that the username to modify is in a global called "ActionUser"
	 * It would be better to pass this information as a parameter, but
	 * that's how it was originally done.  The code in command.c that
	 * calls quickAction() passes ActionUser as the username parameter
	 * in hopes that someday we'll remove the globals and pass parameters.
	 */

	/*
	 * first check for alias/forward, autorepsonder (or even mailing list) 
	 */
	aliasline = valias_select(username, Domain);
	if (aliasline != NULL) {
		/*
		 * Autoresponder/Mailing List detection algorithm:
		 * We're looking for either '/autorespond ' or '/ezmlm-reject ' to
		 * appear in the first line, before a space appears
		 */
		space = strstr(aliasline, " ");
		ar = strstr(aliasline, "/autoresponder ");
		ez = strstr(aliasline, "/ezmlm-reject ");
		if (ar && space && (ar < space)) {
			/* autorepsonder */
			if (action == ACTION_MODIFY)
				modautorespond();
			else
			if (action == ACTION_DELETE)
				delautorespond();
		} else
		if (ez && space && (ez < space)) {
			/* mailing list (cdb-backend only) */
			if (action == ACTION_MODIFY)
				modmailinglist();
			else if (action == ACTION_DELETE)
				delmailinglist();
		} else {
			/*- it's just a forward/alias of some sort */
			if (action == ACTION_MODIFY)
				moddotqmail();
			else if (action == ACTION_DELETE)
				deldotqmail();
		}
	} else
	if (vauth_getpw(username, Domain)) {
		/*- POP/IMAP account */
		if (action == ACTION_MODIFY)
			moduser();
		else
		if (action == ACTION_DELETE) {
			// don't allow deletion of postmaster account
			if (strcasecmp(username, "postmaster") == 0) {
				snprinth(StatusMessage, sizeof (StatusMessage), "%s", html_text[317]);
				show_menu(Username, Domain, Mytime);
				vclose();
			} else
				deluser();
		}
	} else {
		/*
		 * check for mailing list on SQL backend (not in valias_select) 
		 */
		snprintf(dotqmailfn, sizeof (dotqmailfn), ".qmail-%s", username);
		str_replace(dotqmailfn + 7, '.', ':');
		if (stat(dotqmailfn, &fileinfo) == 0) {
			/*
			 * mailing list (MySQL backend) 
			 */
			if (action == ACTION_MODIFY)
				modmailinglist();
			else
			if (action == ACTION_DELETE)
				delmailinglist();
		} else {
		/*
		 * user does not exist 
		 */
			snprinth(StatusMessage, sizeof (StatusMessage), "%s (%H@%H)", html_text[153], username, Domain);
			show_menu(Username, Domain, Mytime);
			vclose();
		}
	}
}

void
getversion_qaiwebadmin_c()
{
	printf("%s\n", sccsidh);
}
