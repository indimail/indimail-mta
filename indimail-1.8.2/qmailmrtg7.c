/*
 * $Log: qmailmrtg7.c,v $
 * Revision 2.7  2011-04-15 20:21:47+05:30  Cprogrammer
 * fixed format for snprintf
 *
 * Revision 2.6  2009-11-26 11:34:08+05:30  Cprogrammer
 * removed system command
 *
 * Revision 2.5  2009-11-24 21:04:49+05:30  Cprogrammer
 * option 'S' no handles bogofilter
 *
 * Revision 2.4  2009-11-23 11:54:22+05:30  Cprogrammer
 * added getversion_qmailmrtg7_c() function
 *
 * Revision 2.3  2009-11-23 11:43:03+05:30  Cprogrammer
 * added ident keyword
 *
 * Revision 2.2  2009-11-23 11:28:37+05:30  Cprogrammer
 * 'Q' option for displaying local/remote queue size
 *
 * Revision 2.1  2009-11-23 10:32:25+05:30  Cprogrammer
 * provide mrtg support for indimail
 *
 *
 * Copyright (C) 1999-2004 Inter7 Internet Technologies, Inc.
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
 *
 *
 * kbo@inter7.com
 * Wrote everything except what is listed below. 
 * 
 * Richard A. Secor <rsecor@seqlogic.com>
 * Added support for rblsmtpd deny
 *
 * Modified for IndiMail by Manvendra Bhangui <manvendra@indimail.org>
 */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

#ifndef lint
static char     sccsid[] = "$Id: qmailmrtg7.c,v 2.7 2011-04-15 20:21:47+05:30 Cprogrammer Stab mbhangui $";
#endif

#define MAX_BUFF 1000
int             BigTodo=1;
int             ConfSplit=151;
char            TmpBuf[MAX_BUFF];
int             tconcurrency;
int             tallow;
int             tdeny;
int             ttotal;
int             success;
int             failure;
int             deferral;
int             unsub;
double          bytes;
int             local;
int             remote;
int             max_files;
int             clicked;
int             viewed;
int             cfound, cerror;
int             tspam, tclean;
int             tcached, tquery;
int             mess_count, todo_count;
time_t          end_time;
time_t          start_time;
unsigned long   get_tai(char *);
void            get_options(int argc, char **argv);
char            TheDir[MAX_BUFF];
char            TheFile[MAX_BUFF];
char            qbase[MAX_BUFF];
void            process_file(char *file_name);
void            usage();
int             get_size(char *dir);
char            TheType;

int
main(int argc, char **argv)
{
	DIR            *mydir;
	struct dirent  *mydirent;
	char           *cmmd;
	unsigned long   secs;
	int             i;

	if (argc != 3 && argc != 4) {
		usage();
		exit(-1);
	}
	snprintf(TheDir, sizeof (TheDir), "%s", argv[2]);
	TheType = *argv[1];
	cmmd = (argc == 4 ? argv[3] : 0);
	switch (TheType) {
	case 'C':
	case 't':
	case 'a':
	case 'm':
	case 'c':
	case 's':
	case 'S':
	case 'b':
	case 'q':
	case 'Q':
	case 'l':
	case 'v':
	case 'u':
	case 'd':
		break;
	default:
		usage();
		exit(-1);
	}
	end_time = time(NULL);
	start_time = end_time - 300;
	tconcurrency = 0;
	tallow = 0;
	tdeny = 0;
	ttotal = 0;
	unsub = 0;
	success = 0;
	failure = 0;
	deferral = 0;
	bytes = 0;
	local = 0;
	remote = 0;
	max_files = 0;
	clicked = 0;
	viewed = 0;
	cfound = 0;
	cerror = 0;
	tspam = 0;
	tclean = 0;
	tquery = 0;
	tcached = 0;

	if (TheType == 'q' || TheType == 'Q') {
		for (i = 1, mess_count = todo_count = 0;;i++)
		{
			snprintf(TmpBuf, sizeof (TmpBuf), "%s/queue%d/%s", TheDir, i,
				TheType == 'Q' ? "local" : "mess");
			if (access(TmpBuf, F_OK))
				break;
			max_files = 0;
			mess_count += get_size(TmpBuf);
			snprintf(TmpBuf, sizeof (TmpBuf), "%s/queue%d/%s", TheDir, i,
				TheType == 'Q' ? "remote" : "todo");
			max_files = 0;
			todo_count += get_size(TmpBuf);
		}
		snprintf(TmpBuf, sizeof (TmpBuf), "%s/nqueue/%s", TheDir,
			TheType == 'Q' ? "local" : "mess");
		max_files = 0;
		mess_count += get_size(TmpBuf);
		snprintf(TmpBuf, sizeof (TmpBuf), "%s/nqueue/%s", TheDir,
			TheType == 'Q' ? "local" : "todo");
		max_files = 0;
		todo_count += get_size(TmpBuf);
		printf("%d\n%d\n", mess_count, todo_count);
		return (0);
	}
	if (!(mydir = opendir(TheDir))) {
		printf("failed to open dir %s\n", TheDir);
		exit(-1);
	}
	while ((mydirent = readdir(mydir)) != NULL) {
		if (mydirent->d_name[0] == '@') {
			secs = get_tai(&mydirent->d_name[1]);
			if (secs > start_time) {
				snprintf(TheFile, sizeof (TheFile), "%s/%s", TheDir, mydirent->d_name);
				process_file(TheFile);
			}
		} else
		if (strcmp(mydirent->d_name, "current") == 0) {
			snprintf(TheFile, sizeof (TheFile), "%s/%s", TheDir, mydirent->d_name);
			process_file(TheFile);
		}
	}
	closedir(mydir);
	switch (TheType) {
	case 'S': /*- bogofilter */
		printf("%i\n%i\n\n\n", tclean * 12, tspam * 12);
		break;
	case 'C': /*- clamav */
		printf("%i\n%i\n\n\n", cfound * 12, cerror * 12);
		break;
	case 't': /*- tcpserver concurrency */
		printf("%d\n", tconcurrency);
		printf("%d\n", tconcurrency);
		break;
	case 'a': /*- tcpserver allow/deny */
		printf("%d\n%d\n\n\n", tallow * 12, tdeny * 12);
		break;
	case 'm': /*- messages */
		printf("%d\n%d\n\n\n", success * 12, (failure + success) * 12);
		break;
	case 'c': /*- remote/local concurrency */
		printf("%d\n%d\n\n\n", local, remote);
		break;
	case 's': /*- success/failures */
		printf("%i\n%i\n\n\n", success * 12, failure * 12);
		break;
	case 'v': /*- clicked/viewed */
		printf("%i\n%i\n\n\n", viewed * 12, clicked * 12);
		break;
	case 'l': /*- lines */
		printf("%i\n%i\n\n\n", ttotal * 12, ttotal * 12);
		break;
	case 'u': /*- lines */
		printf("%i\n%i\n\n\n", unsub * 12, unsub * 12);
		break;
	case 'b': /*- bits */
		printf("%.0f\n", (bytes * 8.0) / 300.0);
		printf("%.0f\n", (bytes * 8.0) / 300.0);
		break;
	case 'd': /*- dnscache */
		printf("%i\n%i\n\n\n", tcached * 12, tquery * 12);
		break;
	}
	return (0);
}

void
process_file(char *file_name)
{
	FILE           *fs;
	unsigned long   secs;
	int             tmpint;
	char           *tmpstr1;
	char           *tmpstr2;

	if ((fs = fopen(file_name, "r")) == NULL)
		return;
	while (fgets(TmpBuf, MAX_BUFF, fs) != NULL) {
		if (TmpBuf[0] != '@')
			continue;
		secs = get_tai(&TmpBuf[1]);
		if (secs < start_time || secs > end_time)
			continue;
		switch (TheType) {
		case 'S':
			if ((tmpstr1 = strstr(TmpBuf, "X-Bogosity: Yes")) != NULL) {
				++tspam;
			} else if ((tmpstr1 = strstr(TmpBuf, "X-Bogosity: No")) != NULL) {
				++tclean;
			}
			break;

		case 'C':
			if ((tmpstr1 = strstr(TmpBuf, "FOUND")) != NULL) {
				++cfound;
			} else if ((tmpstr1 = strstr(TmpBuf, "ERROR")) != NULL) {
				++cerror;
			}
			break;

		case 't':
			if ((tmpstr1 = strstr(TmpBuf, "status:")) != NULL) {
				while (*tmpstr1 != ':')
					++tmpstr1;
				++tmpstr1;

				for (tmpstr2 = tmpstr1 + 1; *tmpstr2 != '/'; ++tmpstr2);
				*tmpstr2 = 0;
				tmpint = atoi(tmpstr1);
				if (tmpint > tconcurrency)
					tconcurrency = tmpint;
			}
			break;

		case 'a':
			if ((tmpstr1 = strstr(TmpBuf, " ok ")) != NULL) {
				++tallow;
			} else if ((tmpstr1 = strstr(TmpBuf, " deny ")) != NULL) {
				++tdeny;
			} else if ((tmpstr1 = strstr(TmpBuf, " rblsmtpd:")) != NULL) {
				++tdeny;
			}
			break;

		case 'c':
			if ((tmpstr1 = strstr(TmpBuf, " status: ")) != NULL) {
				while (*tmpstr1 != '/' && *tmpstr1 != 0)
					++tmpstr1;
				*tmpstr1 = 0;
				tmpstr2 = tmpstr1 + 1;
				--tmpstr1;

				while (*tmpstr1 != ' ')
					--tmpstr1;
				++tmpstr1;
				tmpint = atoi(tmpstr1);
				if (tmpint > local)
					local = tmpint;

				while (*tmpstr2 != '/' && *tmpstr2 != 0)
					++tmpstr2;
				*tmpstr2 = 0;
				--tmpstr2;
				while (*tmpstr2 != ' ')
					--tmpstr2;
				++tmpstr2;
				tmpint = atoi(tmpstr2);
				if (tmpint > remote)
					remote = tmpint;
			}
			break;

		case 's':
		case 'm':
			if (strstr(TmpBuf, "success:"))
				success++;
			if (strstr(TmpBuf, "failure:"))
				failure++;
			if (strstr(TmpBuf, "deferral:"))
				deferral++;
			break;

		case 'v':
			if (strstr(TmpBuf, "viewed:"))
				viewed++;
			if (strstr(TmpBuf, "clicked:"))
				clicked++;
			break;

		case 'u':
			if (strstr(TmpBuf, "unsub:"))
				unsub++;
			break;

		case 'b':
			if ((tmpstr1 = strstr(TmpBuf, ": bytes ")) != NULL) {
				tmpstr1 += 8;
				bytes += atol(tmpstr1);
			}
			break;
		case 'l':
			++ttotal;
			break;
		case 'd':
			if (strstr(TmpBuf, "cached"))
				tcached++;
			if (strstr(TmpBuf, "query"))
				tquery++;
			break;
		}
	}
	fclose(fs);
}

unsigned long
get_tai(char *tmpstr)
{
	unsigned long   secs;
	unsigned long   nanosecs;
	unsigned long   u;

	secs = 0;
	nanosecs = 0;

	for (; *tmpstr != 0; ++tmpstr) {
		u = *tmpstr - '0';
		if (u >= 10) {
			u = *tmpstr - 'a';
			if (u >= 6)
				break;
			u += 10;
		}
		secs <<= 4;
		secs += nanosecs >> 28;
		nanosecs &= 0xfffffff;
		nanosecs <<= 4;
		nanosecs += u;
	}
	secs -= 4611686018427387914ULL;

	return (secs);
}

void            getversion_qmailmrtg7_c();

void
usage()
{
	fprintf(stderr, "usage: type dir\n");
	fprintf(stderr, "where type is one of t, a, m, c, s, b, q, l, v, S, C, d\n");
	fprintf(stderr, "and dir is a directory containing multilog files\n");
	fprintf(stderr, "for q option dir is the qmail queue dir\n");
	getversion_qmailmrtg7_c();
}

int
count_files(char *diri)
{
	DIR            *mydir;
	int             count;
	struct dirent  *mydirent;

	if (!(mydir = opendir(diri)))
		return (0);
	for (count = 0; (mydirent = readdir(mydir)) != NULL;) {
		if (strcmp(mydirent->d_name, ".") != 0 && strcmp(mydirent->d_name, "..") != 0) {
			++count;
		}
	}
	closedir(mydir);
	return (count);
}

int
get_size(diri)
	char           *diri;
{
	int             i;
	int             count;
	char            tmpbuf[200];

	for (i = 0, count = 0; i < ConfSplit; ++i) {
		snprintf(tmpbuf, sizeof (tmpbuf), "%s/%d", diri, i);
		count += count_files(tmpbuf);
	}
	return (count);
}

void
getversion_qmailmrtg7_c()
{
	printf("%s\n", sccsid);
}
