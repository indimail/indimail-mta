/*
 * $Log: process.h,v $
 * Revision 1.2  2014-04-17 11:29:07+05:30  Cprogrammer
 * added limits.h
 *
 * Revision 1.1  2013-05-15 00:14:48+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifndef PROCESS_H
#define PROCESS_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_STDARG_H
#include <stdarg.h>
#endif
#ifdef HAVE_DIRENT_H
#include <dirent.h>
#endif
#ifdef HAVE_LIMITS_H 
#include <limits.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif

#include "defs.h"
#include "utils.h"
#include "process.h"
#include "monitor.h"

#define FREE	1
#define USED    2
#define RESTARTING   3

#define MAXPROCESS MAXFILENAME

extern char    *progname;

struct process_hdr {

	pid_t           pid;
	uint            status;
	struct entry_conf *econf;
};

void            create_proc_table(struct entry_conf **conf_table);

#endif
