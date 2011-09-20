/*
 * $Log: displaytop.c,v $
 * Revision 2.7  2004-10-08 18:41:56+05:30  Cprogrammer
 * unbuffered stdout
 *
 * Revision 2.6  2004-10-02 19:32:06+05:30  Cprogrammer
 * corrected usage of displaytop
 *
 * Revision 2.5  2003-01-08 20:01:32+05:30  Cprogrammer
 * fixed compilation warnings on Solaris
 *
 * Revision 2.4  2002-12-27 16:40:16+05:30  Cprogrammer
 * corrected usage display
 *
 * Revision 2.3  2002-12-27 11:59:10+05:30  Cprogrammer
 * use clrscr() only in interactive_mode
 *
 * Revision 2.2  2002-12-26 00:32:32+05:30  Cprogrammer
 * corrected case when eof was reached and items were less than screen_size
 *
 * Revision 2.1  2002-12-25 22:54:55+05:30  Cprogrammer
 * log entries counting utility
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <curses.h>
#include <term.h>
#include <string.h>

#define ERR_ROW 25

#ifndef	lint
static char     sccsid[] = "$Id: displaytop.c,v 2.7 2004-10-08 18:41:56+05:30 Cprogrammer Stab mbhangui $";
#endif

struct topList
{
	char           *name;
	int             value;
};
static char    *CL, *CM;
static int      batch_mode;

static int      get_options(int, char **, int *);
static int      topCompare(const void *, const void *);
static int      PutStr(char *);
static int      initterm(void);
static void     clrscr();
static void     movcur(int, int);
#ifdef sun
char           *tgoto();
#endif

int
main(int argc, char **argv)
{
	char            buffer[1024], name[56];
	int             i, value, len, count, items, is_sorted, display_count;
	struct topList *topList;

	if(get_options(argc, argv, &display_count))
		return(1);
	if(!(topList = (struct topList *) malloc(sizeof(struct topList) * (display_count + 1))))
	{
		perror("malloc");
		return (1);
	}
	setbuf(stdout, 0);
	if(!batch_mode)
		clrscr();
	for (is_sorted = items = 0, count = 1;; count++)
	{
		if (!fgets(buffer, sizeof(buffer) - 2, stdin))
			break;
		if (sscanf(buffer, "%s %d", name, &value) != 2)
			continue;
		len = strlen(name);
		for (i = 0; i < display_count; i++)
		{
			if (!topList[i].name) /*- new item */
			{
				if (!(topList[i].name = (char *) realloc(topList[i].name, len + 1)))
				{
					perror("malloc");
					return (1);
				}
				strncpy(topList[i].name, name, len + 1);
				topList[i].value = value;
				is_sorted = 0;
				if (items < display_count)
					items++;
				break;
			} else /*- existing item */
			if (!strncmp(topList[i].name, name, len))
			{
				if(value > topList[i].value)
				{
					topList[i].value = value;
					is_sorted = 0;
				}
				break;
			}
		}
		if (i == display_count) /*- new item */
		{
			if(!is_sorted)
			{
				qsort(&topList[0], items, sizeof(struct topList), topCompare);
				is_sorted = 1;
			}
			for (i = 0; i < display_count; i++)
			{
				if (topList[i].value < value)
				{
					if (!(topList[items].name = (char *) realloc(topList[items].name, len + 1)))
					{
						perror("malloc");
						return (1);
					}
					strncpy(topList[items].name, name, len + 1);
					topList[items].value = value;
					qsort(&topList[0], items + 1, sizeof(struct topList), topCompare);
					is_sorted = 1;
					break;
				}
			}
		}
		if (!batch_mode && !(count % display_count))
		{
			if(!is_sorted)
			{
				qsort(&topList[0], items + 1, sizeof(struct topList), topCompare);
				is_sorted = 1;
			}
			movcur(1, 1);
			for (i = 0; i < items; i++)
				printf("%-60s %d\n", topList[i].name, topList[i].value);
		}
	} /*- for (is_sorted = items = 0, count = 1;; count++) */
	if(batch_mode || feof(stdin))
	{
		if(!is_sorted)
			qsort(&topList[0], items + 1, sizeof(struct topList), topCompare);
		if(!batch_mode)
			movcur(1, 1);
		for (i = 0; i < items; i++)
			printf("%-60s %d\n", topList[i].name, topList[i].value);
	}
	return (0);
}

int
topCompare(const void *p1, const void *p2)
{
	if (((struct topList *) p1)->value > ((struct topList *) p2)->value)
		return (-1);
	else
	if (((struct topList *) p1)->value < ((struct topList *) p2)->value)
		return (1);
	return (strncmp(((struct topList *) p1)->name, ((struct topList *) p2)->name, 256));
}

static int
get_options(int argc, char **argv, int *displayCount)
{
	int             c;

	*displayCount = 24;
	while ((c = getopt(argc, argv, "bn:")) != -1)
	{
		switch (c)
		{
		case 'n':
			*displayCount = atoi(optarg);
			break;
		case 'b':
			batch_mode = 1;
			break;
		default:
			fprintf(stderr, "USAGE: displaytop [-b] -n displayCount\n");
			fprintf(stderr, "       -b batch_mode\n");
			return (1);
		}
	}
	return (0);
}

static char     tbuf[4096];
static char   **tp;
int             initflag;
#define DEFAULTTERM "ansi"

static int
initterm()
{
	char           *termtype;
	char           *getenv();

	if(initflag)
		return(0);
	initflag = 1;
	if (!(termtype = getenv("TERM")))
	{
		CL = "\033[;H\033[2J";
		CM = "\033[%d;%dH";
		termtype = DEFAULTTERM;
	}
	if (tgetent(tbuf, termtype) != 1)
	{
		movcur(ERR_ROW, 1);
		(void) printf("ERRMSG> Cannot find termcap entry for %s\n", termtype);
		return (1);
	}
	if (!(CL = (char *) tgetstr("cl", tp)))
	{
		movcur(ERR_ROW, 1);
		(void) printf("ERRMSG> Clear screen capability required\n");
	}
	if (!(CM = (char *) tgetstr("cm", tp)))
	{
		movcur(ERR_ROW, 1);
		(void) printf("ERRMSG> Addressable cursor capability required\n");
	}
	return (0);
}

static int
PutStr(s)
	char           *s;
{
	return (tputs((char *) s, 1, putchar));
}

static void
clrscr(void)
{
	if (!initflag)
		initterm();
	printf("%s", CL);
	return;
}

static void
movcur(x, y)
	int             x, y;
{
	/*- initialize all terminal escape sequences and key defs */
	if (!initflag)
		initterm();
	PutStr((char *) tgoto((char *) CM, y - 1, x - 1));
	return;
}

void
getversion_displaytop_c()
{
	printf("%s\n", sccsid);
	return;
}
