/*
 * $Log: sconfig.h,v $
 * Revision 1.2  2004-10-11 14:01:46+05:30  Cprogrammer
 * added function prototypes
 *
 * Revision 1.1  2004-06-16 01:20:25+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifndef CONFIG_H
#define CONFIG_H

#include "stralloc.h"

#define CONFIG_STR {{0},0}
#define config(c) ((c)->flagconf)
#define config_data(c) (&((c)->sa))


typedef struct
{
	stralloc        sa;
	int             flagconf;
} config_str;

int             config_default(config_str *, char *);
int             config_copy(config_str *, config_str *);
int             config_env(config_str *, char *);
int             config_readline(config_str *, char *);
int             config_readfile(config_str *, char *);

#endif
