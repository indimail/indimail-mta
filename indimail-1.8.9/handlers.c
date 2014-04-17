/*
 * $Log: handlers.c,v $
 * Revision 2.8  2014-04-17 12:08:12+05:30  Cprogrammer
 * added error check for setuid() failure
 *
 * Revision 2.7  2014-04-17 11:38:30+05:30  Cprogrammer
 * added error message for setuid() failure
 *
 * Revision 2.6  2011-07-29 09:26:02+05:30  Cprogrammer
 * fixed gcc 4.6 warnings
 *
 * Revision 2.5  2011-04-08 17:26:15+05:30  Cprogrammer
 * added HAVE_CONFIG_H
 *
 * Revision 2.4  2009-02-19 09:30:43+05:30  Cprogrammer
 * check return value of getcwd
 *
 * Revision 2.3  2009-02-18 09:07:05+05:30  Cprogrammer
 * fixed fgets warning
 *
 * Revision 2.2  2008-07-13 19:43:33+05:30  Cprogrammer
 * port for darwin
 *
 * Revision 2.1  2004-09-05 00:57:20+05:30  Cprogrammer
 * handler routines for Operator Shell
 *
 *
 * This is handlers.c (osh)
 * 
 * ----
 * Copyright (c) 1993 The Regents of the University of California
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that: (1) source code distributions
 * retain the above copyright notice and this paragraph in its entirety, (2)
 * distributions including binary code include the above copyright notice and
 * this paragraph in its entirety in the documentation or other materials
 * provided with the distribution, and (3) all advertising materials mentioning
 * features or use of this software display the following acknowledgement:
 * ``This product includes software developed by the University of California,
 * Los Alamos National Laboratory and its contributors.'' Neither the name of
 * the University nore the names of its contributors may be used to endorse
 * or promote products derived from this software without specific prior written
 * permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

static char    *rcsid = "@(#) $Id: handlers.c,v 2.8 2014-04-17 12:08:12+05:30 Cprogrammer Exp mbhangui $";

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <errno.h>
#if STDC_HEADERS
# include <string.h>
#else
char           *strchr(), *strrchr();
#endif
#ifdef HAVE_STRINGS_H
# include <strings.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <sys/types.h>
#include <sys/param.h>
#ifdef HAVE_FCNTL_H
# include <fcntl.h>
#endif
#include <signal.h>
#include <stdlib.h>
#ifdef HAVE_UDB_H
# include <udb.h>
#endif
#ifdef HAVE_ELF_H
# include <elf.h>
# define N_BADMAG(x) (memcmp((x).e_ident, ELFMAG, SELFMAG))
#elif defined(HAVE_MAGIC_H)
# include <magic.h>
# define N_BADMAG(x) \
	((x).file_type!=RELOC_MAGIC && \
	 (x).file_type!=EXEC_MAGIC  && \
	 (x).file_type!=DEMAND_MAGIC && \
	 (x).file_type!=SHARE_MAGIC)
#elif defined(HAVE_A_OUT_H) /*- a.out is the default */
# include <a.out.h>
#endif
#ifndef N_BADMAG
# define N_BADMAG BADMAG
#endif
#if HAVE_SYS_WAIT_H
# include <sys/wait.h>
#endif
#ifndef WEXITSTATUS
# define WEXITSTATUS(stat_val) ((unsigned)(stat_val) >> 8)
#endif
#ifndef WIFEXITED
# define WIFEXITED(stat_val) (((stat_val) & 255) == 0)
#endif
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/param.h>
#include <curses.h>
#include <pwd.h>
#include <grp.h>

#ifdef HAVE_LIMITS_H
# include <limits.h>
# ifndef NGROUPS_MAX
#  ifndef NGROUPS
#   define NGROUPS 16			/*- Copied from includes */
#  endif
# else
#  ifndef NGROUPS
#   define NGROUPS NGROUPS_MAX
#  endif
# endif
#endif

#include "osh.h"

#ifdef SYSLOG
#include <syslog.h>
#endif


void
logit(value)
	char            value;

{
	extern FILE    *lg;

#ifdef LOGGING
# ifndef SYSLOG
	fprintf(lg, "\t\t%c\n", value);
# else
	if (value == '+')
		syslog_entry("+", 0);
	else
		syslog_entry("-", 0);
# endif
#endif
	return;
}

#ifdef SYSLOG
syslog_entry(string, cont)
	char           *string;
	int             cont;		/*- 0 to log this NOW, 1 to wait for more data */
{
	static char     logentry[1024];

	if (cont)
		strcpy(logentry, string);
	else
	{
		if (*string == '+' || *string == '-')
		{
			strcat(logentry, "         ");
			strcat(logentry, string);
		} else
			strcpy(logentry, string);
		openlog("osh", LOG_NOWAIT | LOG_PID, LOG_USER);
		syslog(SYSLOG_PRIORITY, logentry);
		closelog();
	}
	return;
}
#endif

int
intable(what)
	char           *what;

{
	int             i;
	int             ret = 0;

	for (i = 0; i < NUMENTRY; i++)
		if (!strcmp(what, Table[i].prog_name))
		{
			ret = 1;
			break;
		}
	return ret;
}

#ifndef COMPILE_TABLE
int
acl(name, mode)
	char           *name, mode;

{
	/*
	 * Returns 1 on a +, -1 on a -, and 0 on a not found 
	 */
	struct stat     buf;
	int             temp = 0;
	char            s_string[MAXPATHLEN];
	char            tname[MAXPATHLEN];
	char           *p;
	int             ret = 0;
	extern int      errno;
	int             noent = 0;

#ifdef HAVE_REALPATH
	realpath(name, tname);
#else
	strcpy(tname, name);
#endif
	if (stat(tname, &buf) && errno == ENOENT)
		noent = 1;
	if (*tname != '/')
	{
		if (!getcwd(s_string, sizeof(s_string)))
			strcpy(s_string, "unknown");
		strcat(s_string, "/");
		strcat(s_string, tname);
	} else
		strcpy(s_string, tname);
	while (FileList[temp] != NULL)
	{
		if (!strncmp(s_string, FileList[temp] + 2, strlen(FileList[temp]) - 2) && (*(FileList[temp] + 1) == mode))
		{
			if (*FileList[temp] == '+')
			{
				ret = 1;
				break;
			} else
			{
				ret = (-1);
				break;
			}
		}
		temp++;
	}
	if ((!ret) && (noent || !(buf.st_mode & S_IFDIR)))
		if ((p = strrchr(s_string, '/')) != NULL)
		{
			if (p != s_string)
				*p = '\0';
			else
				*(p + 1) = '\0';
			temp = 0;
			while (FileList[temp] != NULL)
			{
				if (!strncmp(s_string, FileList[temp] + 2, strlen(FileList[temp]) - 2) && (*(FileList[temp] + 1) == mode))
				{
					if (*FileList[temp] == '+')
					{
						ret = 1;
						break;
					} else
					{
						ret = (-1);
						break;
					}
				}
				temp++;
			}
		}
	return ret;
}
#endif

/*
 * Return 1 on everything okay, 0 on something not accessible 
 */
int
check_access(argc, argv)
	int             argc;
	char           *argv[];

{
	struct stat     buf;
	int             i;
#ifdef HAVE_UDB_H
	struct udb     *u_info;
#else
	struct passwd  *u_info;
#endif
	char            x[MAXPATHLEN];
	int             ret = 1, found = 0;
	int             y;
	char           *p;

	for (i = 0; i < argc; i++)
	{
		if (intable(argv[i]))
			continue;

#ifndef COMPILE_TABLE
		if ((y = acl(argv[i], 'r')) == 1)
			continue;			/*- Found in list as a + */
		else
		if (y == -1)
		{
			/*- Found in list as a - */
			ret = 0;
			break;
		}
#endif

		/*
		 * Writability access is checked later, we just are interested in
		 * * if we have a right to look at the path
		 */
		if (!stat(argv[i], &buf))
		{
			/*- File eists, and was statted */
#ifdef OPER_OVERRIDE
			{
				GETGROUPS_T     groups[NGROUPS];
				struct group   *gr;
				int             ngroups, i;

				ngroups = getgroups(NGROUPS, groups);
				for (i = 0; i < ngroups; i++)
				{
					gr = getgrgid(groups[i]);
					if (gr != NULL)
						if (!strcmp(OPER_GROUP, gr->gr_name))
							found = 1;
				}
				if (found)
					continue;	/*- If we're in the operator group, override */
			}
#endif
			if (!access(argv[i], R_OK))
				continue;
			/*
			 * Even if the file owner doesn't approve, go ahead if
			 * we could read the file/directory anyway 
			 */
			if (!strcmp(argv[0], "ls") && !strcmp(argv[1], "-d") && (buf.st_mode & S_IFDIR))
			{
				/*
				 * We're doing an ls -d... Now check that the current
				 * arguments parent is readable 
				 */
				strcpy(x, argv[i]);
				if ((p = strrchr(x, '/')) != NULL)
				{
					/*- Check the parent */
					if (p != x)
						*p = '\0';
					else
						*(p + 1) = '\0';
					if (!access(x, R_OK))
						found = 1;
				} else
				if (!access(".", R_OK))
					found = 1;
			}
			if (found)
				continue;
#ifdef HAVE_UDB_H
			u_info = getudbuid(buf.st_uid);
			snprintf(x, sizeof(x), "%s/%s", u_info->ue_dir, ACCESS_FILE);
#else
			u_info = getpwuid(buf.st_uid);
			snprintf(x, sizeof(x), "%s/%s", u_info->pw_dir, ACCESS_FILE);
#endif
			if (!stat(x, &buf))
				continue;
			else
			{
				ret = 0;
				break;
			}
		}	/*- else it doesn't matter--writability will be checked later */
	}
	return ret;
}

void
fatal(mes)
	char           *mes;

{
	perror(mes);
	exit(1);
}

/*
 * writeable returns 1 if the file is writeable, 0 otherwise 
 */
int
writeable(file)
	char           *file;

{
	int             accessible;
	char            temp[255];
	char           *temp2;
	struct stat     buf;
	char            temp3[255];
	int             k = 0;

	if (*file != '/')
	{
		if (!getcwd(temp3, sizeof(temp3)))
			strcpy(temp3, "unknown");
		strcat(temp3, "/");
		strcat(temp3, file);
	} else
		strcpy(temp3, file);
	stat(temp3, &buf);
	accessible = access(temp3, W_OK);
#ifdef COMPILE_TABLE
	if ((accessible == 0) || (buf.st_uid == getuid()))
#else
	if (((k = acl(temp3, 'w')) != -1) && ((accessible == 0) || (buf.st_uid == getuid()) || (k == 1)))
#endif
		return 1;
	else
	if (k == -1)
		return 0;
	else
	{
		strcpy(temp, temp3);
		temp2 = (char *) strrchr(temp, '/');
		if (temp2 == NULL)
			strcpy(temp, ".");
		else
		{
			if (temp2 != temp)
				*temp2 = '\0';
			else
				*(temp2 + 1) = '\0';	/*- Preserve '/' */
		}
		if (errno == ENOENT)
		{
			/*
			 * If the file doesn't exist, check the path 
			 */
			accessible = access(temp, W_OK);
			/*
			 * fprintf(stderr,"Checking writability of %s.\n",temp); 
			 */
		} else
		{
			/*
			 * If it does exist, and we own the directory, writing is fine 
			 */
			stat(temp, &buf);
			if (buf.st_uid == getuid())
				accessible = 0;	/*- Get REAL uid */
			else
				accessible = (-1);
		}
	}
	if (accessible == 0)
	{
		/*- fprintf(stderr,"Writeable!\n"); */
		return 1;
	} else
		return 0;
}

int
i_alias(argc, argv)
	int             argc;
	char           *argv[];

{
	int             i, found = 0, aspace = 0;

	if (AliasCounter == 100)
	{
		fprintf(stderr, "Too many aliases\n");
		logit('-');
		return(0);
	}
	if (argc == 1)
	{
		printf("Current list of aliases:\n");
		for (i = 0; i < AliasCounter; i++)
			printf("%s  ->  %s\n", AliasList[i].cmd, AliasList[i].alias);
		logit('+');
		return(0);
	}
	if (argc == 2)
	{
		for (i = 0; i < AliasCounter; i++)
			if (!strcmp(argv[1], AliasList[i].cmd))
			{
				AliasList[i].cmd = AliasList[AliasCounter].cmd;
				AliasList[i].alias = AliasList[AliasCounter].alias;
				AliasCounter--;
				found = 1;
			}
		if (!found)
		{
			fprintf(stderr, "Can't unalias %s\n", argv[1]);
			logit('-');
		} else
		{
			fprintf(stderr, "Unaliased %s\n", argv[1]);
			logit('+');
		}
		return(0);
	}

	AliasList[AliasCounter].cmd = (char *) malloc(strlen(argv[1]) + 1);
	strcpy(AliasList[AliasCounter].cmd, argv[1]);
	for (i = 0; i < argc - 2; i++)
		aspace += strlen(argv[i + 2]) + 1;
	AliasList[AliasCounter].alias = (char *) malloc(aspace);
	strcpy(AliasList[AliasCounter].alias, argv[2]);
	for (i = 0; i < argc - 3; i++)
	{
		strcat(AliasList[AliasCounter].alias, " ");
		strcat(AliasList[AliasCounter].alias, argv[i + 3]);
	}
	printf("Alias[%d]: %s  ->  %s\n", AliasCounter, AliasList[AliasCounter].cmd, AliasList[AliasCounter].alias);
	AliasCounter++;
	logit('+');
	return(0);
}

int
i_cp(argc, argv)
	int             argc;
	char           *argv[];

{
	if (argc != 3)
	{
		fprintf(stderr, "cp: Too few/many arguments\n");
		logit('-');
	} else
	if (writeable(argv[argc - 1]))
	{
		logit('+');
		execute(argc, argv);
	} else
	{
		perror("cp");
		logit('-');
	}
	return(0);
}

int
i_ldcache(argc, argv)
	int             argc;
	char           *argv[];

{
	if (argc != 1)
	{
		fprintf(stderr, "ldcache: No arguments allowed\n");
		logit('-');
	} else
	{
		logit('+');
		execute(argc, argv);
	}
	return(0);
}

int
i_vi(argc, argv)
	int             argc;
	char           *argv[];

{
	if (argc != 2)
	{
		fprintf(stderr, "vi: Too few/many arguments\n");
		logit('-');
	} else
	if (writeable(argv[argc - 1]))
	{
		logit('+');
		execute(argc, argv);
	} else
	{
		perror("vi");
		logit('-');
	}
	return(0);
}

int
i_rm(argc, argv)
	int             argc;
	char           *argv[];

{
	char            parent[255];
	char           *n;
	int             i;

	if (argc < 2)
	{
		fprintf(stderr, "rm: Too few arguments\n");
		logit('-');
	} else
	{
		for (i = 2; i <= argc; i++)
			if (!writeable(argv[i - 1]))
			{
				strcpy(parent, argv[i - 1]);
				if ((n = (char *) strrchr(parent, '/')) != NULL)
				{
					if (n != parent)
						*n = '\0';
					else
						*(n + 1) = '\0';
					if (!access(parent, W_OK))
						continue;
					/*
					 * You can remove a file in a directory you would normally have write access to
					 */
				}
				fprintf(stderr, "rm: %s not writeable. No deletions performed.\n", argv[i - 1]);
				logit('-');
				return(0);
			}
		logit('+');
		execute(argc, argv);
	}
	return(0);
}

int
i_done(argc, argv)
	int             argc;
	char           *argv[];

{
	exit(0);	/*- Short circuit exit */
}

int
i_cd(argc, argv)
	int             argc;
	char           *argv[];

{
	int             x;

	if (argc > 2)
	{
		fprintf(stderr, "cd: Too many arguments\n");
		logit('-');
	} else
	{
		if (argc == 2)
			x = chdir(argv[argc - 1]);
		else
			x = chdir(getenv("HOME"));
		if (x)
		{
			fprintf(stderr, "cd: %s\n", strerror(errno));
			logit('-');
		} else
			logit('+');
	}
	return(0);
}

int
i_more(argc, argv)
	int             argc;
	char           *argv[];

{
	FILE           *input_file;
	char           *file_name;
	char            temp[255];
	char            c;
	int             y;
	int             done = 0, noeof = 0;
	int             line = 0, size = 0;

	if (argc > 2)
	{
		logit('-');
		fprintf(stderr, "more: Too many arguments\n");
	} else
	{
		if (argc == 2)
		{
			file_name = argv[argc - 1];
			input_file = fopen(file_name, "r");
		} else
		{
			input_file = (FILE *) fdopen(dup(0), "r");
			close(0);
			open("/dev/tty", O_RDONLY, 0);
			file_name = (char *) malloc(2);
			strcpy(file_name, "-");
		}
		if (input_file == NULL)
		{
			logit('-');
			fprintf(stderr, "more: %s\n", strerror(errno));
			return(0);
		}
		(void) open("/dev/tty", O_RDONLY);

		initscr();
		noecho();
		cbreak();
		scrollok(stdscr, TRUE);
		clear();
		move(1, 0);
		while (!done)
		{
			if (!fgets(temp, 255, input_file))
			{
				if (feof(input_file))
					done = 1;
				else
				{
					fprintf(stderr, "more: %s\n", strerror(errno));
					return(0);
				}
			}
			if (!done)
			{
				line++;
				printw("%s", temp);
				getyx(stdscr, y, c);
				if (y == LINES - 1)
				{
					if (size == 0)
						size = line;
					move(0, 0);
					standout();
					sprintf(temp, "File: %s     Line #: %d-%d", file_name, line - size, line);
					addstr(temp);
					standend();
					clrtoeol();
					move(LINES - 1, 0);
					standout();
					addstr("---MORE---");
					standend();
					refresh();
					c = getch();
					switch (c)
					{
					case ' ':
						clear();
						move(1, 0);
						break;
					case 'q':
						done = 1;
						noeof = 1;
						break;
					case '\n':
					default:
						scroll(stdscr);
						move(LINES - 2, 0);	/*- Where MORE should be now */
						break;
					}
				}
			}
			if (done)
				refresh();
		}	/*- of while */
	}	/*- of else */
	if (!noeof)
	{
		printw("<EOF>:");
		refresh();
		getch();
		printw("\n");
	}
	endwin();
	printf("\n");	/*- Skip down a line */
	logit('+');
	return(0);
}

/*
 * Notes on execute:
 *  1) execute is called AFTER being forked off by the main shell, 
 *     consequently, all setuid's will affect children ONLY and not parents.
 *  2) if effective uid == 0 (ie., we are superuser), we save our current
 *     REAL id (in case execv fails), make our real id 0, then exec.
 *      The reason for this is we need to have a ruid of 0 for shutdown.
 *      Since security is checked thoroughly (we hope) before getting to this
 *      point, and we already have an effective uid of 0, this should really\
 *      have no affect. The only reason I put it here instead of at the\
 *      beginning is because the access(2) call depends on our real id.
 */
#ifdef DARWIN
int
execute(argc, argv)
	int             argc;
	char           *argv[];
{
	int             x;

	x = getuid();
	if (geteuid() == 0 && setuid(0))
		fatal("setuid");
	execv(argv[0], argv);
	fatal("exec");
	if (setuid(x))
		fatal("setuid");
	return(0);
}
#else
int
execute(argc, argv)
	int             argc;
	char           *argv[];
{
	char          **av2;
	int             fd;
#ifdef HAVE_ELF_H
	int             size = sizeof(Elf32_Ehdr);
	Elf32_Ehdr     *temp;
#elif defined(HAVE_MAGIC_H)
	struct magic   *temp;
	int             size = sizeof(struct magic);
#elif !defined(HAVE_XCOFF_H)
	int             size = sizeof(struct exec);
	struct exec    *temp;
#endif
	/*- If you have XCOFF (AIX), we don't need a struct exec */
	char           *buf;
	int             i;
	int             x;


	av2 = (char **) malloc(argc + 1);
	x = getuid();
	if (geteuid() == 0 && setuid(0))
		fatal("setuid");
#ifdef HAVE_XCOFF_H	/*- AIX */
	execv(argv[0], argv);
	fatal("exec");
#else
	if ((fd = open(argv[0], O_RDONLY)) < 0)
		fprintf(stderr, "magic check: %s: %s", argv[0], strerror(errno));
	else
	{
		buf = (char *) malloc(size);
		if (read(fd, buf, size) == -1)
			fatal("read");
#ifdef HAVE_ELF_H
		temp = (Elf32_Ehdr *) buf;
#elif defined(HAVE_MAGIC_H)
		temp = (struct magic *) buf;
#else
		temp = (struct exec *) buf;
#endif /*- HAVE_ELF_H */
		if (N_BADMAG((*temp)))
			if (*buf != '#')
			{
				/*- Has shell header */
				*av2 = "/bin/sh";	/*- bourne shell default?  */
				for (i = 0; i < argc + 1; i++)
					*(av2 + 1 + i) = argv[i];
				execv(*av2, av2);
				fatal("exec");
			}
		execv(argv[0], argv);
		fatal("exec");
	}
#endif /*- HAVE_XCOFF_H */
	if (setuid(x))
		fatal("setuid");
	return(0);
}	/*- of execute routine */
#endif

int
i_help(argc, argv)
	int             argc;
	char           *argv[];

{
	int             i;
	int             x = 1;

	printf("Operator Shell (osh) Version %s\n", VERSION);
	printf("   by Michael Neuman <mcn@EnGarde.com>\n\n\n");
	printf("Defines:\n");
#ifdef COMPILE_TABLE
	printf("COMPILE_TABLE\n");
#else
	printf("NO COMPILE_TABLE\n");
#endif
#ifdef LOGGING
	printf("LOGGING");
# ifdef SYSLOG
	printf(" to SYSLOG\n");
# else
	printf(" to FILE\n");
# endif
#else
	printf("NO LOGGING\n");
#endif
#ifdef CHECK_ACCESS
	printf("CHECK_ACCESS\n");
#else
	printf("NO CHECK_ACCESS\n");
#endif
#ifdef OPER_OVERRIDE
	printf("OPER_OVERRIDE\n");
#else
	printf("NO OPER_OVERRIDE\n");
#endif
	printf("\nCommands accessible:\n");
	for (i = 0; i < (NUMENTRY) + 1; i++)
	{
		if ((int) i / 8 == x)
		{
			printf("\n");
			x++;
		}
		printf("%s\t", Table[i].prog_name);
	}
	printf("\n");
	logit('+');
	return(0);
}

int
i_mount(argc, argv)
	int             argc;
	char           *argv[];

{
	char           *nargv[6];

	if (argc != 3)
	{
		fprintf(stderr, "mount: too few/many arguments\n");
		fprintf(stderr, "Usage: mount device mnt_point\n");
		logit('-');
	} else
	{
		logit('+');
		/*
		 * Create a new arg vector with the flags we want to force 
		 */
		nargv[0] = argv[0];
		nargv[1] = "-t";
		nargv[2] = "pcfs";
		nargv[3] = argv[1];
		nargv[4] = argv[2];
		nargv[5] = 0;
		execute(5, nargv);		/*- There are 5 arguments */
	}
	return(0);
}

int
i_test(argc, argv)
	int             argc;
	char           *argv[];
{
	extern int      prompt;

	/*
	 * If we're prompting, then the user PROBABLY doesn't want to have the shell
	 * exiting on them, tell them so...
	 */
	if (prompt)
	{
		fprintf(stderr, "test: Can not be run by interactive shell.\n");
		logit('-');
		return(0);
	}
	if (argc != 3)
	{
		fprintf(stderr, "test: too few/many arguments.\n");
		fprintf(stderr, "Usage: test [-w|-r] filename\n");
		logit('-');
		exit(0);
	} else
	{
		if (!strcmp(argv[1], "-w"))
		{
			if (writeable(argv[2]))
			{
				logit('+');
				exit(1);
			} else
			{
				logit('-');
				exit(0);
			}
		}
		if (!strcmp(argv[1], "-r"))
		{
			if (!access(argv[2], R_OK))
			{
				logit('+');
				exit(1);
			} else
			{
				logit('-');
				exit(0);
			}
		}
		/*
		 * If we're here, there's a problem 
		 */
		fprintf(stderr, "test: invalid argument '%s'\n", argv[1]);
		fprintf(stderr, "Usage: test [-w|-r] filename\n");
		logit('-');
		exit(0);
	}
	return(0);
}

int
i_exit(argc, argv)
	int             argc;
	char           *argv[];

{
	logout();	/*- Defined in main */
	exit(0);
}

void
getversion_handlers_c()
{
	printf("%s\n", rcsid);
}
