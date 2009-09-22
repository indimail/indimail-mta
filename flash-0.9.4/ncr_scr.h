/*
 * $Log: ncr_scr.h,v $
 * Revision 1.2  2008-06-09 15:32:24+05:30  Cprogrammer
 * changed name to flash
 *
 * Revision 1.1  2002-12-16 01:55:38+05:30  Manny
 * Initial revision
 *
 */
#ifndef _NCR_SCR_H
#   define _NCR_SCR_H

#define BG_COL	     COLOR_BLUE
#define TITLE_COL    COLOR_WHITE
#define NOP_COL	     COLOR_WHITE
#define EXEC_COL     COLOR_YELLOW
#define ARGS_COL     EXEC_COL
#define SUB_COL	     COLOR_RED
#define EXIT_COL     COLOR_GREEN
#define QUIT_COL     EXIT_COL

#define MAIL_ADVISORY_COLOR (COLOR_PAIRS-1)
#define PASSWORD_COLOR (COLOR_PAIRS-2)
#define HIGHLIGHT_COLOR (COLOR_PAIRS-3)

#define TOP_BAR	     "flash  " VERSION " Secure Host Interface"
#define BOT_BAR	     "A - About flash   ? - List Keys   Ctrl-L - Refresh"
#define ARGS_BOT_BAR "Ctrl-L - Refresh Screen   Esc - Abandon Input"

#define ESCAPEKEY    '\033'

struct menu_instance
{
	struct menu_instance *next, *prev;
	struct menu_items *mi_activepane_start, *mi_highlighted;
	WINDOW         *background_win;
	WINDOW         *activepane_win;
	struct menu    *menu;
	struct menu_items **HotKeys;
	int             background_cols, background_lines;
	int             activepane_cols, activepane_lines;
	int             activepane_s, activepane_cursor;
	int             nitems;
};

typedef void    (*DISPLAYCALLBACK) (void);
typedef int     (*TIMEOUTCALLBACK) (int);
typedef enum
{ UNDERMENU, OVERMENU, OVERALL }
DISPLAYCALLBACKWHERE;

struct Callback
{
	struct Callback *next, *prev;
	DISPLAYCALLBACK f;
};

struct TOCallback
{
	struct TOCallback *next, *prev;
	TIMEOUTCALLBACK f;
};

struct DisplayCallbacks
{
	int             initialised;
	struct Callback undermenu, overmenu, overall;
	struct TOCallback timeout;
};

void            init_background(void);
void            init_menus(void);
void            render_instance(struct menu_instance *);
void            nc_lock_screen(void);
void            get_password(WINDOW *, WINDOW *, int, int, char *, int, int);
void            handle_winch(int);
void            nc_about(void);
void            nc_menu_help(void);
void            nc_mailadvisory(void);
void            task_switch(void);
int             nc_runnable_jobs(void);
void            register_display_callback(DISPLAYCALLBACK f, DISPLAYCALLBACKWHERE where);
void            register_timeout_callback(TIMEOUTCALLBACK f);
void            initialise_callbacks(void);

void            handle_timeout(int update);
void            display_screen(int update);

int             checktty(void);

#define NC_CENTRE_AB(a,b) ((a)-(b))/2
#define NC_LEFT_AB(a,b)   0
#define NC_RIGHT_AB(a,b)   ((a)-(b))

#define NC_HCENTRE_SC(a)  (COLS-(a))/2
#define NC_VCENTRE_SC(a)  (LINES-(a))/2
#define NC_LEFT_SC(a)     0
#define NC_RIGHT_SC(a)    (COLS-(a))
#define NC_TOP_SC(a)      0
#define NC_BOTTOM_SC(a)   (LINES-(a))

#include <errno.h>

#endif
