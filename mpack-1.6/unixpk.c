/*-
 * $Log: unixpk.c,v $
 * Revision 1.4  2008-06-13 19:25:09+05:30  Cprogrammer
 * include config.h to prevent warning for strchr
 *
 * Revision 1.3  2008-05-21 18:44:58+05:30  Cprogrammer
 * code beautified
 *
 * Revision 1.2  2004-01-06 14:55:21+05:30  Manny
 * fixed compilation warnings
 *
 * Revision 1.1  2004-01-06 12:44:18+05:30  Manny
 * Initial revision
 *
 *
 * (C) Copyright 1993,1994 by Carnegie Mellon University
 * All Rights Reserved.
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of Carnegie
 * Mellon University not be used in advertising or publicity
 * pertaining to distribution of the software without specific,
 * written prior permission.  Carnegie Mellon University makes no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied
 * warranty.
 *
 * CARNEGIE MELLON UNIVERSITY DISCLAIMS ALL WARRANTIES WITH REGARD TO
 * THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS, IN NO EVENT SHALL CARNEGIE MELLON UNIVERSITY BE LIABLE
 * FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
 * AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING
 * OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "config.h"
#include "common.h"
#include "version.h"
#include "xmalloc.h"

#define MAXADDRESS 100

extern char    *getenv();

extern int      errno;
extern int      optind;
extern char    *optarg;

void            usage(void);
void            sendmail(FILE * infile, char **addr, int start);
int             encode(FILE *, FILE *, char *, FILE *, char *, char *, long, char *, char *);
void            inews(FILE * infile);

int
main(int argc, char **argv)
{
	int             opt, len, i, part;
	long            maxsize = 0;
	char           *fname = 0, *subject = 0, *descfname = 0;
	char           *outfname = 0, *newsgroups = 0, *ctype = 0;
	char           *headers = 0, *p;
	char            sbuf[1024], fnamebuf[4096], outFname[256];
	FILE           *infile = 0, *descfile = 0;

	if ((p = getenv("SPLITSIZE")) && *p >= '0' && *p <= '9')
		maxsize = atoi(p);
	while ((opt = getopt(argc, argv, "s:d:m:c:o:n:")) != EOF)
	{
		switch (opt)
		{
		case 's':
			subject = optarg;
			break;

		case 'd':
			descfname = optarg;
			break;

		case 'm':
			maxsize = atoi(optarg);
			break;

		case 'c':
			ctype = optarg;
			break;

		case 'o':
			outfname = optarg;
			break;

		case 'n':
			newsgroups = optarg;
			break;

		default:
			usage();

		}
	}

	if (ctype)
	{
		if (!strncasecmp(ctype, "text/", 5))
		{
			fprintf(stderr, "This program is not appropriate for encoding textual data\n");
			exit(1);
		}
		if (strncasecmp(ctype, "application/", 12) && strncasecmp(ctype, "audio/", 6) && strncasecmp(ctype, "image/", 6) &&
			strncasecmp(ctype, "video/", 6))
		{
			fprintf(stderr, "Content type must be subtype of application, audio, image, or video\n");
			exit(1);
		}
	}
	if (optind == argc)
	{
		fprintf(stderr, "An input file must be specified\n");
		usage();
	}
	fname = argv[optind++];

	/*
	 * Must have exactly one of -o, -n, or destination addrs 
	 */
	if (optind == argc)
	{
		if (outfname && newsgroups)
		{
			fprintf(stderr, "The -o and -n switches are mutually exclusive.\n");
			usage();
		}
		if (!outfname && !newsgroups)
		{
			fprintf(stderr, "Either an address or one of the -o or -n switches is required\n");
			usage();
		}
		if (newsgroups)
		{
			headers = xmalloc(strlen(newsgroups) + 25);
			sprintf(headers, "Newsgroups: %s\n", newsgroups);
		}
	} else /*- addresses */
	{
		if (outfname)
		{
			fprintf(stderr, "The -o switch and addresses are mutually exclusive.\n");
			usage();
		}
		if (newsgroups)
		{
			fprintf(stderr, "The -n switch and addresses are mutually exclusive.\n");
			usage();
		}
		len = strlen(argv[optind]) + 6;
		for (i = optind + 1;i < argc;i++)
			len += (strlen(argv[i]) + 3);
		headers = xmalloc(len);
		snprintf(headers, len, "To: %s", argv[optind]);
		for (i = optind + 1; i < argc; i++)
		{
			strcat(headers, ",\n\t");
			strcat(headers, argv[i]);
		}
		strcat(headers, "\n");
#if 0
		headers = xmalloc(strlen(argv[optind]) + 25);
		sprintf(headers, "To: %s", argv[optind]);
		for (i = optind + 1; i < argc; i++)
		{
			headers = xrealloc(headers, strlen(headers) + strlen(argv[i]) + 25);
			strcat(headers, ",\n\t");
			strcat(headers, argv[i]);
		}
		strcat(headers, "\n");
#endif
	}
	if (!subject)
	{
		fputs("Subject: ", stdout);
		fflush(stdout);
		if (!fgets(sbuf, sizeof(sbuf), stdin))
		{
			fprintf(stderr, "A subject is required\n");
			usage();
		}
		if ((p = strchr(sbuf, '\n')))
			*p = '\0';
		subject = sbuf;
	}
	if (!outfname)
	{
		pid_t           pid;
		for (pid = getpid();; pid++)
		{
			if ((p = getenv("TMPDIR")))
				snprintf(outFname, sizeof(outFname), "%s/mpack.%d", p, pid);
			else
				snprintf(outFname, sizeof(outFname), "/tmp/mpack.%d", pid);
			if (access(outFname, F_OK))
				break;
		}
		outfname = outFname;
	}
	if (!(infile = fopen(fname, "r")))
	{
		perror(fname);
		exit(1);
	}
	if (descfname)
	{
		descfile = fopen(descfname, "r");
		if (!descfile)
		{
			perror(descfname);
			exit(1);
		}
	}
	if (encode(infile, (FILE *) 0, fname, descfile, subject, headers, maxsize, ctype, outfname))
		exit(1);
	if (optind < argc || newsgroups)
	{
		for (part = 0;; part++)
		{
			sprintf(fnamebuf, "%s.%02d", outfname, part);
			infile = fopen(part ? fnamebuf : outfname, "r");
			if (!infile)
			{
				if (part)
					break;
				continue;
			}
			if (newsgroups)
			{
				inews(infile);
			} else
			{
				sendmail(infile, argv, optind);
			}
			fclose(infile);
			remove(part ? fnamebuf : outfname);
		}
	}
	exit(0);
}

void
usage(void)
{
	fprintf(stderr, "mpack version %s\n", MPACK_VERSION);
	fprintf(stderr, "usage: mpack [-s subj] [-d file] [-m maxsize] [-c content-type] input_file address...\n");
	fprintf(stderr, "       mpack [-s subj] [-d file] [-m maxsize] [-c content-type] -o file input_file\n");
	fprintf(stderr, "       mpack [-s subj] [-d file] [-m maxsize] [-c content-type] -n groups input_file\n");
	exit(1);
}

void
sendmail(FILE * infile, char **addr, int start)
{
	int             status;
	int             pid;

	if (start < 2)
		abort();

#ifdef SCO
	addr[--start] = "execmail";
#else
	addr[--start] = "-oi";
	addr[--start] = "sendmail";
#endif

	do
	{
		pid = fork();
	}
	while (pid == -1 && errno == EAGAIN);

	if (pid == -1)
	{
		perror("fork");
		return;
	}
	if (pid != 0)
	{
		while (pid != wait(&status));
		return;
	}

	dup2(fileno(infile), 0);
	fclose(infile);
#ifdef SCO
	execv("/usr/lib/mail/execmail", addr + start);
#else
	execv("/usr/lib/sendmail", addr + start);
	execv("/usr/sbin/sendmail", addr + start);
#endif
	perror("execv");
	_exit(1);
}

void
inews(FILE * infile)
{
	int             status;
	int             pid;

	do
	{
		pid = fork();
	}
	while (pid == -1 && errno == EAGAIN);

	if (pid == -1)
	{
		perror("fork");
		return;
	}
	if (pid != 0)
	{
		while (pid != wait(&status));
		return;
	}

	dup2(fileno(infile), 0);
	fclose(infile);
	execlp("inews", "inews", "-h", "-S", (char *) 0);
	execl("/usr/local/news/inews", "inews", "-h", "-S", (char *) 0);
	execl("/usr/local/lib/news/inews", "inews", "-h", "-S", (char *) 0);
	execl("/etc/inews", "inews", "-h", "-S", (char *) 0);
	execl("/usr/etc/inews", "inews", "-h", "-S", (char *) 0);
	execl("/usr/news/inews", "inews", "-h", "-S", (char *) 0);
	execl("/usr/news/bin/inews", "inews", "-h", "-S", (char *) 0);
	perror("execl");
	_exit(1);
}

void
warn(void)
{
	abort();
}
