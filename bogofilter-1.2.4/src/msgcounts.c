/* $Id: msgcounts.c 6988 2013-01-20 14:02:48Z m-a $ */

/*****************************************************************************

NAME:
   msgcounts.c -- routines for setting & computing .MSG_COUNT values

AUTHOR:
   David Relson <relson@osagesoftware.com>

******************************************************************************/

#include "common.h"

#include <stdlib.h>

#include "msgcounts.h"

/* Globals */

static char	msg_count_buff[MSG_COUNT_MAX_LEN];
int	msg_count_leng = MSG_COUNT_MAX_LEN; /* DO NOT MAKE THIS SIZE_T! */
char   *msg_count_text = msg_count_buff;

static const char *msg_count_header = "\"" MSG_COUNT "\" ";
static size_t	    msg_count_header_len = 0;

uint	msgs_good = 0;
uint	msgs_bad  = 0;

static	bool	saved = false;

/* Function Definitions */

token_t read_msg_count_line(void)
{
    bool msg_sep;

    if (!saved) {
	if (fgets(msg_count_buff, sizeof(msg_count_buff), fpin) == NULL) {
	    msg_count_leng = 0;
	    return NONE;
	}
    }

    msg_count_leng = strlen(msg_count_buff);

    msg_sep = msg_count_buff[1] == '.' && 
	memcmp(msg_count_buff, msg_count_header, msg_count_header_len) == 0;

    if (!saved && msg_sep) {
	saved = true;
	return NONE;
    }
    else {
	saved = false;
	if (msg_sep)
	    return MSG_COUNT_LINE;
	else
	    return BOGO_LEX_LINE;
    }
}

bool msgcount_more(void)
{
    bool val = saved;
    saved = false;
    return val;
}

void set_msg_counts(u_int32_t good, u_int32_t spam)
{
    msgs_bad = spam;
    msgs_good = good;
}

void set_msg_counts_from_str(char *str)
{
    uint b, g;
    b = atoi(str);
    str = strchr(str, ' ') + 1;
    g = atoi(str);

    set_msg_counts(g, b);

    msg_count_header_len= strlen(msg_count_header);
}

long msg_count_get_token(byte **output)
{
    *output = (byte *)msg_count_text;
    return msg_count_leng;
}
