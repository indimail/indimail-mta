#ifndef PROCESS_H
#define PROCESS_H


#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>


#include "config.h"
#include "defs.h"
#include "utils.h"
#include "process.h"
#include "monitor.h"

#ifdef LINUX
 #ifdef HAVE_DIRENT_H
 #include <dirent.h>
 #endif
#endif

#ifdef SOLARIS
 #include <limits.h>
#endif

#ifdef NETBSD
 #include <limits.h>
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#endif


#define FREE	1
#define USED    2
#define RESTARTING   3

#define MAXPROCESS MAXFILENAME

extern char *progname;

struct process_hdr {

	pid_t pid;
	uint status;
	struct entry_conf *econf;
};

void create_proc_table(struct entry_conf **conf_table);
	
#endif
