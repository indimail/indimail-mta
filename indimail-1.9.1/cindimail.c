/*
 * $Log: cindimail.c,v $
 * Revision 2.8  2012-08-04 08:28:48+05:30  Cprogrammer
 * replaced dupstr() with strdup()
 *
 * Revision 2.7  2008-07-25 16:46:55+05:30  Cprogrammer
 * fixes for Darwin
 *
 * Revision 2.6  2008-07-13 19:05:49+05:30  Cprogrammer
 * fix for Mac
 *
 * Revision 2.5  2003-10-01 03:09:28+05:30  Cprogrammer
 * added PARAMS proto
 *
 * Revision 2.4  2003-04-24 18:09:12+05:30  Cprogrammer
 * changed columns to 5
 *
 * Revision 2.3  2003-04-12 00:23:18+05:30  Cprogrammer
 * replaced admin_command with structure adminCommands for help on all indimail programs
 *
 * Revision 2.2  2003-03-24 19:23:57+05:30  Cprogrammer
 * added indimail programs to list of allowed programs
 *
 * Revision 2.1  2003-03-07 00:42:30+05:30  Cprogrammer
 * command line administration client
 *
 *
 * IndiMail.c -- A tiny application which demonstrates how to use the
 * GNU Readline library.  This application interactively allows users
 * to manipulate files and their modes. 
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: cindimail.c,v 2.8 2012-08-04 08:28:48+05:30 Cprogrammer Stab mbhangui $";
#endif

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/errno.h>
#if !defined (PARAMS)
#  if defined (__STDC__) || defined (__GNUC__) || defined (__cplusplus)
#    define PARAMS(protos) protos
#  else
#    define PARAMS(protos) ()
#  endif
#endif
#ifdef DARWIN
#include "readline/rltypedefs.h"
#include "readline/readline.h"
#include "readline/history.h"
#else
#include <readline/rltypedefs.h>
#include <readline/readline.h>
#include <readline/history.h>
#endif

void            initialize_readline();
int             execute_line(char *);
int             valid_argument(char *, char *);
void            too_dangerous(char *);

extern char    *getwd();
extern char   **completion_matches PARAMS((char *, rl_compentry_func_t *));

/*
 * The names of functions that actually do the manipulation. 
 */
int             com_list(), com_view(), com_rename(), com_stat(), com_pwd();
int             com_delete(), com_help(), com_cd(), com_quit(), com_exec();
/*
 * A structure which contains information on the commands this program
 * can understand. 
 */

typedef struct
{
	char           *name;		/*- User printable name of the function.  */
	Function       *func;		/*- Function to call to do the job.  */
	char           *doc;		/*- Documentation for this function.  */
} COMMAND;

COMMAND         commands[] = {
	{"cd", com_cd, "Change to directory DIR"},
	{"rm", com_delete, "Delete FILE"},
	{"help", com_help, "Display this text"},
	{"?", com_help, "Synonym for `help'"},
	{"ls", com_list, "List files in DIR"},
	{"pwd", com_pwd, "Print the current working directory"},
	{"quit", com_quit, "Quit using IndiMail"},
	{"mv", com_rename, "Rename FILE to NEWNAME"},
	{"stat", com_stat, "Print out statistics on FILE"},
	{"view", com_view, "View the contents of FILE"},
	{"vdeldomain", com_delete, "Delete Domain" },
	{"vi", com_view, "View the contents of FILE"},
	{(char *) NULL, (Function *) NULL, (char *) NULL}
};


/*
 * * Forward declarations. 
 */
char           *stripwhite();
COMMAND        *find_command();

/*
 * * The name of this program, as taken from argv[0]. 
 */
char           *progname, *cmdPtr;
/*
 * * When non-zero, this global means the user is done using this program. 
 */
int             done;

int
main(argc, argv)
	int             argc;
	char          **argv;
{
	char           *line, *s;

	progname = argv[0];
	initialize_readline();		/*- Bind our completer.  */

	/*- Loop reading and executing lines until the user quits.  */
	for (; done == 0;)
	{
		if (!(line = readline("IndiMail> ")))
			break;
		/*
		 * Remove leading and trailing whitespace from the line.
		 * Then, if there is anything left, add it to the history list
		 * and execute it. 
		 */
		s = stripwhite(line);
		if (*s)
		{
			add_history(s);
			execute_line(s);
		}
		free(line);
	}
	exit(0);
}

/*
 * Execute a command line. 
 */
int
execute_line(line)
	char           *line;
{
	register int    i;
	COMMAND        *command;
	char           *word;

	/*- Isolate the command word.  */
	i = 0;
	while (line[i] && whitespace(line[i]))
		i++;
	word = line + i;
	while (line[i] && !whitespace(line[i]))
		i++;
	if (line[i])
		line[i++] = '\0';
	if (!(command = find_command(word)))
	{
		fprintf(stderr, "%s: No such command for IndiMail.\n", word);
		return (-1);
	}
	/*- Get argument to command, if any.  */
	while (whitespace(line[i]))
		i++;
	word = line + i;
	/*- Call the function.  */
	return ((*(command->func)) (word));
}

/*
 * Look up NAME as the name of a command, and return a pointer to that
 * command.  Return a NULL pointer if NAME isn't a command name. 
 */
COMMAND        *
find_command(name)
	char           *name;
{
	register int    i;
	char           *cmdptr1, *cmdptr2;
	char            cmdName[MAX_BUFF];
	static COMMAND  temp;

	for (i = 0; commands[i].name; i++)
	{
		if (!strcmp(name, commands[i].name))
			return (&commands[i]);
	}
	for (cmdptr1 = name; *cmdptr1 && isspace((int) *cmdptr1); cmdptr1++);
	for (cmdptr2 = cmdName; *cmdptr1 && !isspace((int) *cmdptr1); *cmdptr2++ = *cmdptr1++);
	*cmdptr2 = 0;
	if ((cmdptr1 = strrchr(cmdName, '/')))
		cmdptr1++;
	else
		cmdptr1 = cmdName;
	cmdPtr = 0;
	for (i = 0; adminCommands[i].name; i++)
	{
		if ((cmdptr2 = strrchr(adminCommands[i].name, '/')))
			cmdptr2++;
		else
			cmdptr2 = adminCommands[i].name;
		if (!strncmp(cmdptr1, cmdptr2, slen(cmdptr2) + 1))
		{
			cmdPtr = name;
			temp.name = name;
			temp.func = com_exec;
			temp.doc = adminCommands[i].doc;
			return (&temp);
		}
	}
	return ((COMMAND *) NULL);
}

/*
 * Strip whitespace from the start and end of STRING.  Return a pointer
 * into STRING. 
 */
char           *
stripwhite(string)
	char           *string;
{
	register char  *s, *t;

	for (s = string; whitespace(*s); s++) ;
	if (*s == 0)
		return (s);
	t = s + strlen(s) - 1;
	while (t > s && whitespace(*t))
		t--;
	*++t = '\0';
	return s;
}

/*
 * Interface to Readline Completion                
 */

char           *command_generator();
char          **fileman_completion();

/*
 * Tell the GNU Readline library how to complete.  We want to try to complete
 * on command names if this is the first word in the line, or on filenames
 * if not. 
 */
void
initialize_readline()
{
	/*
	 * Allow conditional parsing of the ~/.inputrc file. 
	 */
	rl_readline_name = "IndiMail";
	/*
	 * Tell the completer that we want a crack first. 
	 */
	rl_attempted_completion_function = (CPPFunction *) fileman_completion;
}

/*
 * Attempt to complete on the contents of TEXT.  START and END bound the
 * region of rl_line_buffer that contains the word to complete.  TEXT is
 * the word to complete.  We can use the entire contents of rl_line_buffer
 * in case we want to do some simple parsing.  Return the array of matches,
 * or NULL if there aren't any. 
 */
char          **
fileman_completion(text, start, end)
	char           *text;
	int             start, end;
{
	char          **matches;

	matches = (char **) NULL;

	/*
	 * If this word is at the start of the line, then it is a command
	 * to complete.  Otherwise it is the name of a file in the current
	 * directory. 
	 */
	if (start == 0)
		matches = completion_matches(text, (rl_compentry_func_t *) command_generator);
	return (matches);
}

/*
 * Generator function for command completion.  STATE lets us know whether
 * to start from scratch; without any state (i.e. STATE == 0), then we
 * start at the top of the list. 
 */
char           *
command_generator(text, state)
	char           *text;
	int             state;
{
	static int      list_index, len;
	char           *name;

	/*
	 * If this is a new word to complete, initialize now.  This includes
	 * saving the length of TEXT for efficiency, and initializing the index
	 * variable to 0. 
	 */
	if (!state)
	{
		list_index = 0;
		len = strlen(text);
	}
	/*- Return the next name which partially matches from the command list.  */
	while ((name = commands[list_index].name))
	{
		list_index++;
		if (strncmp(name, text, len) == 0)
			return (strdup(name));
	}
	/*- If no names matched, then return NULL.  */
	return ((char *) NULL);
}

/*
 * String to pass to system ().  This is for the LIST, VIEW and RENAME
 * commands. 
 */
static char     syscom[1024];

/*- List the file(s) named in arg.  */
int
com_list(arg)
	char           *arg;
{
	if (!arg)
		arg = "";
	snprintf(syscom, 1024, "ls %s", arg);
	syscom[1023] = 0;
	return (system(syscom));
}

int
com_view(arg)
	char           *arg;
{
	if (!valid_argument("view", arg))
		return 1;

	snprintf(syscom, 1024, "less %s", arg);
	syscom[1023] = 0;
	return (system(syscom));
}

int
com_rename(arg)
	char           *arg;
{
	too_dangerous("rename");
	return (1);
}

int
com_stat(arg)
	char           *arg;
{
	struct stat     finfo;

	if (!valid_argument("stat", arg))
		return (1);
	if (stat(arg, &finfo) == -1)
	{
		perror(arg);
		return (1);
	}
	printf("Statistics for `%s':\n", arg);
	printf("%s has %ld link%s, and is %ld byte%s in length.\n", arg, finfo.st_nlink,
	 (finfo.st_nlink == 1) ? "" : "s", (long) finfo.st_size, (finfo.st_size == 1) ? "" : "s");
	printf("Inode Last Change at: %s", ctime(&finfo.st_ctime));
	printf("      Last access at: %s", ctime(&finfo.st_atime));
	printf("    Last modified at: %s", ctime(&finfo.st_mtime));
	return (0);
}

int
com_delete(arg)
	char           *arg;
{
	too_dangerous("delete");
	return (1);
}

/*
 * Print out help for ARG, or for all of the commands if ARG is
 * not present. 
 */
int
com_help(arg)
	char           *arg;
{
	register int    i;
	int             printed = 0;
	FILE           *fp;
	char           *ptr;

	if(arg && *arg)
		fp = stdout;
	else
	if(!(fp = popen("less", "w")))
		fp = stdout;
	for (i = 0; commands[i].name; i++)
	{
		if (!*arg || !strcmp(arg, commands[i].name))
		{
			fprintf(fp, "%-24s %s.\n", commands[i].name, commands[i].doc);
			printed++;
		}
	}
	for (i = 0; adminCommands[i].name; i++)
	{
		if((ptr = strrchr(adminCommands[i].name, '/')))
			ptr++;
		else
			ptr = adminCommands[i].name;
		if(!*arg || !strncmp(ptr, arg, MAX_BUFF))
		{
			fprintf(fp, "%-24s %s.\n", ptr, adminCommands[i].doc);
			printed++;
		}
	}
	if (!printed)
	{
		printf("No commands match `%s'.  Possibilties are:\n", arg);
		for (i = 0; commands[i].name; i++)
		{
			/*- Print in five columns.  */
			if (printed == 5)
			{
				printed = 0;
				fprintf(fp, "\n");
			}
			fprintf(fp, "%-16s ", commands[i].name);
			printed++;
		}
		for (i = 0; adminCommands[i].name; i++)
		{
			if((ptr = strrchr(adminCommands[i].name, '/')))
				ptr++;
			else
				ptr = adminCommands[i].name;
			if (printed == 5)
			{
				printed = 0;
				fprintf(fp, "\n");
			}
			fprintf(fp, "%-16s ", ptr);
			printed++;
		}
		if (printed)
			fprintf(fp, "\n");
	}
	if(fp != stdout)
		pclose(fp);
	return (0);
}

/*
 * Change to the directory ARG. 
 */
int
com_cd(arg)
	char           *arg;
{
	if (chdir(arg) == -1)
	{
		perror(arg);
		return 1;
	}

	com_pwd("");
	return (0);
}

/*
 * Print out the current working directory. 
 */
int
com_pwd(ignore)
	char           *ignore;
{
	char            dir[1024], *s;

	if (!(s = getcwd(dir, 1024)))
	{
		printf("Error getting pwd: %s\n", dir);
		return 1;
	}
	printf("Current directory is %s\n", dir);
	return 0;
}

int
com_exec(arg)
	char           *arg;
{
	if (!arg)
		arg = "";
	snprintf(syscom, 1024, "%s %s", cmdPtr, arg);
	syscom[1023] = 0;
	return (system(syscom));
}

/*
 * The user wishes to quit using this program.  Just set DONE non-zero. 
 */
int
com_quit(arg)
	char           *arg;
{
	done = 1;
	return (0);
}

/*
 * Function which tells you that you can't do this. 
 */
void
too_dangerous(caller)
	char           *caller;
{
	fprintf(stderr, "%s: Too dangerous for me to distribute.  Write it yourself.\n", caller);
}

/*
 * Return non-zero if ARG is a valid argument for CALLER, else print
 * an error message and return zero. 
 */
int
valid_argument(caller, arg)
	char           *caller, *arg;
{
	if (!arg || !*arg)
	{
		fprintf(stderr, "%s: Argument required.\n", caller);
		return (0);
	}

	return (1);
}

void
getversion_cindimail_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
