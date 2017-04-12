/*
 * $Log: background.c,v $
 * Revision 1.6  2017-04-13 00:16:23+05:30  Cprogrammer
 * fixed prototype of UupdateBackgrounds()
 *
 * Revision 1.5  2011-07-29 09:23:47+05:30  Cprogrammer
 * fixed gcc warnings
 *
 * Revision 1.4  2009-06-04 10:46:04+05:30  Cprogrammer
 * added conditional inclusion of ncurses
 *
 * Revision 1.3  2009-02-10 16:45:36+05:30  Cprogrammer
 * use ncurses if present
 *
 * Revision 1.2  2002-12-21 19:07:23+05:30  Manny
 * compilation warnings corrected
 *
 * Revision 1.1  2002-12-16 01:54:51+05:30  Manny
 * Initial revision
 *
 * 
 * Background flags
 * 
 * interval t  set default update interval to t
 * random      start on a random screen
 * preload     load,process and store screens at startup
 * 
 * 
 * Individual overlay Flags (note NO SPACES between options or parameters)
 * 
 * @,O,o   y,x   Set offset for overlay
 * R,r           Display overlay in reverse video
 * S,s           Display overlay in standout
 * B,b           Display overlay in bold
 * F,f           Display overlay flashing
 * I,i     t     Override default interval for this screen
 * P,p           Print all spaces in this overlay (ie write over characters on
 * screen with spaces where appropriate)
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
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <syslog.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include"ncr_scr.h"
#include"mystring.h"

extern int      color;
extern time_t   TimeNow;

static int      _BGMS_Random = 0, _BGMS_Preload = 0;
static time_t   _BGMS_UpdateInterval = 10, _BGMS_NextUpdate;

#define BGFLAG_AT          0x0001
#define BGFLAG_REVVID      0x0002
#define BGFLAG_BOLD        0x0004
#define BGFLAG_FLASH       0x0008
#define BGFLAG_STANDOUT    0x0010
#define BGFLAG_PRINTSPACES 0x0020

struct Background
{
	struct Background *next;
	char           *filename;
	int             flags, xoff, yoff;
};

struct BackgroundRing
{
	struct BackgroundRing *next;
	struct Background *screen;
	time_t          frameinterval;
	WINDOW         *window;
};

static struct BackgroundRing *_BGMS_Backgrounds = NULL;

/*
 * #define DEBUG 
 */

static void
DrawScreen(void)
{
	int             i, l;
	FILE           *back_file;
	struct Background *x;
	char           *back_line;

	wattrset(_BGMS_Backgrounds->window, A_NORMAL);
	werase(_BGMS_Backgrounds->window);
	touchwin(_BGMS_Backgrounds->window);

	x = _BGMS_Backgrounds->screen;
	if (x == NULL)
	{
		_BGMS_Backgrounds = _BGMS_Backgrounds->next;
		return;
	}

	back_line = (char *) malloc((COLS + 1) * sizeof(char));
	if (back_line != NULL)
	{
		while (x != NULL)
		{
			back_file = fopen(stradp(x->filename, 0), "r");

			if (back_file != (FILE *) NULL)
			{
				int             c, d;
				int             xstart, ystart, defattr;

				defattr = A_NORMAL;
				if (x->flags & BGFLAG_REVVID)
					defattr |= A_REVERSE;
				if (x->flags & BGFLAG_BOLD)
					defattr |= A_BOLD;
				if (x->flags & BGFLAG_FLASH)
					defattr |= A_BLINK;
				if (x->flags & BGFLAG_STANDOUT)
					defattr |= A_STANDOUT;
				wattrset(_BGMS_Backgrounds->window, defattr);

				xstart = (x->xoff >= 0) ? x->xoff : COLS + x->xoff;
				if (xstart > COLS)
					xstart = COLS;
				else
				if (xstart < 0)
					xstart = 0;
				ystart = (x->yoff >= 0) ? x->yoff : LINES + x->yoff - 2;
				if (ystart > (LINES - 2))
					ystart = LINES - 2;
				else
				if (ystart < 0)
					ystart = 0;

				*back_line = '\0';
				fgets(back_line, COLS + 1, back_file);
				i = strlen(back_line);
				for (l = ystart; (i > 0) && (l < LINES - 2); l++)
				{
					char            cc;
					if (x->flags & BGFLAG_PRINTSPACES)
					{
						for (d = 0, c = xstart; (c < COLS) && (d < i); d++)
						{
							cc = *(back_line + d);
							if (cc == '\t')
								do
								{
									mvwaddch(_BGMS_Backgrounds->window, l, cc++, ' ');
								}
								while ((c % 8) != 0);
							else
								mvwaddch(_BGMS_Backgrounds->window, l, c++, cc);
						}
					} else
					{
						for (d = 0, c = xstart; (c < COLS) && (d < i); d++)
						{
							cc = *(back_line + d);
							if (cc == '\t')
								c += (8 - (c % 8));
							else
							if (!isspace((int) cc))
							{
								if (cc == 2)
									cc = ' ';	/*- Ctrl-B is a drawn-space */
								mvwaddch(_BGMS_Backgrounds->window, l, c++, cc);
							} else
								c++;
						}
					}
					while ((i == COLS) && (*(back_line + i - 1) != '\n'))
					{
						fgets(back_line, COLS + 1, back_file);
						i = strlen(back_line);
					}
					*back_line = '\0';
					fgets(back_line, COLS + 1, back_file);
					i = strlen(back_line);
				}
				fclose(back_file);
			}
			x = x->next;
		}
		free(back_line);
	}
	wattrset(_BGMS_Backgrounds->window, A_NORMAL);
	return;
}

static void
DisplayBackgrounds(void)
{
	if (TimeNow > _BGMS_NextUpdate)
	{
		/*- _BGMS_NextUpdate=TimeNow+_BGMS_UpdateInterval; -*/
		_BGMS_Backgrounds = _BGMS_Backgrounds->next;
		_BGMS_NextUpdate = TimeNow + _BGMS_Backgrounds->frameinterval;
		if (!_BGMS_Preload)
			DrawScreen();
	}
	wattrset(_BGMS_Backgrounds->window, A_NORMAL);
	touchwin(_BGMS_Backgrounds->window);
	wnoutrefresh(_BGMS_Backgrounds->window);
	return;
}

static int
UpdateBackgrounds(int r)
{
	if (TimeNow > _BGMS_NextUpdate)
		return 1;
	else
		return 0;
}

void
ModuleInit(int argc, char **argv, FILE * fp, int *lc)
{
	char           *line;
	char           *backfilelist;
	struct BackgroundRing *x = NULL, **xx;
	struct Background *y, **yy;
	WINDOW         *Bkg_Window = NULL;
	int             nBackgrounds = 0;

	argv++, argc--;
	while (argc)
	{
		if ((strcasecmp(*argv, "interval") == 0) && (argc > 1))
		{
			argc--, argv++;
			sscanf(*argv, "%ld", &_BGMS_UpdateInterval);
		} else
		if (strcasecmp(*argv, "random") == 0)
			_BGMS_Random = 1;
		else
		if (strcasecmp(*argv, "preload") == 0)
			_BGMS_Preload = 1;
		argc--, argv++;
	}
	xx = &_BGMS_Backgrounds;
	line = Readline(fp);
	(*lc)++;
	while (line != NULL)
	{
#ifdef DEBUG
		int             n = 0;
		fprintf(stderr, "Background: ");
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
			if (x != NULL)
				x->next = _BGMS_Backgrounds;
#ifdef DEBUG
			fprintf(stderr, "done\n");
#endif
			break;
		}
		backfilelist = malloc(strlen(line) + 1);
		if (backfilelist == NULL)
		{
			fprintf(stderr, "background: Cannot malloc\n");
			return;
		}
		strcpy(backfilelist, line);
		x = malloc(sizeof(*x));
		if (x == NULL)
		{
			fprintf(stderr, "background: Cannot malloc\n");
			return;
		}
		x->next = NULL;
		x->screen = NULL;
		x->frameinterval = _BGMS_UpdateInterval;
		yy = &x->screen;
		while (*backfilelist == ':')
			backfilelist++;
		while (*backfilelist != '\0')
		{
			y = malloc(sizeof(*y));
			if (y == NULL)
			{
				fprintf(stderr, "background: Cannot malloc\n");
				return;
			}
#ifdef DEBUG
			fprintf(stderr, "[%d]", ++n);
#endif
			y->filename = backfilelist;
			y->flags = 0;
			y->yoff = y->xoff = 0;
			y->next = NULL;

			while ((*backfilelist != '\0') && (*backfilelist != ':') && (*backfilelist != ';'))
				backfilelist++;
			if (*backfilelist == ';')	/*- FLAGS UUGH */
			{
				int             broken = 0, temp;
				char            saved, *rememberme;
				*backfilelist = '\0';
				backfilelist++;
				while (!broken)
				{
					switch (*backfilelist)
					{
					case '@':
					case 'O':
					case 'o':
						rememberme = ++backfilelist;
						while ((isdigit((int) *backfilelist)) || (*backfilelist == '-'))
							backfilelist++;
						saved = *backfilelist;
						*backfilelist = '\0';
						sscanf(rememberme, "%d", &y->yoff);
						*backfilelist = saved;
						if (*backfilelist == ',')
						{
							rememberme = ++backfilelist;
							while ((isdigit((int) *backfilelist)) || (*backfilelist == '-'))
								backfilelist++;
							saved = *backfilelist;
							*backfilelist = '\0';
							sscanf(rememberme, "%d", &y->xoff);
							*backfilelist = saved;
						}
						y->flags |= BGFLAG_AT;
						break;
					case 'I':
					case 'i':
						rememberme = ++backfilelist;
						while (isdigit((int) *backfilelist))
							backfilelist++;
						saved = *backfilelist;
						*backfilelist = '\0';
						sscanf(rememberme, "%d", &temp);
						x->frameinterval = temp;
						*backfilelist = saved;
						break;
					case 'r':
					case 'R':
						y->flags |= BGFLAG_REVVID;
						*backfilelist = '\0';
						backfilelist++;
						break;
					case 'b':
					case 'B':
						y->flags |= BGFLAG_BOLD;
						*backfilelist = '\0';
						backfilelist++;
						break;
					case 'f':
					case 'F':
						y->flags |= BGFLAG_FLASH;
						*backfilelist = '\0';
						backfilelist++;
						break;
					case 's':
					case 'S':
						y->flags |= BGFLAG_STANDOUT;
						*backfilelist = '\0';
						backfilelist++;
						break;
					case 'p':
					case 'P':
						y->flags |= BGFLAG_PRINTSPACES;
						*backfilelist = '\0';
						backfilelist++;
						break;
					default:
						broken = 1;
						continue;
					}
				}
				while ((*backfilelist != '\0') && (*backfilelist != ':'))
					backfilelist++;
			}
			*yy = y;
			yy = &y->next;
			while (*backfilelist == ':')
				*(backfilelist++) = '\0';
		}
#ifdef DEBUG
		fprintf(stderr, "\n");
#endif
		*xx = x;
		xx = &x->next;
		line = Readline(fp);
		(*lc)++;
	}
	_BGMS_NextUpdate = 0;
	if (_BGMS_Backgrounds != NULL)
	{
		if (_BGMS_Preload)
		{
			x = _BGMS_Backgrounds;
			do
			{
				_BGMS_Backgrounds->window = newwin(LINES - 2, 0, 1, 0);
				DrawScreen();
				nBackgrounds++;
				_BGMS_Backgrounds = _BGMS_Backgrounds->next;
			}
			while (_BGMS_Backgrounds != x);
		} else
		{
			Bkg_Window = newwin(LINES - 2, 0, 1, 0);
			x = _BGMS_Backgrounds;
			do
			{
				nBackgrounds++;
				x->window = Bkg_Window;
				x = x->next;
			}
			while (x != _BGMS_Backgrounds);
		}
		for (x = _BGMS_Backgrounds; _BGMS_Backgrounds->next != x; _BGMS_Backgrounds = _BGMS_Backgrounds->next);
		if (_BGMS_Random)
		{
			int             randomiser = (int) TimeNow % nBackgrounds;
			while (randomiser--)
				_BGMS_Backgrounds = _BGMS_Backgrounds->next;
		}
		register_display_callback(DisplayBackgrounds, UNDERMENU);
		register_timeout_callback(UpdateBackgrounds);
	}
	return;
}
