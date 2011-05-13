/*
 * $Id: template.c,v 1.3 2011-05-13 10:36:31+05:30 Cprogrammer Exp mbhangui $
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
#undef PACKAGE_TARNAME
#undef PACKAGE_STRING
#undef PACKAGE_VERSION
#undef PACKAGE_BUGREPORT
#undef PACKAGE_URL
#include <indimail.h>
#undef PACKAGE
#undef VERSION
#undef PACKAGE_NAME
#undef PACKAGE_TARNAME
#undef PACKAGE_STRING
#undef PACKAGE_VERSION
#undef PACKAGE_BUGREPORT
#undef PACKAGE_URL
#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <dirent.h>
#include "alias.h"
#include "autorespond.h"
#include "cgi.h"
#include "forward.h"
#include "limits.h"
#include "mailinglist.h"
#include "printh.h"
#include "qmailadmin.h"
#include "qmailadminx.h"
#include "template.h"
#include "user.h"
#include "util.h"

static char     dchar[4];
void            check_mailbox_flags(char newchar);
void            transmit_block(FILE * fs);
void            ignore_to_end_tag(FILE * fs);
void            get_calling_host();
char           *get_session_val(char *session_var);

/*
 * send an html template to the browser 
 */
int
send_template(char *actualfile)
{
	send_template_now("header.html");
	send_template_now(actualfile);
	send_template_now("footer.html");
	return 0;
}

int
send_template_now(char *filename)
{
	FILE           *fs;
	int             i;
	int             j;
	int             inchar;
	char           *tmpstr;
	struct stat     mystat;
	char            qconvert[11];
	char           *qnote = " MB";
	struct passwd  *vpw;
	char            value[MAX_BUFF];
	char            value2[MAX_BUFF];

	if (strstr(filename, "/") != NULL || strstr(filename, "..") != NULL) {
		printf("warning: invalid file name %s\n", filename);
		return (-1);
	}


	tmpstr = getenv(QMAILADMIN_TEMPLATEDIR);
	if (tmpstr == NULL)
		tmpstr = HTMLLIBDIR;
	snprintf(TmpBuf2, (sizeof (TmpBuf2) - 1), "%s/html/%s", tmpstr, filename);

	if (lstat(TmpBuf2, &mystat) == -1) {
		printf("Warning: cannot lstat '%s', check permissions.<BR>\n", TmpBuf2);
		return (-1);
	}
	if (S_ISLNK(mystat.st_mode)) {
		printf("Warning: '%s' is a symbolic link.<BR>\n", TmpBuf2);
		return (-1);
	}

/*
 * open the template 
 */
	fs = fopen(TmpBuf2, "r");
	if (fs == NULL) {
		printf("%s %s<br>\n", html_text[144], TmpBuf2);
		return 0;
	}

/*
 * parse the template looking for "##" pattern 
 */
	while ((inchar = fgetc(fs)) >= 0) {
	/*
	 * if not '#' then send it 
	 */
		if (inchar != '#') {
			fputc(inchar, actout);

		/*
		 * found a '#' 
		 */
		} else {
		/*
		 * look for a second '#' 
		 */
			inchar = fgetc(fs);
			if (inchar < 0) {
				break;

			/*
			 * found a tag 
			 */
			} else if (inchar == '#') {

				inchar = fgetc(fs);
				if (inchar < 0)
					break;

			/*
			 * switch on the tag 
			 */
				switch (inchar) {
				/*
				 * send stock (user, dom, time) cgi parameters 
				 */
				case '&':
					printh("user=%C&dom=%C&time=%d&", Username, Domain, Mytime);
					break;

				case '~':
					printf("%s", Lang);
					break;

				/*
				 * send the action user parameter 
				 */
				case 'A':
					printh("%H", ActionUser);
					break;

				/*
				 * send the Alias parameter 
				 */
				case 'a':
					printh("%H", Alias);
					break;

				/*
				 * show number of pop accounts 
				 */
				case 'B':
					load_limits();
					count_users();
					if (MaxPopAccounts > -1) {
						printf("%d/%d", CurPopAccounts, MaxPopAccounts);
					} else {
						printf("%d/%s", CurPopAccounts, html_text[229]);
					}
					break;

				/*
				 * show the lines inside a alias table (not used, see ##d) 
				 */
				case 'b':
					show_dotqmail_lines(Username, Domain, Mytime);
					break;

				/*
				 * send the CGIPATH parameter 
				 */
				case 'C':
					printf("%s", CGIPATH);
					break;

				/*
				 * show the lines inside a mailing list table 
				 */
				case 'c':
					show_mailing_list_line2(Username, Domain, Mytime, RealDir);
					break;

				/*
				 * send the domain parameter 
				 */
				case 'D':
					printh("%H", Domain);
					break;

				/*
				 * show the lines inside a forward table 
				 */
				case 'd':
					show_dotqmail_lines(Username, Domain, Mytime);
					break;

				/*
				 * this will be used to parse mod_mailinglist-idx.html 
				 */
				case 'E':
					show_current_list_values();
					break;

				/*
				 * show the lines inside a mailing list table 
				 */
				case 'e':
					show_mailing_list_line(Username, Domain, Mytime, RealDir);
					break;

				/*
				 * display a file (used for mod_autorespond ONLY) 
				 */
				/*
				 * this code should be moved to a function in autorespond.c 
				 */
				case 'F':
				{
					FILE           *fs;
					char           *alias_line;

					alias_line = valias_select(ActionUser, Domain);
				/*
				 * should verify here that alias_line contains "/autorespond " 
				 */

					if ((alias_line = valias_select(ActionUser, Domain))) {
						strcpy(TmpBuf2, alias_line);

					/*
					 * See if it's a Maildir path rather than address 
					 */
						i = strlen(TmpBuf2) - 1;
						if (TmpBuf2[i] == '/') {
							--i;
							for (; TmpBuf2[i] != '/'; --i);
							--i;
							for (; TmpBuf2[i] != '/'; --i);
							for (++i, j = 0; TmpBuf2[i] != '/'; ++j, ++i) {
								TmpBuf3[j] = TmpBuf2[i];
							}
							TmpBuf3[j] = '\0';
							printh("value=\"%H@%H\"></td>\n", TmpBuf3, Domain);
						} else {
							printh("value=\"%H\"></td>\n", &TmpBuf2[1]);
						}
					}
					upperit(ActionUser);
					sprintf(TmpBuf, "%s/message", ActionUser);

					if ((fs = fopen(TmpBuf, "r")) == NULL)
						ack("150", TmpBuf);

					fgets(TmpBuf2, sizeof (TmpBuf2), fs);
					fgets(TmpBuf2, sizeof (TmpBuf2), fs);
					printf("         <td>&nbsp;</td>\n");
					printf("         </tr>\n");
					printf("         <tr>\n");
					printf("         <td align=right><b>%s</b></td>\n", html_text[6]);

				/*
				 * take off newline 
				 */
					i = strlen(TmpBuf2);
					--i;
					TmpBuf2[i] = 0;
					printh("         <td><input type=\"text\" size=40 name=\"alias\" maxlength=128 value=\"%H\"></td>\n",
						   &TmpBuf2[9]);
					printf("         <td>&nbsp;</td>\n");
					printf("        </tr>\n");
					printf("       </table>\n");
					printf("       <textarea cols=80 rows=40 name=\"message\">");
					fgets(TmpBuf2, sizeof (TmpBuf2), fs);
					while (fgets(TmpBuf2, sizeof (TmpBuf2), fs)) {
						printf("%s", TmpBuf2);
					}
					printf("</textarea>");
					fclose(fs);
				}
					break;

				/*
				 * show the forwards 
				 */
				case 'f':
					if (AdminType == DOMAIN_ADMIN) {
						show_forwards(Username, Domain, Mytime);
					}
					break;

				/*
				 * show the mailing list digest subscribers 
				 */
				case 'G':
					if (AdminType == DOMAIN_ADMIN) {
						show_list_group_now(2);
					}
					break;

				/*
				 * show the lines inside a autorespond table 
				 */
				case 'g':
					show_autorespond_line(Username, Domain, Mytime, RealDir);
					break;

				/*
				 * show returnhttp (from TmpCGI) 
				 */
				case 'H':
					GetValue(TmpCGI, value, "returnhttp=", sizeof (value));
					printh("%C", value);
					break;

				/*
				 * show the counts 
				 */
				case 'h':
					show_counts();
					break;

				case 'I':
					show_dotqmail_file(ActionUser);
					break;

				/*
				 * check for user forward and forward/store vacation 
				 */
				case 'i':
					parse_users_dotqmail(fgetc(fs));
					break;

				/*
				 * show mailbox flag status 
				 */
				/*
				 * James Raftery <james@now.ie> 12 Dec. 2002 
				 */
				case 'J':
					check_mailbox_flags(fgetc(fs));
					break;

				/*
				 * show number of mailing lists 
				 */
				/*
				 * Added by <mbowe@pipeline.com.au> 5th June 2003 
				 */
				case 'j':
					load_limits();
					count_mailinglists();
					if (MaxMailingLists > -1) {
						printf("%d/%d", CurMailingLists, MaxMailingLists);
					} else {
						printf("%d/%s", CurMailingLists, html_text[229]);
					}
					break;

				/*
				 * show number of forwards 
				 */
				/*
				 * Added by <mbowe@pipeline.com.au> 5th June 2003 
				 */
				case 'k':
					load_limits();
					count_forwards();
					if (MaxForwards > -1) {
						printf("%d/%d", CurForwards, MaxForwards);
					} else {
						printf("%d/%s", CurForwards, html_text[229]);
					}
					break;
				/*
				 * show number of autoresponders 
				 */
				/*
				 * Added by <mbowe@pipeline.com.au> 5th June 2003 
				 */
				case 'K':
					load_limits();
					count_autoresponders();
					if (MaxAutoResponders > -1) {
						printf("%d/%d", CurAutoResponders, MaxAutoResponders);
					} else {
						printf("%d/%s", CurAutoResponders, html_text[229]);
					}
					break;

				/*
				 * show the aliases stuff 
				 */
				case 'l':
					if (AdminType == DOMAIN_ADMIN) {
						show_aliases();
					}
					break;

				/*
				 * login username 
				 */
				case 'L':
					if (strlen(Username) > 0) {
						printh("%H", Username);
					} else if (TmpCGI && GetValue(TmpCGI, value, "user=", sizeof (value)) == 0) {
						printh("%H", value);
					} else
						printf("postmaster");
					break;

				/*
				 * show the mailing list subscribers 
				 */
				case 'M':
					if (AdminType == DOMAIN_ADMIN) {
						show_list_group_now(0);
					}
					break;

				/*
				 * show the mailing lists 
				 */
				case 'm':
					if (AdminType == DOMAIN_ADMIN) {
						show_mailing_lists(Username, Domain, Mytime);
					}
					break;

				/*
				 * parse include files 
				 */
				case 'N':
					i = 0;
					TmpBuf[i] = fgetc(fs);
					if (TmpBuf[i] == '/') {
						printf("%s", html_text[144]);
					} else {
						for (; TmpBuf[i] != '\0' && TmpBuf[i] != '#' && i < sizeof (TmpBuf) - 1;) {
							TmpBuf[++i] = fgetc(fs);
						}
						TmpBuf[i] = '\0';
						if ((strstr(TmpBuf, "../")) != NULL) {
							printf("%s: %s", html_text[144], TmpBuf);
						} else if ((strcmp(TmpBuf, filename)) != 0) {
							send_template_now(TmpBuf);
						}
					}
					break;

				/*
				 * show returntext (from TmpCGI) 
				 */
				case 'n':
					GetValue(TmpCGI, value, "returntext=", sizeof (value));
					printh("%H", value);
					break;

				/*
				 * build a pulldown menu of all POP/IMAP users 
				 */
				case 'O':
				{
					struct passwd  *pw;

					pw = vauth_getall(Domain, 1, 1);
					while (pw != NULL) {
						printh("<option value=\"%H\">%H</option>\n", pw->pw_name, pw->pw_name);
						pw = vauth_getall(Domain, 0, 0);
					}
				}
					break;

				/*
				 * show the mailing list moderators 
				 */
				case 'o':
					if (AdminType == DOMAIN_ADMIN) {
						show_list_group_now(1);
					}
					break;

				/*
				 * display mailing list prefix 
				 */
				case 'P':
				{
					TmpBuf3[0] = '\0';
					get_mailinglist_prefix(TmpBuf3);
					printh("%H", TmpBuf3);
				}
					break;

				/*
				 * show POP/IMAP users 
				 */
				case 'p':
					show_user_lines(Username, Domain, Mytime, RealDir);
					break;

				/*
				 * show quota usage 
				 */
				case 'Q':
					vpw = vauth_getpw(ActionUser, Domain);
					if (strncmp(vpw->pw_shell, "NOQUOTA", 2) != 0) {
						long            diskquota = 0;
						int             maxmsg = 0;
						char            path[256];

						quota_to_megabytes(qconvert, vpw->pw_shell);
						snprintf(path, sizeof (path), "%s/" MAILDIR, vpw->pw_dir);
						diskquota = check_quota(path, (mdir_t *) &maxmsg);
						printf("%-2.2lf /", ((double) diskquota) / 1048576.0);	/* Convert to MB */
					}
					break;

				/*
				 * display user's quota (mod user page) 
				 */
				case 'q':
					vpw = vauth_getpw(ActionUser, Domain);
					if (strncmp(vpw->pw_shell, "NOQUOTA", 2) != 0) {
						quota_to_megabytes(qconvert, vpw->pw_shell);
						printf("%s", qconvert);
					} else {
						if (AdminType == DOMAIN_ADMIN)
							printf("NOQUOTA");
						else
							printf("%s", html_text[229]);
					}
					break;

				/*
				 * show returntext/returnhttp if set in CGI vars 
				 */
				case 'R':
					GetValue(TmpCGI, value, "returntext=", sizeof (value));
					GetValue(TmpCGI, value2, "returnhttp=", sizeof (value2));
					if (*value != '\0') {
						printh("<A HREF=\"%C\">%H</A>", value2, value);
					}
					break;

				/*
				 * show the autoresponder stuff 
				 */
				case 'r':
					if (AdminType == DOMAIN_ADMIN) {
						show_autoresponders(Username, Domain, Mytime);
					}
					break;

				/*
				 * send the status message parameter 
				 */
				case 'S':
					printf("%s", StatusMessage);
					break;

				/*
				 * show the catchall name 
				 */
				case 's':
					get_catchall();
					break;

				/*
				 * send the time parameter 
				 */
				case 'T':
					printf("%u", (unsigned int) Mytime);
					break;

				/*
				 * transmit block? 
				 */
				case 't':
					transmit_block(fs);
					break;

				/*
				 * send the username parameter 
				 */
				case 'U':
					printh("%H", Username);
					break;

				/*
				 * show the users 
				 */
				case 'u':
					show_users(Username, Domain, Mytime);
					break;

				/*
				 * show version number 
				 */
				case 'V':
					printf("<a href=\"http://sourceforge.net/projects/indimail/\">%s</a> %s<BR>", PACKAGE, VERSION);
					printf("<a href=\"http://www.indimail.org/\">%s</a> %s<BR>", PACKAGE, VERSION);
					break;

				/*
				 * display the main menu 
				 */
				/*
				 * move this to a function... 
				 */
				case 'v':
					printh("<font size=\"2\" color=\"#000000\"><b>%H</b></font><br><br>", Domain);
					printf("<font size=\"2\" color=\"#ff0000\"><b>%s</b></font><br>", html_text[1]);
					if (AdminType == DOMAIN_ADMIN) {

						if (MaxPopAccounts != 0) {
							printh("<a href=\"%s\">", cgiurl("showusers"));
							printf("<font size=\"2\" color=\"#000000\"><b>%s</b></font></a><br>", html_text[61]);
						}

						if (MaxForwards != 0 || MaxAliases != 0) {
							printh("<a href=\"%s\">", cgiurl("showforwards"));
							printf("<font size=\"2\" color=\"#000000\"><b>%s</b></font></a><br>", html_text[122]);
						}

						if (MaxAutoResponders != 0) {
							printh("<a href=\"%s\">", cgiurl("showautoresponders"));
							printf("<font size=\"2\" color=\"#000000\"><b>%s</b></a></font><br>", html_text[77]);
						}

						if (*EZMLMDIR != 'n' && MaxMailingLists != 0) {
							printh("<a href=\"%s\">", cgiurl("showmailinglists"));
							printf("<font size=\"2\" color=\"#000000\"><b>%s</b></font></a><br>", html_text[80]);
						}
					} else {
					/*
					 * the quota code in here is kinda screwy and could use review
					 * * then again, with recent changes, the non-admin shouldn't
					 * * even get to this page.
					 */
						long            diskquota = 0;
						int             maxmsg = 0;
						char            path[256];
						vpw = vauth_getpw(Username, Domain);

						printh("<a href=\"%s&moduser=%C\">", cgiurl("moduser"), Username);
						printh("<font size=\"2\" color=\"#000000\"><b>%s %H</b></font></a><br><br>", html_text[111], Username);
						if (strncmp(vpw->pw_shell, "NOQUOTA", 2) != 0) {
							quota_to_megabytes(qconvert, vpw->pw_shell);
						} else {
							sprintf(qconvert, "%s", html_text[229]);
							qnote = "";
						}
						printf("<font size=\"2\" color=\"#000000\"><b>%s:</b><br>%s %s %s", html_text[249], html_text[253],
							   qconvert, qnote);
						printf("<br>%s ", html_text[254]);
						snprintf(path, sizeof (path), "%s/" MAILDIR, vpw->pw_dir);
						diskquota = check_quota(path, (mdir_t *) &maxmsg);
						printf("%-2.2lf MB</font><br>", ((double) diskquota) / 1048576.0);	/* Convert to MB */
					}

					if (AdminType == DOMAIN_ADMIN) {
						printf("<br>");

						if (MaxPopAccounts != 0) {
							printh("<a href=\"%s\">", cgiurl("adduser"));
							printf("<font size=\"2\" color=\"#000000\"><b>%s</b></font></a><br>", html_text[125]);
						}

						if (MaxForwards != 0) {
							printh("<a href=\"%s\">", cgiurl("adddotqmail"));
							printf("<font size=\"2\" color=\"#000000\"><b>%s</b></font></a><br>", html_text[127]);
						}

						if (MaxAutoResponders != 0) {
							printh("<a href=\"%s\">", cgiurl("addautorespond"));
							printf("<font size=\"2\" color=\"#000000\"><b>%s</b></a></font><br>", html_text[128]);
						}

						if (*EZMLMDIR != 'n' && MaxMailingLists != 0) {
							printh("<a href=\"%s\">", cgiurl("addmailinglist"));
							printf("<font size=\"2\" color=\"#000000\"><b>%s</b></font></a><br>", html_text[129]);
						}
					}
					break;

				/*
				 * Password 
				 */
				case 'W':
					printh("%H", Password);
					break;

				/*
				 * dictionary entry, followed by three more chars for the entry # 
				 */
				case 'X':
					for (i = 0; i < 3; ++i)
						dchar[i] = fgetc(fs);
					dchar[i] = 0;
					i = atoi(dchar);
					if ((i >= 0) && (i <= MAX_LANG_STR))
						printf("%s", html_text[atoi(dchar)]);
					break;

				/*
				 * exit / logout link/text 
				 */
				case 'x':
					strcpy(value, get_session_val("returntext="));
					if (strlen(value) > 0) {
						printh("<a href=\"%C\">%H", get_session_val("returnhttp="), value);
					} else {
						printh("<a href=\"%s\">%s", cgiurl("logout"), html_text[218]);
					}
					printf("</a>\n");
					break;

				/*
				 * returnhttp 
				 */
				case 'y':
					printh("%C", get_session_val("returnhttp="));
					break;

				/*
				 * returntext 
				 */
				case 'Y':
					printh("%H", get_session_val("returntext="));
					break;

				/*
				 * send the image URL directory 
				 */
				case 'Z':
					printf("%s", IMAGEURL);
					break;

				/*
				 * display domain on login page (last used, value of dom in URL,
				 * * or guess from hostname in URL).
				 */
				case 'z':
					if (strlen(Domain) > 0) {
						printh("%H", Domain);
					} else if (TmpCGI && GetValue(TmpCGI, value, "dom=", sizeof (value)) == 0) {
						printh("%H", value);
#ifdef DOMAIN_AUTOFILL
					} else {
						get_calling_host();
#endif
					}
					break;

				default:
					break;
				}
			/*
			 * didn't find a tag, so send out the first '#' and 
			 * the current character
			 * 
			 */
			} else {
				fputc('#', actout);
				fputc(inchar, actout);
			}
		}
	}
	fclose(fs);
	fflush(actout);
	vclose();

	return 0;
}

/*
 * Display status of mailbox flags 
 */
/*
 * James Raftery <james@now.ie> 12 Dec. 2002 / 15 Apr. 2003 
 */
void
check_mailbox_flags(char newchar)
{
	static struct passwd *vpw = NULL;

	if (vpw == NULL)
		vpw = vauth_getpw(ActionUser, Domain);

	switch (newchar) {

	/*
	 * 1: "checked" if V_USER0 is set 
	 */
	/*
	 * 2: "checked" if V_USER0 is unset 
	 */
	case '1':
		if (vpw->pw_gid & V_USER0)
			printf("checked");
		break;

	case '2':
		if (!(vpw->pw_gid & V_USER0))
			printf("checked");
		break;

	/*
	 * 3: "checked" if V_USER1 is set 
	 */
	/*
	 * 4: "checked" if V_USER1 is unset 
	 */
	case '3':
		if (vpw->pw_gid & V_USER1)
			printf("checked");
		break;

	case '4':
		if (!(vpw->pw_gid & V_USER1))
			printf("checked");
		break;

	/*
	 * 5: "checked" if V_USER2 is set 
	 */
	/*
	 * 6: "checked" if V_USER2 is unset 
	 */
	case '5':
		if (vpw->pw_gid & V_USER2)
			printf("checked");
		break;

	case '6':
		if (!(vpw->pw_gid & V_USER2))
			printf("checked");
		break;

	/*
	 * 7: "checked" if V_USER3 is set 
	 */
	/*
	 * 8: "checked" if V_USER3 is unset 
	 */
	case '7':
		if (vpw->pw_gid & V_USER3)
			printf("checked");
		break;

	case '8':
		if (!(vpw->pw_gid & V_USER3))
			printf("checked");
		break;

	default:
		break;
	}

	return;
}

void
transmit_block(FILE * fs)
{
/*
 * tests to see if text between ##tX and ##tt should be transmitted 
 */
/*
 * where X is a letter corresponding to one of the below values     
 */
/*
 * jeff.hedlund@matrixsi.com                                        
 */
	char            inchar;

	inchar = fgetc(fs);
	switch (inchar) {
	case 'a':
	/*
	 * administrative commands 
	 */
		if (AdminType != DOMAIN_ADMIN) {
			ignore_to_end_tag(fs);
		}
		break;
	case 'h':
#ifndef HELP
		ignore_to_end_tag(fs);
#endif
		break;
	case 'm':
#ifndef ENABLE_MYSQL
		ignore_to_end_tag(fs);
#endif
		break;
	case 'q':
#ifndef MODIFY_QUOTA
		ignore_to_end_tag(fs);
#endif
		break;
	case 's':
#ifndef MODIFY_SPAM
		ignore_to_end_tag(fs);
#endif
		break;
	case 't':
	/*
	 * explicitly break if it was ##tt, that's an end tag 
	 */
		break;
	case 'u':
	/*
	 * user (not administrative) 
	 */
		if (AdminType != USER_ADMIN) {
			ignore_to_end_tag(fs);
		}
		break;
	}
}

void
ignore_to_end_tag(FILE * fs)
{
/*
 * simply looks for the ending tag (##tt) ignoring everything else 
 */
/*
 * jeff.hedlund@matrixsi.com                                       
 */
	int             nested = 0;
	int             inchar;

	inchar = fgetc(fs);
	while (inchar >= 0) {
		if (inchar == '#') {
			inchar = fgetc(fs);
			if (inchar == '#') {
				inchar = fgetc(fs);
				if (inchar == 't') {
					inchar = fgetc(fs);
					if (inchar == 't' && nested == 0)
						return;
					else if (inchar != 't')
						nested++;
					else
						nested--;
				}
			}
		}
		inchar = fgetc(fs);
	}
}

void
get_calling_host()
{
/*
 * printout the virtual host that matches the HTTP_HOST 
 */
	char           *srvnam;
	char           *domptr;
	int             count = 0;
	int             l;
	FILE           *fs;
	char            dombuf[255];

	sprintf(dombuf, "%s/control/virtualdomains", QMAILDIR);

	fs = fopen(dombuf, "r");
	if (fs != NULL) {
		if ((srvnam = getenv("HTTP_HOST")) != NULL) {
			lowerit(srvnam);
			while (fgets(TmpBuf, sizeof (TmpBuf), fs) != NULL && count == 0) {
			/*
			 * strip off newline 
			 */
				l = strlen(TmpBuf);
				l--;
				TmpBuf[l] = 0;
			/*
			 * virtualdomains format:  "domain:domain", get to second one 
			 */
				domptr = TmpBuf;
				for (; *domptr != 0 && *domptr != ':'; domptr++);
				domptr++;
				lowerit(domptr);
				if (strstr(srvnam, domptr) != NULL) {
					printh("%H", domptr);
					count = 1;
				}
			}
			fclose(fs);
		}
	}
}

char           *
get_session_val(char *session_var)
{
/*
 * returns the value of session_var, first checking the .qw file for saved 
 */
/*
 * value, or the TmpCGI if it's not yet been saved                         
 */
	static char     value[MAX_BUFF];
	char            dir[MAX_BUFF];
	char           *retval;
	FILE           *fs_qw;
	struct passwd  *vpw;
	memset(value, 0, sizeof (value));
	memset(dir, 0, sizeof (dir));
	retval = "";
	if ((vpw = vauth_getpw(Username, Domain)) != NULL) {
		sprintf(dir, "%s/" MAILDIR "/%u.qw", vpw->pw_dir, (unsigned int) Mytime);
		fs_qw = fopen(dir, "r");
		if (fs_qw != NULL) {
			memset(TmpBuf, 0, sizeof (TmpBuf));
			if (fgets(TmpBuf, sizeof (TmpBuf), fs_qw) != NULL) {
				if (GetValue(TmpBuf, value, session_var, sizeof (value)) == 0)
					retval = value;
			}
			fclose(fs_qw);
		}
	} else if (TmpCGI) {
		if (GetValue(TmpCGI, value, session_var, sizeof (value)) == 0)
			retval = value;
	}
	return (retval);
}

void
getversion_qatemplate_c()
{
	printf("%s\n", sccsidh);
}
