/*
 * $Log: valias.c,v $
 * Revision 2.9  2015-12-12 14:36:55+05:30  Cprogrammer
 * display aliases for a domain if Email is specified as @domain
 *
 * Revision 2.8  2014-07-03 00:08:40+05:30  Cprogrammer
 * added option to track all alias which deliver to an address
 *
 * Revision 2.7  2011-11-09 19:45:51+05:30  Cprogrammer
 * removed getversion
 *
 * Revision 2.6  2009-11-25 12:53:55+05:30  Cprogrammer
 * do not allow empty alias line
 *
 * Revision 2.5  2009-09-13 12:48:20+05:30  Cprogrammer
 * added 'm' option to getopt for previous ignore option
 *
 * Revision 2.4  2009-09-13 12:46:38+05:30  Cprogrammer
 * added option to ignore requirement of destination email address to be local
 *
 * Revision 2.3  2008-06-13 10:35:25+05:30  Cprogrammer
 * fixed compilation eror if VALIAS not defined
 *
 * Revision 2.2  2004-07-03 23:53:52+05:30  Cprogrammer
 * check return status of parse_email
 *
 * Revision 2.1  2003-03-24 19:25:52+05:30  Cprogrammer
 * preconnect to mysql before invoking actual functions
 *
 * Revision 1.10  2002-02-24 22:46:32+05:30  Cprogrammer
 * added getversion
 *
 * Revision 1.9  2001-12-27 01:29:21+05:30  Cprogrammer
 * version and verbose switch update
 *
 * Revision 1.8  2001-12-27 00:49:18+05:30  Cprogrammer
 * added update option
 *
 * Revision 1.7  2001-12-27 00:31:45+05:30  Cprogrammer
 * corrected delete option
 *
 * Revision 1.6  2001-12-09 23:55:56+05:30  Cprogrammer
 * removed unused variable User
 *
 * Revision 1.5  2001-12-08 17:45:22+05:30  Cprogrammer
 * usage message change
 *
 * Revision 1.4  2001-11-28 23:02:15+05:30  Cprogrammer
 * getversion() function call added
 *
 * Revision 1.3  2001-11-24 12:20:31+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.2  2001-11-20 10:56:25+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.1  2001-10-24 18:15:19+05:30  Cprogrammer
 * Initial revision
 *
 * manage alias files in valias database 
 * 
 * Copyright (C) 1999,2001 Inter7 Internet Technologies, Inc.
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

#include "indimail.h"
#ifndef	lint
static char     sccsid[] = "$Id: valias.c,v 2.9 2015-12-12 14:36:55+05:30 Cprogrammer Exp mbhangui $";
#endif

#ifdef VALIAS
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pwd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>

char            Email[MAX_BUFF];
char            Alias[MAX_BUFF];
char            Domain[MAX_BUFF];
char            AliasLine[MAX_BUFF];
char            OAliasLine[MAX_BUFF];

#define VALIAS_SELECT 0
#define VALIAS_INSERT 1
#define VALIAS_DELETE 2
#define VALIAS_UPDATE 3
#define VALIAS_TRACK  4

int             AliasAction, ignore_mailstore;

void            usage();
int             get_options(int argc, char **argv);

int
main(argc, argv)
	int             argc;
	char           *argv[];
{
	char           *tmpalias_line;

	if(get_options(argc, argv))
		return(1);
	if (vauth_open((char *) 0))
		return (1);
	switch (AliasAction)
	{
	case VALIAS_SELECT:
		if (!*Alias)
		{
			for(;;)
			{
				if(!(tmpalias_line = valias_select_all(Alias, Domain, MAX_BUFF)))
					break;
				printf("%s@%s -> %s\n", Alias, Domain, tmpalias_line);
			}
		} else
		{
			for(;;)
			{
				if(!(tmpalias_line = valias_select(Alias, Domain)))
					break;
				printf("%s@%s -> %s\n", Alias, Domain, tmpalias_line);
			}
		}
		break;
	case VALIAS_INSERT:
			valias_insert(Alias, Domain, AliasLine, ignore_mailstore);
		break;
	case VALIAS_DELETE:
			valias_delete(Alias, Domain, AliasLine);
		break;
	case VALIAS_UPDATE:
			valias_update(Alias, Domain, OAliasLine, AliasLine);
		break;
	case VALIAS_TRACK:
			for(;;)
			{
				if(!(tmpalias_line = valias_track(Alias, Domain, Email, MAX_BUFF)))
					break;
				printf("%s@%s -> %s\n", Alias, Domain, Email);
			}
		break;
	default:
		fprintf(stderr, "error, Alias Action is invalid %d\n", AliasAction);
		break;
	}
	return(0);
}

void
usage()
{
	fprintf(stderr, "usage: valias [options] [email_address | domain_name]\n");
	fprintf(stderr, "options: -V ( print version number )\n");
	fprintf(stderr, "         -v ( verbose )\n");
	fprintf(stderr, "         -s ( show aliases )\n");
	fprintf(stderr, "         -S ( track aliases )\n");
	fprintf(stderr, "         -d alias_line (delete alias line)\n");
	fprintf(stderr, "         -i alias_line (insert alias line)\n");
	fprintf(stderr, "         -u old_alias_line -i new_alias_line (update alias line)\n");
#ifdef CLUSTERED_SITE
	fprintf(stderr, "         -m ( ignore requirement of email_address to be local)\n");
#endif
}

int
get_options(int argc, char **argv)
{
	int             c;
	extern char    *optarg;
	extern int      optind;

	memset(Alias, 0, MAX_BUFF);
	memset(Email, 0, MAX_BUFF);
	memset(Domain, 0, MAX_BUFF);
	memset(AliasLine, 0, MAX_BUFF);
	AliasAction = VALIAS_SELECT;
	while ((c = getopt(argc, argv, "vmsSu:d:i:")) != -1)
	{
		switch (c)
		{
#ifdef CLUSTERED_SITE
		case 'm':
			ignore_mailstore = 1;
#endif
		case 'v':
			verbose = 1;
			break;
		case 'S':
			AliasAction = VALIAS_TRACK;
			break;
		case 's':
			AliasAction = VALIAS_SELECT;
			break;
		case 'u':
			AliasAction = VALIAS_UPDATE;
			if (!*optarg)
			{
				fprintf(stderr, "You cannot have an empty alias line\n");
				usage();
				return(1);
			}
			scopy(OAliasLine, optarg, MAX_BUFF);
			break;
		case 'd':
			AliasAction = VALIAS_DELETE;
			scopy(AliasLine, optarg, MAX_BUFF);
			break;
		case 'i':
			if(AliasAction != VALIAS_UPDATE)
				AliasAction = VALIAS_INSERT;
			if (!*optarg)
			{
				fprintf(stderr, "You cannot have an empty alias line\n");
				usage();
				return(1);
			}
			scopy(AliasLine, optarg, MAX_BUFF);
			break;
		default:
			usage();
			return(1);
		}
		if (c == 's')
			break;
	}
	if (optind < argc)
	{
		scopy(Email, argv[optind++], MAX_BUFF);
		if (parse_email(Email, Alias, Domain, MAX_BUFF))
		{
			fprintf(stderr, "%s: Email too long\n", Email);
			return(1);
		}
	}
	if (AliasAction != VALIAS_SELECT && !*Email)
	{
		usage();
		fprintf(stderr, "must supply alias email address or a domain name\n");
		return(1);
	}
	return(0);
}
#else
int
main()
{
	printf("IndiMail not configured with --enable-valias=y\n");
	return(0);
}
#endif

void
getversion_valias_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
