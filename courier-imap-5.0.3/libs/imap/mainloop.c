/*
** Copyright 1998 - 2006 Double Precision, Inc.
** See COPYING for distribution information.
*/

#if	HAVE_CONFIG_H
#include	"config.h"
#endif
#include	<stdio.h>
#if HAVE_STDLIB_H
#include	<stdlib.h>
#endif
#ifdef HAVE_INTTYPES_H
#include    <inttypes.h>
#endif
#include	<string.h>
#include	<errno.h>
#include	<ctype.h>
#include	<fcntl.h>
#include	<signal.h>
#include	<unistd.h>
#include	"imaptoken.h"
#include	"imapwrite.h"
#include	"numlib/numlib.h"

static const char rcsid[]="$Id: mainloop.c,v 1.10 2006/07/07 23:17:18 mrsam Exp $";

extern int do_imap_command(const char *, int *);

extern unsigned long header_count, body_count;
extern unsigned long bytes_received_count, bytes_sent_count;
extern time_t start_time;

static RETSIGTYPE sigexit(int n)
{
	static char byemsg[]="* BYE IMAP4erv1 server shut down by signal.\r\n";
	const char *a=getenv("AUTHENTICATED");
	char buf[NUMBUFSIZE];
	char msg[1024];
	int l = 0;
	const char *tls=getenv("IMAP_TLS");
	const char *ip=getenv("TCPREMOTEIP");

	if (write(1, byemsg, sizeof(byemsg)-1) < 0)
		exit(1);

	bytes_sent_count += sizeof(byemsg);

	libmail_str_time_t(time(NULL)-start_time, buf);

	if (tls && atoi(tls))
		tls=", starttls=1";
	else
		tls="";

	if (a && *a)
		l = snprintf(msg, sizeof(msg) - 1, "NOTICE: Disconnected during shutdown by signal, user=%s, "
			"ip=[%s], headers=%lu, body=%lu, rcvd=%lu, sent=%lu, time=%s%s\n",
			a, ip, header_count, body_count, bytes_received_count, bytes_sent_count,
			buf, tls);
	else
		l = snprintf(msg, sizeof(msg) - 1, "NOTICE: Disconnected during shutdown by signal, ip=[%s], time=%s%s\n",
			getenv("TCPREMOTEIP"),
			buf, tls);

	if (l > 0 && write(2, msg, l))
		; /* Suppress gcc warning */

	exit(0);
#if	RETSIGTYPE != void
	return (0);
#endif
}


void cmdfail(const char *tag, const char *msg)
{
#if SMAP
	const char *p=getenv("PROTOCOL");

	if (p && strcmp(p, "SMAP1") == 0)
		writes("-ERR ");
	else
#endif
	{
		writes(tag);
		writes(" NO ");
	}
	writes(msg);
}

void cmdsuccess(const char *tag, const char *msg)
{
#if SMAP
	const char *p=getenv("PROTOCOL");

	if (p && strcmp(p, "SMAP1") == 0)
		writes("+OK ");
	else
#endif
	{
		writes(tag);
		writes(" OK ");
	}
	writes(msg);
}

void mainloop(void)
{
	int noerril = 0;
	uint64_t max_atom_size = 0;
	char *tag, *ptr;

	if (!max_atom_size) {
		max_atom_size = (ptr = getenv("MAX_ATOM_SIZE")) ? strtoull(ptr, 0, 0) : IT_MAX_ATOM_SIZE;
		if (!(tag = (char *) malloc(max_atom_size + 1)))
			write_error_exit("error allocating atom");
	}
	signal(SIGTERM, sigexit);
	signal(SIGHUP, sigexit);
	signal(SIGINT, sigexit);

	for (;;)
	{
	struct	imaptoken *curtoken;

		read_timeout(30 * 60);
		curtoken=nexttoken_nouc();
		tag[0]=0;
		if (curtoken->tokentype == IT_ATOM ||
			curtoken->tokentype == IT_NUMBER)
		{
		int	rc;
		int flushflag = 0;

			if (strlen(tag)+strlen(curtoken->tokenbuf) > max_atom_size)
				write_error_exit("max atom size too small");
		  		
			strncat(tag, curtoken->tokenbuf, max_atom_size);
			rc=do_imap_command(tag, &flushflag);

			if (rc == 0)
			{
				noerril = 0;
				writeflush();
				read_eol();
				if (flushflag)
					readflush();
				continue;
			}
			if (rc == -2)
				continue;
		}

		noerril++;
		if (noerril > 9)
		{
			errno = 0;
			write_error_exit("TOO MANY CONSECUTIVE PROTOCOL VIOLATIONS");
		}
		read_eol();
		cmdfail(tag[0] ? tag:"*",
			"Error in IMAP command received by server.\r\n");
		writeflush();
	}
}
