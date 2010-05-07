/*-
 * Copyright (C) 2004 Ben Goodwin
 * This file is part of the nsvs package
 *
 * The nsvs package is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * The nsvs package is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with the nsvs package; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/*
 * $Id: config.c,v 1.3 2005/01/31 01:09:21 cinergi Exp $ 
 */
static const char rcsid[] = "$Id: config.c,v 1.3 2005/01/31 01:09:21 cinergi Exp $";

#include "common.h"
#include <string.h>
#include <stdlib.h>
#include "nssd.h"

conf_t          conf;

/*
 * Get the next key/value pair from an open file
 * return NTRUE if a key/val pair is found
 * return NFALSE if EOF or error
 * Lines can begin (column 0) with a '#' for comments
 * key/val pairs must be in the following format
 *      key   val
 * Whitespace can be spaces or tabs
 * Keys must be one word (no whitespace)
 * Values can be anything
 * Line continuation is supported if EOL is strictly a '\'
 *      (no trailing whitespace)
 *
 * The in-memory copy of LINE is modified by this routine
 *
 * fh = open file handle
 * key = config key loaded here
 * key_size = storage size of key
 * val = config value loaded here
 * val_size = storage size of val
 */
static int
_next_key(FILE * fh, char *key, int key_size, char *val, int val_size)
{
	char            line[MAX_LINE_SIZE];
	char           *ccil;		/* Current Character In Line */
	char           *cur;
	size_t          size;
	int             fetch_key = 1;

	*val = *key = '\0';
	/*- Loop through file until a key/val pair is found or EOF */
	while (fgets(line, sizeof (line), fh) != NULL) {
		if (*line == '#')
			continue;

		ccil = line;
		if (fetch_key) {
			cur = ccil;
			/*- Get key - anything but CR/LF or whitespace */
			ccil += strcspn(ccil, "\n\r \t");
			if (ccil == cur)
				continue;		/* No key */
			*ccil = '\0';		/* Null-terminate the key */
			/*- Key found, move on */
			++ccil;
			/*- Save the key */
			strncpy(key, cur, key_size);
			key[key_size - 1] = '\0';	/* strncpy doesn't guarantee null-term */
		}

		/*- Skip leading whitespace */
		ccil += strspn(ccil, " \t");
		cur = ccil;
		/*- Get value - anything but CR/LF */
		size = strcspn(ccil, "\n\r");
		if (!size && fetch_key)
			continue;			/* No value */
		ccil += size;
		if (*(ccil - 1) == '\\') {
			fetch_key = 0;		/* Next line continues a value */
			*(ccil - 1) = '\0';	/* Remove the '\' */
			size--;
		} else {
			fetch_key = 1;		/* Next line starts with a key */
			*ccil = '\0';		/* Null-terminate the value */
		}
		/*- Append what we just snarfed to VAL */
		strncat(val, cur, val_size - 1);
		val_size -= size;
		if (val_size <= 0) {
			nssd_log(LOG_WARNING, "%s: Config value for '%s' too long", __FUNCTION__, key);
			return 0;
		}
		if (!fetch_key)			/* Next line continues a value */
			continue;
		return 1;
	}
	return 0;
}

/*- "type" of value */
typedef enum {
	CV_STRING,					/* char[] */
	CV_INTEGER,					/* int */
} config_value_t;

/*- maps config key to spot in 'conf' */
typedef struct {
	char           *name;		/* key string */
	int             type;		/* config_value_t */
	void           *ptr;		/* where in 'conf' to load this value */
} config_info_t;

int
load_config(const char *file)
{
	char            key[MAX_KEY_SIZE];
	char            val[MAX_VAL_SIZE];
	config_info_t  *c;
	FILE           *fh;

	/*- map config key to 'conf' location; must be NULL-terminated */
	config_info_t   config_fields[] = {
		{"getpwnam", CV_STRING, conf.getpwnam},
		{"getpwuid", CV_STRING, conf.getpwuid},
		{"getspnam", CV_STRING, conf.getspnam},
		{"getpwent", CV_STRING, conf.getpwent},
		{"getspent", CV_STRING, conf.getspent},
		{"getgrnam", CV_STRING, conf.getgrnam},
		{"getgrgid", CV_STRING, conf.getgrgid},
		{"getgrent", CV_STRING, conf.getgrent},
		{"memsbygid", CV_STRING, conf.memsbygid},
		{"gidsbymem", CV_STRING, conf.gidsbymem},
		{"host", CV_STRING, conf.host},
		{"port", CV_INTEGER, &conf.port},
		{"socket", CV_STRING, conf.socket},
		{"username", CV_STRING, conf.username},
		{"password", CV_STRING, conf.password},
		{"database", CV_STRING, conf.database},
		{"pidfile", CV_STRING, conf.pidfile},
		{"threads", CV_INTEGER, &conf.threads},
		{"timeout", CV_INTEGER, &conf.timeout},
		{"facility", CV_STRING, conf.facility},
		{"priority", CV_STRING, conf.priority},

		{NULL}
	};

	if ((fh = fopen(file, "r")) == NULL)
		return 0;

	memset(&conf, 0, sizeof (conf));

	/*- Set all the defaults, then read the config */
	conf.threads = DEFAULT_THREADS;
	strncpy(conf.priority, DEFAULT_PRIORITY, sizeof (conf.priority));
	strncpy(conf.facility, DEFAULT_FACILITY, sizeof (conf.facility));
	strncpy(conf.pidfile, DEFAULT_PIDFILE, sizeof (conf.pidfile));

	/*- Read the config - step through all key/val pairs available */
	while (_next_key(fh, key, sizeof (key), val, sizeof (val))) {
		for (c = config_fields; c->name; c++) {
			if (!strcmp(key, c->name)) {
				if (c->type == CV_STRING)
					strncpy(c->ptr, val, sizeof (val));
				else if (c->type == CV_INTEGER)
					*((int *) c->ptr) = atoi(val);
			}
		}
	}

	fclose(fh);

	/*- We *must* have a host and database */
	if (!conf.host[0] || !conf.database[0])
		return 0;

	/*- Convert syslog prio/facil strings to integer counterparts */
	conf.ipriority = str2priority(conf.priority);
	conf.ifacility = str2facility(conf.facility);
	/*- Cap conf.threads */
	if (conf.threads > MAX_THREADS)
		conf.threads = MAX_THREADS;
	return 1;
}
