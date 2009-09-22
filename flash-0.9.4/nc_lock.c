/*
 * $Log: nc_lock.c,v $
 * Revision 1.4  2009-06-04 10:46:38+05:30  Cprogrammer
 * added conditional inclusion of ncurses
 *
 * Revision 1.3  2008-07-17 21:38:32+05:30  Cprogrammer
 * moved progname to variables.h
 *
 * Revision 1.2  2008-06-09 15:31:46+05:30  Cprogrammer
 * added GPL copyright notice
 *
 * Revision 1.1  2002-12-16 01:55:10+05:30  Manny
 * Initial revision
 *
 *
 * Does Screen Locking for ncurses module
 *
 * Copyright (C) 1996 Stephen Fegan
 *
 * The dorain() function was lifted almost directly from the ncurses
 * 1.9.9e distribution the following line is displayed at the top of
 * rain.c. I suppose it means something to someone...
 *
 * rain 11/3/1980 EPS/CITHEP
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
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
#include <sys/stat.h>

#include "ncr_scr.h"
#include "screen.h"
#include "exec.h"
#include "misc.h"
#include "set.h"
#include "variables.h"

void            dorain(int reinitialise);
float           ranf(void);

extern bool     color;
extern int      Timeout;

#define cursor(col,row) move(row,col)

void
nc_lock_screen(void)
{
	WINDOW         *b_win, *f_win;
	char            password[21], verify[21], *backdoor;
	char           *temp;
	int             timeout = 0;
	chtype          ch;
	struct stat     st;

	timeout = find_variable("lockscreensaver", &temp);
	if ((timeout == 0) || (temp == NULL) || (sscanf(temp, "%d", &timeout) != 1) || (timeout <= 0))
		timeout = -1;

	find_variable("lockbackdoor", &backdoor);

	ch = ' ' | A_BOLD;
	if (color == TRUE)
		ch |= COLOR_PAIR(PASSWORD_COLOR);

	b_win = newwin(4, 42, (LINES - 4) / 2, (COLS - 42) / 2);
	werase(b_win);
	box(b_win, 0, 0);
	wnoutrefresh(b_win);

	f_win = derwin(b_win, 2, 40, 1, 1);
	wattrset(f_win, A_BOLD);
	if (color)
		wattron(f_win, COLOR_PAIR(PASSWORD_COLOR));

	do
	{
		werase(f_win);
		wbkgd(f_win, ch);
		mvwaddstr(f_win, 0, 0, "  Enter Password >                    < ");
		wnoutrefresh(f_win);
		doupdate();
		get_password(b_win, f_win, 0, 18, password, 20, -1);
		if (*password == '\0')
			return;

		mvwaddstr(f_win, 1, 0, " Verify Password >                    < ");
		wnoutrefresh(f_win);
		doupdate();
		get_password(b_win, f_win, 1, 18, verify, 20, -1);
	}
	while (strcmp(password, verify) != 0);

	delwin(f_win);
	delwin(b_win);

	b_win = newwin(3, 41, (LINES - 3) / 2, (COLS - 41) / 2);
	werase(b_win);
	box(b_win, 0, 0);

	f_win = derwin(b_win, 1, 39, 1, 1);
	wattrset(f_win, A_BOLD);
	if (color)
		wattron(f_win, COLOR_PAIR(PASSWORD_COLOR));

	fstat(0, &st);
	fchmod(0, 0600);

#if 1
	werase(stdscr);
	wbkgd(f_win, ch);
	wnoutrefresh(stdscr);
	wnoutrefresh(b_win);
#endif

	do
	{
#if 0
		werase(stdscr);
		wbkgd(f_win, ch);
		wnoutrefresh(stdscr);
		doupdate();

		touchwin(b_win);
		wnoutrefresh(b_win);
#endif
		werase(f_win);
		mvwaddstr(f_win, 0, 0, " Enter Password >                    < ");
		wnoutrefresh(f_win);
		doupdate();
		get_password(b_win, f_win, 0, 17, verify, 20, timeout);
	}
	while ((strcmp(password, verify) != 0) && ((backdoor == NULL) || (strcmp(shacrypt(verify, backdoor), backdoor) != 0)));

	fchmod(0, st.st_mode);

	delwin(f_win);
	delwin(b_win);

	return;
}

void
get_password(WINDOW * b_win, WINDOW * win, int Line, int Col, char *area, int maxc, int timeout)
{
	int             cin;
	int             i = 0;
	int             col = Col;
	struct termios  term;
	cc_t            erase = 0, kill = 0;
	static int      reinitialise = 1;

	if (tcgetattr(0, &term) >= 0)
	{
		erase = term.c_cc[VERASE];
		kill = term.c_cc[VKILL];
	}

	wmove(win, Line, col);
	wrefresh(win);

	keypad(win, TRUE);

	if (timeout == -1)
	{
		reinitialise = 1;
		wtimeout(win, Timeout);
	} else
		wtimeout(win, timeout);

	doupdate();

	errno = 0;
	while ((cin = wgetch(win)) != '\n' && cin != ESCAPEKEY)
	{
		if (errno == EIO)
		{
			GRAB_BACK_TTY;
			errno = 0;
			continue;
		}

		if (cin == ERR)
		{
			if (checktty() == 0)
				exit(0);
			if (timeout != -1)
			{
				dorain(reinitialise);
				if (reinitialise == 1)
					reinitialise = 0;
				touchwin(win);
				wnoutrefresh(win);
				touchwin(b_win);
				wnoutrefresh(b_win);
				doupdate();
			} else
			{
				display_screen(0);
				touchwin(win);
				wnoutrefresh(win);
				touchwin(b_win);
				wnoutrefresh(b_win);
				doupdate();
			}
		}

		else
		if ((cin == erase) || (cin == KEY_BACKSPACE))
		{
			if (i > 0)
			{
				i--, col--;
				wmove(win, Line, col);
				waddch(win, ' ');
				wmove(win, Line, col);
			}
		}

		else
		if (cin == kill)
		{
			while (i)
			{
				i--, col--;
				wmove(win, Line, col);
				waddch(win, ' ');
				wmove(win, Line, col);
			}
		}

		else
		if (cin == '\014')		/*
								 * ctrl-l 
								 */
			wrefresh(curscr);

		else
		if (i < maxc)
		{
			waddch(win, '*');
			*(area + i) = cin;
			i++, col++;
		}
		wrefresh(win);
	}

	*(area + i) = '\0';

	return;
}

void
dorain(int reinitialise)
{
	int             x, y;
	static int      j = 0;
	static int      xpos[5], ypos[5];
	float           r;
	float           c;

	/*
	 * nl();
	 */

	r = (float) (LINES - 4);
	c = (float) (COLS - 4);
	if (reinitialise)
	{
		for (j = 5; --j >= 0;)
		{
			xpos[j] = (int) (c * ranf()) + 2;
			ypos[j] = (int) (r * ranf()) + 2;
		}
		j = 0;
	}

	x = (int) (c * ranf()) + 2;
	y = (int) (r * ranf()) + 2;

	cursor(x, y);
	addch('.');

	cursor(xpos[j], ypos[j]);
	addch('o');

	if (j == 0)
		j = 4;
	else
		--j;
	cursor(xpos[j], ypos[j]);
	addch('O');

	if (j == 0)
		j = 4;
	else
		--j;
	cursor(xpos[j], ypos[j] - 1);
	addch('-');
	cursor(xpos[j] - 1, ypos[j]);
	addstr("|.|");
	cursor(xpos[j], ypos[j] + 1);
	addch('-');

	if (j == 0)
		j = 4;
	else
		--j;
	cursor(xpos[j], ypos[j] - 2);
	addch('-');
	cursor(xpos[j] - 1, ypos[j] - 1);
	addstr("/ \\");
	cursor(xpos[j] - 2, ypos[j]);
	addstr("| O |");
	cursor(xpos[j] - 1, ypos[j] + 1);
	addstr("\\ /");
	cursor(xpos[j], ypos[j] + 2);
	addch('-');

	if (j == 0)
		j = 4;
	else
		--j;
	cursor(xpos[j], ypos[j] - 2);
	addch(' ');
	cursor(xpos[j] - 1, ypos[j] - 1);
	addstr("   ");
	cursor(xpos[j] - 2, ypos[j]);
	addstr("     ");
	cursor(xpos[j] - 1, ypos[j] + 1);
	addstr("   ");
	cursor(xpos[j], ypos[j] + 2);
	addch(' ');
	xpos[j] = x;
	ypos[j] = y;

	wnoutrefresh(stdscr);
}

float
ranf(void)
{
	float           rv;
	long            r = rand();

	r &= 077777;
	rv = ((float) r / 32767.);
	return rv;
}
