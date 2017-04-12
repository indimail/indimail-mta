/*-
 * $NetBSD: fortune.c,v 1.8 1995/03/23 08:28:40 cgd Exp $  
 * Copyright (c) 1986, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Ken Arnold.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * Modified September, 1995, Amy A. Lewis
 * 1: removed all file-locking dreck.  Unnecessary
 * 2: Fixed bug that made fortune -f report a different list than
 *    fortune with any other parameters, or none, and which forced
 *    the program to read only one file (named 'fortunes')
 * 3: removed the unnecessary print_file_list()
 * 4: Added "OFFDIR" to pathnames.h as the directory in which offensive
 *    fortunes are kept.  This considerably simplifies our life by
 *    permitting us to dispense with a lot of silly tests for the string
 *    "-o" at the end of a filename.
 * 5: I think the problems with trying to find filenames were fixed by
 *    the change in the way that offensive files are defined.  Two birds,
 *    one stone!
 * 6: Calculated probabilities for all files, so that -f will print them.
 *
 * Changes Copyright (c) 1997 Dennis L. Clark.  All rights reserved.
 *
 *    The changes in this file may be freely redistributed, modified or
 *    included in other software, as long as both the above copyright
 *    notice and these conditions appear intact.
	*
 * Modified May 1997, Dennis L. Clark (dbugger@progsoc.uts.edu.au)
 *  + Various portability fixes
 *  + Percent selection of files with -a now works on datafiles which
 *    appear in both unoffensive and offensive directories (see man page
 *    for details)
 *  + The -s and -l options are now more consistant in their
 *    interpretation of fortune length
 *  + The -s and -l options can now be combined wit the -m option
 */

#if 0				/*- comment out the stuff here, and get rid of silly warnings */
#ifndef lint
static char     copyright[] = "@(#) Copyright (c) 1986, 1993\n\
	The Regents of the University of California.  All rights reserved.\n";
#endif /*- not lint */
#ifndef lint
#if 0
static char     sccsid[] = "@(#)fortune.c	8.1 (Berkeley) 5/31/93";
#else
static char     rcsid[] = "$NetBSD: fortune.c,v 1.8 1995/03/23 08:28:40 cgd Exp $";
#endif
#endif /*- not lint */
#endif /*- killing warnings */

#define		PROGRAM_NAME		"fortune"
#define		PROGRAM_VERSION		"1.0"

#include "config.h"
#include	<stdio.h>
#ifdef HAVE_STRING_H
#include	<string.h>
#endif
#include	<time.h>
#ifdef HAVE_STDLIB_H
#include	<stdlib.h>
#endif
#ifdef HAVE_UNISTD_H
#include	<unistd.h>
#endif
#include	<fcntl.h>
#include	<dirent.h>
#include	<ctype.h>
#include	<assert.h>
#ifdef HAVE_SYS_STAT_H
#include	<sys/stat.h>
#endif
#include	<sys/types.h>
#ifdef HAVE_NETINET_IN_H
#include	<netinet/in.h>
#endif

/*
 * This makes GNU libc to prototype the BSD regex functions 
 */
#ifdef BSD_REGEX
#define	_REGEX_RE_COMP
#endif

#ifdef HAVE_REGEX_H
#include	<regex.h>
#endif
#ifdef HAVE_REGEXP_H
#include	<regexp.h>
#endif
#ifdef HAVE_RX_H
#include	<rx.h>
#endif
#include "strfile.h"

#define	TRUE	1
#define	FALSE	0
#define	bool	short
#define	MINW	6				/*- minimum wait if desired */
#define	CPERS	20				/*- # of chars for each sec */
#define	POS_UNKNOWN	((off_t) -1)	/*- pos for file unknown */
#define	NO_PROB		(-1)		/*- no prob specified for file */

#ifdef DEBUG
#define	DPRINTF(l,x)	if (Debug >= l) fprintf x;
#undef	NDEBUG
#else
#define	DPRINTF(l,x)
#define	NDEBUG	1
#endif

typedef struct fd
{
	int             percent;
	int             fd, datfd;
	off_t           pos;
	FILE           *inf;
	char           *name;
	char           *path;
	char           *datfile, *posfile;
	bool            read_tbl;
	bool            was_pos_file;
	STRFILE         tbl;
	int             num_children;
	struct fd      *child, *parent;
	struct fd      *next, *prev;
} FILEDESC;

bool            Found_one;				/*- did we find a match? */
bool            Find_files = FALSE;		/*- just find a list of proper fortune files */
bool            Wait = FALSE;			/*- wait desired after fortune */
bool            Short_only = FALSE;		/*- short fortune desired */
bool            Long_only = FALSE;		/*- long fortune desired */
bool            Offend = FALSE;			/*- offensive fortunes only */
bool            All_forts = FALSE;		/*- any fortune allowed */
bool            Equal_probs = FALSE;	/*- scatter un-allocated prob equally */

#ifndef NO_REGEX
bool            Match = FALSE;	/*- dump fortunes matching a pattern */
#endif
#ifdef DEBUG
bool            Debug = FALSE;	/*- print debug messages */
#endif

unsigned char  *Fortbuf = NULL;					/*- fortune buffer for -m */
int             Fort_len = 0, Spec_prob = 0, 	/*- total prob specified on cmd line */
                Num_files, Num_kids, 			/*- totals of files and children. */
                SLEN = 160; 					/*- max. characters in a "short" fortune */
off_t           Seekpts[2];					 	/*- seek pointers to fortunes */
FILEDESC       *File_list = NULL,				/*- Head of file list */
               *File_tail = NULL,				/*- Tail of file list */
               *Fortfile;						/*- Fortune file to use */
STRFILE         Noprob_tbl;						/*- sum of data for all no prob files */

#ifdef BSD_REGEX
#define	RE_COMP(p)	re_comp(p)
#define	BAD_COMP(f)	((f) != NULL)
#define	RE_EXEC(p)	re_exec(p)
#else
#ifdef POSIX_REGEX
#define	RE_COMP(p)	regcomp(&Re_pat, (p), REG_NOSUB)
#define	BAD_COMP(f)	((f) != 0)
#define	RE_EXEC(p)	(regexec(&Re_pat, (p), 0, NULL, 0) == 0)
regex_t         Re_pat;
#else
#define NO_REGEX
#endif /*- POSIX_REGEX */
#endif /*- BSD_REGEX */

int             add_dir(register FILEDESC *);

char           *
program_version(void)
{
	static char     buf[BUFSIZ];
	(void) sprintf(buf, "%s version %s", PROGRAM_NAME, PROGRAM_VERSION);
	return buf;
}

void
usage(void)
{
	(void) fprintf(stderr, "%s\n", program_version());
	(void) fprintf(stderr, "fortune [-a");
#ifdef	DEBUG
	(void) fprintf(stderr, "D");
#endif /*- DEBUG */
	(void) fprintf(stderr, "f");
#ifndef	NO_REGEX
	(void) fprintf(stderr, "i");
#endif /*- NO_REGEX */
	(void) fprintf(stderr, "losw]");
#ifndef	NO_REGEX
	(void) fprintf(stderr, " [-m pattern]");
#endif /*- NO_REGEX */
	(void) fprintf(stderr, " [-n number] [ [#%%] file/directory/all]\n");
	exit(1);
}

#define	STR(str)	((str) == NULL ? "NULL" : (str))


/*-
 * calc_equal_probs:
 *      Set the global values for number of files/children, to be used
 * in printing probabilities when listing files
 */
void
calc_equal_probs(void)
{
	FILEDESC       *fiddlylist;

	Num_files = Num_kids = 0;
	fiddlylist = File_list;
	while (fiddlylist != NULL)
	{
		Num_files++;
		Num_kids += fiddlylist->num_children;
		fiddlylist = fiddlylist->next;
	}
}

/*-
 * print_list:
 *      Print out the actual list, recursively.
 */
void
print_list(register FILEDESC * list, int lev)
{
	while (list != NULL)
	{
		fprintf(stderr, "%*s", lev * 4, "");
		if (list->percent == NO_PROB)
		{
			if (!Equal_probs)
				/*
				 * This, with some changes elsewhere, gives proper percentages for every case
				 * fprintf(stderr, "___%%"); 
				 */
				fprintf(stderr, "%5.2f%%", (100.0 - Spec_prob) * list->tbl.str_numstr / Noprob_tbl.str_numstr);
			else
			if (lev == 0)
				fprintf(stderr, "%5.2f%%", 100.0 / Num_files);
			else
				fprintf(stderr, "%5.2f%%", 100.0 / Num_kids);
		} else
			fprintf(stderr, "%5.2f%%", 1.0 * list->percent);
		fprintf(stderr, " %s", STR(list->name));
		DPRINTF(1, (stderr, " (%s, %s, %s)\n", STR(list->path), STR(list->datfile), STR(list->posfile)));
		putc('\n', stderr);
		if (list->child != NULL)
			print_list(list->child, lev + 1);
		list = list->next;
	}
}

#ifndef	NO_REGEX
/*-
 * conv_pat:
 *      Convert the pattern to an ignore-case equivalent.
 */
char           *
conv_pat(register char *orig)
{
	register char  *sp;
	register unsigned int cnt;
	register char  *new;

	cnt = 1;	/*- allow for '\0' */
	for (sp = orig; *sp != '\0'; sp++)
		if (isalpha(*sp))
			cnt += 4;
		else
			cnt++;
	if ((new = malloc(cnt)) == NULL)
	{
		fprintf(stderr, "pattern too long for ignoring case\n");
		exit(1);
	}

	for (sp = new; *orig != '\0'; orig++)
	{
		if (islower(*orig))
		{
			*sp++ = '[';
			*sp++ = *orig;
			*sp++ = toupper(*orig);
			*sp++ = ']';
		} else
		if (isupper(*orig))
		{
			*sp++ = '[';
			*sp++ = *orig;
			*sp++ = tolower(*orig);
			*sp++ = ']';
		} else
			*sp++ = *orig;
	}
	*sp = '\0';
	return new;
}
#endif /*- NO_REGEX */

/*-
 * do_malloc:
 *      Do a malloc, checking for NULL return.
 */
void           *
do_malloc(unsigned int size)
{
	void           *new;

	if ((new = malloc(size)) == NULL)
	{
		(void) fprintf(stderr, "fortune: out of memory.\n");
		exit(1);
	}
	return new;
}

/*
 * do_free:
 *      Free malloc'ed space, if any.
 */
void
do_free(void *ptr)
{
	if (ptr != NULL)
		free(ptr);
}

/*-
 * copy:
 *      Return a malloc()'ed copy of the string
 */
char           *
copy(char *str, unsigned int len)
{
	char           *new, *sp;

	new = do_malloc(len + 1);
	sp = new;
	do
	{
		*sp++ = *str;
	}
	while (*str++);
	return new;
}

/*-
 * new_fp:
 *      Return a pointer to an initialized new FILEDESC.
 */
FILEDESC       *
new_fp(void)
{
	register FILEDESC *fp;

	fp = (FILEDESC *) do_malloc(sizeof *fp);
	fp->datfd = -1;
	fp->pos = POS_UNKNOWN;
	fp->inf = NULL;
	fp->fd = -1;
	fp->percent = NO_PROB;
	fp->read_tbl = FALSE;
	fp->next = NULL;
	fp->prev = NULL;
	fp->child = NULL;
	fp->parent = NULL;
	fp->datfile = NULL;
	fp->posfile = NULL;
	return fp;
}

/*-
 * is_dir:
 *      Return TRUE if the file is a directory, FALSE otherwise.
 */
int
is_dir(char *file)
{
	auto struct stat sbuf;

	if (stat(file, &sbuf) < 0)
		return FALSE;
	return (sbuf.st_mode & S_IFDIR);
}

/*-
 * is_fortfile:
 *      Return TRUE if the file is a fortune database file.  We try and
 *      exclude files without reading them if possible to avoid
 *      overhead.  Files which start with ".", or which have "illegal"
 *      suffixes, as contained in suflist[], are ruled out.
 */
int
is_fortfile(char *file, char **datp, char **posp)
{
	register int    i;
	register char  *sp;
	register char  *datfile;
	static char    *suflist[] = {
		"dat", "pos", "c", "h", "p", "i", "f",
		"pas", "ftn", "ins.c", "ins,pas",
		"ins.ftn", "sml",
		NULL
	}; /*- list of "illegal" suffixes" */

	DPRINTF(2, (stderr, "is_fortfile(%s) returns ", file));
	if ((sp = strrchr(file, '/')) == NULL)
		sp = file;
	else
		sp++;
	if (*sp == '.')
	{
		DPRINTF(2, (stderr, "FALSE (file starts with '.')\n"));
		return FALSE;
	}
	if ((sp = strrchr(sp, '.')) != NULL)
	{
		sp++;
		for (i = 0; suflist[i] != NULL; i++)
			if (strcmp(sp, suflist[i]) == 0)
			{
				DPRINTF(2, (stderr, "FALSE (file has suffix \".%s\")\n", sp));
				return FALSE;
			}
	}

	datfile = copy(file, (unsigned int) (strlen(file) + 4));	/*- +4 for ".dat" */
	strcat(datfile, ".dat");
	if (access(datfile, R_OK) < 0)
	{
		free(datfile);
		DPRINTF(2, (stderr, "FALSE (no \".dat\" file)\n"));
		return FALSE;
	}
	if (datp != NULL)
		*datp = datfile;
	else
		free(datfile);
	DPRINTF(2, (stderr, "TRUE\n"));
	return TRUE;
}

/*-
 * add_file:
 *      Add a file to the file list.
 */
int
add_file(int percent, register char *file, char *dir, FILEDESC ** head, FILEDESC ** tail, FILEDESC * parent)
{
	register FILEDESC *fp;
	register int    fd;
	register char  *path;
	register bool   was_malloc;
	register bool   isdir;
	auto char      *sp;
	auto bool       found;

	if (dir == NULL)
	{
		path = file;
		was_malloc = FALSE;
	} else
	{
		path = do_malloc((unsigned int) (strlen(dir) + strlen(file) + 2));
		(void) strcat(strcat(strcpy(path, dir), "/"), file);
		was_malloc = TRUE;
	}
	if ((isdir = is_dir(path)) && parent != NULL)
	{
		if (was_malloc)
			free(path);
		return FALSE;			/*- don't recurse */
	}

	DPRINTF(1, (stderr, "trying to add file \"%s\"\n", path));
	if ((fd = open(path, 0)) < 0)
	{
		found = FALSE;
		if (dir == NULL && (strchr(file, '/') == NULL))
		{
			if (((sp = strrchr(file, '-')) != NULL) && (strcmp(sp, "-o") == 0))
			{
				/*
				 * BSD-style '-o' offensive file suffix 
				 */
				*sp = '\0';
				found = add_file(percent, file, OFFDIR, head, tail, parent);
				/*
				 * put the suffix back in for better identification later 
				 */
				*sp = '-';
			} else
			if (All_forts)
				found = (add_file(percent, file, FORTDIR, head, tail, parent) ||
						 add_file(percent, file, OFFDIR, head, tail, parent));
			else
			if (Offend)
				found = add_file(percent, file, OFFDIR, head, tail, parent);
			else
				found = add_file(percent, file, FORTDIR, head, tail, parent);
		}
		if (!found && parent == NULL && dir == NULL)
			perror(path);
		if (was_malloc)
			free(path);
		return found;
	}
	DPRINTF(2, (stderr, "path = \"%s\"\n", path));
	fp = new_fp();
	fp->fd = fd;
	fp->percent = percent;
	fp->name = file;
	fp->path = path;
	fp->parent = parent;
	if ((isdir && !add_dir(fp)) || (!isdir && !is_fortfile(path, &fp->datfile, &fp->posfile)))
	{
		if (parent == NULL)
			fprintf(stderr, "fortune:%s not a fortune file or directory\n", path);
		if (was_malloc)
			free(path);
		do_free(fp->datfile);
		do_free(fp->posfile);
		if (fp->fd >= 0)
			close(fp->fd);
		free(fp);
		return FALSE;
	}
	if (*head == NULL)
		*head = *tail = fp;
	else
	if (fp->percent == NO_PROB)
	{
		(*tail)->next = fp;
		fp->prev = *tail;
		*tail = fp;
	} else
	{
		(*head)->prev = fp;
		fp->next = *head;
		*head = fp;
	}

	return TRUE;
}

/*-
 * add_dir:
 *      Add the contents of an entire directory.
 */
int
add_dir(register FILEDESC * fp)
{
	register DIR   *dir;
	register struct dirent *dirent;
	auto FILEDESC  *tailp;
	auto char      *name;

	close(fp->fd);
	fp->fd = -1;
	if ((dir = opendir(fp->path)) == NULL)
	{
		perror(fp->path);
		return FALSE;
	}
	tailp = NULL;
	DPRINTF(1, (stderr, "adding dir \"%s\"\n", fp->path));
	fp->num_children = 0;
	while ((dirent = readdir(dir)) != NULL)
	{
		if (dirent->d_name[0] == 0)
			continue;
		name = strdup(dirent->d_name);
		if (add_file(NO_PROB, name, fp->path, &fp->child, &tailp, fp))
			fp->num_children++;
		else
			free(name);
	}
	if (fp->num_children == 0)
	{
		fprintf(stderr, "fortune: %s: No fortune files in directory.\n", fp->path);
		return FALSE;
	}
	return TRUE;
}

/*-
 * form_file_list:
 *      Form the file list from the file specifications.
 */
int
form_file_list(register char **files, register int file_cnt)
{
	register int    i, percent;
	register char  *sp;

	if (file_cnt == 0)
	{
		if (All_forts)
			return (add_file(NO_PROB, FORTDIR, NULL, &File_list, &File_tail, NULL) &
					add_file(NO_PROB, OFFDIR, NULL, &File_list, &File_tail, NULL));
		else
		if (Offend)
			return add_file(NO_PROB, OFFDIR, NULL, &File_list, &File_tail, NULL);
		else
			return add_file(NO_PROB, FORTDIR, NULL, &File_list, &File_tail, NULL);
	}
	for (i = 0; i < file_cnt; i++)
	{
		percent = NO_PROB;
		if (!isdigit(files[i][0]))
			sp = files[i];
		else
		{
			percent = 0;
			for (sp = files[i]; isdigit(*sp); sp++)
				percent = percent * 10 + *sp - '0';
			if (percent > 100)
			{
				fprintf(stderr, "percentages must be <= 100\n");
				return FALSE;
			}
			if (*sp == '.')
			{
				fprintf(stderr, "percentages must be integers\n");
				return FALSE;
			}
			/*
			 * If the number isn't followed by a '%', then
			 * it was not a percentage, just the first part
			 * of a file name which starts with digits.
			 */
			if (*sp != '%')
			{
				percent = NO_PROB;
				sp = files[i];
			} else
			if (*++sp == '\0')
			{
				if (++i >= file_cnt)
				{
					fprintf(stderr, "percentages must precede files\n");
					return FALSE;
				}
				sp = files[i];
			}
		}
		if (strcmp(sp, "all") == 0)
			sp = FORTDIR;
		if (!add_file(percent, sp, NULL, &File_list, &File_tail, NULL))
			return FALSE;
	}
	return TRUE;
}

/*
 *    This routine evaluates the arguments on the command line
 */
void
getargs(int argc, char **argv)
{
#ifndef NO_REGEX
	register int    ignore_case;
	register char  *pat = NULL;

#endif /*- NO_REGEX */
	extern char    *optarg;
	extern int      optind;
	int             ch;

#ifndef NO_REGEX
	ignore_case = FALSE;
#endif
#ifdef ENABLE_OFFENSIVE
#ifdef DEBUG
	while ((ch = getopt(argc, argv, "aDefilm:n:osvw")) != EOF)
#else
	while ((ch = getopt(argc, argv, "aefilm:n:osvw")) != EOF)
#endif /*- DEBUG */
#else
#ifdef DEBUG
	while ((ch = getopt(argc, argv, "aDefilm:n:svw")) != EOF)
#else
	while ((ch = getopt(argc, argv, "aefilm:n:svw")) != EOF)
#endif /*- DEBUG */
#endif
		switch (ch)
		{
		case 'a':			/*- any fortune */
			All_forts++;
			break;
#ifdef DEBUG
		case 'D':
			Debug++;
			break;
#endif /*- DEBUG */
		case 'e':
			Equal_probs++;	/*- scatter un-allocted prob equally */
			break;
		case 'f':			/*- find fortune files */
			Find_files++;
			break;
		case 'l':			/*- long ones only */
			Long_only++;
			Short_only = FALSE;
			break;
		case 'n':
			SLEN = atoi(optarg);
			break;
		case 'o':			/*- offensive ones only */
			Offend++;
			break;
		case 's':			/*- short ones only */
			Short_only++;
			Long_only = FALSE;
			break;
		case 'w':			/*- give time to read */
			Wait++;
			break;
#ifdef	NO_REGEX
		case 'i':			/*- case-insensitive match */
		case 'm':			/*- dump out the fortunes */
			(void) fprintf(stderr, "fortune: can't match fortunes on this system (Sorry)\n");
			exit(0);
#else /*- NO_REGEX */
		case 'm'		:	/*- dump out the fortunes */
			Match++;
			pat = optarg;
			break;
		case 'i':			/*- case-insensitive match */
			ignore_case++;
			break;
#endif /*- NO_REGEX */
		case 'v':
			(void) printf("%s\n", program_version());
			exit(0);
		case '?':
		default:
			usage();
		}
	argc -= optind;
	argv += optind;

	if (!form_file_list(argv, argc))
		exit(1);	/*- errors printed through form_file_list() */
#ifdef DEBUG
	/*
	 * if (Debug >= 1)
	 * 	print_list(File_list, 0); 
	 */
#endif /*- DEBUG */
/*- If (Find_files) print_list() moved to main */
#ifndef NO_REGEX
	if (pat != NULL)
	{
		if (ignore_case)
			pat = conv_pat(pat);
		if (BAD_COMP(RE_COMP(pat)))
		{
#ifndef REGCMP
			fprintf(stderr, "%s\n", pat);
#else /*- REGCMP */
			fprintf(stderr, "bad pattern: %s\n", pat);
#endif /*- REGCMP */
		}
	}
#endif /*- NO_REGEX */
}

/*-
 * init_prob:
 *      Initialize the fortune probabilities.
 */
void
init_prob(void)
{
	register FILEDESC *fp, *last;
	register int    percent, num_noprob, frac;

	/*
	 * Distribute the residual probability (if any) across all
	 * files with unspecified probability (i.e., probability of 0)
	 * (if any).
	 */

	percent = 0;
	num_noprob = 0;
	last = NULL;
	for (fp = File_tail; fp != NULL; fp = fp->prev)
	{
		if (fp->percent == NO_PROB)
		{
			num_noprob++;
			if (Equal_probs)
				last = fp;
		} else
			percent += fp->percent;
	}
	DPRINTF(1, (stderr, "summing probabilities:%d%% with %d NO_PROB's\n", percent, num_noprob));
	if (percent > 100)
	{
		fprintf(stderr, "fortune: probabilities sum to %d%%!\n", percent);
		exit(1);
	} else
	if (percent < 100 && num_noprob == 0)
	{
		fprintf(stderr, "fortune: no place to put residual probability (%d%%)\n", percent);
		exit(1);
	} else
	if (percent == 100 && num_noprob != 0)
	{
		fprintf(stderr, "fortune: no probability left to put in residual files\n");
		exit(1);
	}
	Spec_prob = percent;	/*- this is for -f when % is specified on cmd line */
	percent = 100 - percent;
	if (Equal_probs)
	{
		if (num_noprob != 0)
		{
			if (num_noprob > 1)
			{
				frac = percent / num_noprob;
				DPRINTF(1, (stderr, ", frac = %d%%", frac));
				for (fp = File_list; fp != last; fp = fp->next)
					if (fp->percent == NO_PROB)
					{
						fp->percent = frac;
						percent -= frac;
					}
			}
			last->percent = percent;
			DPRINTF(1, (stderr, ", residual = %d%%", percent));
		} else
		{
			DPRINTF(1, (stderr, ", %d%% distributed over remaining fortunes\n", percent));
		}
	}
	DPRINTF(1, (stderr, "\n"));

#ifdef DEBUG
	/*-
	 * if (Debug >= 1)
	 * 	print_list(File_list, 0); 
	 * Causes crash with new %% code 
	 */
#endif
}

/*-
 * zero_tbl:
 *      Zero out the fields we care about in a tbl structure.
 */
void
zero_tbl(register STRFILE * tp)
{
	tp->str_numstr = 0;
	tp->str_longlen = 0;
	tp->str_shortlen = -1;
}

/*-
 * sum_tbl:
 *      Merge the tbl data of t2 into t1.
 */
void
sum_tbl(register STRFILE * t1, register STRFILE * t2)
{
	t1->str_numstr += t2->str_numstr;
	if (t1->str_longlen < t2->str_longlen)
		t1->str_longlen = t2->str_longlen;
	if (t1->str_shortlen > t2->str_shortlen)
		t1->str_shortlen = t2->str_shortlen;
}

/*-
 * get_tbl:
 *      Get the tbl data file the datfile.
 */
void
get_tbl(FILEDESC * fp)
{
	auto int        fd;
	register FILEDESC *child;

	if (fp->read_tbl)
		return;
	if (fp->child == NULL)
	{
		if ((fd = open(fp->datfile, 0)) < 0)
		{
			perror(fp->datfile);
			exit(1);
		}
		if (read(fd, (char *) &fp->tbl, sizeof fp->tbl) != sizeof fp->tbl)
		{
			fprintf(stderr, "fortune: %s corrupted\n", fp->path);
			exit(1);
		}
		fp->tbl.str_version = ntohl(fp->tbl.str_version);
		fp->tbl.str_numstr = ntohl(fp->tbl.str_numstr);
		fp->tbl.str_longlen = ntohl(fp->tbl.str_longlen);
		fp->tbl.str_shortlen = ntohl(fp->tbl.str_shortlen);
		fp->tbl.str_flags = ntohl(fp->tbl.str_flags);
		close(fd);
	} else
	{
		zero_tbl(&fp->tbl);
		for (child = fp->child; child != NULL; child = child->next)
		{
			get_tbl(child);
			sum_tbl(&fp->tbl, &child->tbl);
		}
	}
	fp->read_tbl = TRUE;
}

/*-
 * sum_noprobs:
 *      Sum up all the noprob probabilities, starting with fp.
 */
void
sum_noprobs(register FILEDESC * fp)
{
	static bool     did_noprobs = FALSE;

	if (did_noprobs)
		return;
	zero_tbl(&Noprob_tbl);
	while (fp != NULL)
	{
		get_tbl(fp);
		/*
		 * This conditional should help us return correct values for -f
		 * when a percentage is specified 
		 */
		if (fp->percent == NO_PROB)
			sum_tbl(&Noprob_tbl, &fp->tbl);
		fp = fp->next;
	}
	did_noprobs = TRUE;
}

/*-
 * pick_child
 *      Pick a child from a chosen parent.
 */
FILEDESC       *
pick_child(FILEDESC * parent)
{
	register FILEDESC *fp;
	register int    choice;

	if (Equal_probs)
	{
		choice = random() % parent->num_children;
		DPRINTF(1, (stderr, "    choice = %d (of %d)\n", choice, parent->num_children));
		for (fp = parent->child; choice--; fp = fp->next)
			continue;
		DPRINTF(1, (stderr, "    using %s\n", fp->name));
		return fp;
	} else
	{
		get_tbl(parent);
		choice = random() % parent->tbl.str_numstr;
		DPRINTF(1, (stderr, "    choice = %d (of %ld)\n", choice, parent->tbl.str_numstr));
		for (fp = parent->child; choice >= fp->tbl.str_numstr; fp = fp->next)
		{
			choice -= fp->tbl.str_numstr;
			DPRINTF(1, (stderr, "\tskip %s, %ld (choice = %d)\n", fp->name, fp->tbl.str_numstr, choice));
		}
		DPRINTF(1, (stderr, "    using %s, %ld\n", fp->name, fp->tbl.str_numstr));
		return fp;
	}
}

/*-
 * open_dat:
 *      Open up the dat file if we need to.
 */
void
open_dat(FILEDESC * fp)
{
	if (fp->datfd < 0 && (fp->datfd = open(fp->datfile, 0)) < 0)
	{
		perror(fp->datfile);
		exit(1);
	}
}

/*-
 * get_pos:
 *      Get the position from the pos file, if there is one.  If not,
 *      return a random number.
 */
void
get_pos(FILEDESC * fp)
{
	assert(fp->read_tbl);
	if (fp->pos == POS_UNKNOWN)
	{
		fp->pos = random() % fp->tbl.str_numstr;
	}
	if (++(fp->pos) >= fp->tbl.str_numstr)
		fp->pos -= fp->tbl.str_numstr;
	DPRINTF(1, (stderr, "pos for %s is %ld\n", fp->name, fp->pos));
}

/*-
 * get_fort:
 *      Get the fortune data file's seek pointer for the next fortune.
 */
void
get_fort(void)
{
	register FILEDESC *fp;
	register int    choice;

	if (File_list->next == NULL || File_list->percent == NO_PROB)
		fp = File_list;
	else
	{
		choice = random() % 100;
		DPRINTF(1, (stderr, "choice = %d\n", choice));
		for (fp = File_list; fp->percent != NO_PROB; fp = fp->next)
			if (choice < fp->percent)
				break;
			else
			{
				choice -= fp->percent;
				DPRINTF(1, (stderr, "    skip \"%s\", %d%% (choice = %d)\n", fp->name, fp->percent, choice));
			}
		DPRINTF(1, (stderr, "using \"%s\", %d%% (choice = %d)\n", fp->name, fp->percent, choice));
	}
	if (fp->percent != NO_PROB)
		get_tbl(fp);
	else
	{
		if (fp->next != NULL)
		{
			sum_noprobs(fp);
			choice = random() % Noprob_tbl.str_numstr;
			DPRINTF(1, (stderr, "choice = %d (of %ld) \n", choice, Noprob_tbl.str_numstr));
			while (choice >= fp->tbl.str_numstr)
			{
				choice -= fp->tbl.str_numstr;
				fp = fp->next;
				DPRINTF(1, (stderr, "    skip \"%s\", %ld (choice = %d)\n", fp->name, fp->tbl.str_numstr, choice));
			}
			DPRINTF(1, (stderr, "using \"%s\", %ld\n", fp->name, fp->tbl.str_numstr));
		}
		get_tbl(fp);
	}
	if (fp->child != NULL)
	{
		DPRINTF(1, (stderr, "picking child\n"));
		fp = pick_child(fp);
	}
	Fortfile = fp;
	get_pos(fp);
	open_dat(fp);
	lseek(fp->datfd, (off_t) (sizeof fp->tbl + fp->pos * sizeof Seekpts[0]), 0);
	read(fp->datfd, Seekpts, sizeof Seekpts);
	Seekpts[0] = ntohl(Seekpts[0]);
	Seekpts[1] = ntohl(Seekpts[1]);
}

/*-
 * open_fp:
 *      Assocatiate a FILE * with the given FILEDESC.
 */
void
open_fp(FILEDESC * fp)
{
	if (fp->inf == NULL && (fp->inf = fdopen(fp->fd, "r")) == NULL)
	{
		perror(fp->path);
		exit(1);
	}
}

#ifndef	NO_REGEX
/*-
 * maxlen_in_list
 *      Return the maximum fortune len in the file list.
 */
int
maxlen_in_list(FILEDESC * list)
{
	register FILEDESC *fp;
	register int    len, maxlen;

	maxlen = 0;
	for (fp = list; fp != NULL; fp = fp->next)
	{
		if (fp->child != NULL)
		{
			if ((len = maxlen_in_list(fp->child)) > maxlen)
				maxlen = len;
		} else
		{
			get_tbl(fp);
			if (fp->tbl.str_longlen > maxlen)
				maxlen = fp->tbl.str_longlen;
		}
	}
	return maxlen;
}

/*-
 * matches_in_list
 *      Print out the matches from the files in the list.
 */
void
matches_in_list(FILEDESC * list)
{
	unsigned char  *sp;
	register FILEDESC *fp;
	int             in_file, nchar;

	for (fp = list; fp != NULL; fp = fp->next)
	{
		if (fp->child != NULL)
		{
			matches_in_list(fp->child);
			continue;
		}
		DPRINTF(1, (stderr, "searching in %s\n", fp->path));
		open_fp(fp);
		sp = Fortbuf;
		in_file = FALSE;
		while (fgets(sp, Fort_len, fp->inf) != NULL)
			if (!STR_ENDSTRING(sp, fp->tbl))
				sp += strlen(sp);
			else
			{
				*sp = '\0';
				nchar = sp - Fortbuf;
				DPRINTF(1, (stdout, "nchar = %d\n", nchar));
				if ((nchar < SLEN || !Short_only) && (nchar > SLEN || !Long_only) && RE_EXEC(Fortbuf))
				{
					if (!in_file)
					{
						fprintf(stderr, "(%s)\n%c\n", fp->name, fp->tbl.str_delim);
						Found_one = TRUE;
						in_file = TRUE;
					}
					fwrite(Fortbuf, 1, sp - Fortbuf, stdout);
					printf("%c\n", fp->tbl.str_delim);
				}
				sp = Fortbuf;
			}
	}
}

/*-
 * find_matches:
 *      Find all the fortunes which match the pattern we've been given.
 */
int
find_matches(void)
{
	Fort_len = maxlen_in_list(File_list);
	DPRINTF(2, (stderr, "Maximum length is %d\n", Fort_len));
	/*
	 * extra length, "%\n" is appended 
	 */
	Fortbuf = do_malloc((unsigned int) Fort_len + 10);

	Found_one = FALSE;
	matches_in_list(File_list);
	return Found_one;
	/*- NOTREACHED */
}
#endif /*- NO_REGEX */

void
display(FILEDESC * fp)
{
	register char  *p, ch;
	char            line[BUFSIZ];

	open_fp(fp);
	fseek(fp->inf, (long) Seekpts[0], 0);
	for (Fort_len = 0; fgets(line, sizeof line, fp->inf) != NULL && !STR_ENDSTRING(line, fp->tbl); Fort_len++)
	{
		if (fp->tbl.str_flags & STR_ROTATED) {
			for (p = line; (ch = *p); ++p) {
				if (isupper(ch))
					*p = 'A' + (ch - 'A' + 13) % 26;
				else
				if (islower(ch))
					*p = 'a' + (ch - 'a' + 13) % 26;
			}
		}
		fputs(line, stdout);
	}
	fflush(stdout);
}

/*-
 * fortlen:
 *      Return the length of the fortune.
 */
int
fortlen(void)
{
	register int    nchar;
	char            line[BUFSIZ];

	if (!(Fortfile->tbl.str_flags & (STR_RANDOM | STR_ORDERED)))
		nchar = (Seekpts[1] - Seekpts[0]) - 2;	/*- for %^J delimiter */
	else
	{
		open_fp(Fortfile);
		fseek(Fortfile->inf, (long) Seekpts[0], 0);
		nchar = 0;
		while (fgets(line, sizeof line, Fortfile->inf) != NULL && !STR_ENDSTRING(line, Fortfile->tbl))
			nchar += strlen(line);
	}
	Fort_len = nchar;
	return nchar;
}

int
max(register int i, register int j)
{
	return (i >= j ? i : j);
}

int
main(int ac, char *av[])
{
	getargs(ac, av);

#ifndef NO_REGEX
	if (Match)
		exit(find_matches() != 0);
#endif
	init_prob();
	if (Find_files)
	{
		sum_noprobs(File_list);
		if (Equal_probs)
			calc_equal_probs();
		print_list(File_list, 0);
		exit(0);
	}
	srandom((int) (time((time_t *) NULL) + getpid()));
	do
	{
		get_fort();
	}
	while ((Short_only && fortlen() > SLEN) || (Long_only && fortlen() <= SLEN));

	display(Fortfile);

	if (Wait)
	{
		fortlen();
		sleep((unsigned int) max(Fort_len / CPERS, MINW));
	}
	exit(0);
	/*- NOTREACHED */
}
