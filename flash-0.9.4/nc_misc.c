/*
 * $Log: nc_misc.c,v $
 * Revision 1.3  2009-06-04 10:46:44+05:30  Cprogrammer
 * added conditional inclusion of ncurses
 *
 * Revision 1.2  2008-06-09 15:31:58+05:30  Cprogrammer
 * added GPL copyright notice
 *
 * Revision 1.1  2002-12-16 01:55:12+05:30  Manny
 * Initial revision
 *
 *
 * Displays menu and takes input
 *
 * Copyright (C) 1995,1996 Stephen Fegan
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
#include <unistd.h>
#include <termios.h>
#include <syslog.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include <sys/time.h>
#include <sys/types.h>

#include "ncr_scr.h"
#include "screen.h"
#include "misc.h"
#include "exec.h"
#include "set.h"

#define DEFAULT_MAIL_CHECK 1

extern bool     color;
extern int      gotwinch, Timeout;
extern WINDOW  *background_window;
extern char     draw_box;

#define DISPLAY_TEXT \
{\
   int n_to_print=n-start;\
   if(n_to_print>fcols)n_to_print=fcols;\
   werase(f_win);\
   wbkgd(f_win,ch);\
   mvwaddnstr(f_win,0,0,args+start,n_to_print);\
   wnoutrefresh(f_win);\
   wmove(f_win,0,x-start);\
   doupdate();\
}\

int
get_args(char *args, int maxlen, char *title)
{
	WINDOW         *b_win, *f_win;
	struct termios  term;
	int             c, fcols;
	chtype          ch;
	char           *end = args, *here = args;
	cc_t            erase = 0, kill = 0;
	int             n = 0, start = 0, x = 0;

	if (tcgetattr(0, &term) >= 0)
	{
		erase = term.c_cc[VERASE];
		kill = term.c_cc[VKILL];
	}
	fcols = COLS - 4;
	if (fcols > 72)
		fcols = 72;
	ch = ' ' | A_BOLD;
	if (color == TRUE)
		ch |= COLOR_PAIR((MENU_EXEC + 1));
	b_win = newwin(5, fcols + 4, NC_VCENTRE_SC(5), NC_HCENTRE_SC(fcols + 4));
	werase(b_win);
	/*
	 * wbkgd(b_win,ch);
	 */
	box(b_win, 0, 0);
	wattrset(b_win, A_BOLD);
	mvwaddstr(b_win, 0, NC_CENTRE_AB(fcols + 4, strlen(title)), title);
	if (color == TRUE)
	{
		int             r, c;
		wattrset(b_win, COLOR_PAIR((MENU_EXEC + 1)));
		for (r = 1; r < 4; r++)
			for (c = 1; c < fcols + 3; c++)
				mvwaddch(b_win, r, c, ' ');
	}
	wattrset(b_win, A_NORMAL);
	wnoutrefresh(b_win);
	f_win = derwin(b_win, 1, fcols + 1, 2, 2);
	werase(f_win);
	wbkgd(f_win, ch);
	wnoutrefresh(f_win);
	keypad(f_win, TRUE);
	doupdate();
	errno = 0;
	wtimeout(f_win, Timeout);
	while ((c = wgetch(f_win)) != '\n' && c != ESCAPEKEY && c != 7)
	{
		if (errno == EIO)
		{
			GRAB_BACK_TTY;
			errno = 0;
			continue;
		}
		if (c == erase)
			c = KEY_BACKSPACE;
		else
		if (c == kill)
			c = 7;				/*- Ugg - Hack - see later */
		switch (c)
		{
		case ERR:
			if (checktty() == 0)
				exit(0);
			handle_timeout(0);
			touchwin(b_win);
			wnoutrefresh(b_win);
			touchwin(f_win);
			wnoutrefresh(b_win);
			doupdate();
			break;
		case KEY_BACKSPACE:
			if (x > 0)
			{
				if (here != end)
					memmove(here - 1, here, n - x);
				if ((start > 0) && ((x == start) || (n <= (start + fcols))))
					start--;
				here--, end--, n--, x--;
				DISPLAY_TEXT;
			} else
			{
				beep();
				doupdate();
			}
			break;
		case KEY_DC:
		case '\004':			/*- ctrl d */
			if (here < end)
			{
				memmove(here, here + 1, n - x - 1);
				n--, end--;
				DISPLAY_TEXT;
			} else
			{
				beep();
				doupdate();
			}
			break;
		case '\001':			/*- ctrl a */
			x = start = 0;
			here = args;
			DISPLAY_TEXT;
			break;
		case '\005':			/*- ctrl e */
			start = n < fcols ? 0 : n - fcols;
			x = n;
			here = end;
			DISPLAY_TEXT;
			break;
		case 7:				/*- kill character - we filter out real Ctrl-G earlier */
			if (n > 0)
			{
				here = end = args;
				n = start = x = 0;
				DISPLAY_TEXT;
			}
			break;
		case '\014':			/*- ctrl-l */
			display_screen(0);
			touchwin(b_win);
			wnoutrefresh(b_win);
			touchwin(f_win);
			wnoutrefresh(b_win);
			doupdate();
			wrefresh(curscr);
			break;
		case KEY_LEFT:
			if (x > 0)
			{
				if (x == start)
					start--;
				x--, here--;
				DISPLAY_TEXT;
			} else
			{
				beep();
				doupdate();
			}
			break;
		case KEY_RIGHT:
			if (x < n)
			{
				if ((x - start) == fcols)
					start++;
				x++, here++;
				DISPLAY_TEXT;
			} else
			{
				beep();
				doupdate();
			}
			break;
		default:
			if (isprint(c))
			{
				if (n < maxlen)
				{
					if ((x - start) == fcols)
						start++;
					if (here != end)
						memmove(here + 1, here, n - x);
					*here++ = c;
					end++, n++, x++;
					if (x - start > fcols)
						start++;
					DISPLAY_TEXT;
				} else
				{
					beep();
					doupdate();
				}
			} else
			{
				beep();
				doupdate();
			}
			break;
		}
	}
	wtimeout(f_win, -1);
	delwin(f_win);
	delwin(b_win);
	if ((c == ESCAPEKEY) || (c == 7))
		return 1;
	*end = '\0';
	return 0;
}

int
checktty(void)
{
	struct timeval  t;
	fd_set          ifds;
	int             n;
	char            c;

	FD_ZERO(&ifds);
	FD_SET(0, &ifds);
	t.tv_sec = 0;
	t.tv_usec = 0;

	n = select(1, &ifds, NULL, NULL, &t);
	if (n == 1)
	{
		n = read(0, &c, 1);
		if (n == 1)
			ungetch(c);
	} else
		n = 1;

	return n;
}
