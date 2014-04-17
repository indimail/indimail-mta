/*
 * $Log: main.c,v $
 * Revision 1.2  2014-04-17 11:27:49+05:30  Cprogrammer
 * added username argument to set_user()
 *
 * Revision 1.1  2013-05-15 00:29:21+05:30  Cprogrammer
 * Initial revision
 *
 * Copyright (C) 1998-2002 Martin Roesch <roesch@sourcefire.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

// For further information about licensing please refer to the LICENSE file

#include<stdio.h>

#include "config.h"

#ifdef HAVE_GETOPT_H
#include <getopt.h>
#define HAVE_GETOPTLONG 1
#else
#ifdef SOLARIS
#include <unistd.h>
#endif
#endif

#include "parser.h"
#include "process.h"
#include "user.h"
#include "sig.h"

#define READ_STDIN "-"

#define PARENT_MODE(x)      (x)->parent_mode

#define SET_DEFAULT_CONF(c) (c)->retry = DEFAULT_RETRY; \
			    (c)->matchsleep = DEFAULT_MATCHSLEEP; \
			    (c)->matchcount = DEFAULT_MATCHCOUNT; \
			    (c)->readall = FALSE ; \
			    (c)->parent_mode = FALSE ; \
			    (c)->pattern = (c)->cmd =  NULL ;\
			    (c)->watchfile = (c)->parent_conffile = NULL

char           *progname;

static struct entry_conf def_conf;
extern struct entry_conf **conf_table;


#ifdef HAVE_GETOPTLONG
static struct option const long_options[] = {
	{"match", required_argument, 0, 'm'},
	{"exec", required_argument, 0, 'e'},
	{"match_sleep", optional_argument, 0, 's'},
	{"config-file", optional_argument, 0, 'c'},
	{"user", optional_argument, 0, 'u'},
	{"retry", optional_argument, 0, 'r'},
	{"help", no_argument, 0, 'h'},
	{"verbose", no_argument, 0, 'v'},
	{"from-begin", no_argument, 0, 'b'},
	{NULL, 0, NULL, 0}
};
#endif

void
usage(int e)
{

	fprintf(stderr, "usage: %s OPTIONS... FILE \n", progname);
	fprintf(stderr, "\nwhere OPTIONS are:\n");
	fprintf(stderr, "\n\
	-m, --match=PATTERN	watch for the regular expression PATTERN in file.");
	fprintf(stderr, "\n\
	-e, --exec=COMMAND	if any match is found, execute COMMAND [see note bellow]");
	fprintf(stderr, "\n\
	-u, --user=USER		monitor the file as USER. Only valid if run as root.");
	fprintf(stderr, "\n\
	-s, --match-sleep=SEC	specify how many SECONDS we should disable exec after\n\
                        	the first match. Default is %d seconds.", DEFAULT_MATCHSLEEP);
	fprintf(stderr, "\n\
	-i, --match-count=COUNT	specify after how many matches we should enable exec\n\
	                        Default is %d.", DEFAULT_MATCHCOUNT);
	fprintf(stderr, "\n\
	-r, --retry=RETRIES	how many times we'll try to read the file before\n\
				giving up. Default is %d .", DEFAULT_RETRY);
	fprintf(stderr, "\n\
	-b, --from-begin	If set, read the file from the beggining. The defaul\n\
				is to start reading from the end of the file [like tail -f].");
	fprintf(stderr, "\n\
	-v, --verbose		Output file contents (just like 'tail -f'). Only in normal mode.");

	fprintf(stderr, "\n\
	-c, --config-file=CONFFILE  PARENT MODE: specify the configuration file which\n\
				    defines a set of files to monitor.\n \
				    If set, we assume PARENT MODE.\n");
	fprintf(stderr, "\n\n");
	fprintf(stderr, "EXAMPLES:\n");
	fprintf(stderr, "\n\
	SINGLE MODE: watch for REGEX pattern occurrence in the specified file.\n\
       		If found, COMMAND is executed:\n");
	fprintf(stderr, "\n\
		%s --match 'E[Rr]{2}or' --exec '/bin/pager' /var/log/messages\n", progname);
	fprintf(stderr, "\n\
	PARENT MODE: read the configuration file specified CONFFILE which contains\n\
       		information to which files and what to monitor:\n");
	fprintf(stderr, "\n\
		%s --config-file /etc/sswatchd.conf\n", progname);
	fprintf(stderr, "NOTES:\n");
	fprintf(stderr, "\n\
	COMMAND executed by -e or --exec options is passed to a default shell with\n\
	argument '-c'\n");
	fflush(stderr);
	exit(e);

}

int
get_options(int argc, char **argv)
{
	char            c;

#ifdef HAVE_GETOPTLONG
	while ((c = getopt_long(argc, argv, "m:r:u:e:s:i:c:bvh", long_options, NULL)) != -1)
#else
	while ((c = getopt(argc, argv, "m:r:u:e:s:i:c:bvh")) != -1)
#endif
	{
		switch (c) {
		case 0:
			break;
		case 'm':
			def_conf.pattern = optarg;
			break;
		case 'e':
			def_conf.cmd = optarg;
			break;
		case 'r':
			def_conf.retry = atoi(optarg);
			break;
		case 's':
			def_conf.matchsleep = atoi(optarg);
			break;
		case 'i':
			def_conf.matchcount = atoi(optarg);
			break;
		case 'u':
			def_conf.user = optarg;
			break;
		case 'b':
			def_conf.readall = 1;
			break;
		case 'v':
			def_conf.verbose = 1;
			break;
		case 'c':				//parent mode only
			def_conf.parent_conffile = optarg;
			def_conf.parent_mode = TRUE;
			break;
		case 'h':
			usage(0);
		default:
			usage(1);
		}
	}
	if (PARENT_MODE(&def_conf) != TRUE) {
		if ((!def_conf.pattern) || (!def_conf.cmd)) {
			fprintf(stderr, "error: forgot to specify -m/--match or -e/--exec ?\n\n");
			usage(1);
		}
		if (!argv[optind]) {
			fprintf(stderr, "ok, no filename specified, using stdin.. \n\n");
			def_conf.watchfile = READ_STDIN;
		} else {
			def_conf.watchfile = argv[optind];
		}
	}
	return optind;
}


int
main(int argc, char **argv)
{

	sig_init();
	progname = argv[0];
	SET_DEFAULT_CONF(&def_conf);
	if (get_options(argc, argv) <= 1) {
		usage(1);
	}
	if (def_conf.user)
		set_user(get_user_id(def_conf.user), get_guser_id(def_conf.user), def_conf.user);
	if (PARENT_MODE(&def_conf)) {

		handle_conf_file(def_conf.parent_conffile);
		if (!conf_table) {
			debug("Error while creating conf_table");
			exit(1);
		}
		create_proc_table((struct entry_conf **) conf_table);
	} else {
		debug("Monitoring %s...", def_conf.watchfile);
		return (monitor_file(&def_conf));
	}
	return (0);
}
