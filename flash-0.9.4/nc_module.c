/*
 * $Log: nc_module.c,v $
 * Revision 1.4  2009-06-04 10:46:47+05:30  Cprogrammer
 * added conditional inclusion of ncurses
 *
 * Revision 1.3  2008-06-09 15:32:04+05:30  Cprogrammer
 * added GPL copyright notice
 *
 * Revision 1.2  2002-12-21 19:09:14+05:30  Manny
 * corrected compilation warnings
 *
 * Revision 1.1  2002-12-16 01:55:14+05:30  Manny
 * Initial revision
 *
 *
 * Some internal modules
 *
 * Copyright (C) 1995-1997 Stephen Fegan
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
 * please send patches (w/ explanation) or advice to: `flash@netsoc.ucd.ie'
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
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <syslog.h>
#include <ctype.h>
#include <time.h>

#include "ncr_scr.h"
#include "screen.h"
#include "mystring.h"
#include "misc.h"
#include "exec.h"
#include "set.h"
#include "module.h"

struct DisplayCallbacks DCBs;

extern bool     color;
extern time_t   TimeNow;

void
initialise_callbacks(void)
{
	if (DCBs.initialised == 1)
		return;
	DCBs.initialised = 1;
	DCBs.undermenu.next = DCBs.undermenu.prev = &DCBs.undermenu;
	DCBs.overmenu.next = DCBs.overmenu.prev = &DCBs.overmenu;
	DCBs.overall.next = DCBs.overall.prev = &DCBs.overall;
	DCBs.timeout.next = DCBs.timeout.prev = &DCBs.timeout;
}

void
register_display_callback(DISPLAYCALLBACK f, DISPLAYCALLBACKWHERE where)
{
	struct Callback *cb, *cbl;
	cb = xmalloc(sizeof(*cb));
	cb->f = f;

	switch (where)
	{
	case OVERMENU:
		cbl = &DCBs.overmenu;
		break;

	case OVERALL:
		cbl = &DCBs.overall;
		break;

	default:
	case UNDERMENU:
		cbl = &DCBs.undermenu;
		break;
	}

	cb->next = cbl;
	cb->prev = cbl->prev;
	cbl->prev->next = cb;
	cbl->prev = cb;

	return;
}

void
register_timeout_callback(TIMEOUTCALLBACK f)
{
	struct TOCallback *cb, *cbl;

	cbl = &DCBs.timeout;
	cb = xmalloc(sizeof(*cb));
	cb->f = f;

	cb->next = cbl;
	cb->prev = cbl->prev;
	cbl->prev->next = cb;
	cbl->prev = cb;

	return;
}



/******************************************************************************\
*                                                                              *
*  ooo        ooooo                 .o8              oooo                      *
*  `88.       .888'                "888              `888                      *
*   888b     d'888   .ooooo.   .oooo888  oooo  oooo   888   .ooooo.   .oooo.o  *
*   8 Y88. .P  888  d88' `88b d88' `888  `888  `888   888  d88' `88b d88(  "8  *
*   8  `888'   888  888   888 888   888   888   888   888  888ooo888 `"Y88b.   *
*   8    Y     888  888   888 888   888   888   888   888  888    .o o.  )88b  *
*  o8o        o888o `Y8bod8P' `Y8bod88P"  `V88V"V8P' o888o `Y8bod8P' 8""888P'  *
*                                                                              *
\******************************************************************************/


/*
 * 
 * BACKGROUND - displays (and overlays) background images 
 * 
 */

static WINDOW  *Background_Window = NULL;

void
DisplayBackground(void)
{
	wattrset(Background_Window, A_NORMAL);
	touchwin(Background_Window);
	wnoutrefresh(Background_Window);
	return;
}

void
InitBackground(int argc, char **argv, FILE * fp, int *lc)
{
	int             i, l;
	FILE           *back_file;
	char           *back_line, *back_file_list, *backgrounds;
	char           *Back_files[5], **back_files = Back_files;
	int             n_back_files;

	argv++, argc--;

	if (argc == 0)
		return;

	if (Background_Window == NULL)
		Background_Window = newwin(LINES - 2, 0, 1, 0);

	werase(Background_Window);

	backgrounds = *argv;

	back_file_list = xmalloc(strlen(backgrounds) + 1);

	strcpy(back_file_list, backgrounds);
	back_line = (char *) xmalloc((COLS + 1) * sizeof(char));

	n_back_files = strtokenize(back_file_list, ':', back_files, 5);

	wattrset(Background_Window, A_NORMAL);

	if (n_back_files)
	{
		back_file = fopen(stradp(*(back_files), 0), "r");

		if (back_file != (FILE *) NULL)
		{
			fgets(back_line, COLS + 1, back_file);
			for (l = 0; (!feof(back_file)) && (l < LINES - 2); l++)
			{
				i = strlen(back_line);
				mvwaddnstr(Background_Window, l, 0, back_line, i > COLS ? COLS : i);

				while ((i == COLS) && (*(back_line + i - 1) != '\n'))
				{
					fgets(back_line, COLS + 1, back_file);
					i = strlen(back_line);
				}
				fgets(back_line, COLS + 1, back_file);
			}
			fclose(back_file);
		}
		n_back_files--, back_files++;
	}

	while (n_back_files)
	{
		back_file = fopen(stradp(*(back_files), 0), "r");

		if (back_file != (FILE *) NULL)
		{
			int             c, d;

			fgets(back_line, COLS + 1, back_file);
			for (l = 0; (!feof(back_file)) && (l < LINES - 2); l++)
			{
				i = strlen(back_line);

				for (d = c = 0; (c < COLS) && (d < i); d++)
				{
					if (*(back_line + d) == '\t')
						c += (8 - (c % 8));
					else
					if (!isspace((int) *(back_line + d)))
						mvwaddch(Background_Window, l, c++, *(back_line + d));
					else
						c++;
				}
				while ((i == COLS) && (*(back_line + i - 1) != '\n'))
				{
					fgets(back_line, COLS + 1, back_file);
					i = strlen(back_line);
				}
				fgets(back_line, COLS + 1, back_file);
			}
			fclose(back_file);
		}
		n_back_files--, back_files++;
	}
	free(back_file_list);
	free(back_line);
	wattrset(Background_Window, A_NORMAL);
	register_display_callback(DisplayBackground, UNDERMENU);
	return;
}


/*
 * 
 * BARCLOCK - Displays a clock in the upper right of the screen
 * 
 */

static char    *DOW[] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
static WINDOW  *BarClock_Win = NULL;
static int      BarClock_l_min = -1;

static void
DisplayBarClock(void)
{
	struct tm      *tm;
	char            at[20];

	tm = localtime(&TimeNow);

	if (BarClock_l_min != tm->tm_min)
	{
		BarClock_l_min = tm->tm_min;

		sprintf(at, "%3s %d/%d %d:%2.2d%2s", DOW[tm->tm_wday], tm->tm_mday, tm->tm_mon + 1, ((tm->tm_hour + 11) % 12) + 1,
				tm->tm_min, tm->tm_hour > 11 ? "pm" : "am");

		werase(BarClock_Win);
		mvwaddstr(BarClock_Win, 0, 17 - strlen(at), at);
		wnoutrefresh(BarClock_Win);
	} else
		touchwin(BarClock_Win);

	wnoutrefresh(BarClock_Win);
	return;
}

static int
UpdateBarClock(int r)
{
	struct tm      *tm;

	tm = localtime(&TimeNow);

	return (BarClock_l_min != tm->tm_min) ? 1 : 0;
}

void
InitBarClock(int argc, char **argv, FILE * fp, int *lc)
{
	argv++, argc--;
	BarClock_Win = newwin(1, 17, 0, NC_RIGHT_SC(18));
	wbkgd(BarClock_Win, ' ' | A_REVERSE);
	register_display_callback(DisplayBarClock, UNDERMENU);
	register_timeout_callback(UpdateBarClock);
	return;
}


/*
 * 
 * Mail Check - Check mail and inform user
 * 
 */

static void
DisplayMailCheck(void)
{
	WINDOW         *mailadvisory_window;
	int             mail;

	mail = mail_check(0);

	if (mail < 0)
		return;
	else
	if (mail == 0)
		return;
	else
	{
		mailadvisory_window = newwin(4, 10, LINES - 5, COLS - 10);
		wattrset(mailadvisory_window, ((color) ? 0 : A_REVERSE) | A_BOLD);
		if (color)
			wattron(mailadvisory_window, COLOR_PAIR(MAIL_ADVISORY_COLOR));
		box(mailadvisory_window, 0, 0);
		mvwaddstr(mailadvisory_window, 1, 1, "NEW MAIL");
		mvwaddstr(mailadvisory_window, 2, 1, "RECEIVED");
		wattrset(mailadvisory_window, A_NORMAL);
		wnoutrefresh(mailadvisory_window);
		delwin(mailadvisory_window);
		if (mail == 2)
			beep();
		return;
	}
}


static int
UpdateMailCheck(int r)
{
	if (r)
		return 0;
	return (mail_check(1));
}

void
InitMailCheck(int argc, char **argv, FILE * fp, int *lc)
{
	argv++, argc--;
	register_display_callback(DisplayMailCheck, UNDERMENU);
	register_timeout_callback(UpdateMailCheck);
	return;
}
