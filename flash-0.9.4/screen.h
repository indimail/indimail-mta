/*
 * $Log: screen.h,v $
 * Revision 1.1  2002-12-16 01:55:43+05:30  Manny
 * Initial revision
 *
 */
#if !defined (_SCREEN_H)
#   define _SCREEN_H

#include "parse.h"

void            init_scr();
void            pause_scr();
void            close_scr();

void            do_menu(struct menu *);
void            write_str(char *);
void            clear_scr();
void            clear_close_scr();
int             readchar();
int             get_args(char *, int, char *);

#endif
