/*
 * $Log: monitor.h,v $
 * Revision 1.1  2013-05-15 00:14:29+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifndef MONITOR_H
#define MONITOR_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_PCRE_H
#include <pcre.h>
#endif
#ifdef HAVE_REGEX_H
#include <regex.h>
#endif


#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#include "defs.h"
#include "parsers.h"
#include "utils.h"


unsigned int    active;

void            match_sleep(unsigned int tsleep);
ssize_t         read_data(int fd, struct entry_conf *cur_cfg);
void            recompile(struct entry_conf *cur_cfg);
int             rematch(struct entry_conf *cur_cfg, char *line);
int             monitor_file(struct entry_conf *cur_cfg);

#endif
