/*
 * $Log: nc_init.c,v $
 * Revision 1.3  2009-06-04 10:46:32+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.2  2008-06-09 15:31:33+05:30  Cprogrammer
 * added GPL copyright notice
 *
 * Revision 1.1  2002-12-16 01:55:07+05:30  Manny
 * Initial revision
 *
 *
 * Initialises the ncurses system. 
 *
 * Copyright (C) 1996 Stephen Fegan
 * Portions Copyright (C) 1995 Brian Cully
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
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <syslog.h>

#include "ncr_scr.h"
#include "mystring.h"
#include "screen.h"
#include "parse.h"
#include "misc.h"
#include "exec.h"
#include "menu.h"
#include "set.h"

bool            color;
int             gotwinch = 0;

WINDOW         *topbar, *bottombar;

static void
setup_colors()
{
	init_pair(MENU_TITLE + 1, TITLE_COL, BG_COL);
	init_pair(MENU_NOP + 1, NOP_COL, BG_COL);
	init_pair(MENU_EXEC + 1, EXEC_COL, BG_COL);
	init_pair(MENU_MODULE + 1, EXEC_COL, BG_COL);
	init_pair(MENU_ARGS + 1, ARGS_COL, BG_COL);
	init_pair(MENU_SUB + 1, SUB_COL, BG_COL);
	init_pair(MENU_EXIT + 1, EXIT_COL, BG_COL);
	init_pair(MENU_QUIT + 1, QUIT_COL, BG_COL);

	init_pair(MAIL_ADVISORY_COLOR, COLOR_YELLOW, COLOR_BLACK);
	init_pair(PASSWORD_COLOR, COLOR_WHITE, COLOR_RED);
#ifdef DONT_HIGHLIGHT_WITH_REVERSE
	init_pair(HIGHLIGHT_COLOR, COLOR_WHITE, COLOR_RED);
#endif
}

/*
 * Interface to main module
 * Initialize the screen, must be called before anything else 
 */
void
init_scr()
{
	sigset_t        newmask, oldmask;
	struct termios  t;

	if (isendwin() == TRUE)
	{
		return;
	}

	/*
	 * 
	 * It looks as if initscr resets the default signal handlers. Block sigint, sigquit  
	 * and sigtstp until after we are through, then quickly ignore them and allow signals
	 * through again 
	 */

	sigemptyset(&newmask);
	sigemptyset(&oldmask);

	sigaddset(&newmask, SIGINT);
	sigaddset(&newmask, SIGTSTP);
	sigaddset(&newmask, SIGQUIT);
	sigaddset(&newmask, SIGWINCH);

	sigprocmask(SIG_BLOCK, &newmask, &oldmask);

	initscr();

	if ((color = has_colors()))
	{
		start_color();
		setup_colors();
	}
	cbreak();
	noecho();
	keypad(stdscr, TRUE);

	signal(SIGINT, SIG_IGN);
	signal(SIGTSTP, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	/*
	 * signal (SIGTTIN, SIG_IGN);
	 * signal (SIGTTOU, SIG_IGN);
	 */

	if (tcgetattr(1, &t) != -1)
	{
		t.c_lflag |= TOSTOP;
		tcsetattr(1, TCSAFLUSH, &t);
	}

	sigprocmask(SIG_SETMASK, &oldmask, (sigset_t *) NULL);

	initialise_callbacks();
	init_background();
}

void
init_background(void)
{
	int             i;

	topbar = newwin(1, COLS, 0, 0);
	bottombar = newwin(1, COLS, LINES - 1, 0);

	werase(topbar);
	werase(bottombar);

	wattrset(topbar, A_REVERSE);
	wattrset(bottombar, A_REVERSE);

	mvwaddstr(topbar, 0, 0, TOP_BAR);
	for (i = strlen(TOP_BAR); i < COLS; i++)
		waddch(topbar, ' ');

	mvwaddstr(bottombar, 0, 0, BOT_BAR);
	for (i = strlen(BOT_BAR); i < COLS; i++)
		waddch(bottombar, ' ');

	wattrset(topbar, A_NORMAL);
	wattrset(bottombar, A_NORMAL);

	return;
}

/*
 * Interface to main module
 * Close the output, must be called when finished 
 */
void
close_scr()
{
	char            c = '\n';
	/*
	 * wmove (background_window, LINES - 1, 0);
	 * wrefresh (background_window);
	 */
	move(LINES - 1, 0);
	endwin();
	fflush(NULL);
	write(1, &c, 1);
	tcdrain(1);
}

void
clear_close_scr()
{
	endwin();
	putp(clear_screen);
	fflush(stdout);
}

void
pause_scr()
{
	endwin();
}

void
handle_winch(int s)
{
	gotwinch = 1;
}
