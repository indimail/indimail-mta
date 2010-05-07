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
 * $Id: nsvsd.h,v 1.2 2005/01/31 01:09:22 cinergi Exp $ 
 */
#define MAX_LINE_SIZE       1024	/* Max line length in config file */
#define MAX_KEY_SIZE        128	/* Max length of a key in cfg file */
#define MAX_VAL_SIZE        1024	/* Max length of a val in cfg file */
#define MAX_QUERY_SIZE      2048	/* Max length of MySQL query */
#define MAX_USERNAME_SIZE   128

#define DEFAULT_THREADS     5
#define MAX_THREADS         20
#define DEFAULT_PRIORITY    "LOG_NOTICE"
#define DEFAULT_FACILITY    "LOG_DAEMON"
#define DEFAULT_RUNAS       "indimail"
#define DEFAULT_PIDFILE     "/var/run/nssd.pid"
#define DEFAULT_TIMEOUT     3

#define READ_TIMEOUT        1
#define WRITE_TIMEOUT       1

/*
 * Sql queries to execute for ... 
 */
typedef struct {
	char            getpwuid[MAX_VAL_SIZE];
	char            getpwnam[MAX_VAL_SIZE];
	char            getspnam[MAX_VAL_SIZE];
	char            getpwent[MAX_VAL_SIZE];
	char            getspent[MAX_VAL_SIZE];
	char            getgrnam[MAX_VAL_SIZE];
	char            getgrgid[MAX_VAL_SIZE];
	char            getgrent[MAX_VAL_SIZE];
	char            gidsbymem[MAX_VAL_SIZE];
	char            memsbygid[MAX_VAL_SIZE];
	char            host[MAX_VAL_SIZE];
	int             port;
	char            socket[MAX_VAL_SIZE];
	char            username[MAX_VAL_SIZE];
	char            password[MAX_VAL_SIZE];
	char            database[MAX_VAL_SIZE];
	char            pidfile[MAX_VAL_SIZE];
	int             threads;
	int             timeout;
	char            facility[MAX_VAL_SIZE];
	int             ifacility;
	char            priority[MAX_VAL_SIZE];
	int             ipriority;
} conf_t;

extern conf_t   conf;

typedef struct {
	int32_t         num_fields;
	char           *query;
} query_conf_t;

void            nssd_log(int prio, const char *fmt, ...);
int             str2priority(char *str);
int             str2facility(char *str);
int             load_config(const char *file);
