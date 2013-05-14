/*
 * $Log: action.h,v $
 * Revision 1.1  2013-05-15 00:13:51+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifndef ACTION_H
#define ACTION_H

#include <stdio.h>
#include "utils.h"
#include "parsers.h"


#define SHELL     "/bin/sh"
#define SHELL_ARG "-c"

char           *substitute_args(char *cmd, char **args_list, int nmatch);
int             shell_exec(struct entry_conf *cur_cfg, char *line, int nmatch);

#endif
