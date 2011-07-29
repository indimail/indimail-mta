/*
 * $Log: rc.c,v $
 * Revision 1.4  2011-07-29 09:24:18+05:30  Cprogrammer
 * fixed gcc warnings
 *
 * Revision 1.3  2008-07-17 21:38:55+05:30  Cprogrammer
 * moved progname to variables.h
 *
 * Revision 1.2  2008-06-09 15:32:44+05:30  Cprogrammer
 * added GPL copyright notice
 *
 * Revision 1.1  2002-12-16 01:55:18+05:30  Manny
 * Initial revision
 *
 *
 * Run Commands from initialisation files or RCBlocks in the main menu.
 *
 * Copyright (C) Stephen Fegan 1995, 1996
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
 * this program; if not, write to the Free Software Foundation, Inc., 675 Mass
 * Ave, Cambridge, MA 02139, USA.
 * 
 * please send patches or advice on flash to: `flash@netsoc.ucd.ie'
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<pwd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>
#include<errno.h>

#include"set.h"
#include"rc.h"
#include"exec.h"
#include"parse.h"
#include"misc.h"
#include"mystring.h"
#include"parseline.h"
#include"variables.h"

#define RC_ELSE "else"
#define RC_ENDIF "endif"
#define RC_IF "if"

struct rc_commands recognised_rc_commands[] = {
	{"restrict", rcflag_system_only, rc_restrict},
	{"setenv", 0, rc_setenv},
	{"unsetenv", 0, rc_unsetenv},
	{"set-f", 0, rc_set},
	{"set", 0, rc_set},
	{"unset", 0, rc_unset},
	{"exec", 0, rc_exec},
	/*
	 * { "system", 0, rc_system }, 
	 */
	{"pause", 0, rc_pause},
	{"sleep", 0, rc_sleep},
	{"logout", 0, rc_logout},
	{RC_IF, 0, rc_if},
	{RC_ELSE, 0, rc_else},
	{RC_ENDIF, 0, rc_endif},
	{NULL, 0, NULL},
};

enum
{ RC_RUN, RC_RUN_IF, RC_FIND_ELSE_ENDIF, RC_FIND_ENDIF }
rc_mode;

void            processrccommandline(char *rcline, enum rcfile_type rctype, char *errorprefix);

void
rc_file(int wordc, char **wordv, FILE * fp, int *line_no)
{
	char           *directive = *wordv;
	char           *rcfilename;
	enum rcfile_type rctype = USER;

	wordv++, wordc--;

	rc_mode = RC_RUN;

	if (wordc == 0)
	{
		printf("%d: %s: No file specified !\n", *line_no, directive);
		return;
	}

	rcfilename = *(wordv + wordc - 1);

	wordc--;

	while (wordc)
	{
		if (strcasecmp(*wordv, "system") == 0)
			rctype = SYSTEM;
		else
		if (strcasecmp(*wordv, "login") == 0)
		{
			if (!login_shell)
				return;
		} else
			printf("%d: %s: Unrecognised option %s\n", *line_no, directive, *wordv);
		wordv++, wordc--;
	}

	processrcfile(rcfilename, rctype);

	return;
}

void
rc_block(int wordc, char **wordv, FILE * fp, int *line_no)
{
	char           *line;
	int             linen = *line_no, processthisblock = 1;
	char           *directive = *wordv;
	enum rcfile_type rctype = SYSTEM;
	char            errorprefix[] = "Menu File line XXXXXXXXXX";
	char           *linep;
	int             testc;
	char           *testv;

	wordv++, wordc--;

	linep = errorprefix + sizeof(errorprefix) - 11;

	rc_mode = RC_RUN;

	while (wordc)
	{
		if ((strcasecmp(*wordv, "login") == 0))
			processthisblock = login_shell;
		else
			printf("%d: %s: Unrecognised option %s\n", *line_no, directive, *wordv);
		wordv++, wordc--;
	}

	line = Readline(fp);
	linen++;
	while (line)
	{
#ifdef DEBUG
		fprintf(stderr, "%s\n", line);
#endif
		if (parseline(line, &testc, &testv, 1) < 0)
			parse_error("Parseline Error", linen);

		if (testc == 0)
		{
			line = Readline(fp);
			linen++;
			continue;
		}

		if (strcasecmp(testv, RCBLOCK_END) == 0)
		{
			*line_no = linen;
			return;
		}

		sprintf(linep, "%-6d", linen);
		if (processthisblock)
			processrccommandline(line, rctype, errorprefix);

		line = Readline(fp);
		linen++;
	}

	*line_no = linen;
	return;
}

int
processrcfile(char *rcfilename, enum rcfile_type rctype)
{
	char           *rcfilename_full = NULL;
	char            rcline[MAX_LINE];
	struct passwd  *pwd;
	FILE           *rcfp;

	switch (rctype)
	{
	case SYSTEM:
		rcfilename_full = stradp(rcfilename, 0);
		break;
	case USER:
		pwd = getpwuid(getuid());
		if (pwd != (struct passwd *) NULL)
		{
			rcfilename_full = (char *) xmalloc((strlen(pwd->pw_dir) + strlen(rcfilename) + 2) * sizeof(char));
			strcpy(rcfilename_full, pwd->pw_dir);
			strcat(rcfilename_full, "/");
			strcat(rcfilename_full, rcfilename);
		} else
			rcfilename_full = rcfilename;

		break;
	}

	if (access(rcfilename_full, R_OK))
		return -1;

	if ((rcfp = fopen(rcfilename_full, "r")) == NULL)
	{
		fprintf(stderr, "%s: Could not open rcfile %s\n", progname, rcfilename_full);
		perror(progname);
		return -1;
	}

	while (fgets(rcline, MAX_LINE, rcfp) != NULL)
		processrccommandline(rcline, rctype, rcfilename_full);

	fclose(rcfp);
	return 0;
}

void
processrccommandline(char *rcline, enum rcfile_type rctype, char *errorprefix)
{
	char           *argv[MAX_LINE];
	struct rc_commands *this;
	int             argc = 0;

	if (parseline(rcline, &argc, argv, MAX_LINE - 1) > 0)
	{
		if (argc == 0)			/*
								 * || (**argv == COMMENT))
								 */
			return;
		*(argv + argc) = (char *) NULL;

		for (this = recognised_rc_commands; (this->runrc) != NULL; this++)
			if (strcasecmp((this->command), *argv) == 0)
				break;

		if (this->runrc != NULL)
		{
			if ((rctype == USER) && ((this->flags & rcflag_restricted) || (this->flags & rcflag_system_only)))
				fprintf(stderr, "%s: Illegal use of restricted option %s\n", progname, *argv);
			else
			{
				switch (rc_mode)
				{
				case RC_RUN:
				case RC_RUN_IF:
					(this->runrc) (argc, argv);
					break;

				case RC_FIND_ELSE_ENDIF:
					/*
					 * Look for RC_ELSE or RC_END ignoring all else 
					 */
					if ((strcmp(*argv, RC_ELSE) == 0) || (strcmp(*argv, RC_ENDIF) == 0) || (strcmp(*argv, RC_IF) == 0))
						(this->runrc) (argc, argv);
					break;

				case RC_FIND_ENDIF:
					/*
					 * Look for RC_END 
					 */
					if ((strcmp(*argv, RC_ENDIF) == 0) || (strcmp(*argv, RC_IF) == 0))
						(this->runrc) (argc, argv);
					break;
				}
			}
		} else
		{
#ifdef ALLOW_POO
			if (strcmp(*argv, "poos") == 0)
				rc_arseme(argc, argv);
			else
#endif
				fprintf(stderr, "%s: %s Unrecognised command\n", errorprefix, *argv);
		}

		for (; argc; --argc)
			free((void *) *(argv + argc));
	}
}


void
rc_setenv(int argc, char **argv)
{
	if (argc != 3)
		fprintf(stderr, "setenv: Too %s arguments !\n", argc < 3 ? "few" : "many");
	else
		setenv(*(argv + 1), *(argv + 2), 1);

	return;
}

void
rc_unsetenv(int argc, char **argv)
{
	if (argc != 2)
		fprintf(stderr, "unsetenv: Too %s arguments !\n", argc < 2 ? "few" : "many");
	else
		unsetenv(*(argv + 1));

	return;
}

void
rc_set(int argc, char **argv)
{
	int             resettable = 1;

	if (strcasecmp(*argv, "set-f") == 0)
		resettable = 0;

	if (argc == 1)
		fprintf(stderr, "set: Too few arguments !\n");

	else
	if (argc == 2)
		set_variable(*(argv + 1), NULL, resettable);

	else
	if (argc == 3)
		set_variable(*(argv + 1), *(argv + 2), resettable);

	else
	if (argc == 4)
		set_variable(*(argv + 1), *(argv + 2), resettable);

	else
		fprintf(stderr, "set: Too many arguments !\n");


	return;
}

void
rc_unset(int argc, char **argv)
{
	if (argc != 2)
		fprintf(stderr, "unset: Too %s arguments !\n", argc < 2 ? "few" : "many");
	else
		unset_variable(*(argv + 1));

	return;
}

void
rc_exec(int argc, char **argv)
{
	if (argc < 2)
		fprintf(stderr, "exec: Too few arguments !\n");

	argv++, argc--;

	do_exec((char *) NULL, argc, argv, EXEC_NEVERPAUSE | EXEC_NONBGPROC);

	return;
}

void
rc_system(int argc, char **argv)
{
	fprintf(stderr, "In system\n");
}

void
rc_pause(int argc, char **argv)
{
	int             n;
	char            cc;
	argv++, argc--;

	for (; argc; argv++, argc--)
		printf("%s ", *argv);
	printf("\n");

	fflush(stdin), fflush(stdout);
	do
	{
		do
		{
			errno = 0;
			n = read(0, &cc, 1);
			if (n == 0)
				exit(0);
			if (errno == EIO)
				GRAB_BACK_TTY;
		}
		while (errno == EIO && errno != ENOTTY);
	}
	while (cc != '\n');
	fflush(stdin);

	return;
}

void
rc_sleep(int argc, char **argv)
{
	unsigned int    sleeplength;

	if (argc != 2)
		fprintf(stderr, "sleep: Too %s arguments !\n", argc < 2 ? "few" : "many");
	else
	if (sscanf(*(argv + 1), "%ud", &sleeplength) == 1)
		sleep(sleeplength);

	return;
}

void
rc_logout(int argc, char **argv)
{
	exit(EXIT_SUCCESS);
}

void
rc_set_booloption(int argc, char **argv, int *booloption)
{
	char           *option = *argv;

	argv++, argc--;

	switch (argc)
	{
	case 0:
		*booloption = *booloption ? 0 : 1;
		break;
	case 1:
		if ((strcasecmp(*argv, "y") == 0) || (strcasecmp(*argv, "on") == 0))
			*booloption = 1;
		else
		if ((strcasecmp(*argv, "n") == 0) || (strcasecmp(*argv, "off") == 0))
			*booloption = 0;
		else
			fprintf(stderr, "%s: Unrecognised option %s\n", option, *argv);
		break;
	default:
		fprintf(stderr, "%s: Too many arguments !\n", option);
		break;
	}

	return;
}

void
rc_restrict(int argc, char **argv)
{
	char           *option;
	struct rc_commands *this;
	int             except = 0;

	option = *argv;
	argv++, argc--;

	while (argc)
	{
		/*
		 * if(**argv==COMMENT)break;
		 * 
		 * else
		 */ if ((strcmp(*argv, "except") == 0) && (!except))
			except = 1;

		else
		if ((strcmp(*argv, "all") == 0) && (!except))
			for (this = recognised_rc_commands; (this->runrc) != NULL; this++)
				this->flags |= rcflag_restricted;

		else
		if ((strcmp(*argv, "none") == 0) && (!except))
			for (this = recognised_rc_commands; (this->runrc) != NULL; this++)
				this->flags &= (!rcflag_restricted);

		else
		{
			for (this = recognised_rc_commands; (this->runrc) != NULL; this++)
				if (strcmp((this->command), *argv) == 0)
				{
					if (except)
						this->flags &= (!rcflag_restricted);
					else
						this->flags |= rcflag_restricted;
					break;
				}

			if ((this->runrc) == NULL)
				fprintf(stderr, "%s: Unrecognised restriction %s\n", option, *argv);
		}
		argv++, argc--;
	}
	return;
}

void
rc_if(int argc, char **argv)
{
	int             success = 0;
	int             not = 0;

	argc--;
	argv++;

	if (rc_mode != RC_RUN)
	{
		fprintf(stderr, "Cannot nest if blocks yet\n");
		sleep(1);
		rc_mode = RC_FIND_ENDIF;
		return;
	}

	if ((argc != 0) && (strcmp(*argv, "!") == 0))
		not = 1, argc--, argv++;

	if (argc != 0)
	{
		success = do_exec((char *) NULL, argc, argv, EXEC_NEVERPAUSE | EXEC_NONBGPROC);
		if (((not == 0) && (success == EXIT_SUCCESS)) || ((not == 1) && (success != EXIT_SUCCESS)))
			success = 1;
		else
			success = 0;
	}

	if (success == 1)
		rc_mode = RC_RUN_IF;
	else
		rc_mode = RC_FIND_ELSE_ENDIF;

	return;
}

void
rc_else(int argc, char **argv)
{
	argc--;
	argv++;

	switch (rc_mode)
	{
	case RC_RUN:
		fprintf(stderr, "That's a funny place to put an else.\n");
		break;

	case RC_FIND_ENDIF:
		fprintf(stderr, "Internal inconsistancy error.\n");
		break;

	case RC_RUN_IF:
		rc_mode = RC_FIND_ENDIF;
		break;

	case RC_FIND_ELSE_ENDIF:
		if ((argc) && (strcmp(*argv, RC_IF) == 0))
		{
			rc_mode = RC_RUN;
			rc_if(argc, argv);
		} else
			rc_mode = RC_RUN_IF;
		break;
	}

	return;
}

void
rc_endif(int argc, char **argv)
{
	argc--;
	argv++;

	switch (rc_mode)
	{
	case RC_RUN:
		fprintf(stderr, "Thats a funny place to put an endif.\n");
		break;

	case RC_FIND_ELSE_ENDIF:
	case RC_FIND_ENDIF:
	case RC_RUN_IF:
		rc_mode = RC_RUN;
		break;
	}
	return;
}

#ifdef ALLOW_POO
void
rc_arseme(int argc, char **argv)
{
	char           *option;
	struct rc_commands *this;
	int             arses = 0;

	option = *argv;
	argv++, argc--;

	while ((argc) && (strcmp(*argv, "poos") == 0))
		arses++, argc--, argv++;

	if (arses < 5)
		return;

	for (this = recognised_rc_commands; (this->runrc) != NULL; this++)
		this->flags &= (!rcflag_restricted);
	return;
}
#endif
