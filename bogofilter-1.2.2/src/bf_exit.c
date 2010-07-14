/* $Id: bf_exit.c 6246 2005-09-11 12:51:52Z relson $ */

/*****************************************************************************

NAME:
   bf_exit.c -- bogofilter exit

AUTHOR:
   David Relson <relson@osagesoftware.com>

******************************************************************************/

#include "common.h"
#include "wordlists.h"

void bf_exit(void)
{
    /* Ensure wordlists are closed */
    close_wordlists(false);

    return;
}
