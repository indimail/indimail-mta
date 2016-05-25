/*
 * $Log: main.c,v $
 * Revision 1.6  2016-05-25 09:11:38+05:30  Cprogrammer
 * use SYSCONFDIR for base directory
 *
 * Revision 1.5  2011-07-29 09:23:56+05:30  Cprogrammer
 * fixed gcc warnings
 *
 * Revision 1.4  2008-07-17 21:37:22+05:30  Cprogrammer
 * moved global variables to variables.[c,h]
 *
 * Revision 1.3  2008-06-09 15:30:47+05:30  Cprogrammer
 * added original Copyright notice
 *
 * Revision 1.2  2008-05-21 18:35:01+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.1  2002-12-16 01:54:59+05:30  Manny
 * Initial revision
 *
 *
 * This is a program to display menus on a text tty, good for limited access
 * guest accounts, or just for user menus.
 *
 * Copyright (C) 1996 Stephen Fegan
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; see the file COPYING. If not write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * please send patches or advice to: `flash@netsoc.ucd.ie' 
 */

/*
 * Include configuration data 
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <pwd.h>
#include <syslog.h>
#include <signal.h>
#include <time.h>

#include "parse.h"
#include "mystring.h"
#include "screen.h"
#include "event.h"
#include "exec.h"
#include "set.h"
#include "rc.h"
#include "debug.h"
#include "misc.h"
#include "variables.h"

extern char     delim;
extern char     escape;
extern char     comment;

void            handle_children(void);
void            die_by_signal(int s);

char           *menufile;
struct menu    *main_menu;

static void
usage()
{
	fprintf(stderr, "Usage: %s [-hV] [-b <directory>] [-c <command>] [menu file]\n\n\
   -b <directory>\tSet base directory to <directory>\n\
   -c <command>\t\tA normal shell would run <command>, we dump it to syslog\n\
   -d <filename>\tSet debug file to <filename>\n\
   -h\t\t\tDisplay help and exit\n\
   -l\t\t\tLog all execs to syslog\n\
   -V\t\t\tDisplay version and compile-time options then exit\n\
   -v\t\t\tDisplay run-time variables, menus, environment\n\
   [menu file]\t\tMenu file to load from\n\n\
With no options [menu file] is set to `%s'\n", progname, DEFAULT_MENU);
}

static void
version()
{
	fprintf(stderr, "flash version %s, Copyright (C) 1996 Stephen Fegan <flash@netsoc.ucd.ie>\n\n\
Compile-time options:\n\
Default menu: `%s', Base directory: `%s'\n", VERSION, DEFAULT_MENU, SYSCONFDIR);
}

int
main(int argc, char *argv[])
{
	struct passwd  *ppwd, pwd;
	struct sigaction s_act;
	char            c;
	char           *filename = DEFAULT_MENU;
	int             ShowVariables = 0;
#if 0
	pid_t           shell_pgid;
#endif
	sigset_t        smask;

	int             debug_file = 0;


	progname = argv[0];
	login_shell = (*progname == '-');
	time(&start_time);

	ppwd = getpwuid(getuid());
	memcpy(&pwd, ppwd, sizeof(struct passwd));
	endpwent();

#if 0
	while (tcgetpgrp(0) != (shell_pgid = getpgrp()))
		kill(-shell_pgid, SIGTTIN);
#endif

	/*
	 * Signal Stuff 
	 */

	signal(SIGINT, SIG_IGN);
	signal(SIGTSTP, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);

	signal(SIGTTIN, SIG_IGN);
	signal(SIGTTOU, SIG_IGN);

	s_act.sa_handler = (void (*)(int)) handle_children;
	sigemptyset(&s_act.sa_mask);
	s_act.sa_flags = 0;
	sigaction(SIGCHLD, &s_act, (struct sigaction *) NULL);

	setpgid(0, 0);
#if 0
	shell_pgid = getpgrp();
#endif

	s_act.sa_handler = (void (*)(int)) handle_snarfed_tty;
	sigemptyset(&s_act.sa_mask);
	s_act.sa_flags = 0;
	if ((sigaction(SIGTTIN, &s_act, (struct sigaction *) NULL) == -1) ||
		(sigaction(SIGTTOU, &s_act, (struct sigaction *) NULL) == -1))
	{
		perror("Could not set signal handler");
		exit(EXIT_FAILURE);
	}

	sigemptyset(&smask);
	sigprocmask(SIG_SETMASK, &smask, NULL);


	/*
	 * Read command line options 
	 */

	/*
	 * Use our own messages 
	 */
	opterr = 0;

	while ((c = getopt(argc, argv, "lhVvb:d:c:")) != -1)
		switch (c)
		{
		case 'b':
			if ((optarg == NULL) || (strlen(optarg) == 0))
			{
				usage();
				exit(1);
			}
			setbasedirectory(optarg);
			break;
		case 'c':
			if ((optarg == NULL) || (strlen(optarg) == 0))
			{
				usage();
				exit(1);
			}
			syslog(LOG_LOCAL1 | LOG_WARNING, "%d: flash: USE OF OPTION -c: %s", getpid(), optarg);
			break;
		case 'd':
			if ((optarg == NULL) || (strlen(optarg) == 0))
			{
				usage();
				exit(1);
			}
			set_debug_file(optarg);
			debug_file = 1;
			break;
		case 'h':
			usage();
			exit(0);
		case 'l':
			set_variable("logging", NULL, 0);
			break;
		case 'V':
			version();
			exit(0);
		case 'v':
			ShowVariables = 1;
			break;
		case '?':
			if (isprint(optopt))
				fprintf(stderr, "Unknown option: `-%c'.\n", optopt);
			else
				fprintf(stderr, "Unknown option character: `\\x%x'.\n", optopt);
			usage();
			exit(1);
		default:
			abort();
		}

	if (optind < argc)
		filename = argv[optind];

	global_variables();

	init_schedule();
	init_jobq();
	signal(SIGHUP, die_by_signal);
	signal(SIGTERM, die_by_signal);

	init_scr();
	pause_scr();

	if (parsefile(filename) == -1)
	{
		fprintf(stderr, "%s: error while parsing\n", progname);
		exit(EXIT_FAILURE);
	}

	balance_menu_tree();
	main_menu = find_mainmenu();
	if (main_menu == NULL)
	{
		fprintf(stderr, "%s: No Menus !!\n", progname);
		freemem(menufile);
		exit(1);
	}

	openlog(pwd.pw_name, 0, LOG_USER);

	/*
	 * Setup global HotKeys 
	 */

	setupglobalhotkeys();

	/*
	 * Looking over peoples' shoulders
	 */

	if (find_variable("logging", NULL) == 1)
		syslog(LOG_LOCAL1 | LOG_INFO, "%d: flash started %s", getpid(), filename);

	if (ShowVariables)
		showconfig(filename);

	init_scr();
	do_menu(main_menu);
	close_scr();

	signal(SIGHUP, SIG_IGN);
	signal(SIGTERM, SIG_IGN);
	finish_jobq();
	finish_schedule();

	if (find_variable("logging", NULL) == 1)
		syslog(LOG_LOCAL1 | LOG_INFO, "%d: flash finished", getpid());

	if (debug_file == 1)
	{
		close_debug_file();
	}

	/*
	 * we outtie 
	 */
	exit(0);
}
