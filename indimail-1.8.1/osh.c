/*
 * $Log: osh.c,v $
 * Revision 2.6  2011-04-08 17:26:54+05:30  Cprogrammer
 * added HAVE_CONFIG_H
 *
 * Revision 2.5  2009-02-24 22:39:22+05:30  Cprogrammer
 * removed left over semi colon
 *
 * Revision 2.4  2009-02-19 09:31:18+05:30  Cprogrammer
 * removed compiler warnings
 *
 * Revision 2.3  2005-12-29 23:04:15+05:30  Cprogrammer
 * added prototype for getEnvConfigStr
 *
 * Revision 2.2  2005-12-29 22:47:04+05:30  Cprogrammer
 * use getEnvConfigStr to set variables from environment variables
 *
 * Revision 2.1  2004-09-05 00:56:17+05:30  Cprogrammer
 * Operator Shell
 *
 *
 * This is main.c (osh)
 * 
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

static char    *rcsid = "@(#) $Id: osh.c,v 2.6 2011-04-08 17:26:54+05:30 Cprogrammer Stab mbhangui $";

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <string.h>
#include <errno.h>
#ifdef HAVE_MALLOC_H
# include <malloc.h>
#endif
#include <stdlib.h>
#include <sys/types.h>
#if HAVE_SYS_WAIT_H
# include <sys/wait.h>
#endif
#ifndef WEXITSTATUS
# define WEXITSTATUS(stat_val) ((unsigned)(stat_val) >> 8)
#endif
#ifndef WIFEXITED
# define WIFEXITED(stat_val) (((stat_val) & 255) == 0)
#endif
#ifdef HAVE_FCNTL_H
# include <fcntl.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <signal.h>
#ifdef HAVE_DIRENT_H
# include <dirent.h>
#else
# define dirent direct
# if HAVE_SYS_NDIR_H
#  include <sys/ndir.h>
# endif
# if HAVE_SYS_DIR_H
#  include <sys/dir.h>
# endif
# if HAVE_NDIR_H
#  include <ndir.h>
# endif
#endif
#ifdef HAVE_SYS_UTSNAME_H
# include <sys/utsname.h>
#endif
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/param.h>
#include <curses.h>
#include <pwd.h>
#include <grp.h>
#if TM_IN_SYS_TIME
# include <sys/time.h>
#else
# include <time.h>
#endif

#ifdef HAVE_LIMITS_H
#include <limits.h>
#ifndef NGROUPS_MAX
#define NGROUPS 16	/*- Copied from includes */
#else
#ifndef NGROUPS
#define NGROUPS NGROUPS_MAX
# endif
#endif	/*- ngroups_max */
#endif
# include "osh.h"

#if defined(LOGGING) && !defined(SYSLOG)
FILE           *lg;
#endif

#ifndef COMPILE_TABLE
struct entry    Table[MAXTABLESIZE];
char           *FileList[100];
int             NUMENTRY;
#endif

struct alias    AliasList[100];
int             AliasCounter;

struct passwd  *pw, pwh;

FILE           *inputfp = NULL;
char            host[17];
char            LogFile[1024];
char            work_table[MAXPATHLEN];
static struct hand Internal[] = {
   { "more",   i_more   },
   { "cd",     i_cd     },
   { "cp",     i_cp,    },
   { "help",   i_help   },
   { "rm",     i_rm     },
   { "alias",  i_alias  },
   { "vi",     i_vi     },
   { "ldcache",i_ldcache},
   { "mount",  i_mount  },
   { "test",   i_test   },
   { "logout", i_exit   }, /* No handler, immediately exit */
   { "exit"  , i_exit   }, /* ditto */
   { NULL,     NULL     }
}; 

void            getEnvConfigStr(char **, char *, char *);

static TOKEN
gettoken(iword)
	char           *iword;
{
	enum
	{ NEUTRAL, GTGT, INQUOTE, INWORD }
	state = NEUTRAL;
	int             c;
	char           *w;
	int             pgetc();
	void            pungetc();

	w = iword;
	while ((c = pgetc()) != EOF)
	{
		switch (state)
		{
		case NEUTRAL:
			switch (c)
			{
			case ';':
				return (TSEMI);
			case '&':
				return (TAMP);
			case '$':
				return (TDOLLAR);
			case '|':
				return (TPIPE);
			case '<':
				return (TLT);
			case '\n':
				return (TNL);
			case ' ':
			case '\t':
				continue;
			case '>':
				state = GTGT;
				continue;
			case '"':
				state = INQUOTE;
				continue;
			default:
				state = INWORD;
				*w++ = c;
				continue;
			}
		case GTGT:
			if (c == '>')
				return (TGTGT);
			pungetc(c, inputfp);
			return (TGT);
		case INQUOTE:
			switch (c)
			{
			case '\\':
				*w++ = pgetc();
				continue;
			case '"':
				*w = '\0';
				return (TWORD);
			default:
				*w++ = c;
				continue;
			}
		case INWORD:
			switch (c)
			{
			case ';':
			case '&':
			case '|':
			case '<':
			case '>':
			case '\n':
			case ' ':
			case '\t':
				pungetc(c, inputfp);
				*w = '\0';
				return (TWORD);
			default:
				*w++ = c;
				continue;
			}
		}
	}
	return (TEOF);
}

static char     inputstring[1024];
int             prompt = 0;

static void
iopen(argc, argv)
	int             argc;
	char           *argv[];
{
	int             i = (-1);
	int             found = 0;

	if (argc == 1)
	{	
		inputfp = stdin;
		prompt = 1;				/*- Turn on prompting */
	} else
	{
		while (i < NUMENTRY)	/*- Look through the table to find if it's a command */
		{
			if (strcmp(Table[++i].prog_name, argv[1]) == 0)
			{
				found = 1;
				break;
			}
		}
		if (found)
		{						
			/*- It's a command, input is a string */
			inputfp = (FILE *) 1;
			strncpy(inputstring, argv[1], 1024);
			for (i = 3; i <= argc; i++)
			{
				strncat(inputstring, " ", 1024 - strlen(inputstring));
				strncat(inputstring, argv[i - 1], 1024 - strlen(inputstring));
			}
			strncat(inputstring, "\n", 1024 - strlen(inputstring));	/*- So it's a command */
		} else
		{
			/*- It's a file, input is that file */
			if (access(argv[1], R_OK))
			{
				fprintf(stderr, "No access to shell script\n");
				exit(1);
			}
			inputfp = fopen(argv[1], "r");
			if (inputfp == NULL)
			{
				perror("Can't open shell script");
				exit(1);
			}
		}
	}
	return;
}

static char    *pgetcptr = NULL;

void
pungetc(value, fp)
	int             value;
	FILE           *fp;
{
	if (fp)
	{
		if (fp != (FILE *) 1)
			ungetc(value, fp);
		else
		if (fp && pgetcptr)
			pgetcptr--;
	}
	return;
}

/*
 * pgetc should eventually be made nicely buffered 
 * XXX 
 */
int
pgetc()
{
	int             i;
	static int      column = 0;
	int             retval;

	if (inputfp == NULL)
		return (EOF);
	if (inputfp == (FILE *) 1)
	{
		if (!pgetcptr)
			pgetcptr = inputstring;
		if (*pgetcptr == 0)
		{
			inputfp = NULL;
			retval = EOF;
		} else
		{
			retval = *pgetcptr;
			pgetcptr++;
		}
		return (retval);
	}
	i = fgetc(inputfp);
	if (i == EOF)
	{
		inputfp = NULL;
	} else
	{
		if (column == 0 && i == '#')
		{
			while ((i = fgetc(inputfp)) != EOF && i != '\n' && i != '\r');
			i = fgetc(inputfp);
			column++;
		} else
		{
			if (i == '\n' || i == '\r')
				column = 0;
			else
				column++;
		}
	}
	return (i);
}

void
logout()
{
	time_t          timer;
	char           *date_time;

	if (prompt)
		printf("Exit\n");
#ifdef LOGGING
#ifndef SYSLOG
	time(&timer);
	date_time = ctime(&timer);
	fprintf(lg, "logout: %s left osh at %s", pw->pw_name, date_time);
	fclose(lg);
#else
	{
		char            ebuf[80];

		sprintf(ebuf, "logout: %s left osh", pw->pw_name);
		syslog_entry(ebuf, 0);
	}
#endif
#endif
}

void
ignoresig()
{
	if (signal(SIGINT, SIG_IGN) == SIG_ERR || signal(SIGQUIT, SIG_IGN) == SIG_ERR)
		perror("signal");
	signal(SIGHUP, exit);
	signal(SIGTERM, exit);
}

void
entrysig()
{
	if (signal(SIGINT, SIG_DFL) == SIG_ERR || signal(SIGQUIT, SIG_DFL) == SIG_ERR)
		perror("signal");
}

static int
redirect(srcfd, srcfile, dstfd, dstfile, append, bckgrnd)
	int             srcfd, dstfd, append, bckgrnd;
	char           *srcfile, *dstfile;
{
	int             flags, fd;
	char           *test[1];

	if (srcfd == 0 && bckgrnd)
	{
		strcpy(srcfile, "/dev/null");
		srcfd = BADFD;
	}
	if (srcfd != 0)
	{
		if (close(0) == -1)
			perror("close");
		if (srcfd > 0)
		{
			if (dup(srcfd) != 0)
				fatal("dup");
		} else
		{
#ifdef CHECK_ACCESS
			/*
			 * Setup dummy argv[] for check_access 
			 */
			test[0] = (char *) malloc(strlen(srcfile) + 1);
			strcpy(test[0], srcfile);
			if (!check_access(1, test))
			{
				fprintf(stderr, "Consult permission not set\n");
#ifdef LOGGING
				logit('-');
#endif
				return (0);
			}
			free(test[0]);
#endif /*- CHECK_ACCESS */
			if (open(srcfile, O_RDONLY, 0) == -1)
			{
				fprintf(stderr, "Can't open %s\n", srcfile);
				return (0);
			}
		}
	}
	if (dstfd != 1)
	{
		if (close(1) == -1)
			perror("close");
		if (dstfd > 1)
		{
			if (dup(dstfd) != 1)
				fatal("dup");
		} else
		{
			int             dstfd;

			if (writeable(dstfile))
			{
				flags = O_WRONLY | O_CREAT;
				if (!append)
					flags |= O_EXCL;	/*- This handles race condition problems */
				if ((dstfd = open(dstfile, flags, 0666)) == -1)
				{
					if (errno == EEXIST)
						fprintf(stderr, "Will not clobber existing file %s\n", dstfile);
					else
						fprintf(stderr, "Can't create %s\n", dstfile);
					return (0);
				}
				if (!append)
				{
					char            buf[80], *cptr;

					strncpy(buf, dstfile, 80);
					if ((cptr = (char *) strrchr(buf, '/')) != NULL)
					{
						if (cptr != buf)
							*cptr = '\0';
						else
							*(cptr + 1) = '\0';
						if (!access(buf, W_OK))
							/*
							 * if we can write to the parent normally, do
							 * the chown, otherwise, leave it as root 
							 */
#ifdef HAVE_FCHOWN
							if (fchown(dstfd, getuid(), getgid()) == -1)
							{
								fprintf(stderr, "fchown: %s: %s\n", dstfile, strerror(errno));
								return (0);
							}
#else
							/*
							 * Cray is braindead and doesn't have an fchown.
							 */
							if (chown(dstfile, getuid(), getgid()) == -1)
							{
								fprintf(stderr, "chown: %s: %s\n", dstfile, strerror(errno));
								return (0);
							}
#endif
					} else
					{
						/*- otherwise we're in the current directory */
						if (getcwd(buf, sizeof(buf)) && !access(buf, W_OK))
#ifdef HAVE_FCHOWN
							if (fchown(dstfd, getuid(), getgid()) == -1)
							{
								fprintf(stderr, "chown: %s: %s\n", dstfile, strerror(errno));
								return (0);
							}
#else
							if (chown(dstfile, getuid(), getgid()) == -1)
							{
								fprintf(stderr, "chown: %s: %s\n", dstfile, strerror(errno));
								return (0);
							}
#endif
					}
				} else	/*- (if append) */
				if (lseek(1, 0L, 2) == -1)
					perror("lseek");
			} else
			{
				fprintf(stderr, "osh: redirect to %s: Permission denied\n", dstfile);
				return (0);
			}
		}
	}
	for (fd = 3; fd < 20; fd++)
		(void) close(fd);
	return (1);
}


int
builtin(argc, argv, srcfd, srcfile, dstfd, dstfile, append, bckgrnd)
	int             argc, srcfd, dstfd, append, bckgrnd;
	char           *argv[], *srcfile, *dstfile;
{
	register int    i = (-1);
	int             pid;
	int             found = 0;

	while (i < NUMENTRY)
		if (strcmp(Table[++i].prog_name, argv[0]) == 0)
		{
			found = 1;
			break;
		}
	if (!found)
		return (0);	/*- We couldn't find it */
	if (Table[i].path != NULL)
	{
		free(argv[0]);
		argv[0] = (char *) malloc(strlen(Table[i].path) + 1);
		strcpy(argv[0], Table[i].path);
		/*- Put the path in */
	}
	if (strcmp(Table[i].prog_name, "cd") == 0)
	{
		/*- Can't put this in bg */
		(*(Table[i].handler)) (argc, argv);
		return (-2);
	} else
	if (strcmp(Table[i].prog_name, "alias") == 0)
	{
		/*- Can't put in bg */
		(*(Table[i].handler)) (argc, argv);
		return (-2);
	} else
	if (strcmp(Table[i].prog_name, "setenv") == 0)
	{
		/*- Can't put in bg */
		(*(Table[i].handler)) (argc, argv);
		return (-2);
	} else
	if (strcmp(Table[i].prog_name, "test") == 0)
	{
		/* Can't put in bg */
		(*(Table[i].handler)) (argc, argv);
		return (-2);
	} else
	if (strcmp(Table[i].prog_name, "exit") == 0)
	{
		(*(Table[i].handler)) (argc, argv);
		return (-2);
	} else
	if (strcmp(Table[i].prog_name, "logout") == 0)
	{
		(*(Table[i].handler)) (argc, argv);
		return (-2);
	} else
	{
		switch (pid = fork())
		{
		case -1:
			fprintf(stderr, "Can't create new process\n");
			return (0);
		case 0:
			if (!bckgrnd)
				entrysig();
			if (Table[i].handler == execute)
				logit('?');
			if (redirect(srcfd, srcfile, dstfd, dstfile, append, bckgrnd))
				(*(Table[i].handler)) (argc, argv);
			_exit(0);	/*- Just in case it comes back */
		default:
			if (srcfd > 0 && (close(srcfd) == -1))
				perror("close src");
			if (dstfd > 1 && (close(dstfd) == -1))
				perror("close dst");
			if (bckgrnd)
				printf("%d\n", pid);
			return (pid);
		}
	}
}

int
invoke(argc, argv, srcfd, srcfile, dstfd, dstfile, append, bckgrnd)
	int             argc, srcfd, dstfd, append, bckgrnd;
	char           *argv[], *srcfile, *dstfile;

{
	int             pid;

	if (argc == 0)
		return (0);
	if ((pid = builtin(argc, argv, srcfd, srcfile, dstfd, dstfile, append, bckgrnd)))
		return (pid);
	else
	if (pid != -2)
	{
		/*- -2 will be the code for any foreground command */
		fprintf(stderr, "Unknown/restricted command.\n");
		return (0);
	}
	return (0);
}	/*- of routine */


static TOKEN
command(waitpid, makepipe, pipepfd)
	int            *waitpid, *pipepfd, makepipe;
{
	TOKEN           token, term;
	int             argc, srcfd, dstfd, pid, pfd[2], append = 0;
	char           *argv[MAXARG + 1], srcfile[MAXFNAME], dstfile[MAXFNAME];
	char            word[MAXWORD], env[40];	/*- Size of an environment variable */
	char           *env2;
	char           *x;
	time_t          timer;
	struct tm      *dt;

	argc = 0;
	srcfd = 0;
	dstfd = 1;
	while (1)
	{
		switch (token = gettoken(word))
		{
		case TWORD:
			if (argc == MAXARG)
			{
				fprintf(stderr, "Too many args\n");
				break;
			}
			if ((x = expand(argv, &argc, word)) != NULL)
			{
				fprintf(stderr, "%s\n", x);
				while (--argc >= 0)
					free(argv[argc]);
				return (0);
			}
			continue;
		case TLT:
			if (makepipe)
			{
				fprintf(stderr, "Extra <\n");
				break;
			}
			if (gettoken(srcfile) != TWORD)
			{
				fprintf(stderr, "Illegal <\n");
				break;
			}
			srcfd = BADFD;
			continue;
		case TGT:
		case TGTGT:
			if (dstfd != 1)
			{
				fprintf(stderr, "Extra > or >>\n");
				break;
			}
			if (gettoken(dstfile) != TWORD)
			{
				fprintf(stderr, "Illegal > or >>\n");
				break;
			}
			dstfd = BADFD;
			append = (token == TGTGT);
			continue;
		case TDOLLAR:
			if (gettoken(env) != TWORD)
			{
				fprintf(stderr, "Illegal environment variable\n");
				break;
			}
			if ((env2 = getenv(env)) == NULL)
			{
				char            temp[255];
				char           *temp2;

				strncpy(temp, env, sizeof(temp));
				if ((temp2 = (char *) strrchr(temp, '/')) != NULL)
				{
					if (temp2 != temp)
						*temp2 = '\0';
					else
						*(temp2 + 1) = '\0';
					if ((env2 = getenv(temp)) != NULL)
					{
						strcat(env2, "/");
						strcat(env2, temp2 + 1);
					}
				}
			}
			if (env2 == NULL)
			{
				fprintf(stderr, "Nonexistent environment variable\n");
				break;
			}
			if ((argv[argc] = (char *) malloc(strlen(env2) + 1)) == NULL)
			{
				fprintf(stderr, "Out of arg memory\n");
				break;
			}
			strcpy(argv[argc], env2);
			argc++;
			continue;
		case TPIPE:
		case TAMP:
		case TSEMI:
		case TNL:
			argv[argc] = NULL;
			if (token == TPIPE)
			{
				if (dstfd != 1)
				{
					fprintf(stderr, "> or >> conflicts with |\n");
					break;
				}
				term = command(waitpid, TRUE, &dstfd);
			} else
				term = token;
			if ((token == TNL) || (token == TSEMI))
			{
#ifdef LOGGING
				if (argc > 0)
				{
					int             kk = (-1);
#ifndef SYSLOG

					time(&timer);
					dt = localtime(&timer);
					fprintf(lg, "%s (%d/%d/%d %02d:%02d:%02d)", pw->pw_name, dt->tm_mon + 1, dt->tm_mday, dt->tm_year,
							dt->tm_hour, dt->tm_min, dt->tm_sec);
					while (++kk < argc)
						fprintf(lg, "%s ", argv[kk]);
#else
					sprintf(ebuf, "(%s)", pw->pw_name);
					while (++kk < argc)
					{
						strcat(ebuf, " ");
						strcat(ebuf, argv[kk]);
					}
					syslog_entry(ebuf, 1);
#endif
				}
#endif
#ifdef CHECK_ACCESS
				if (!check_access(argc, argv))
				{
					/*- One or more of the files is inaccessible */
					while (--argc >= 0)
						free(argv[argc]);
					fprintf(stderr, "Consult permission not set\n");
#ifdef LOGGING
					logit('-');
#endif
					return (TNL);
				}
#endif /*- CHECK ACCESS */
			}
			if (makepipe)
			{
				if (pipe(pfd) == -1)
					perror("pipe");
				*pipepfd = pfd[1];
				srcfd = pfd[0];
			}
			pid = invoke(argc, argv, srcfd, srcfile, dstfd, dstfile, append, term == TAMP);
#ifdef LOGGING
			if (argc > 0 && pid == 0)	/*- Failed without a handler */
				logit('-');
#endif
			if ((token != TPIPE) && (pid != -1))
				*waitpid = pid;
			if (argc == 0 && (token != TNL || srcfd > 1))
				fprintf(stderr, "Missing command\n");
			while (--argc >= 0)
				free(argv[argc]);
			return (term);
		case TEOF:
			logout();
			exit(0);
		}	/*- of switch */
	}	/*- of while (1) */
}	/*- of command */

#ifndef HAVE_GETHOSTNAME
gethostname(host, len)
	char           *host;
	int             len;
{
	struct utsname  name;

	uname(&name);
	strncpy(host, name.nodename, len);
	return 0;
}
#endif

void
do_prompt()
{
	char            cwd[MAXPATHLEN];

	if (!prompt)
		return;
	if (!getcwd(cwd, sizeof(cwd)))
		strncpy(cwd, "unknown", MAXPATHLEN);
	printf("%s.%s:%s>", host, pw->pw_name, cwd);
	fflush(stdout);
	/*- 
	 * Prints a prompt like:
	 * beta,/u0/mcn #> 
	 */
}

void
pwdcopy(from, to)
	struct passwd  *from, *to;
{
	to->pw_name = (char *) malloc(strlen(from->pw_name) + 1);
	strcpy(to->pw_name, from->pw_name);
	to->pw_uid = from->pw_uid;
	to->pw_gid = from->pw_gid;
	to->pw_gecos = (char *) malloc(strlen(from->pw_gecos) + 1);
	strcpy(to->pw_gecos, from->pw_gecos);
	to->pw_dir = (char *) malloc(strlen(from->pw_dir) + 1);
	strcpy(to->pw_dir, from->pw_dir);
	to->pw_shell = (char *) malloc(strlen(from->pw_shell) + 1);
	strcpy(to->pw_shell, from->pw_shell);
	return;
}

void
grpcopy(from, to)
	struct group   *from, *to;
{
	/*
	 * Only copy what we use 
	 */
	to->gr_name = (char *) malloc(strlen(from->gr_name));
	memcpy(to->gr_name, from->gr_name, strlen(from->gr_name));
}

int
ingroup(what)
	char           *what;

{
	GETGROUPS_T     groups[NGROUPS];
	struct group   *gr;
	int             ngroups, i;
	int             found = 0;

	ngroups = getgroups(NGROUPS, groups);
	for (i = 0; i < ngroups; i++)
	{
		gr = getgrgid(groups[i]);
		if (gr != NULL)
			if (!strcmp(gr->gr_name, what))
				found = 1;
	}
	return (1 - found);
}

void
get_table(name, group)
	char           *name;
	char           *group;

{
	FILE           *table;
	char           *ptr;
	char           *who = (char *) malloc(16);	/*- Number of chars in the login name */
	char            dummy[255];
	int             i, x;
	char           *prog = (char *) malloc(MAXPATHLEN);
	char           *path = (char *) malloc(MAXPATHLEN);
	int             bp;

	getEnvConfigStr(&ptr, "TABLE_NAME", TABLE_NAME);
	strncpy(work_table, ptr, sizeof(work_table));
	bp = strlen(work_table) - 2;
	if (!strncmp("{}", work_table + bp, 2))
	{
		work_table[bp] = '\0';
		strcat(work_table, host);
	}
	i = 0;
	if ((table = fopen(work_table, "r")) == NULL)
	{
		fprintf(stderr, "TABLE_NAME=%s\n", work_table);
		perror("Can't open command Matrix! (FATAL)");
		exit(1);
	}
	while (!feof(table))
	{
		do
		{
			if ((fgets(dummy, 255, table) == NULL) && ferror(table))
			{
				fprintf(stderr, "TABLE_NAME=%s, Read error. [%s]\n", work_table, strerror(errno));
				perror("Osh terminated");
				exit(1);
			}
		} while (*dummy == '#');	/*- Skip comments */
		if (sscanf(dummy, "%16s", who) != 1)	/*- fgets saves the \n */
		{
			fprintf(stderr, "TABLE_NAME=%s, Read error.\n", work_table);
			perror("Osh terminated");
			exit(1);
		}
		if ((strcmp(who, name) == 0) || (ingroup(who) == 0) || (strcmp(who, "ALL") == 0))
		{
			do
			{
				if (!fgets(dummy, 255, table))	/*- This should get the { */
				{
					fprintf(stderr, "TABLE_NAME=%s, Read error.\n", work_table);
					perror("Osh terminated");
					exit(1);
				}
			}
			while (*dummy == '#');
			if (!fgets(dummy, 255, table))
			{
				fprintf(stderr, "TABLE_NAME=%s, Read error.\n", work_table);
				perror("Osh terminated");
				exit(1);
			}
			while (strncmp(dummy, "}", 1) != 0 && !feof(table))
			{
				if (*dummy == '#')
					continue;	/*- Comment */
				if ((*dummy == '+') || (*dummy == '-'))
				{				/*- ACL Entry */
					int             temp = 0;

					while (FileList[temp] != NULL)
						temp++;
					/*
					 * sscanf(dummy,"%s",prog); Get rid of the \n 
					 */
					strncpy(prog, dummy, strlen(dummy));
					prog[strlen(dummy) - 1] = '\0';
					FileList[temp] = (char *) malloc(strlen(prog) + 1);
					strcpy(FileList[temp], prog);
					FileList[temp + 1] = NULL;
				} else
				{
					sscanf(dummy, "%s %s", prog, path);
					if ((Table[i].prog_name = (char *) malloc(strlen(prog) + 1)) == NULL)
					{
						perror("malloc");
						exit(1);
					}
					if ((Table[i].path = (char *) malloc(strlen(path) + 1)) == NULL)
					{
						perror("malloc2");
						exit(1);
					}
					strcpy(Table[i].prog_name, prog);
					strcpy(Table[i].path, path);
					Table[i].handler = execute;
					for (x = 0; x < NUMINT; x++)
						if (strcmp(Table[i].prog_name, Internal[x].prog_name) == 0)
						{
							Table[i].handler = Internal[x].handler;
							break;
						}
					if (strcmp(Table[i].path, "NULL") == 0)
						Table[i].path = NULL;
					i++;
				}		/*- of if not an acl entry */
				if (!fgets(dummy, 255, table)) ;
			}	/*- Of while reading the table */
		}	/*- Of if a match */
	}	/*- Of while */
	if (i == 0)
	{
		fprintf(stderr, "Can't load user/group from matrix! (FATAL)\n");
		exit(1);
	}
	NUMENTRY = i - 1;
	fclose(table);
}

int
main(argc, argv)				/*- real shell */
	int             argc;
	char           *argv[];
{
	int             pid;
	TOKEN           term;
	extern void     ignoresig();
	extern void     waitfor();
	struct group   *gp, gph;
	time_t          timer;
	struct tm      *dt;
	char           *date_time;
	char           *t;
	char            buf[40];
	int             i;
	char           *x, *ptr;
#ifdef HAVE_SYS_UTSNAME
	struct utsname  un;
#endif

#ifndef HAVE_SYS_UTSNAME
	gethostname(host, 16);
	if ((x = strchr(host, '.')) != NULL)
		*x = 0;					/*- Kill FQDN */
#else
	uname(&un);
	strncpy(host, un.nodename, 16);
#endif

	ignoresig();
	AliasCounter = 0;
#ifndef COMPILE_TABLE
	FileList[0] = NULL;
#endif
	t = getenv("TERM");
	if (!strcmp(t, "network") && argc == 1)
	{
		printf("Terminal Type (%s): ", t);
		if (!fgets(buf, 7, stdin))
		{
			perror("fgets");
			return (1);
		}
		for (i = 0; i < 100; i++)
			if (!strncmp(environ[i], "TERM=", 5))
			{
				strncpy(environ[i] + 5, buf, strlen(buf));
				*(environ[i] + 5 + strlen(buf) - 1) = '\0';
				break;
			}
	}
	pwdcopy(getpwuid(getuid()), &pwh);
	pw = &pwh;
	grpcopy(getgrgid(pw->pw_gid), &gph);
	gp = &gph;
	if (argc == 1)
	{
		printf("%s (%s)\n", pw->pw_gecos, pw->pw_name);
		printf("Operator Shell version %s\n", VERSION);
	}
	time(&timer);
	dt = localtime(&timer);
	if(!(ptr = getenv("HOME")))
	{
		fprintf(stderr, "HOME Not set\n");
		exit(1);
	}
	snprintf(LogFile, sizeof(LogFile), "%s/logs", ptr);
	if(access(LogFile, F_OK) && mkdir(LogFile, 0700))
	{
		fprintf(stderr, "mkdir: %s: %s\n", LogFile, strerror(errno));
		exit(1);
	}
	snprintf(LogFile, sizeof(LogFile), "%s/logs/oshlog.%02d%02d%02d%02d%02d%02d", ptr, dt->tm_mday, dt->tm_mon, dt->tm_year + 1900, dt->tm_hour, dt->tm_min, dt->tm_sec);
	if ((lg = fopen(LogFile, "a")) == NULL)
	{
		perror(LogFile);
		exit(1);
	}
	setbuf(lg, 0);
#ifdef LOGGING
# ifndef SYSLOG
	date_time = ctime(&timer);
	fprintf(lg, "LOGIN: %s ran osh at %s", pw->pw_name, date_time);
# else
	{
		char            ebuf[80];

		sprintf(ebuf, "LOGIN: %s ran osh", pw->pw_name);
		syslog_entry(ebuf, 0);
	}
# endif
#endif

#ifndef COMPILE_TABLE
	if (gp == NULL)
		get_table(pw->pw_name, NULL);
	else
		get_table(pw->pw_name, gp->gr_name);
#endif
	iopen(argc, argv);
	do_prompt();
	while (1)
	{
		term = command(&pid, FALSE, NULL);
		if (term != TAMP && pid != 0)
			waitfor(pid);
		if (term == TNL)
			do_prompt();
		/*-
		 * for (fd=3;fd<20;fd++) (void)fclose(fd); 
		 */
	}

}

#ifndef MAXSIG
#define MAXSIG 19
#endif

void
statusprt(pid, status)
	int             pid, status;
{

	int             code;
	static char    *sigmsg[] = {
		"",
		"Hangup",
		"Interrupt",
		"Quit",
		"Illegal Instruction",
		"Trace Trap",
		"IOT instruction",
		"EMT instruction",
		"Floating point exception",
		"Kill",
		"Bus error",
		"Segmentation violation",
		"Bad arg to system call",
		"Write on pipe",
		"Alarm clock",
		"Terminate signal",
		"User signal 1",
		"User signal 2",
		"Death of child",
		"Power fail"
	};

	if (status != 0 && pid != 0)
		printf("Process %d: ", pid);
	if (lowbyte(status) == 0)
	{
		if ((code = highbyte(status)) != 0)
		/*
		 * printf("Exit code %d\n",code); 
		 */
		/*
		 * XXX - LOG THE EXIT CODE somehow? Pug suggests a new log entry 
		 */
			logit('-');
		else
			logit('+');
	} else
	{
		if ((code = status & 0177) <= MAXSIG)
			printf("%s", sigmsg[code]);
		else
			printf("Signal #%d", code);
		if ((status & 0200) == 0200)
			printf("-core dumped");
		printf("\n");
	}
}

void
waitfor(pid)
	int             pid;
{
	int             wpid, status;

	while ((wpid = wait(&status)) != pid && wpid != -1)
		statusprt(wpid, status);
	if (wpid == pid)
		statusprt(0, status);
}

void
getversion_osh_c()
{
	printf("%s\n", rcsid);
}
