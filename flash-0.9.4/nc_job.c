/*
 * $Log: nc_job.c,v $
 * Revision 1.4  2009-06-04 10:46:35+05:30  Cprogrammer
 * added conditional inclusion of ncurses
 *
 * Revision 1.3  2008-06-09 15:31:39+05:30  Cprogrammer
 * added GPL copyright notice
 *
 * Revision 1.2  2002-12-21 19:09:06+05:30  Manny
 * corrected indentation
 *
 * Revision 1.1  2002-12-16 01:55:09+05:30  Manny
 * Initial revision
 *
 *
 * Does job control menus
 *
 * Copyright (C) 1995,1996 Stephen Fegan
 *
 * Portions Copyright (C) 1995  Brian Cully
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
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <syslog.h>
#include <setjmp.h>
#include <signal.h>
#include <time.h>

#include "exec.h"
#include "menu.h"
#include "screen.h"
#include "ncr_scr.h"

extern int      color, Timeout;
extern struct job_q *JobHead;
extern sigjmp_buf jmpbuf;
extern volatile sig_atomic_t canjump;

void
task_switch(void)
{
	WINDOW         *b_win, *f_win;
	volatile chtype ch;
	char            Temp[80], *temp;
	char            inst[] = " Enter - Run   TAB - Next   K - %s   S - Stop   B - Background ";
	int             c;
	int             done = 0;
	int             newp = 0;
	static struct job_q *j;
	struct job_q   *jp;
	struct tm      *tm;

	b_win = newwin(6, 70, (LINES - 6) / 2, (COLS - 70) / 2);
	werase(b_win);
	box(b_win, 0, 0);
	wnoutrefresh(b_win);
	f_win = derwin(b_win, 4, 68, 1, 1);
	ch = ' ';
	if (color == TRUE)
		ch |= COLOR_PAIR((MENU_TITLE + 1));
	j = JobHead->prev;
	while (j != JobHead)
	{
		if ((j->rst == S_RUN) || (j->rst == S_STOP))
			break;
		j = j->prev;
	}
	if (j == JobHead)
		return;
	do
	{
		jp = j;
		sigsetjmp(jmpbuf, 1);
		canjump = 1;
		tm = localtime(&j->start);
		newp = 0;
		done = 0;
		temp = Temp;
		display_screen(0);
		touchwin(b_win);
		wnoutrefresh(b_win);
		werase(f_win);
		wbkgd(f_win, ch);
		wattrset(f_win, A_NORMAL);
		sprintf(temp, inst, ((j->jflags & JOBF_HUPSENT) == 0) ? "Hangup" : "Kill");
		mvwaddstr(f_win, 3, (68 - strlen(temp)) / 2, temp);
		sprintf(temp, "%d/%d %d:%2.2d%2s  [%d] ", tm->tm_mday, tm->tm_mon + 1, ((tm->tm_hour + 11) % 12) + 1, tm->tm_min,
				tm->tm_hour > 11 ? "pm" : "am", (int) j->pgrp);
		strncat(temp, j->cmdline, 66 - strlen(temp) - strlen(job_status(j)) - 3);
		strcat(temp, " <");
		strcat(temp, job_status(j));
		strcat(temp, ">");
		if (strlen(temp) > 66)
			*(temp + 66) = '\0';
		if ((j->rst == S_RUN) || (j->rst == S_STOP))
			wattrset(f_win, A_BOLD);
		mvwaddstr(f_win, 1, (68 - strlen(temp)) / 2, temp);
		wnoutrefresh(f_win);
		doupdate();
		wtimeout(f_win, Timeout);
		do
		{
			do
			{
				errno = 0;
				c = wgetch(f_win);
				if (errno == EIO)
					GRAB_BACK_TTY
			} while (errno != 0 && errno != ENOTTY);
			switch (c)
			{
			case ERR:
				newp = 1;
				break;
			case '\t':
				canjump = 0;
				j = j->prev;
				while (j != jp)
				{
					if ((j->rst == S_RUN) || (j->rst == S_STOP))
						break;
					j = j->prev;
				}
				newp = 1;
				break;
			case 'K':
				canjump = 0;
				kill_job(j);
				newp = 1;
				break;
			case 'S':
				canjump = 0;
				stop_job(j);
				newp = 1;
				break;
			case 'B':
				canjump = 0;
				run_in_bg(j);
				newp = 1;
				break;
			case '\n':
				canjump = 0;
				clear_close_scr();
				run_in_fg(j);
				init_scr();
				done = 1;
				break;
			case 12:		/*- Ctrl L */
				canjump = 0;
				wrefresh(curscr);
				canjump = 1;
				break;
			case ESCAPEKEY:
			case KEY_LEFT:
			case 2:		/*- Ctrl B */
			case 4:		/*- Ctrl D */
				canjump = 0;
				done = 1;
				break;
			}
		} while ((newp == 0) && (done == 0));
	} while (done == 0);
	wtimeout(f_win, -1);
	delwin(f_win);
	delwin(b_win);
	clean_jobq(0);
	return;
}

int             nc_runnable_jobs(void)
{
	WINDOW         *b_win, *f_win;
	chtype          ch;
	char            temp1[] = "There are still jobs in the background. Press Ctrl-D to";
	char            temp2[] = "kill them and logout or any other to return to the menu";
	char            c;

	if(!runnable_jobs())
		return 0;

	b_win = newwin(6, strlen(temp1) + 6, (LINES - 5) / 2, (COLS - strlen(temp1) - 6) / 2);
	werase(b_win);
	box(b_win, 0, 0);
	wnoutrefresh(b_win);
	ch = ' ' | A_BOLD;
	if (color == TRUE)
		ch |= COLOR_PAIR(PASSWORD_COLOR);
	f_win = derwin(b_win, 4, strlen(temp1), 1, 3);
	wattrset(f_win, A_BOLD);
	if (color)
		wattron(f_win, COLOR_PAIR(PASSWORD_COLOR));
	werase(f_win);
	wbkgd(f_win, ch);
	mvwaddstr(f_win, 1, 0, temp1);
	mvwaddstr(f_win, 2, 0, temp2);
	wnoutrefresh(f_win);
	beep();
	doupdate();
	do
	{
		errno = 0;
		c = wgetch(f_win);
		if (errno == EIO)
			GRAB_BACK_TTY
	} while (errno != 0 && errno != ENOTTY);
	delwin(f_win);
	delwin(b_win);
	if (c == 4)
		return 0;
	else
		return 1;
}
