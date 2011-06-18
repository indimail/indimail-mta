/*
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
 * $Id: nssd_misc.c,v 1.1 2011-06-18 11:38:41+05:30 Cprogrammer Exp mbhangui $ 
 */
#include "common.h"
#include <string.h>
#include <stdarg.h>
#include "nssd.h"

typedef struct _code {
	char           *c_name;
	int             c_val;
} CODE;

CODE            prioritynames[] = {
	{"emerg", LOG_EMERG},
	{"alert", LOG_ALERT},
	{"crit", LOG_CRIT},
	{"err", LOG_ERR},
	{"warning", LOG_WARNING},
	{"notice", LOG_NOTICE},
	{"info", LOG_INFO},
	{"debug", LOG_DEBUG},
	{NULL, -1}
};

CODE            facilitynames[] = {
	{"auth", LOG_AUTH},
#ifdef LOG_AUTHPRIV
	{"authpriv", LOG_AUTHPRIV},
#endif
	{"cron", LOG_CRON},
	{"daemon", LOG_DAEMON},
#ifdef LOG_FTP
	{"ftp", LOG_FTP},
#endif
	{"kern", LOG_KERN},
	{"lpr", LOG_LPR},
	{"mail", LOG_MAIL},
	{"news", LOG_NEWS},
	{"syslog", LOG_SYSLOG},
	{"user", LOG_USER},
	{"uucp", LOG_UUCP},
	{"local0", LOG_LOCAL0},
	{"local1", LOG_LOCAL1},
	{"local2", LOG_LOCAL2},
	{"local3", LOG_LOCAL3},
	{"local4", LOG_LOCAL4},
	{"local5", LOG_LOCAL5},
	{"local6", LOG_LOCAL6},
	{"local7", LOG_LOCAL7},
	{NULL, -1}
};

int
str2priority(char *str)
{
	CODE           *c;

	for (c = prioritynames; c->c_name; c++) {
		if (!strcmp(str, c->c_name))
			return c->c_val;
	}
	return LOG_NOTICE;
}

int
str2facility(char *str)
{
	CODE           *c;

	for (c = facilitynames; c->c_name; c++) {
		if (!strcmp(str, c->c_name))
			return c->c_val;
	}
	return LOG_DAEMON;
}

#define MAX_MSG_SIZE 1024
extern int      debug_level;
void
nssd_log(int prio, const char *fmt, ...)
{
	va_list         ap;
	char            msg[MAX_MSG_SIZE];

	if (prio > conf.ipriority && prio > debug_level)
		return;
	va_start(ap, fmt);
	vsnprintf(msg, sizeof (msg), fmt, ap);
	if (debug_level) {
		fprintf(stderr, "%s\n", msg);
	} else
		syslog(prio, "%d: %s", getpid(), msg);
	va_end(ap);
}
