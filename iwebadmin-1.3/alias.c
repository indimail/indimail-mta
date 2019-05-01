/*
 * $Id: alias.c,v 1.4 2019-05-01 23:21:06+05:30 Cprogrammer Exp mbhangui $
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
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <errno.h>
#include <dirent.h>
#include "alias.h"
#include "forward.h"
#include "iwebadmin.h"
#include "iwebadminx.h"
#include "dotqmail.h"
#include "html.h"
#include "limits.h"
#include "util.h"
#include "printh.h"
#include "show.h"
#include "template.h"

char           *dotqmail_alias_command(char *command);

int
show_aliases(void)
{
	if (AdminType != DOMAIN_ADMIN) {
		snprintf(StatusMessage, sizeof (StatusMessage), "%s", html_text[142]);
		vclose();
		exit(0);
	}
	send_template("show_alias.html");
	return 0;
}

struct aliasentry {
	char            alias_name[MAX_FILE_NAME];
	char            alias_command[MAX_BUFF];
	struct aliasentry *next;
};

struct aliasentry *firstalias = NULL, *curalias = NULL;

void
add_alias_entry(char *alias_name, char *alias_command)
{
	if (firstalias == NULL) {
		firstalias = malloc(sizeof (struct aliasentry));
		curalias = firstalias;
	} else {
		curalias->next = malloc(sizeof (struct aliasentry));
		curalias = curalias->next;
	}
	curalias->next = NULL;
	strcpy(curalias->alias_name, alias_name);
	strcpy(curalias->alias_command, alias_command);
}

struct aliasentry *
get_alias_entry()
{
	struct aliasentry *temp;

	temp = curalias->next;
	free(curalias);
	return temp;
}

void
show_dotqmail_lines(char *user, char *dom, time_t mytime)
{
	int             moreusers = 0;
	char            alias_user[MAX_BUFF];
	char            alias_copy[MAX_BUFF];
	char            alias_name[MAX_FILE_NAME];
	char           *alias_domain;
	char           *alias_name_from_command;
	int             stop, k, startnumber;
	int             page;
	char            this_alias[MAX_FILE_NAME];
#ifdef VALIAS
	char           *alias_line;
#else
	DIR            *mydir;
	struct dirent  *mydirent;
	FILE           *fs;
	int             i, j, m, n;
	struct dirent **namelist;
	struct stat     sbuf;
#endif

	if (AdminType != DOMAIN_ADMIN) {
		snprintf(StatusMessage, sizeof (StatusMessage), "%s", html_text[142]);
		vclose();
		exit(0);
	}

	page = atoi(Pagenumber);
	if (page == 0)
		page = 1;

	startnumber = MAXALIASESPERPAGE * (page - 1);
	k = 0;

#ifdef VALIAS
	if (*SearchUser) {
		startnumber = 0;
		alias_line = valias_select_all(alias_name, Domain, MAX_BUFF);
		while (alias_line != NULL) {
			strcpy(this_alias, alias_name);
			alias_name_from_command = dotqmail_alias_command(alias_line);
			if (alias_name_from_command != NULL || *alias_line == '#') {
				if (strcasecmp(SearchUser, alias_name) <= 0)
					break;
				startnumber++;
			}
		/*
		 * burn through remaining lines for this alias, if necessary 
		 */
			while ((alias_line != NULL) && (strcmp(this_alias, alias_name) == 0)) {
				alias_line = valias_select_all(alias_name, Domain, MAX_BUFF);
			}
		}
		page = startnumber / MAXALIASESPERPAGE + 1;
		sprintf(Pagenumber, "%d", page);
	}

	alias_line = valias_select_all(alias_name, Domain, MAX_BUFF);
	while (alias_line != NULL) {
		strcpy(this_alias, alias_name);
		alias_name_from_command = dotqmail_alias_command(alias_line);
		if (alias_name_from_command != NULL || *alias_line == '#') {
			k++;

			if (k > MAXALIASESPERPAGE + startnumber) {
				moreusers = 1;
				break;
			}
			if (k > startnumber) {
				if (*alias_line == '#') {
					alias_line = valias_select_all(alias_name, Domain, MAX_BUFF);
					if (strcmp(this_alias, alias_name) != 0) {
					/*
					 * single comment, treat as blackhole 
					 */
						add_alias_entry(this_alias, "#");
						continue;
					} else {
						alias_name_from_command = dotqmail_alias_command(alias_line);
					}
				}
				while (1) {
					if (alias_name_from_command != NULL) {
						add_alias_entry(alias_name, alias_name_from_command);
					}
					alias_line = valias_select_all(alias_name, Domain, MAX_BUFF);

				/*
				 * exit if we run out of alias lines, or go to a new alias name 
				 */
					if ((alias_line == NULL) || (strcmp(this_alias, alias_name) != 0))
						break;

					alias_name_from_command = dotqmail_alias_command(alias_line);
				}
			}
		}
	/*
	 * burn through remaining lines for this alias, if necessary 
	 */
		while ((alias_line != NULL) && (strcmp(this_alias, alias_name) == 0)) {
			alias_line = valias_select_all(alias_name, Domain, MAX_BUFF);
		}
	}
#endif
	curalias = firstalias;
	while (curalias != NULL) {
		strcpy(this_alias, curalias->alias_name);
	/*
	 * display the entry 
	 *
	 * We assume that if first char is '#', this is a blackhole.
	 * * This is a big assumption, and may cause problems at some point.
	 */
		printf("%s", HTML_ALIAS_ROW_START);
		qmail_button(this_alias, "deldotqmail", user, dom, mytime, "trash.png");
		if (*curalias->alias_command == '#')
			printf("%s", HTML_EMPTY_TD);	/* don't allow modify on blackhole */
		else
			qmail_button(this_alias, "moddotqmail", user, dom, mytime, "modify.png");
		printh(HTML_ALIAS_NAME, this_alias);
		printf(HTML_ALIAS_DEST_START);

		stop = 0;
		if (*curalias->alias_command == '#') {
		/*
		 * this is a blackhole account 
		 */
			printf(HTML_ALIAS_BLACKHOLE, html_text[303]);
			stop = 1;
		}
		while (!stop) {
			strcpy(alias_copy, curalias->alias_command);
		/*
		 * get the domain alone from alias_copy 
		 */
			for (alias_domain = alias_copy; *alias_domain != '\0' && *alias_domain != '@' && *alias_domain != ' '; alias_domain++);

		/*
		 * if a local user, strip domain name from address 
		 */
			if ((*alias_domain == '@') && (strcasecmp(alias_domain + 1, Domain) == 0)) {
			/*
			 * strip domain name from address 
			 */
				*alias_domain = '\0';

				if (!check_local_user(alias_copy)) {
				/*
				 * make it red so it jumps out -- this is no longer a valid forward 
				 */
					snprinth(alias_user, sizeof (alias_user), HTML_ALIAS_INVALID, curalias->alias_command);
				} else {
					snprinth(alias_user, sizeof (alias_user), HTML_ALIAS_LOCAL, alias_copy);
				}
			} else {
				snprinth(alias_user, sizeof (alias_user), HTML_ALIAS_REMOTE, curalias->alias_command);
			}

		/*
		 * find next entry, so we know if we should print a , or not 
		 */
			while (1) {
				curalias = get_alias_entry();

			/*
			 * exit if we run out of alias lines, or go to a new alias name 
			 */
				if ((curalias == NULL) || (strcmp(this_alias, curalias->alias_name) != 0)) {
					stop = 1;
					printf("%s", alias_user);
					break;
				}

				printf("%s, ", alias_user);
				break;
			}
		}
	/*
	 * burn through any remaining entries 
	 */
		while ((curalias != NULL) && (strcmp(this_alias, curalias->alias_name) == 0)) {
			curalias = get_alias_entry();
		}
		printf("%s%s", HTML_ALIAS_DEST_END, HTML_ALIAS_ROW_END);
	}

	if (AdminType == DOMAIN_ADMIN) {

		print_user_index("showforwards", 4, user, dom, mytime);

		printf("%s", HTML_ALIAS_FOOTER_START);
		printf("%s", HTML_MENU_START);
	/*
	 * When searching for a user on systems using .qmail files, we make things
	 * easy by starting the page with the first matching address.  As a result,
	 * the previous page will be 'page' and not 'page-1'.  Refresh is accomplished
	 * by repeating the search.
	 */
		if (*SearchUser && ((startnumber % MAXALIASESPERPAGE) != 1)) {
		// previous page
			printh(HTML_ALIAS_SHOWPAGE, cgiurl("showforwards"), page, html_text[135]);
			printf("%s", HTML_MENU_SEP);
		// refresh
			printh(HTML_ALIAS_DOSEARCH, cgiurl("showforwards"), SearchUser, html_text[136]);
		} else {
			if (page > 1) {
			// previous page
				printh(HTML_ALIAS_SHOWPAGE, cgiurl("showforwards"), page - 1, html_text[135]);
				printf("%s", HTML_MENU_SEP);
			}
		// refresh
			printh(HTML_ALIAS_SHOWPAGE, cgiurl("showforwards"), page, html_text[136]);
		}
		if (moreusers) {
			printf("%s", HTML_MENU_SEP);
		// next page
			printh(HTML_ALIAS_SHOWPAGE, cgiurl("showforwards"), page + 1, html_text[137]);
		}
		printf("%s", HTML_MENU_END);
		printf("%s", HTML_ALIAS_FOOTER_END);
	}
}

/*
 * This Function shows the inside of a .qmail file, 
 * with the edit mode
 *
 */
void
show_dotqmail_file(char *user)
{
	char            alias_user[MAX_BUFF];
	char           *alias_domain;
	char           *alias_name_from_command;
	char           *alias_line;
	int             firstrow;
	int             j;

	if (AdminType != DOMAIN_ADMIN) {
		snprintf(StatusMessage, sizeof (StatusMessage), "%s", html_text[142]);
		vclose();
		exit(0);
	}
	for(;;)
	{
		if (!(alias_line = valias_select(user, Domain)))
			break;
		alias_name_from_command = dotqmail_alias_command(alias_line);
		/*
		 * Make sure it is valid before displaying it. 
		 */
		if (alias_name_from_command != NULL)
			add_alias_entry(user, alias_line);
	}

	curalias = firstalias;
	firstrow = 1;
	while (curalias != NULL) {
		alias_line = curalias->alias_command;
		alias_name_from_command = dotqmail_alias_command(alias_line);
		strcpy(alias_user, alias_name_from_command);
	/*
	 * get the domain alone from alias_user 
	 */
		alias_domain = alias_user;
		for (; *alias_domain != '\0' && *alias_domain != '@' && *alias_domain != ' '; alias_domain++);
		alias_domain++;
		if (strcmp(alias_domain, Domain) == 0) {
		/*
		 * if a local user, exclude the domain 
		 */
			strcpy(TmpBuf3, alias_user);
			for (j = 0; TmpBuf3[j] != 0 && TmpBuf3[j] != '@'; j++);
			TmpBuf3[j] = 0;
			if (check_local_user(TmpBuf3)) {
				snprinth(alias_user, sizeof (alias_user), HTML_ALIAS_LOCAL, TmpBuf3);
			} else {
			/*
			 * make it red so it jumps out -- this is no longer a valid forward 
			 */
				snprinth(alias_user, sizeof (alias_user), HTML_ALIAS_INVALID, alias_name_from_command);
			}
		} else {
			snprinth(alias_user, sizeof (alias_user), HTML_ALIAS_REMOTE, alias_name_from_command);
		}
		printf("%s", HTML_ALIAS_MOD_ROW_START);
		if (firstrow) {
			firstrow = 0;
			printh(HTML_ALIAS_MOD_NAME, user);
		} else {
			printh(HTML_ALIAS_MOD_NAME, "");
		}
		printf(HTML_ALIAS_MOD_DEST, alias_user);
		printh(HTML_ALIAS_MOD_DELETE, cgiurl("moddotqmailnow"), user, alias_line);

		printf("%s", HTML_ALIAS_MOD_ROW_END);
		curalias = get_alias_entry();
	}
}

int
onevalidonly(char *user)
{
	char           *alias_line;
	int             lines;

	lines = 0;
	for (;;)
	{
		if (!(alias_line = valias_select(user, Domain)))
			break;
		/*
		 * check to see if it is an invalid line , if so skip to next 
		 */
		if (dotqmail_alias_command(alias_line) != NULL)
			lines++;
	}
	return (lines < 2);
}

void
moddotqmail()
{
	if (AdminType != DOMAIN_ADMIN) {
		snprintf(StatusMessage, sizeof (StatusMessage), "%s", html_text[142]);
		vclose();
		exit(0);
	}
	send_template("mod_dotqmail.html");
}

void
moddotqmailnow()
{
	if (strcmp(ActionUser, "default") == 0) {
		snprintf(StatusMessage, sizeof (StatusMessage), "%s", html_text[142]);
		vclose();
		exit(0);
	}

	if (strcmp(Action, "delentry") == 0) {
		if (onevalidonly(ActionUser)) {
			snprintf(StatusMessage, sizeof (StatusMessage), "%s\n", html_text[149]);
		} else if (dotqmail_del_line(ActionUser, LineData)) {
			snprintf(StatusMessage, sizeof (StatusMessage), "%s %d\n", html_text[150], 1);
		} else {
			snprintf(StatusMessage, sizeof (StatusMessage), "%s\n", html_text[151]);
		}
	} else if (strcmp(Action, "add") == 0) {
		if (!adddotqmail_shared(ActionUser, Newu, 0)) {
			snprinth(StatusMessage, sizeof (StatusMessage), "%s %H\n", html_text[152], Newu);
		}
	} else {
		snprintf(StatusMessage, sizeof (StatusMessage), "%s\n", html_text[155]);
	}
	moddotqmail();
	vclose();
	exit(0);
}

void
adddotqmail()
{
	count_forwards();
	load_limits();
	if (MaxForwards != -1 && CurForwards >= MaxForwards) {
		snprintf(StatusMessage, sizeof (StatusMessage), "%s %d\n", html_text[157], MaxForwards);
		show_menu(Username, Domain, Mytime);
		vclose();
		exit(0);
	}
	send_template("add_forward.html");
}


void
adddotqmailnow()
{
	if (AdminType != DOMAIN_ADMIN && !(AdminType == USER_ADMIN && strcmp(ActionUser, Username) == 0)) {
		snprintf(StatusMessage, sizeof (StatusMessage), "%s", html_text[142]);
		vclose();
		exit(0);
	}
	count_forwards();
	load_limits();
	if (MaxForwards != -1 && CurForwards >= MaxForwards) {
		snprintf(StatusMessage, sizeof (StatusMessage), "%s %d\n", html_text[157], MaxForwards);
		send_template("add_forward.html");
		vclose();
		exit(0);
	}
	if (adddotqmail_shared(Alias, ActionUser, -1)) {
		adddotqmail();
		vclose();
		exit(0);
	} else {

		snprintf(StatusMessage, sizeof (StatusMessage), "%s\n", html_text[152]);
		show_forwards(Username, Domain, Mytime);
	}
}

int
adddotqmail_shared(char *forwardname, char *dest, int create)
{
	if (strlen(forwardname) <= 0) {
		snprinth(StatusMessage, sizeof (StatusMessage), "%s\n", html_text[163]);
		return (-1);
	/*
	 * make sure forwardname is valid 
	 */
	} else
	if (fixup_local_name(forwardname)) {
		snprinth(StatusMessage, sizeof (StatusMessage), "%s %H\n", html_text[163], forwardname);
		return (-1);
	/*
	 * check to see if we already have a user with this name (only for create) 
	 */
	} else
	if (create != 0 && check_local_user(forwardname)) {
		snprinth(StatusMessage, sizeof (StatusMessage), "%s %H\n", html_text[175], forwardname);
		return (-1);
	}

	if (strcmp(dest, "#") == 0) {
		if (dotqmail_add_line(forwardname, "#")) {
			snprintf(StatusMessage, sizeof (StatusMessage), "%s %d\n", html_text[150], 2);
			return (-1);
		}
		return 0;
	}

	/*
	 * see if forwarding to a local user 
	 */
	if (strstr(dest, "@") == NULL) {
		if (check_local_user(dest) == 0) {
			snprintf(StatusMessage, sizeof (StatusMessage), "%s\n", html_text[161]);
			return (-1);
		} else {
		/*
		 * make it an email address 
		 */
			sprintf(dest, "%s@%s", dest, Domain);
		}
	}

	/*
	 * check that it's a valid email address 
	 */
	if (check_email_addr(dest)) {
		snprinth(StatusMessage, sizeof (StatusMessage), "%s %H\n", html_text[162], dest);
		return (-1);
	}
	snprintf(TmpBuf2, sizeof (TmpBuf2), "&%s", dest);
	if (dotqmail_add_line(forwardname, TmpBuf2)) {
		snprintf(StatusMessage, sizeof (StatusMessage), "%s %d\n", html_text[150], 2);
		return (-1);
	}

	return (0);
}

void
deldotqmail()
{

	if (AdminType != DOMAIN_ADMIN) {
		snprintf(StatusMessage, sizeof (StatusMessage), "%s", html_text[142]);
		vclose();
		exit(0);
	}
	send_template("del_forward_confirm.html");

}

void
deldotqmailnow()
{

	if (AdminType != DOMAIN_ADMIN && !(AdminType == USER_ADMIN && !strcmp(ActionUser, Username))) {
		snprintf(StatusMessage, sizeof (StatusMessage), "%s", html_text[142]);
		show_menu(Username, Domain, Mytime);
		vclose();
		exit(0);
	}
	/*
	 * check to see if we already have a user with this name 
	 */
	if (fixup_local_name(ActionUser)) {
		snprinth(StatusMessage, sizeof (StatusMessage), "%s %H\n", html_text[160], Alias);
		deldotqmail();
		vclose();
		exit(0);
	}

	if (!(dotqmail_delete_files(ActionUser))) {
		snprinth(StatusMessage, sizeof (StatusMessage), "%s %H %H\n", html_text[167], Alias, ActionUser);
	} else {
		snprinth(StatusMessage, sizeof (StatusMessage), "%s %H %H\n", html_text[168], Alias, ActionUser);
	}

	/*
	 * don't display aliases/forwards if we just deleted the last one 
	 */
	count_forwards();
	if (CurForwards == 0 && CurBlackholes == 0) {
		show_menu(Username, Domain, Mytime);
	} else {
		snprintf(SearchUser, sizeof (SearchUser), "%s", ActionUser);
		show_forwards(Username, Domain, Mytime);
	}
}

char           *
dotqmail_alias_command(char *line)
{
	int             len;
	static char     user[501];
	static char     command[MAX_BUFF];
	char           *s;
	char           *b;

	if (line == NULL)
		return NULL;	/* null pointer */
	if (*line == 0)
		return NULL;	/* blank line */
	if (*line == '#')
		return NULL;	/* comment */

	/*
	 * copy everything up to the first whitespace 
	 */
	for (len = 0; line[len] != 0 && isspace(line[len]) == 0; len++) {
		command[len] = line[len];
	}
	command[len] = 0;
	/*
	 * If it ends with a slash and starts with a / or . then
	 * this is a Maildir delivery, local alias
	 */
	if ((command[len - 1] == '/') && ((command[0] == '/') || (command[0] == '.'))) {
		strcpy(user, command);
		user[len - 1] = 0;
		b = NULL;				/* pointer to mailbox name */

		if ((s = strrchr(user, '/')) == NULL)
			return NULL;
		if (strcmp(s, "/" MAILDIR) != 0) {
			b = s + 2;			/* point to name of submailbox */
			*s = '\0';			/* add NULL */
			if ((s = strrchr(user, '/')) == NULL)
				return NULL;
			if (strcmp(s, "/" MAILDIR) != 0)
				return NULL;
		}

		*s = '\0';
		if ((s = strrchr(user, '/')) == NULL)
			return NULL;
		if (b != NULL) {
			snprinth(user, sizeof (user), "%H <I>(%H)</I>", s + 1, b);
		} else {
			snprinth(user, sizeof (user), "%H", s + 1);
		}
		return (user);
		/*
		 * if it's an email address then display the forward 
		 */
	} else
	if (!check_email_addr((command + 1))) {
		if (*command == '&')
			return (&command[1]);
		else
			return (command);
		/*
		 * if it is a program then 
		 */
	} else
	if (command[0] == '|') {

		/*
		 * do not display ezmlm programs 
		 */
		if (strstr(command, "ezmlm") != 0)
			return (NULL);
		/*
		 * do not display autoresponder programs 
		 */
		if (strstr(command, "autoresponder") != 0)
			return (NULL);
		/*
		 * otherwise, display the program 
		 *
		 * back up to pipe or first slash to remove path 
		 */
		while (line[len] != '/' && line[len] != '|')
			len--;
		len++;					/* len is now first char of program name */
		snprinth(command, sizeof (command), "<I>%H</I>", &line[len]);
		return (command);

	} else {
		/*
		 * otherwise just report nothing 
		 */
		return (NULL);
	}
}

void
getversion_qaalias_c()
{
	printf("%s\n", sccsidh);
}
