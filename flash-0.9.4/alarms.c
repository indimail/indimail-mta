/*
 * $Log: alarms.c,v $
 * Revision 1.5  2017-12-04 20:49:04+05:30  Cprogrammer
 * fixed possible overflow
 *
 * Revision 1.4  2009-06-04 10:45:44+05:30  Cprogrammer
 * added conditional inclusion of ncurses
 *
 * Revision 1.3  2009-02-10 16:45:17+05:30  Cprogrammer
 * use ncurses if present
 *
 * Revision 1.2  2002-12-21 19:07:13+05:30  Manny
 * compilation warnings corrected
 *
 * Revision 1.1  2002-12-16 01:54:43+05:30  Manny
 * Initial revision
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
#include "menu.h"
#include "misc.h"
#include "exec.h"

extern int      color;
extern time_t   TimeNow;
extern int      Timeout;

struct Alarm
{
	time_t          time;
	char           *message;
};

struct Alarm   *WAtimes;
static time_t   WAalarm = 0, WAon = 20, WAwarn = 300, WAlast = 0;
static int      WAInit = 0, WASet = 0, WAntimes = 0, WAnmalloc = 0, WAbeeped = 0;
static char    *WAalarmfile;
WINDOW         *WAwin = NULL;

#define SECINWEEK 604800L
#define SECINDAY  86400L
#define SECINHOUR 3600L
#define WEEKSTART 345600L

static struct Alarm *
FindNextAlarm(time_t * altime)
{
	struct Alarm   *x;
	time_t          woffset, wstart;
	int             i;

	woffset = (TimeNow - WEEKSTART) % SECINWEEK;
	wstart = TimeNow - woffset;
	for (i = 0, x = WAtimes; i < WAntimes; i++, x++)
		if (x->time > woffset)
			break;
	if (i == WAntimes)
	{
		x = WAtimes;
		wstart += SECINWEEK;
	}
	*altime = (wstart + x->time);
	return x;
}

static WINDOW  *
DrawAlarmWin(struct Alarm *x)
{
	char            message[COLS], fmt[128];
	WINDOW         *WAwin;
	int             cols;

	WAlast = TimeNow;

	cols = strlen(x->message) + 10;
	if (cols > COLS)
		cols = COLS;
	if (cols > 68)
		cols = 68;
	sprintf(fmt, "Alarm:%%-%d.%ds", cols - 10, cols - 10);
	sprintf(message, fmt, x->message);
	WAwin = newwin(3, cols, 1, 0);
	if (WAwin)
	{
		wattrset(WAwin, ((color) ? 0 : A_REVERSE) | A_BOLD);
		if (color)
			wattron(WAwin, COLOR_PAIR(MAIL_ADVISORY_COLOR));
		werase(WAwin);
		box(WAwin, 0, 0);
		mvwaddstr(WAwin, 1, 2, message);
		wattrset(WAwin, A_NORMAL);
	}
	return WAwin;
}

static int
UpdateWA(int r)
{
	time_t          ontime = WAalarm - WAwarn;
	time_t          offtime = WAalarm + WAon;

	if ((r) || (!WASet))
		return 0;
	if ((TimeNow >= ontime) && (TimeNow <= offtime) && (WAlast < ontime))
		return 1;
	else
	if ((TimeNow >= offtime) && (WAlast > ontime) && (WAlast < offtime))
		return 1;

	return 0;
}

static void
DisplayWA(void)
{
	time_t          ontime, offtime;

	if (!WASet)
		return;

	ontime = WAalarm - WAwarn;
	offtime = WAalarm + WAon;
	if (TimeNow >= offtime)
	{
		struct Alarm   *al;
		delwin(WAwin);
		WAwin = NULL;
		al = FindNextAlarm(&WAalarm);
		WAwin = DrawAlarmWin(al);
		ontime = WAalarm - WAwarn;
		offtime = WAalarm + WAon;
		WAbeeped = 0;
	}
	if ((TimeNow >= ontime) && (TimeNow < offtime))
	{
		touchwin(WAwin);
		wnoutrefresh(WAwin);
		if (!WAbeeped)
		{
			beep();
			WAbeeped = 1;
		}
	}
}

static int
WAcmp(const void *a, const void *b)
{
	return ((struct Alarm *) a)->time - ((struct Alarm *) b)->time;
}

void
ModuleInit(int argc, char **argv, FILE * fp, int *l)
{
	char           *ModName;
	char           *line;
	FILE           *afp;

	if (WAInit == 1)
		return;
	ModName = *argv;
	argv++, argc--;
	WAInit = 1;
	if (argc)
	{
		char           *temp = stradp(*argv, 1);
		argv++, argc--;
		WAalarmfile = malloc(strlen(temp) + 1);
		if (WAalarmfile == NULL)
			return;
		strcpy(WAalarmfile, temp);
	} else
		WAalarmfile = ".alarms";
	afp = fopen(WAalarmfile, "r");
	if (afp != NULL)
	{
		int             major, minor;
		line = Readline(afp);
		if (sscanf(line, "alarms v%d.%d", &major, &minor) != 2)
		{
			fclose(afp);
			fprintf(stderr, "%s: %s exists but is not alarm file\n", ModName, WAalarmfile);
			return;
		}

		if (major == 1)
		{
			line = Readline(afp);
			while (line != NULL)
			{
				time_t          time;
				char           *message;

				for (message = line; (*message != '\0') && (!isspace((int) *message)); message++);
				if (*message != '\0')
				{
					*(message++) = '\0';
					while ((*message != '\0') && (isspace((int) *message)))
						message++;
				}

				if (sscanf(line, "%ld", &time) == 1)
				{
					struct Alarm   *x;
					if (WAntimes == WAnmalloc)
					{
						WAnmalloc = (WAnmalloc == 0) ? 20 : (WAnmalloc * 2);
						WAtimes = xrealloc(WAtimes, WAnmalloc * sizeof(*WAtimes));
					}
					x = &WAtimes[WAntimes];
					x->time = time;
					x->message = malloc(strlen(message) + 1);
					if (x->message == NULL)
						return;
					strcpy(x->message, message);
					WAntimes++;
				}
				line = Readline(afp);
			}
			fclose(afp);
		} else
		{
			fclose(afp);
			fprintf(stderr, "%s: %s has unknown version %d.%d\n", ModName, WAalarmfile, major, minor);
			return;
		}
	}
	if (WAntimes > 0)
	{
		struct Alarm   *x;
#ifdef DEBUG
		int             n;
#endif

		qsort(WAtimes, WAntimes, sizeof(*WAtimes), WAcmp);
#ifdef DEBUG
		for (x = WAtimes, n = 0; n < WAntimes; n++, x++)
			fprintf(stderr, "%s: [%d] %ld : %s\n", ModName, n, x->time, x->message);
#endif
		time(&TimeNow);
		x = FindNextAlarm(&WAalarm);
		WAwin = DrawAlarmWin(x);
		WAbeeped = 0;
		WASet = 1;
	}
	register_display_callback(DisplayWA, UNDERMENU);
	register_timeout_callback(UpdateWA);
}

#define WINL 22
#define WINC 80

static void
DisplayMainWin(WINDOW * fwin, int D, int H, int hours[7][24], char *mesgs[7][24])
{
	int             h, d;
	int             X, Y;
	char            dow[7][4] = { "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun" };
	char            instructions[80] = "Cursor Keys - Move | Space - Set/Unset Alarm | F - Finish";

	wattrset(fwin, A_BOLD);
	if (color == TRUE)
		wattron(fwin, COLOR_PAIR((MENU_EXEC + 1)));
	for (h = 0; h < 24; h++)
	{
		int             x = 6 + h * 3;
		char            buffer[5];

		sprintf(buffer, "%2d", h);
		mvwaddstr(fwin, 1, x, buffer);
	}
	for (d = 0; d < 7; d++)
		mvwaddstr(fwin, 3 + d * 2, 1, dow[d]);
	wattrset(fwin, A_BOLD);
	if (color == TRUE)
		wattron(fwin, COLOR_PAIR((MENU_TITLE + 1)));
	for (h = 0; h < 24; h++)
		for (d = 0; d < 7; d++)
			if (hours[d][h] == 1)
			{
				int             x = 6 + h * 3, y = 3 + d * 2;
				mvwaddstr(fwin, y, x, "XX");
			}
	wattrset(fwin, A_NORMAL);
	if (color == TRUE)
		wattron(fwin, COLOR_PAIR((MENU_TITLE + 1)));
	if ((hours[D][H] == 1) && (mesgs[D][H] != NULL))
	{
		char            buffer[61];
		strncpy(buffer, mesgs[D][H], 60);
		buffer[60] = '\0';
		mvwaddstr(fwin, WINL - 5, ((WINC - 2) - strlen(buffer)) / 2, buffer);
	}
	wattrset(fwin, A_BOLD);
	if (color == TRUE)
		wattron(fwin, COLOR_PAIR((MENU_EXEC + 1)));
	Y = 2 + D * 2;
	X = 5 + H * 3;
	wmove(fwin, Y, X);
	whline(fwin, ACS_ULCORNER, 1);
	wmove(fwin, Y, X + 1);
	whline(fwin, 0, 2);
	wmove(fwin, Y + 2, X);
	whline(fwin, ACS_LLCORNER, 1);
	wmove(fwin, Y + 2, X + 1);
	whline(fwin, 0, 2);
	wmove(fwin, Y + 2, X + 3);
	whline(fwin, ACS_LRCORNER, 1);
	wmove(fwin, Y + 1, X);
	wvline(fwin, 0, 1);
	wmove(fwin, Y, X + 3);
	whline(fwin, ACS_URCORNER, 1);
	wmove(fwin, Y + 1, X + 3);
	wvline(fwin, 0, 1);
	mvwaddstr(fwin, WINL - 4, ((WINC - 2) - strlen(instructions)) / 2, instructions);
}

int
ModuleMain(int argc, char **argv)
{
	WINDOW         *bwin, *fwin, *mwin;
	chtype          cha;
	int             gotch, meslen = 0;
	int             hours[7][24];
	char           *mesgs[7][24];
	char            message[41];
	int             D = 0, H = 9, n, finished = 0, getmessage = 0;
	time_t          t;
	FILE           *fp;
	struct Alarm   *x;

	if (!WAInit)
		return EXIT_FAILURE;
	WASet = 0;
	memset(hours, 0, sizeof(hours));
	for (x = WAtimes, n = 0; n < WAntimes; x++, n++)
	{
		t = x->time;
		if ((t % SECINHOUR) == 0)
		{
			hours[t / SECINDAY][(t % SECINDAY) / SECINHOUR] = 1;
			mesgs[t / SECINDAY][(t % SECINDAY) / SECINHOUR] = x->message;
		} else
		if (x->message)
			free(x->message);
	}
	WAntimes = 0;
	bwin = newwin(WINL, WINC, (LINES - WINL) / 2, (COLS - WINC) / 2);
	if (bwin == NULL)
	{
		if (WAwin != NULL)
			delwin(WAwin);
		x = FindNextAlarm(&WAalarm);
		WAwin = DrawAlarmWin(x);
		WAbeeped = 0;
		WASet = 1;
		return EXIT_FAILURE;
	}
	box(bwin, 0, 0);
	fwin = derwin(bwin, WINL - 2, WINC - 2, 1, 1);
	cha = ' ' | A_BOLD;
	if (color == TRUE)
		cha |= COLOR_PAIR((MENU_TITLE + 1));
	mwin = derwin(fwin, 3, 42, (WINL - 5) / 2, (WINC - 44) / 2);
	while (!finished)
	{
		int             goagain;

		werase(fwin);
		wbkgd(fwin, cha);

		DisplayMainWin(fwin, D, H, hours, mesgs);
		display_screen(0);
		touchwin(bwin);
		wnoutrefresh(bwin);
		touchwin(fwin);
		wnoutrefresh(fwin);
		if (getmessage)
		{
			wattrset(mwin, A_BOLD);
			if (color == TRUE)
				wattron(mwin, COLOR_PAIR((MENU_TITLE + 1)));
			werase(mwin);
			wbkgd(mwin, cha);
			box(mwin, 0, 0);
			mvwaddstr(mwin, 0, 16, " Message ");
			if (color == TRUE)
				wattron(mwin, COLOR_PAIR((MENU_EXEC + 1)));
			mvwaddstr(mwin, 1, 1, message);
			wmove(mwin, 1, 1 + meslen);
			wnoutrefresh(mwin);
			doupdate();
			keypad(mwin, TRUE);
			do
			{
				errno = 0;
				gotch = wgetch(mwin);
				if (errno == EIO)
					GRAB_BACK_TTY
			} while (errno != 0);
			if ((isprint(gotch)) && (meslen < 40))
				message[meslen++] = gotch;
			else
			if (gotch == '\n')
			{
				char           *ms;
				ms = malloc(meslen + 1);
				memcpy(ms, message, meslen);
				*(ms + meslen) = '\0';
				mesgs[D][H] = ms;
				getmessage = 0;
			} else
			if (((gotch == KEY_BACKSPACE) || (gotch == 8)) && (meslen > 0))
				message[--meslen] = ' ';
			else
				beep();
			continue;
		}
		doupdate();
		goagain = 1;
		keypad(fwin, TRUE);
		wtimeout(fwin, Timeout);
		while (goagain == 1)
		{
			goagain = 0;
			do
			{
				errno = 0;
				gotch = wgetch(fwin);
				if (errno == EIO)
				GRAB_BACK_TTY
			} while (errno != 0);
			switch (gotch)
			{
			case ERR:
				break;
			case KEY_LEFT:
				if (H > 0)
					H--;
				break;
			case KEY_RIGHT:
				if (H < 23)
					H++;
				break;
			case KEY_UP:
				if (D > 0)
					D--;
				break;
			case KEY_DOWN:
				if (D < 6)
					D++;
				break;
			case ' ':
			case '\r':
			case '\n':
				if (hours[D][H] == 0)
				{
					memset(message, ' ', sizeof(message));
					message[sizeof(message) - 1] = '\0';
					meslen = 0;
					hours[D][H] = 1;
					mesgs[D][H] = NULL;
					getmessage = 1;
				} else
				{
					hours[D][H] = 0;
					free(mesgs[D][H]);
				}
				break;
			case 'f':
			case 'F':
				finished = 1;
				break;
			case 12:
				wrefresh(curscr);
				break;
			default:
				goagain = 1;
				break;
			}
		}
	}
	delwin(mwin);
	delwin(fwin);
	delwin(bwin);
	for (D = 0; D < 7; D++)
	{
		for (H = 0; H < 24; H++)
		{
			if (hours[D][H] == 1)
			{
				if (WAntimes == WAnmalloc)
				{
					WAnmalloc = (WAnmalloc == 0) ? 20 : (WAnmalloc * 2);
					WAtimes = xrealloc(WAtimes, WAnmalloc * sizeof(*WAtimes));
				}
				x = &WAtimes[WAntimes];
				x->time = D * SECINDAY + H * SECINHOUR;
				x->message = mesgs[D][H];
				WAntimes++;
			}
		}
	}
	fp = fopen(WAalarmfile, "w");
	if (fp != NULL)
	{
		fprintf(fp, "alarms v1.0\n");
		for (n = 0, x = WAtimes; n < WAntimes; n++, x++)
			fprintf(fp, "%ld %s\n", x->time, x->message);
		fclose(fp);
	}

	if (WAntimes)
	{
		if (WAwin != NULL)
			delwin(WAwin);
		time(&TimeNow);
		x = FindNextAlarm(&WAalarm);
		WAwin = DrawAlarmWin(x);
		WAbeeped = 0;
		WASet = 1;
	}
	return EXIT_SUCCESS;
}
