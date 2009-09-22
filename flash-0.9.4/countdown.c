/*
 * $Log: countdown.c,v $
 * Revision 1.4  2009-06-04 10:46:13+05:30  Cprogrammer
 * added conditional inclusion of ncurses
 *
 * Revision 1.3  2009-02-10 16:45:49+05:30  Cprogrammer
 * use ncurses if present
 *
 * Revision 1.2  2002-12-21 19:07:38+05:30  Manny
 * removed conflict of variable name with function box()
 *
 * Revision 1.1  2002-12-16 01:54:53+05:30  Manny
 * Initial revision
 *
 * 
 * Countdown - Display the number of seconds until... 
 * 
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_NCURSES_NCURSES_H
#include <ncurses/ncurses.h>
#else
#include <curses.h>
#endif

#ifdef HAVE_NCURSES_TERM_H
#include <ncurses/term.h>
#else
#ifdef HAVE_TERM_H
#include <term.h>
#endif
#endif

#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <syslog.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "ncr_scr.h"
#include "mystring.h"
#include "misc.h"

extern int      color;
extern time_t   TimeNow;

static WINDOW  *CDMS_window = NULL;
static int      CDMS_initialised = 0;
static int      CDMS_lines, CDMS_cols, CDMS_time_line, CDMS_time_col, CDMS_timelen;
static time_t   CDMS_Time = 946684800L, CDMS_Display_Scale = 1;
static char     CDMS_format[20];

static void
DisplayCountdown(void)
{
	time_t          Countdown = CDMS_Time;

	if (TimeNow < Countdown)
	{
		if (CDMS_timelen)
		{
			Countdown -= TimeNow;
			Countdown /= CDMS_Display_Scale;
			/*
			 * wattrset (CDMS_window,  ((color)? 0:A_REVERSE) | A_BOLD);
			 * if(color)wattron(CDMS_window, COLOR_PAIR (MAIL_ADVISORY_COLOR));
			 */
			mvwprintw(CDMS_window, CDMS_time_line, CDMS_time_col, CDMS_format, (long) Countdown);
			/*
			 * wattrset(CDMS_window,A_NORMAL);
			 */
		}
		wnoutrefresh(CDMS_window);
	}
}

static int
UpdateCountdown(int r)
{
	if (TimeNow < CDMS_Time)
		return 1;
	else
		return 0;
}

void
ModuleInit(int argc, char **argv, FILE * fp, int *lc)
{
	char          **text = NULL, *line, *indx;
	int             linesmalloced = 0;
	int             cols, i, box_var = 1, ax, ay;
	struct tm       ct = { 0, 0, 0, 1, 0, 100, 0, 0, -1 };
	chtype          ch;

	if (CDMS_initialised)
		return;
	argv++, argc--;
	if (argc)
	{
		int             d = 1, m = 0, y = 100;
		argc--;
		sscanf(*(argv++), "%d/%d/%d", &d, &m, &y);
		if (y > 1900)
			y -= 1900;
		ct.tm_mday = d;
		ct.tm_mon = m - 1;
		ct.tm_year = y;
	}
	if (argc)
	{
		int             h = 0, m = 0, s = 0;
		argc--;
		sscanf(*(argv++), "%d:%d:%d", &h, &m, &s);
		ct.tm_hour = h;
		ct.tm_min = m;
		ct.tm_sec = s;
	}
	while (argc)
	{
		if (strcasecmp(*argv, "seconds") == 0)
			CDMS_Display_Scale = 1L;
		else
		if (strcasecmp(*argv, "minutes") == 0)
			CDMS_Display_Scale = 60L;
		else
		if (strcasecmp(*argv, "hours") == 0)
			CDMS_Display_Scale = 3600L;
		else
		if (strcasecmp(*argv, "days") == 0)
			CDMS_Display_Scale = 86400L;
		else
		if (strcasecmp(*argv, "weeks") == 0)
			CDMS_Display_Scale = 604800L;
		else
		if (strcasecmp(*argv, "nobox") == 0)
			box_var = 0;
		argv++, argc--;
	}
	CDMS_Time = mktime(&ct);
	line = Readline(fp);
	(*lc)++;
	ax = box_var ? 4 : 0;
	ay = box_var ? 2 : 0;
	while (line != NULL)
	{
#ifdef DEBUG
		fprintf(stderr, "Countdown: ");
#endif
		if ((*line == '\0') || (*line == '#'))
		{
#ifdef DEBUG
			fprintf(stderr, "comment or empty line\n");
#endif
			line = Readline(fp);
			(*lc)++;
			continue;
		}
		if (strcasecmp(line, "endmodule") == 0)
		{
#ifdef DEBUG
			fprintf(stderr, "Endmodule found!!\n");
#endif
			break;
		}
#ifdef DEBUG
		else
			fprintf(stderr, "%s\n", line);
#endif
		if (CDMS_lines == linesmalloced)
		{
			linesmalloced = (linesmalloced > 0) ? 2 * linesmalloced : 2;
			text = xrealloc(text, linesmalloced * sizeof(*text));
		}
		cols = strlen(line);
		if (cols > (COLS - ax))
			cols = COLS - ax, *(line + cols) = '\0';
		if (cols > CDMS_cols)
			CDMS_cols = cols;
		*(text + CDMS_lines) = xmalloc(cols + 1);
		strcpy(*(text + CDMS_lines), line);
		if (CDMS_timelen == 0)
		{
			indx = strchr(line, '@');
			if (indx != NULL)
			{
				CDMS_time_line = CDMS_lines;
				CDMS_time_col = indx - line;
				while (*indx++ == '@')
					CDMS_timelen++;
			}
		}
		CDMS_lines++;
		if (CDMS_lines == (LINES - ax))
		{
			line = Readline(fp);
			(*lc)++;
			while (strcasecmp(line, "endmodule") != 0)
			{
				line = Readline(fp);
				(*lc)++;
			}
			break;
		}
		line = Readline(fp);
		(*lc)++;
	}
	if (CDMS_cols > (COLS - ax))
		CDMS_cols = COLS - ax;
	ch = ' ';
	CDMS_window = newwin(CDMS_lines + ay, CDMS_cols + ax, LINES - (CDMS_lines + ay + 1), 0);
	ch |= ((color) ? 0 : A_REVERSE) | A_BOLD;
	if (color)
		ch |= COLOR_PAIR(MAIL_ADVISORY_COLOR);
	wbkgd(CDMS_window, ch);
	werase(CDMS_window);
	if (box_var)
		box(CDMS_window, 0, 0);
	ax /= 2;
	ay /= 2;
	for (i = 0; i < CDMS_lines; i++)
		mvwaddnstr(CDMS_window, i + ay, ax, *(text + i), CDMS_cols);
	if (CDMS_timelen)
	{
		sprintf(CDMS_format, "%%-%d.%dld", CDMS_timelen, CDMS_timelen);
		CDMS_time_col += ax;
		CDMS_time_line += ay;
	}
#ifdef DEBUG
	fprintf(stderr, "Countdown: L:%d Y:%d X:%d F:%s S:%ld\n", CDMS_timelen, CDMS_time_line, CDMS_time_col, CDMS_format,
			CDMS_Display_Scale);
	sleep(5);
#endif
	register_display_callback(DisplayCountdown, UNDERMENU);
	register_timeout_callback(UpdateCountdown);
	return;
}
