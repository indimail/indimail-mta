/* $Id: configfile.h 5656 2005-03-16 03:55:14Z m-a $ */

/*****************************************************************************

NAME:
   configfile.h -- prototypes and definitions for bogoconfig.c.

AUTHOR:
   David Relson <relson@osagesoftware.com>

******************************************************************************/

#ifndef CONFIGFILE_H
#define CONFIGFILE_H

#include "getopt.h"

/* Definitions */

typedef enum arg_pass_e {
    PASS_1_CLI = 1,		/* 1 - first command line pass  */
    PASS_2_CFG = 2,		/* 2 - config file options ...  */
    PASS_3_CLI = 3		/* 3 - second command line pass */
} arg_pass_t;

/* Global variables */

extern char *config_file_name;

extern void remove_comment(const char *line);
extern bool process_config_files(bool warn_on_error, struct option *lopts);
extern bool process_config_option(const char *arg, bool warn_on_error, priority_t precedence, struct option *lopts);
extern bool read_config_file(const char *fname, bool tilde_expand, bool warn_on_error, priority_t precedence, struct option *lopts);
extern bool process_config_option_and_val(const char *name, const char *val, bool warn_on_error, priority_t precedence);
extern bool process_config_option_as_arg(const char *arg, const char *val, priority_t precedence, struct option *lopts);

#endif
