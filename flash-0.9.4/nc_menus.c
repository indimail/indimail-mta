/*
 * $Log: nc_menus.c,v $
 * Revision 1.5  2012-12-13 09:28:39+05:30  Cprogrammer
 * fixed indentation
 *
 * Revision 1.4  2011-07-29 09:24:13+05:30  Cprogrammer
 * fixed gcc warnings
 *
 * Revision 1.3  2009-06-04 10:46:41+05:30  Cprogrammer
 * added conditional inclusion of ncurses
 *
 * Revision 1.2  2008-06-09 15:31:52+05:30  Cprogrammer
 * added GPL copyright notice
 *
 * Revision 1.1  2002-12-16 01:55:11+05:30  Manny
 * Initial revision
 *
 *
 * Displays menu and takes input
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
 * ANY WARRANTY; without even the implied warranty of MECRHANTABILITY or FITNESS
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
#include <time.h>

#include "ncr_scr.h"
#include "mystring.h"
#include "screen.h"
#include "parse.h"
#include "misc.h"
#include "exec.h"
#include "menu.h"
#include "set.h"

#define DEFAULT_TIMEOUT 2

int             has_menu_access(struct menu *menu);

extern bool     color;
extern int      gotwinch;
extern struct menu *main_menu;
extern WINDOW  *topbar, *bottombar;
extern struct DisplayCallbacks DCBs;

char            draw_box = 1;
struct menu_instance *mi_head = NULL;
int             Timeout = DEFAULT_TIMEOUT;
extern time_t   TimeNow;

static void
display_items(WINDOW * win, struct menu_items *src, int cols, int hilite, int nocolour)
{
	int             i;
	int             prespace = 0, postspace = 0, width;
	unsigned int    align = (src->flags & MIF_ALIGN);

	wattrset(win, A_NORMAL);
#ifndef DONT_HIGHLIGHT_WITH_REVERSE
	wattrset(win, hilite ? A_REVERSE : A_BOLD);
	if ((color) && (!nocolour))
		wattron(win, COLOR_PAIR((src->type + 1)));
#else
	if ((color) && (!nocolour))
	{
		wattrset(win, A_BOLD);
		wattron(win, hilite ? COLOR_PAIR(HIGHLIGHT_COLOR) : COLOR_PAIR((src->type + 1)));
	} else
		wattrset(win, hilite ? A_REVERSE : A_BOLD);
#endif

	width = strlen(src->name) + (src->type == MENU_SUB ? 2 : 0);

	if (align == MIF_NONE)
		align = (src->type == MENU_TITLE) ? MIF_CENTRE : MIF_LEFT;

	switch (align)
	{
	case MIF_LEFT:
		prespace = NC_LEFT_AB(cols, width);
		postspace = NC_RIGHT_AB(cols, width);
		break;
	case MIF_RIGHT:
		prespace = NC_RIGHT_AB(cols, width);
		postspace = NC_LEFT_AB(cols, width);
		break;
	case MIF_CENTRE:
		prespace = NC_CENTRE_AB(cols, width);
		postspace = NC_RIGHT_AB(cols, width + prespace);
		break;
	}

	switch (src->type)
	{
	case MENU_SUB:
		for (i = 0; i < prespace; i++)
			waddch(win, ' ');
		/*- waddch(win, '<'); -*/
		waddstr(win, src->name);
		waddch(win, '-');
		waddch(win, '>');
		break;

	case MENU_NOP:
		wattroff(win, hilite ? A_REVERSE : A_BOLD);
		if (strlen(src->name) >= 1)
		{
			for (i = 0; i < prespace; i++)
				waddch(win, ' ');
			waddstr(win, src->name);
		} else
		{
			postspace = 0;
			whline(win, ACS_HLINE, cols);
		}
		break;

	default:
		for (i = 0; i < prespace; i++)
			waddch(win, ' ');
		waddstr(win, src->name);
		break;
	}

	if (postspace > 0)
		for (i = 0; i < postspace; i++)
			waddch(win, ' ');

	wattrset(win, A_NORMAL);
}

#define ISCLICKABLE(x) ((x!=MENU_TITLE)&&(x!=MENU_NOP))
#define M_ISSET(x)    ((mi->menu->flags & x)!=0)
#define M_ISNOTSET(x) ((mi->menu->flags & x)==0)
#define M_DRAW_BOX    ((draw_box==1)&&M_ISNOTSET(MF_NOBOX))
#define M_SCROLL_BAR  (M_ISSET(MF_SCROLLBAR))
#define CANCLICK(x)   (((x->menu->flags & MF_NOCURSOR)==0)&&(x->mi_highlighted!=NULL)&&(x->activepane_cursor>=0)&&(x->activepane_cursor<x->activepane_lines))

void
findfirstclickableitem(struct menu_instance *mi)
{
	struct menu_items *q = mi->mi_activepane_start;
	int             i;

	for (i = 0; i < mi->activepane_lines; i++)
	{
		if ((q == NULL) || (ISCLICKABLE(q->type)))
			break;
		q = q->next;
	}

	if ((q == NULL) || (i == mi->activepane_lines))
		mi->activepane_cursor = mi->activepane_lines - 1, mi->mi_highlighted = NULL;
	else
		mi->activepane_cursor = i, mi->mi_highlighted = q;
}

void
findlastclickableitem(struct menu_instance *mi)
{
	struct menu_items *q = mi->mi_activepane_start;
	int             i;

	mi->mi_highlighted = NULL;

	for (i = 0; i < mi->activepane_lines; i++)
	{
		if (ISCLICKABLE(q->type))
			mi->mi_highlighted = q, mi->activepane_cursor = i;
		q = q->next;
	}
}

static void
displaypane(struct menu_instance *mi)
{
	struct menu_items *q = mi->mi_activepane_start;
	chtype          ch;
	int             i;
	int             nocolour = (mi->menu->flags & MF_NOCOLOUR);

	if (M_SCROLL_BAR)
	{
		int             n, nhead, y, j, s, e;

		for (n = 0; (n < mi->activepane_lines) && (q != NULL); n++)
			q = q->next;
		q = mi->menu->head;
		for (nhead = 0; q != NULL; nhead++)
			q = q->next;
		q = mi->mi_activepane_start;

		y = nhead + (M_DRAW_BOX && (nhead > 0) ? 1 : 0) + 1;

		wattrset(mi->background_win, A_NORMAL);
		if ((color) && (!nocolour))
			wattron(mi->background_win, COLOR_PAIR((MENU_TITLE + 1)));

		for (j = 0; j < mi->activepane_lines; j++, y++)
			mvwaddch(mi->background_win, y, mi->activepane_cols + (M_DRAW_BOX ? 1 : 0) + 1, ' ');

		y = nhead + (M_DRAW_BOX && (nhead > 0) ? 1 : 0) + 1;
		s = y + (mi->activepane_s * mi->activepane_lines) / mi->nitems;
		e = y + ((n + mi->activepane_s - 1) * mi->activepane_lines) / mi->nitems;

		for (y = s; y <= e; y++)
			mvwaddch(mi->background_win, y, mi->activepane_cols + (M_DRAW_BOX ? 1 : 0) + 1, ACS_BOARD);
	}

	ch = ' ';
	if ((color == TRUE) && (!nocolour))
		ch |= COLOR_PAIR((MENU_NOP + 1));
	wbkgd(mi->activepane_win, ch);
	werase(mi->activepane_win);
	ch = ' ';
	wbkgdset(mi->activepane_win, ch);

	/*
	 * if((color==TRUE)&&(!nocolour))ch|=COLOR_PAIR((MENU_NOP+1));
	 * wbkgdset(mi->background_win,ch);
	 * wattrset(mi->background_win,A_NORMAL);
	 */

	q = mi->mi_activepane_start;
	for (i = 0; i < mi->activepane_lines; i++)
	{
		if (q == NULL)
			break;

		wmove(mi->activepane_win, i, 0);
		display_items(mi->activepane_win, q, mi->activepane_cols,
					  ((q == mi->mi_highlighted) &&
					   ((mi->menu->flags & MF_NOCURSOR) == 0) ? TRUE : FALSE), (mi->menu->flags & MF_NOCOLOUR));

		q = q->next;
	}

	wmove(mi->activepane_win, mi->activepane_cursor, mi->activepane_cols - 1);
}

static void
scrolldown(struct menu_instance *mi, int n)
{
	struct menu_items *q = mi->mi_activepane_start;
	int             i, backtrack = 1;

	if (n == 0)
		n = mi->activepane_lines, backtrack = 0;
	else
	if (n < 0)
		n = -n, backtrack = 0;

	i = 0;
	q = mi->mi_activepane_start;

	while ((q != NULL) && (i < n))
	{
		q = q->next;
		i++;
	}

	if (q == NULL)
		return;

	mi->mi_activepane_start = q;
	mi->activepane_s += i;
	mi->activepane_cursor -= i;

	if (backtrack == 1)
	{
		i = 0;

		while ((q != NULL) && (i < mi->activepane_lines))
		{
			q = q->next;
			i++;
		}

		q = mi->mi_activepane_start;

		while ((q->prev != NULL) && (i < mi->activepane_lines))
		{
			q = q->prev;
			i++;
			mi->activepane_cursor++;
			mi->activepane_s--;
		}

		mi->mi_activepane_start = q;
	}

	return;
}

static void
scrollup(struct menu_instance *mi, int n)
{
	struct menu_items *q = mi->mi_activepane_start;
	int             i;

	if (n == 0)
		n = mi->activepane_lines;
	else
	if (n < 0)
		n = -n;

	i = 0;
	q = mi->mi_activepane_start;

	while ((q != NULL) && (i < n))
	{
		q = q->prev;
		i++;
	}

	if (q == NULL)
		mi->mi_activepane_start = mi->menu->data, mi->activepane_s = 0, mi->activepane_cursor += (i - 1);
	else
		mi->mi_activepane_start = q, mi->activepane_s -= i, mi->activepane_cursor += i;

	return;
}

void
pagedown(struct menu_instance *mi, int n)
{
	scrolldown(mi, -n);

	findfirstclickableitem(mi);

	displaypane(mi);
}

void
pageup(struct menu_instance *mi, int n)
{
	scrollup(mi, -n);

	findfirstclickableitem(mi);

	displaypane(mi);
}

/*
 * Go down at least one row, skipping comments (MENU_NOP) and titles (MENU_TITLE)
 * *row == a pointer to the current row
 * **p == menu item list to sort through and display 
 */

void
down_hilite(struct menu_instance *mi)
{
	struct menu_items *q = mi->mi_highlighted;
	int             i;

	if (q == NULL)
	{
		scrolldown(mi, mi->activepane_lines - 1);
		findfirstclickableitem(mi);
		displaypane(mi);
	} else
	{
		i = mi->activepane_cursor;

		i++;
		q = q->next;
		while (q != NULL)
		{
			if (ISCLICKABLE(q->type))
				break;
			i++;
			q = q->next;
		}

		if ((q == NULL) || (i >= mi->activepane_lines))
		{
			if (q != NULL)
			{
				mi->mi_highlighted = q;
				mi->activepane_cursor = i;
			}
			scrolldown(mi, mi->activepane_lines - 1);
			if ((mi->activepane_cursor < 0) || (mi->activepane_cursor >= mi->activepane_lines))
				findfirstclickableitem(mi);
			displaypane(mi);
		} else
		{
			wmove(mi->activepane_win, mi->activepane_cursor, 0);
			display_items(mi->activepane_win, mi->mi_highlighted, mi->activepane_cols, FALSE, (mi->menu->flags & MF_NOCOLOUR));

			mi->mi_highlighted = q;
			mi->activepane_cursor = i;

			wmove(mi->activepane_win, mi->activepane_cursor, 0);
			display_items(mi->activepane_win, mi->mi_highlighted, mi->activepane_cols, TRUE, (mi->menu->flags & MF_NOCOLOUR));

			wmove(mi->activepane_win, mi->activepane_cursor, mi->activepane_cols - 1);
		}
	}
}

void
up_hilite(struct menu_instance *mi)
{
	struct menu_items *q = mi->mi_highlighted;
	int             i;

	if (q == NULL)
	{
		scrollup(mi, mi->activepane_lines - 1);
		findlastclickableitem(mi);
		displaypane(mi);
	} else
	{
		i = mi->activepane_cursor;

		i--;
		q = q->prev;
		while (q != NULL)
		{
			if (ISCLICKABLE(q->type))
				break;
			i--;
			q = q->prev;
		}

		if ((q == NULL) || (i < 0))
		{
			if (q != NULL)
			{
				mi->mi_highlighted = q;
				mi->activepane_cursor = i;
			}
			scrollup(mi, mi->activepane_lines - 1);
			if ((mi->activepane_cursor < 0) || (mi->activepane_cursor >= mi->activepane_lines))
				findlastclickableitem(mi);

			displaypane(mi);
		} else
		{
			wmove(mi->activepane_win, mi->activepane_cursor, 0);
			display_items(mi->activepane_win, mi->mi_highlighted, mi->activepane_cols, FALSE, (mi->menu->flags & MF_NOCOLOUR));

			mi->mi_highlighted = q;
			mi->activepane_cursor = i;

			wmove(mi->activepane_win, mi->activepane_cursor, 0);
			display_items(mi->activepane_win, mi->mi_highlighted, mi->activepane_cols, TRUE, (mi->menu->flags & MF_NOCOLOUR));

			wmove(mi->activepane_win, mi->activepane_cursor, mi->activepane_cols - 1);
		}
	}
}

void
init_menus(void)
{
	struct menu_instance *mi;

	for (mi = mi_head; mi != NULL; mi = mi->next)
		render_instance(mi);

	return;
}

struct menu_instance *
add_menu_instance(struct menu *menu)
{
	struct menu_instance *mi;
	struct menu_instance *mi_new;

	mi_new = (struct menu_instance *) xmalloc(sizeof(*mi));

	if (mi_head == NULL)
	{
		mi_head = mi_new;
		mi_new->prev = NULL;
	} else
	{
		for (mi = mi_head; mi->next != NULL; mi = mi->next);
		mi->next = mi_new;
		mi_new->prev = mi;
	}

	mi = mi_new;
	mi->next = NULL;

	mi->menu = menu;

	mi->HotKeys = xmalloc(256 * sizeof(struct menu_items *));
	setuphotkeys(0, mi->HotKeys, menu);

	mi->background_win = NULL;
	mi->activepane_win = NULL;

	mi->background_cols = mi->background_lines = 0;
	mi->activepane_cols = mi->activepane_lines = 0;

	mi->activepane_s = mi->activepane_cursor = 0;

	mi->mi_activepane_start = mi->mi_highlighted = mi->menu->data;
	mi->nitems = 0;
	/*
	 * mi->row = 0; 
	 */

	return mi;
}

struct menu_instance *
rubout_menu_instance(struct menu_instance *mi)
{
	struct menu_instance *mi_p = mi->prev;
	struct menu_instance *mi_n = mi->next;

	if (mi_n)
		mi_n->prev = mi_p;

	if (mi_p)
		mi_p->next = mi_n;
	else
		mi_head = mi_n;

	if (mi->activepane_win != NULL)
		delwin(mi->activepane_win);
	if (mi->background_win != NULL)
		delwin(mi->background_win);

	free(mi->HotKeys);
	free((void *) mi);

	return mi_p;
}


void
render_instance(struct menu_instance *mi)
{
	struct menu_items *items = mi->menu->data, *head = mi->menu->head, *tail = mi->menu->tail, *q;
	int             nhead, ntail;
	int             ncols, nlines, i;
	int             menul, menuc;
	int             nocolour = (mi->menu->flags & MF_NOCOLOUR);

	mi->nitems = 0;
	nhead = ntail = nlines = ncols = 0;

	while (items->prev)
		items = items->prev;

	q = items;
	while (q)
	{
		if ((i = strlen(q->name) + ((q->type == MENU_SUB) ? 2 : ((q->type == MENU_TITLE) ? 2 : 0))) > ncols)
			ncols = i;
		mi->nitems++;
		q = q->next;
	}

	q = head;
	while (q)
	{
		if ((i = strlen(q->name) + ((q->type == MENU_SUB) ? 2 : ((q->type == MENU_TITLE) ? 2 : 0))) > ncols)
			ncols = i;
		nhead++;
		q = q->next;
	}

	q = tail;
	while (q)
	{
		if ((i = strlen(q->name) + ((q->type == MENU_SUB) ? 2 : ((q->type == MENU_TITLE) ? 2 : 0))) > ncols)
			ncols = i;
		ntail++;
		q = q->next;
	}

	mi->background_lines = mi->nitems + nhead + ntail + (nhead > 0 ? 1 : 0) + (ntail > 0 ? 1 : 0) + (M_DRAW_BOX ? 2 : 0);
	mi->background_cols = ncols + (M_DRAW_BOX ? 2 : 0) + (M_SCROLL_BAR ? 2 : 0);

	if (M_ISSET(MF_FIXEDSIZE))
	{
		if (mi->menu->ncols > 0)
			mi->background_cols = mi->menu->ncols;
		if (mi->menu->nlines > 0)
			mi->background_lines = mi->menu->nlines;
	}

	if (mi->background_lines > LINES)
		mi->background_lines = LINES;
	if (mi->background_cols > COLS)
		mi->background_cols = COLS;

	mi->activepane_lines =
		mi->background_lines - (nhead + ntail + (nhead > 0 ? 1 : 0) + (ntail > 0 ? 1 : 0) + (M_DRAW_BOX ? 2 : 0));
	mi->activepane_cols = mi->background_cols - ((M_DRAW_BOX ? 2 : 0) + (M_SCROLL_BAR ? 2 : 0));

	if ((mi->activepane_lines <= 0) || (mi->background_cols <= 0))
		error("No space in active pane");

	if (mi->menu->flags & MF_TOP)
		menul = (mi->background_lines >= LINES) ? 0 : 1;
	else
	if (mi->menu->flags & MF_BOTTOM)
		menul = (mi->background_lines >= LINES) ? 0 : LINES - mi->background_lines - 1;
	else
		menul = (LINES - mi->background_lines) / 2;

	if (mi->menu->flags & MF_OFFSET)
	{
		menul += mi->menu->offl;
		if (menul < 0)
			menul = 0;
		else
		if ((menul + mi->background_lines) > LINES)
			menul = LINES - mi->background_lines;
	}

	if (mi->menu->flags & MF_LEFT)
		menuc = (mi->background_cols >= COLS) ? 0 : 1;
	else
	if (mi->menu->flags & MF_RIGHT)
		menuc = (mi->background_cols >= COLS) ? 0 : COLS - mi->background_cols - 1;
	else
		menuc = (COLS - mi->background_cols) / 2;

	if (mi->menu->flags & MF_OFFSET)
	{
		menuc += mi->menu->offc;
		if (menuc < 0)
			menuc = 0;
		else
		if ((menul + mi->background_cols) > COLS)
			menuc = COLS - mi->background_cols;
	}

	mi->background_win = newwin(mi->background_lines, mi->background_cols, menul, menuc);

	if (mi->background_win == NULL)
		error("Couldn't create border window");

	/*
	 * if((color==TRUE)&&(!nocolour))ch|=COLOR_PAIR((MENU_NOP+1));
	 * wbkgd(mi->background_win,ch);
	 */
	werase(mi->background_win);
	if (M_DRAW_BOX)
		box(mi->background_win, 0, 0);

	mi->activepane_win =
		derwin(mi->background_win, mi->activepane_lines, mi->activepane_cols, nhead + (nhead > 0 ? 1 : 0) + (M_DRAW_BOX ? 1 : 0),
			   (M_DRAW_BOX ? 1 : 0));

	if (mi->activepane_win == NULL)
		error("Couldn't create main window");

	if (nhead > 0)
	{
		i = M_DRAW_BOX ? 1 : 0;
		q = mi->menu->head;
		while (q)
		{
			wmove(mi->background_win, i, M_DRAW_BOX ? 1 : 0);
			display_items(mi->background_win, q, mi->background_cols - (M_DRAW_BOX ? 2 : 0), FALSE,
						  (mi->menu->flags & MF_NOCOLOUR));
			q = q->next;
			i++;
		}

		wattrset(mi->background_win, A_NORMAL);
		if ((color == TRUE) && (!nocolour))
			wattron(mi->background_win, COLOR_PAIR((MENU_NOP + 1)));

		wmove(mi->background_win, i, M_DRAW_BOX ? 1 : 0);
		whline(mi->background_win, ACS_HLINE, mi->background_cols - (M_DRAW_BOX ? 2 : 0));

		wattrset(mi->background_win, A_NORMAL);

		wmove(mi->background_win, i, 0);
		if (M_DRAW_BOX)
			waddch(mi->background_win, ACS_LTEE);
		wmove(mi->background_win, i, mi->background_cols - 1);
		if (M_DRAW_BOX)
			waddch(mi->background_win, ACS_RTEE);
	}

	q = mi->mi_activepane_start = mi->menu->data;
	mi->activepane_s = 0;
	mi->activepane_cursor = 0;

	if (ntail > 0)
	{
		i = mi->background_lines - ntail - 1 - (M_DRAW_BOX ? 1 : 0);
		q = mi->menu->tail;

		wattrset(mi->background_win, A_NORMAL);
		if ((color == TRUE) && (!nocolour))
			wattron(mi->background_win, COLOR_PAIR((MENU_NOP + 1)));

		wmove(mi->background_win, i, M_DRAW_BOX ? 1 : 0);
		whline(mi->background_win, ACS_HLINE, mi->background_cols - (M_DRAW_BOX ? 2 : 0));

		wattrset(mi->background_win, A_NORMAL);

		wmove(mi->background_win, i, 0);
		if (M_DRAW_BOX)
			waddch(mi->background_win, ACS_LTEE);
		wmove(mi->background_win, i, mi->background_cols - 1);
		if (M_DRAW_BOX)
			waddch(mi->background_win, ACS_RTEE);

		i++;
		while (q)
		{
			wmove(mi->background_win, i, M_DRAW_BOX ? 1 : 0);
			display_items(mi->background_win, q, mi->background_cols - (M_DRAW_BOX ? 2 : 0), FALSE,
						  (mi->menu->flags & MF_NOCOLOUR));
			q = q->next;
			i++;
		}
	}

	if (M_SCROLL_BAR)
	{
		wattrset(mi->background_win, A_NORMAL);
		if ((color == TRUE) && (!nocolour))
			wattron(mi->background_win, COLOR_PAIR((MENU_NOP + 1)));

		wmove(mi->background_win, nhead + (M_DRAW_BOX && (nhead > 0) ? 1 : 0), mi->activepane_cols + (M_DRAW_BOX ? 1 : 0));
		wvline(mi->background_win, ACS_VLINE,
			   mi->activepane_lines + (M_DRAW_BOX ? 2 : ((nhead > 0 ? 1 : 0) + (ntail > 0 ? 1 : 0))));

		if ((M_DRAW_BOX) || (nhead > 0))
		{
			wattrset(mi->background_win, A_NORMAL);
			if ((nhead > 0) && (color == TRUE) && (!nocolour))
				wattron(mi->background_win, COLOR_PAIR((MENU_NOP + 1)));
			wmove(mi->background_win, nhead + (M_DRAW_BOX && (nhead > 0) ? 1 : 0), mi->activepane_cols + (M_DRAW_BOX ? 1 : 0));
			waddch(mi->background_win, ACS_TTEE);
		}
		if ((M_DRAW_BOX) || (ntail > 0))
		{
			wattrset(mi->background_win, A_NORMAL);
			if ((ntail > 0) && (color == TRUE) && (!nocolour))
				wattron(mi->background_win, COLOR_PAIR((MENU_NOP + 1)));
			wmove(mi->background_win, mi->background_lines - ntail - (M_DRAW_BOX && (ntail > 0) ? 1 : 0) - 1,
				  mi->activepane_cols + (M_DRAW_BOX ? 1 : 0));
			waddch(mi->background_win, ACS_BTEE);
		}

		wattrset(mi->background_win, A_NORMAL);
	}

	findfirstclickableitem(mi);

	displaypane(mi);
}

void
display_menu(struct menu_instance *mi_c)
{
	if (mi_c->background_win)
		wnoutrefresh(mi_c->background_win);
	if (mi_c->activepane_win)
		wnoutrefresh(mi_c->activepane_win);

	doupdate();
}

void
display_screen(int update)
{
	struct menu_instance *mi;
	struct Callback *cb, *cbl;

	time(&TimeNow);

	werase(stdscr);
	wnoutrefresh(stdscr);

	touchwin(topbar);
	wnoutrefresh(topbar);
	touchwin(bottombar);
	wnoutrefresh(bottombar);

	for (cbl = &DCBs.undermenu, cb = cbl->next; cb != cbl; cb = cb->next)
		(*cb->f) ();

	for (mi = mi_head; mi != NULL; mi = mi->next)
	{
		if (mi->background_win)
			touchwin(mi->background_win), wnoutrefresh(mi->background_win);
		if (mi->activepane_win)
			touchwin(mi->activepane_win), wnoutrefresh(mi->activepane_win);
	}

	for (cbl = &DCBs.overmenu, cb = cbl->next; cb != cbl; cb = cb->next)
		(*cb->f) ();
	for (cbl = &DCBs.overall, cb = cbl->next; cb != cbl; cb = cb->next)
		(*cb->f) ();

	if (update)
		doupdate();
}

void
handle_timeout(int update)
{
	struct TOCallback *cb, *cbl;
	int             redisp = 0;

	time(&TimeNow);

	for (cbl = &DCBs.timeout, cb = cbl->next; cb != cbl; cb = cb->next)
		if ((*cb->f) (redisp) == 1)
			redisp = 1;

	if ((redisp) && (update != -1))
		display_screen(update);

	return;
}

void
do_menu(struct menu *firstmenu)
{
	struct menu_instance *mi_c, *mi_ip;
	struct menu    *menu;
	struct menu_items *HK;
	int             noclobber, gotch;

	if (!(firstmenu))
		return;

	noclobber = find_variable("noclobber", NULL);

	Timeout *= 1000;

	if (find_variable("notimeout", NULL))
		Timeout = -1;

	mi_c = add_menu_instance(firstmenu);
	mi_ip = mi_c->prev;

	render_instance(mi_c);
	timeout(Timeout);

	display_screen(1);

	while ((mi_c) && (mi_c != mi_ip))
	{
		do
		{
			errno = 0;
			gotch = getch();
			if (errno == EIO)
				GRAB_BACK_TTY
		} while (errno != 0 && errno != ENOTTY);

		switch (gotch)
		{
		case ERR: /*- fprintf(stderr,"%d\n",errno); */
			if (checktty() == 0)
				return;
			handle_timeout(1);
			break;
		case ESCAPEKEY:
		case KEY_LEFT:
		case 2:	/*- Ctrl B */
			if (mi_c->prev)
			{
				mi_c = rubout_menu_instance(mi_c);
				display_screen(1);
			} else
			if (!noclobber)
			{
				if (nc_runnable_jobs())
					display_screen(1);
				else
					mi_c = rubout_menu_instance(mi_c);
			}
			break;
		case '\n':
		case KEY_RIGHT:
			if (CANCLICK(mi_c))
			{
				switch (mi_c->mi_highlighted->type)
				{
				case MENU_SUB:
					menu = find_menu(mi_c->mi_highlighted->args);
					if (menu != NULL)
					{
						if (menu != mi_c->menu)
						{
							if (has_menu_access(menu))
							{
								if (mi_c->menu->flags & MF_OVERLAY)
									rubout_menu_instance(mi_c);
								mi_c = add_menu_instance(menu);
								render_instance(mi_c);
							}
							display_screen(1);
						}
					} else
						fprintf(stderr, "No menu named %s\n", mi_c->mi_highlighted->name);
					break;
				case MENU_EXIT:
					mi_c = rubout_menu_instance(mi_c);
					if (mi_c)
						display_screen(1);
					break;
				case MENU_QUIT:
					if (nc_runnable_jobs())
						display_screen(1);
					else
					{
						while (mi_c)
							mi_c = rubout_menu_instance(mi_c);
					}
					break;
				default:
					timeout(-1);
					exec_item(mi_c->mi_highlighted);
					timeout(Timeout);
					if (mi_c->menu->flags & MF_OVERLAY)
					{
						mi_c = rubout_menu_instance(mi_c);
						if (mi_c)
							display_screen(1);
					} else
					display_screen(1);
					break;
				} /*- switch (mi_c->mi_highlighted->type) */
				break;
			}
		case KEY_NEXT:
		case '\t':
			task_switch();
			display_screen(1);
			break;
		case KEY_DOWN:
		case 14: /*- Ctrl N */
			if ((mi_c->menu->flags & MF_NOCURSOR) == 0)
			{
				down_hilite(mi_c);
				display_screen(1);
			}
			break;
		case KEY_UP:
		case 16: /*- Ctrl P */
			if ((mi_c->menu->flags & MF_NOCURSOR) == 0)
			{
				up_hilite(mi_c);
				display_screen(1);
			}
			break;
		case KEY_NPAGE:
		case 5:
			pagedown(mi_c, 0);
			display_menu(mi_c);
			break;
		case KEY_PPAGE:
		case 23:
			pageup(mi_c, 0);
			display_menu(mi_c);
			break;
		/*-
		case 7:
			timeout(-1);
			display_file (PATH_TO_GPL);
			timeout(Timeout);
			display_screen(1);
			break;
		*/
		case 8:
		case KEY_BACKSPACE:	/*- Ug */
		case '?':
			timeout(-1);
			nc_menu_help();
			timeout(Timeout);
			display_screen(1);
			break;
		case 'A':
			timeout(-1);
			nc_about();
			timeout(Timeout);
			display_screen(1);
			break;
		case 'L':
			timeout(-1);
			nc_lock_screen();
			timeout(Timeout);
			display_screen(1);
			break;
		case 4: /*- if(!noclobber) */
			{
				if (nc_runnable_jobs())
					display_screen(1);
				else
				{
					while (mi_c)
						mi_c = rubout_menu_instance(mi_c);
				}
			}
			break;

		case 12:
			display_screen(1);
			wrefresh(curscr);
			break;
		case 11:
			menu = find_menu("HotKeys");
			if (menu != NULL)
			{
				if (menu != mi_c->menu)
				{
					mi_c = add_menu_instance(menu);
					render_instance(mi_c);
					display_screen(1);
				}
			}
			break;
		default:
			if ((mi_c->HotKeys != NULL) && ((int) gotch >= 0) && ((int) gotch <= 255))
			{
				HK = *(mi_c->HotKeys + (int) gotch);
				if (HK != NULL)
				{
					switch (HK->type)
					{
					case MENU_SUB:
						menu = find_menu(HK->args);
						if (menu != NULL)
						{
							if (menu != mi_c->menu)
							{
								if (has_menu_access(menu))
								{
									mi_c = add_menu_instance(menu);
									render_instance(mi_c);
								}
								display_screen(1);
							}
						} else
							fprintf(stderr, "No menu named %s\n", HK->name);
						break;

					case MENU_EXIT:
						mi_c = rubout_menu_instance(mi_c);
						if (mi_c)
							display_screen(1);
						break;

					case MENU_QUIT:
						while (mi_c)
							mi_c = rubout_menu_instance(mi_c);
						break;

					default:
						timeout(-1);
						exec_item(HK);
						timeout(Timeout);
						display_screen(1);
						break;
					}
				}
			}
			break;
		}
	}
	timeout(-1);
}

int
has_menu_access(struct menu *menu)
{
	WINDOW         *b_win, *f_win;
	char            password[26];
	char           *cpassword;
	int             hasaccess = 0;
	chtype          ch;

	if ((menu->flags & MF_PASSWORD) == 0)
		return 1;

	b_win = newwin(7, 40, (LINES - 4) / 2, (COLS - 40) / 2);
	werase(b_win);
	box(b_win, 0, 0);
	wnoutrefresh(b_win);

	f_win = derwin(b_win, 5, 38, 1, 1);
	ch = ' ' | A_BOLD;
	if (color == TRUE)
		ch |= COLOR_PAIR(PASSWORD_COLOR);
	wattrset(f_win, A_BOLD);
	if (color)
		wattron(f_win, COLOR_PAIR(PASSWORD_COLOR));

	werase(f_win);
	wbkgd(f_win, ch);
	mvwaddstr(f_win, 1, 0, " This menu is protected by a password ");
	mvwaddstr(f_win, 3, 0, " Password >                         < ");
	wnoutrefresh(f_win);
	doupdate();

	get_password(b_win, f_win, 3, 11, password, 25, -1);

	cpassword = shacrypt(password, menu->password);

	hasaccess = !strcmp(cpassword, menu->password);

	if ((hasaccess) && (find_variable("logging", NULL) == 1))
		syslog(LOG_LOCAL1 | LOG_INFO, "%d: Access to protected menu %s", getpid(), menu->name);

	delwin(f_win);
	delwin(b_win);

	return hasaccess;
}
