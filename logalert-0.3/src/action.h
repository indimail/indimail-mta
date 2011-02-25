#ifndef ACTION_H
#define ACTION_H

#include<stdio.h>
#include "utils.h"
#include "parsers.h"


#define SHELL "/bin/sh"
#define SHELL_ARG "-c"

char * substitute_args(char *cmd, char **args_list,int nmatch);
int shell_exec(struct entry_conf *cur_cfg, char *line, int nmatch);

#endif
