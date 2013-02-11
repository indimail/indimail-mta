#ifndef MONITOR_H
#define MONITOR_H

#include "config.h"

#include<stdio.h>
#include<stdlib.h>

#ifdef HAVE_PCRE_H
#include<pcre.h>
#endif

#ifdef HAVE_REGEX_H
#include<regex.h>
#endif


#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif


#include "defs.h"
//#include "action.h"
#include "parsers.h"
#include "utils.h"


unsigned int active;

void match_sleep(unsigned int tsleep);
ssize_t read_data(int fd, struct entry_conf *cur_cfg);
void recompile(struct entry_conf *cur_cfg);
int rematch(struct entry_conf *cur_cfg, char *line);
int monitor_file(struct entry_conf *cur_cfg);

#endif
