/*
 * $Log: parse.c,v $
 * Revision 1.3  2008-07-17 21:38:38+05:30  Cprogrammer
 * moved progname, GlobalHotKeys to variables.h
 *
 * Revision 1.2  2008-06-09 15:32:31+05:30  Cprogrammer
 * added GPL copyright notice
 *
 * Revision 1.1  2002-12-16 01:55:15+05:30  Manny
 * Initial revision
 *
 *
 * Parses menu file into a linked list
 *
 * Copyright (C) 1996  Stephen Fegan
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
 * please send patches or advice to: `flash@netsoc.ucd.ie'
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "parseline.h"
#include "mystring.h"
#include "parse.h"
#include "module.h"
#include "menu.h"
#include "misc.h"
#include "exec.h"
#include "rc.h"
#include "variables.h"

char            delim = DELIM;
char            comment = COMMENT;

extern char     draw_box;

void            set_nobox(int, char **, FILE *, int *);
void            include_file(int, char **, FILE *, int *);

typedef void    (*PROCESSFN) (int, char **, FILE *, int *);

struct directive
{
	char           *str;
	PROCESSFN       func;
};

struct directive menu_directives[] = {
	{MENU_BEGIN, define_menu},
	{DIREC_RCFILE, rc_file},
	{RCBLOCK_BEGIN, rc_block},
	{DIREC_NOBOX, set_nobox},
	{MODULE_BEGIN, module},
	{INCLUDE_FILE, include_file},
	{SET_MAIN_MENU, set_mainmenu},
	{NULL, NULL}
};

static          PROCESSFN
get_directive(char *name)
{
	int             i = 0, found = 0;

	while (menu_directives[i].str)
	{
		if (!strcasecmp(name, menu_directives[i].str))
		{
			found = 1;
			break;
		}
		i++;
	}

	if (!found)
		return NULL;

#ifdef DEBUG
	fprintf(stderr, "Directive: `%s'\n", menu_directives[i].str);
#endif

	return menu_directives[i].func;
}

void
set_nobox(int wordc, char **wordv, FILE * fp, int *menuline)
{
	draw_box = 0;
	return;
}

void
include_file(int wordc, char **wordv, FILE * fp, int *menuline)
{
	wordc--, wordv++;
	while (wordc--)
		parsefile(*wordv++);
	return;
}

/*
 * 
 * Parses the file. Generates Binary Search Tree for menus and processes
 * RCBlock / RCFile entries.
 * 
 * menufile == loaded menu file
 * 
 * returns -1 on error, 0 otherwise
 * 
 */

int
parsefile(char *filename)
{
	char           *wordv[MAX_LINE];
	char           *line;
	int             wordc = 0;
	int             menuline = 0;
	FILE           *fp;

#ifdef DEBUG
	fprintf(stderr, "In parsefile()\n");
#endif

	filename = stradp(filename, 0);

	/*
	 * Load Files, get stats, and malloc() buffer, read file into buffer 
	 */

	if ((fp = fopen(filename, "r")) == NULL)
	{
		fprintf(stderr, "%s: Couldn't open file %s for reading\n", progname, filename);
		return -1;
	}
#ifdef DEBUG
	fprintf(stderr, "Opened file: %s\n", filename);
#endif

	line = Readline(fp);
	menuline++;

	while (line != NULL)
	{
		PROCESSFN       func;

#ifdef DEBUG
		fprintf(stderr, "parsefile: %d: %s\n", menuline, line);
#endif

		if (parseline(line, &wordc, wordv, MAX_LINE - 1) < 0)
			parse_error("Parseline error", menuline);

		if (wordc)
		{
			func = get_directive(*wordv);

			if (func)
				(*func) (wordc, wordv, fp, &menuline);
			else
			{
				char            mess[80];
				sprintf(mess, "Unrecognised directive %-30s\n", *wordv);
				parse_error(mess, menuline);
			}

			while (wordc--)
				free(*(wordv + wordc));
		}

		line = Readline(fp);
		menuline++;
	}

	fclose(fp);

	return 0;
}

/*
 * frees up allocated memory
 * ** On entry:
 * **    menuptr == linked menu list
 * **    menufile == loaded menu file
 * ** On Exit:
 * **    nothing, DONT USE menuptr OR menufile without allocating them again
 */

void
freemem(char *menufile)
{
	freemenus();
	free(menufile);
}

void
setupglobalhotkeys(void)
{
	int             i;
	struct menu    *menu;
	struct menu_items *item;

	menu = find_menu(MENU_NAME_HOTKEYS);
	if (menu == NULL)
		return;

	GlobalHotKeys = xmalloc(256 * sizeof(struct menu_items *));
	for (i = 0; i < 256; i++)
		*(GlobalHotKeys + i) = NULL;

	item = menu->data;
	while (item != NULL)
	{
		if (item->flags & MIF_HOTKEY)
			*(GlobalHotKeys + (int) item->hotkey) = item;
		item = item->next;
	}

	return;
}

/*
 * This routine set up the menu HotKeys. Local HotKeys overide global
 * ones. I'm not sure I'm happy at this but it is the way it goes at
 * the moment
 */
void
setuphotkeys(int numberoptions, struct menu_items **HotKeys, struct menu *menu)
{
	struct menu_items *item;
	int             i;

	if (HotKeys == NULL)
		return;

	if (GlobalHotKeys != NULL)
		for (i = 0; i < 256; i++)
			*(HotKeys + i) = *(GlobalHotKeys + i);

	if (numberoptions)
	{
		int             j = 1;
		for (item = menu->data; (item) && (j <= 9); item = item->next)
			if ((item->type = MENU_SUB) || (item->type = MENU_ARGS) || (item->type = MENU_EXEC) || (item->type = MENU_QUIT) ||
				(item->type = MENU_EXIT) || (item->type = MENU_MODULE))
				*(HotKeys + j + '1' - 1) = item, j++;
	}

	for (item = menu->data; item; item = item->next)
		if (item->flags & MIF_HOTKEY)
			*(HotKeys + (int) item->hotkey) = item;

	return;
}

void
parse_error(char *message, int line)
{
	fprintf(stderr, "Parse Error: Line %d: %s\n", line, message);
	exit(EXIT_FAILURE);
}
