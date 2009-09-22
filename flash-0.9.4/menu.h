/*
 * $Log: menu.h,v $
 * Revision 1.1  2002-12-16 01:55:33+05:30  Manny
 * Initial revision
 *
 */
#if !defined(_MENU_H)
#define _MENU_H

enum mitem_t
{
	MENU_TITLE = 0, MENU_NOP, MENU_EXEC, MENU_SUB, MENU_EXIT, MENU_QUIT,
	MENU_ARGS, MENU_LINE, MENU_MODULE
};

#define MENU_OP_TITLE    "Title"
#define MENU_OP_EXEC     "Exec"
#define MENU_OP_SUB      "SubMenu"
#define MENU_OP_NOP      "Nop"
#define MENU_OP_EXIT     "Exit"
#define MENU_OP_QUIT     "Quit"
#define MENU_OP_ARGS     "Args"
#define MENU_OP_LINE     "Line"
#define MENU_OP_MODULE   "Module"

#define MOPT_NOBOX       "nobox"
#define MOPT_LEFT        "left"
#define MOPT_RIGHT       "right"
#define MOPT_TOP         "top"
#define MOPT_BOTTOM      "bottom"
#define MOPT_OFFSET      "offset"
#define MOPT_SIZE        "size"
#define MOPT_PASSWORD    "password"
#define MOPT_NOCOLOUR    "nocolour"
#define MOPT_SCROLLBAR   "scrollbar"
#define MOPT_NOCURSOR    "nocursor"
#define MOPT_OVERLAYMENU "overlay"

#define M_NONE          0x00
#define M_ARGS	        0x01
#define M_PROMPT       	0x02
#define M_FLAGS         0x04

#define DEFAULT_PROMPT	"Enter filename"

#define MF_NONE      0x0000
#define MF_NOBOX     0x0001
#define MF_LEFT      0x0002
#define MF_RIGHT     0x0004
#define MF_TOP       0x0008
#define MF_BOTTOM    0x0010
#define MF_HMASK     (MF_LEFT | MF_RIGHT)
#define MF_VMASK     (MF_TOP | MF_BOTTOM)
#define MF_OFFSET    0x0020
#define MF_FIXEDSIZE 0x0040
#define MF_PASSWORD  0x0080
#define MF_NOCOLOUR  0x0100
#define MF_SCROLLBAR 0x0200
#define MF_NOCURSOR  0x0400
#define MF_OVERLAY   0x0800

#define MIF_NONE     0x0000
#define MIF_HEAD     0x0001
#define MIF_TAIL     0x0002
#define MIF_LEFT     0x0004
#define MIF_RIGHT    0x0008
#define MIF_CENTRE   0x0010
#define MIF_ALIGN    (MIF_RIGHT | MIF_LEFT | MIF_CENTRE)
#define MIF_HOTKEY   0x0020
#define MIF_ARGS     0x0040

#define MIF_NOPAUSE     0x0100
#define MIF_PAGE        0x0200
#define MIF_BACKGROUND  0x0400
#define MIF_EXEC_MASK   (MIF_NOPAUSE | MIF_PAGE | MIF_BACKGROUND)

#define MENU_NAME_HOTKEYS  "HotKeys"
#define MENU_NAME_MAIN     "Main"

struct menu_items
{
	enum mitem_t    type;
	char           *name;
	char           *args;
	char           *prompt;
	unsigned int    flags;
	unsigned char   hotkey;
	struct menu_items *next;
	struct menu_items *prev;
};

struct menu
{
	char           *name;
	char           *password;
	unsigned int    flags;
	int             offl, offc;
	int             nlines, ncols;
	struct menu_items *data;
	struct menu_items *head, *tail;
};

char           *process_menu(char *, int *);
struct menu    *find_mainmenu(void);
struct menu    *find_menu(char *);
void            define_menu(int, char **, FILE *, int *);
void            freemenus(void);
void            balance_menu_tree(void);
void            set_mainmenu(int, char **, FILE *, int *);

#endif
