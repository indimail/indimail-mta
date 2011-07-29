/*
 * $Log: menu.c,v $
 * Revision 1.3  2011-07-29 09:24:03+05:30  Cprogrammer
 * fixed gcc warnings
 *
 * Revision 1.2  2008-07-17 21:38:13+05:30  Cprogrammer
 * moved progname to variables.h
 *
 * Revision 1.1  2002-12-16 01:55:01+05:30  Manny
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>

#include "menu.h"
#include "parse.h"
#include "misc.h"
#include "tree.h"
#include "mystring.h"
#include "variables.h"

struct token
{
	char           *str;
	enum mitem_t    token;
	int             flags;
};

struct token    menu_tokens[] = {
	{MENU_OP_TITLE, MENU_TITLE, M_FLAGS},
	{MENU_OP_NOP, MENU_NOP, M_FLAGS},
	{MENU_OP_EXIT, MENU_EXIT, M_FLAGS},
	{MENU_OP_SUB, MENU_SUB, M_ARGS | M_FLAGS},
	{MENU_OP_EXEC, MENU_EXEC, M_ARGS | M_FLAGS},
	{MENU_OP_QUIT, MENU_QUIT, M_FLAGS},
	{MENU_OP_ARGS, MENU_ARGS, M_ARGS | M_PROMPT | M_FLAGS},
	{MENU_OP_MODULE, MENU_MODULE, M_ARGS | M_FLAGS},
	{NULL, NO_TOKEN, M_NONE}
};

extern char     delim;
extern char     comment;
struct dict    *Menus = NULL;
static char    *menu_name_main = MENU_NAME_MAIN;

static void
initialise_menu(struct menu *m)
{
	m->name = NULL;
	m->password = NULL;
	m->flags = MF_NONE;
	m->offl = m->offc = 0;
	m->nlines = m->ncols = 0;
	m->data = m->head = m->tail = NULL;
}

struct menu    *
new_menu(void)
{
	struct menu    *m;

	m = (struct menu *) xmalloc(sizeof(*m));

	initialise_menu(m);

	return m;
}

static int
Menu_name_compare(void *a, void *b)
{
	return strcmp(((struct menu *) a)->name, ((struct menu *) b)->name);
}

void
add_menu(struct menu *m, int line)
{
	if (Menus == NULL)
		Menus = new_dict(Menu_name_compare);

	if (!add_unique_node(Menus, (void *) m))
	{
		char            temp[80];
		sprintf(temp, "Duplicate Menu: %s", m->name);
		parse_error(temp, line);
	}

	return;
}

struct menu    *
find_menu(char *name)
{
	struct menu     k;
	k.name = name;

	if (Menus != NULL)
		return (struct menu *) find_node(Menus, &k);
	else
		return NULL;
}

struct menu    *
find_mainmenu(void)
{
	return find_menu(menu_name_main);
}

void
set_mainmenu(int wordc, char **wordv, FILE * fp, int *menuline)
{
	if (wordc < 2)
		fprintf(stderr, "%s: %s: Must specify name\n", progname, *wordv);
	else
	{
		menu_name_main = xmalloc(strlen(*(wordv + 1)) + 1);
		strcpy(menu_name_main, *(wordv + 1));
	}
}

void
freemenu(struct menu *m)
{
	struct menu_items *item;

	item = m->data;
	while ((item) && (item->prev))
		item = item->prev;

	while (item != NULL)
	{
		m->data = item->next;
#ifndef NOCOPY
		if (item->name)
			free(item->name);
		if (item->name)
			free(item->args);
		if (item->name)
			free(item->prompt);
#endif
		free(item);
		item = m->data;
	}

	if (m->name)
		free(m->name);
	if (m->password)
		free(m->password);

	free(m);
}

void
freemenus(void)
{
	if (Menus != NULL)
		visit_nodes(Menus, (void (*)(void *)) freemenu);
}

void
balance_menu_tree(void)
{
	balance_tree(Menus);
}

static int
get_token(char *name, int *flags)
{
	int             i = 0, found = 0;

	while (menu_tokens[i].str)
	{
		if (!strcasecmp(name, menu_tokens[i].str))
		{
			found = 1;
			break;
		}
		i++;
	}

	if (!found)
	{
		*flags = 0;
		return NO_TOKEN;
	} else
	{
		*flags = menu_tokens[i].flags;
		return menu_tokens[i].token;
	}
}

static struct menu_items *
alloc_items(char *line, int i)
{
	int             flags;
	char            errmess[MENU_BUFF];
	char           *Tokens[20], **tokens;
	int             ntokens;
	struct menu_items *data;

	tokens = Tokens;
	ntokens = strtokenize(line, delim, tokens, 20);
	ntokens--;

	if (ntokens < 2)
		return NULL;

	data = (struct menu_items *) xmalloc(sizeof(struct menu_items));

	/*
	 * Scan for tokens, then dissemate information 
	 */
	if ((data->type = get_token(tokens[0], &flags)) == NO_TOKEN)
	{
		sprintf(errmess, "No such token `%s'\n", tokens[0]);
		parse_error(errmess, i);
	}
	ntokens--, tokens++;

#ifdef NOCOPY
	data->name = *tokens;
#else
	data->name = (char *) xmalloc(strlen(*tokens) + 1);
	strcpy(data->name, *tokens);
#endif
	ntokens--, tokens++;

	data->args = NULL;
	data->prompt = (flags & M_PROMPT) ? DEFAULT_PROMPT : NULL;
	data->flags = MIF_NONE;
	data->hotkey = 'Z';

	if ((ntokens) && (flags != M_NONE))
	{
		char           *args_p = NULL;
		char           *prompt_p = NULL;
		char           *flags_p = NULL;
		int             maxoptions;

		if ((flags & M_ARGS) == 0)
		{
			if (flags & M_FLAGS)
				flags_p = tokens[0];
		} else
		{
			/*
			 * This is the number of options that we want at most 
			 */
			maxoptions = 1		/*
								 * M_ARGS 
								 */  + (flags & M_PROMPT) + (flags & M_FLAGS);

			if ((ntokens == 1) || (maxoptions == 1))
				args_p = tokens[0];
			else
			if ((ntokens == 2) || (maxoptions == 2))
			{
				if (flags & M_PROMPT)
					args_p = tokens[0], prompt_p = tokens[1];
				else
				if (flags & M_FLAGS)
					args_p = tokens[1], flags_p = tokens[0];
				else
					error("Warning: Boolean Logic failure");	/*
																 * This should not happen 
																 */
			} else				/*
								 * if(ntokens>=3) 
								 */
			{
				args_p = tokens[1];
				prompt_p = tokens[2];
				flags_p = tokens[0];
			}
		}

		if (args_p != NULL)
		{
#ifdef NOCOPY
			data->args = args_p;
#else
			data->args = (char *) xmalloc(strlen(args_p) + 1);
			strcpy(data->args, args_p);
#endif
		}

		if (prompt_p != NULL)
		{
#ifdef NOCOPY
			data->prompt = prompt_p;
#else
			data->prompt = (char *) xmalloc(strlen(prompt_p) + 1);
			strcpy(data->prompt, prompt_p);
#endif
		}

		if (flags_p != NULL)
		{
			while (*flags_p)
			{
				switch (*flags_p)
				{
				case 'H':
				case 'h':
					data->flags |= MIF_HEAD;
					break;
				case 'T':
				case 't':
					data->flags |= MIF_TAIL;
					break;
				case 'L':
				case 'l':
					data->flags = (data->flags & ~MIF_ALIGN) | MIF_LEFT;
					break;
				case 'R':
				case 'r':
					data->flags = (data->flags & ~MIF_ALIGN) | MIF_RIGHT;
					break;
				case 'C':
				case 'c':
					data->flags = (data->flags & ~MIF_ALIGN) | MIF_CENTRE;
					break;
				case 'K':
				case 'k':
					if (*(flags_p + 1))
					{
						data->flags |= MIF_HOTKEY;
						flags_p++;
						data->hotkey = *flags_p;
					}
					break;
				case 'N':
				case 'n':
					data->flags |= MIF_NOPAUSE;
					break;
				case 'P':
				case 'p':
					data->flags |= MIF_PAGE;
					break;
				case '&':
					data->flags |= MIF_BACKGROUND;
					break;
				case 'A':
				case 'a':
					data->flags |= MIF_ARGS;
					break;
				}
				flags_p++;
			}
		}
	} else
	if (flags & M_ARGS)
		parse_error("Premature end-of-line\n", i);

	data->next = data->prev = NULL;

#ifdef DEBUG
	fprintf(stderr, "  Item:%s:%s:%s:%x:%c\n", data->name, data->args, data->prompt, data->flags, data->hotkey);
#endif

	return data;
}

static void
process_menu_items(struct menu *menu, FILE * fp, int *start_line)
{
	int             i;
	char           *line;
	struct menu_items *d;
	struct menu_items **attach_here;

	i = *start_line;

	line = Readline(fp);
	i++;

	while (line)
	{
		/*
		 * Skip blank lines and comments 
		 */
		strwhite(line);
		if ((!*line) || (*line == comment))
		{
			line = Readline(fp);
			i++;
			continue;
		}

		/*
		 * Close menu on MENU_END, otherwise define node 
		 */
		if (!strncasecmp(line, MENU_END, sizeof(MENU_END) - 1))
		{
#ifdef DEBUG
			fprintf(stderr, "%s found\n", MENU_END);
#endif
			break;
		} else
		{
			d = alloc_items(line, i);

			if ((d->flags & MIF_HEAD) != 0)
				attach_here = &(menu->head);
			else
			if ((d->flags & MIF_TAIL) != 0)
				attach_here = &(menu->tail);
			else
				attach_here = &(menu->data);

			if (*attach_here == NULL)
				*attach_here = d;
			else
			{
				d->prev = *attach_here;
				d->next = (*attach_here)->next;
				(*attach_here)->next = d;
				if (d->next != NULL)
					d->next->prev = d;
				*attach_here = d;
			}
		}

		line = Readline(fp);
		i++;
	}

	/*
	 * rewind linked list 
	 */
	while (menu->data->prev != NULL)
		menu->data = menu->data->prev;

	if (menu->head != NULL)
		while (menu->head->prev != NULL)
			menu->head = menu->head->prev;

	if (menu->tail != NULL)
		while (menu->tail->prev != NULL)
			menu->tail = menu->tail->prev;

	*start_line = i;
	return;
}

void
define_menu(int wordc, char **wordv, FILE * fp, int *i)
{
	struct menu    *menu;

#ifdef DEBUG
	printf("define_menu()\n");
#endif

	wordv++, wordc--;

	if (wordc == 0)
		parse_error("Premature end-of-line\n", *i);

	menu = new_menu();

	/*
	 * Store menu name, for easy retrieval 
	 */
	menu->name = (char *) xmalloc(strlen(*(wordv + wordc - 1)) + 1);
	strcpy(menu->name, *(wordv + wordc - 1));

	wordc--;
	while (wordc)
	{
		if (strcasecmp(*wordv, MOPT_NOBOX) == 0)
			menu->flags |= MF_NOBOX;
		else
		if (strcasecmp(*wordv, MOPT_LEFT) == 0)
			menu->flags = (menu->flags & ~MF_HMASK) | MF_LEFT;
		else
		if (strcasecmp(*wordv, MOPT_RIGHT) == 0)
			menu->flags = (menu->flags & ~MF_HMASK) | MF_RIGHT;
		else
		if (strcasecmp(*wordv, MOPT_TOP) == 0)
			menu->flags = (menu->flags & ~MF_VMASK) | MF_TOP;
		else
		if (strcasecmp(*wordv, MOPT_BOTTOM) == 0)
			menu->flags = (menu->flags & ~MF_VMASK) | MF_BOTTOM;
		else
		if (strcasecmp(*wordv, MOPT_OFFSET) == 0)
		{
			if (wordc >= 3)
			{
				menu->flags |= MF_OFFSET;
				wordc--, wordv++;
				sscanf(*wordv, "%d", &menu->offl);
				wordc--, wordv++;
				sscanf(*wordv, "%d", &menu->offc);
			} else
				wordc = 1;
		} else
		if (strcasecmp(*wordv, MOPT_SIZE) == 0)
		{
			if (wordc >= 3)
			{
				menu->flags |= MF_FIXEDSIZE;
				wordc--, wordv++;
				sscanf(*wordv, "%d", &menu->nlines);
				wordc--, wordv++;
				sscanf(*wordv, "%d", &menu->ncols);
			} else
				wordc = 1;
		} else
		if (strcasecmp(*wordv, MOPT_PASSWORD) == 0)
		{
			if (wordc >= 2)
			{
				menu->flags |= MF_PASSWORD;
				wordc--, wordv++;
				menu->password = xmalloc((strlen(*wordv) + 1) * sizeof(char));
				strcpy(menu->password, *wordv);
			}
		} else
		if (strcasecmp(*wordv, MOPT_NOCOLOUR) == 0)
			menu->flags |= MF_NOCOLOUR;
		else
		if (strcasecmp(*wordv, MOPT_SCROLLBAR) == 0)
			menu->flags |= MF_SCROLLBAR;
		else
		if (strcasecmp(*wordv, MOPT_NOCURSOR) == 0)
			menu->flags |= MF_NOCURSOR;
		else
		if (strcasecmp(*wordv, MOPT_OVERLAYMENU) == 0)
			menu->flags |= MF_OVERLAY;

		wordc--, wordv++;
	}

	/*
	 * Loop through, defining menu, quit at MENU_END 
	 */
	process_menu_items(menu, fp, i);

	add_menu(menu, *i);
#ifdef DEBUG
	fprintf(stderr, "Menu added to BST\n");
#endif

	return;
}
