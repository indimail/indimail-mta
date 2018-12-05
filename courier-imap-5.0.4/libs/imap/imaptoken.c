/*
** Copyright 1998 - 2005 Double Precision, Inc.
** See COPYING for distribution information.
*/

#if	HAVE_CONFIG_H
#include	"config.h"
#endif

#include	"imaptoken.h"
#include	"imapwrite.h"
#include	<stdio.h>
#include	<ctype.h>
#include	<stdlib.h>
#include	<string.h>
#include	<sys/types.h>
#include	<sys/time.h>
#include	"numlib/numlib.h"
#if HAVE_STDLIB_H
#include	<stdlib.h>
#endif
#include	"imapd.h"
#include	"unicode/courier-unicode.h"
#if	HAVE_UNISTD_H
#include	<unistd.h>
#endif

#ifndef	BUFSIZ
#define	BUFSIZ	8192
#endif

static const char rcsid[]="$Id: imaptoken.c,v 1.20 2005/10/01 03:23:44 mrsam Exp $";

static struct imaptoken curtoken;
static char readbuf[BUFSIZ];

char *imap_readptr=0;
size_t imap_readptrleft=0;
time_t start_time;

static time_t readtimeout;

extern FILE *debugfile;

extern unsigned long header_count, body_count;

unsigned long bytes_received_count = 0; /* counter for received bytes */
unsigned long bytes_sent_count; /* counter for sent bytes (imapwrite.c) */

extern void bye();

void bye_msg(const char *type)
{
	const char *a=getenv("AUTHENTICATED");
	char buf[NUMBUFSIZE];
	const char *tls=getenv("IMAP_TLS");

	libmail_str_time_t(time(NULL)-start_time, buf);

	if (tls && atoi(tls))
		tls=", starttls=1";
	else
		tls="";

	if (a && *a)
		fprintf(stderr, "%s, user=%s, "
			"ip=[%s], headers=%lu, body=%lu, rcvd=%lu, sent=%lu, time=%s%s\n",
			type,
			a, getenv("TCPREMOTEIP"), header_count, body_count, bytes_received_count, bytes_sent_count,
			buf, tls);
	else
		fprintf(stderr, "DEBUG: Disconnected, ip=[%s], time=%s%s\n",
			getenv("TCPREMOTEIP"),
			buf, tls);
}

void disconnected()
{
	bye_msg("INFO: DISCONNECTED");
	bye();
}

static void disconnected_timeout(void)
{
	writes("* BYE Disconnected for inactivity.\r\n");
	writeflush();
	bye_msg("INFO: TIMEOUT");
	bye();
}

int doidle(time_t idletimeout, int extraFd)
{
fd_set fds;
struct timeval tv;
time_t t;

       time(&t);
       if (t >= readtimeout)   disconnected_timeout();
       if (imap_readptrleft > 0) return 1;

       FD_ZERO(&fds);
       FD_SET(0, &fds);

       if (extraFd > 0)
       {
	       FD_SET(extraFd, &fds);
       }
       else
       {
	       extraFd=0;
       }

       tv.tv_sec=idletimeout;
       tv.tv_usec=0;

       select(extraFd + 1, &fds, 0, 0, &tv);
       return (FD_ISSET(0, &fds));
}

size_t doread(char *buf, size_t bufsiz)
{
fd_set	fds;
struct	timeval	tv;
time_t	t;
int n = 0;

	time(&t);
	if (t >= readtimeout)	disconnected_timeout();

	FD_ZERO(&fds);
	FD_SET(0, &fds);
	tv.tv_sec=readtimeout - t;
	tv.tv_usec=0;

	if (select(1, &fds, 0, 0, &tv) <= 0)
	{
		disconnected_timeout();
		return (0);
	}
	if (!FD_ISSET(0, &fds) || (n=read(0, buf, bufsiz)) <= 0)
	{
		if ( n > 0 )
			bytes_received_count += n; /* count received bytes */
		disconnected();
		return (0);
	}
	if ( n > 0 )
		bytes_received_count += n; /* count received bytes */
	return (n);
}

void readflush()
{
	imap_readptrleft=0;
}

void readfill()
{
	imap_readptrleft=doread(readbuf, sizeof(readbuf));
	imap_readptr=readbuf;
}

#define	UNREAD(c) (*--imap_readptr=(c), ++imap_readptrleft)

void unread(int c)
{
	UNREAD(c);
}

void read_eol()
{
int	c;

	while ( (c=READ()) != '\n')
		;
	curtoken.tokentype=IT_EOL;
}

void read_timeout(time_t t)
{
time_t	tt;

	time(&tt);
	readtimeout=tt+t;
}

static void alloc_tokenbuf(unsigned l)
{
	if (l >= curtoken.tokenbuf_size)
	{
	char	*p=curtoken.tokenbuf ? realloc(curtoken.tokenbuf, l + 256):
			malloc(l + 256);

		if (!p)
			write_error_exit("malloc");

		curtoken.tokenbuf_size = l+256;
		curtoken.tokenbuf=p;
	}
}

static char LPAREN_CHAR='(';
static char RPAREN_CHAR=')';
static char LBRACKET_CHAR='[';
static char RBRACKET_CHAR=']';

void ignorepunct()
{
	LPAREN_CHAR=RPAREN_CHAR=LBRACKET_CHAR=RBRACKET_CHAR='\n';
}

#if SMAP

void smap_readline(char *buffer, size_t bufsize)
{
	int c;

	while ((c=READ()) != '\n')
	{
		if (bufsize > 1)
		{
			*buffer++ = c;
			--bufsize;
		}
	}
	*buffer=0;
}

#endif

static int ignore_output_func(const char *ptr, size_t cnt, void *ignore)
{
	return 0;
}

static struct imaptoken *do_readtoken_nolog(int touc);

static struct imaptoken *do_readtoken(int touc)
{
	struct imaptoken *tok=do_readtoken_nolog(touc);

	if (debugfile)
	{
		char	*p=0;

		fprintf(debugfile, "READ: ");
		switch (tok->tokentype) {
		case IT_ATOM:
			p=curtoken.tokenbuf; fprintf(debugfile, "ATOM"); break;
		case IT_NUMBER:
			p=curtoken.tokenbuf; fprintf(debugfile, "NUMBER"); break;
		case IT_QUOTED_STRING:
			p=curtoken.tokenbuf; fprintf(debugfile, "QUOTED_STRING"); break;
		case IT_LPAREN:
			fprintf(debugfile, "LPAREN"); break;
		case IT_RPAREN:
			fprintf(debugfile, "RPAREN"); break;
		case IT_NIL:
			fprintf(debugfile, "NIL"); break;
		case IT_ERROR:
			fprintf(debugfile, "ERROR"); break;
		case IT_EOL:
			fprintf(debugfile, "EOL"); break;
		case IT_LBRACKET:
			fprintf(debugfile, "LBRACKET"); break;
		case IT_RBRACKET:
			fprintf(debugfile, "RBRACKET"); break;
		case IT_LITERAL_STRING_START:
			fprintf(debugfile, "{%lu}", tok->tokennum); break;
		case IT_LITERAL8_STRING_START:
			fprintf(debugfile, "~{%lu}", tok->tokennum); break;
		}

		if (p)
			fprintf(debugfile, ": %s", p);
		fprintf(debugfile, "\n");
		fflush(debugfile);
	}
	return tok;
}

static struct imaptoken *do_readtoken_nolog(int touc)
{
int	c=0;
unsigned l;
static unsigned long max_atom_size = 0;
char *ptr;

#define	appendch(c)	alloc_tokenbuf(l+1); curtoken.tokenbuf[l++]=(c);

	if (curtoken.tokentype == IT_ERROR)	return (&curtoken);

	do
	{
		c=READ();
	} while (c == '\r' || c == ' ' || c == '\t');

	if (c == '\n')
	{
		UNREAD(c);
		curtoken.tokentype=IT_EOL;
		return (&curtoken);
	}
	c=(unsigned char)c;
	if (c == LPAREN_CHAR)
	{
		curtoken.tokentype=IT_LPAREN;
		return (&curtoken);
	}

	if (c == RPAREN_CHAR)
	{
		curtoken.tokentype=IT_RPAREN;
		return (&curtoken);
	}

	if (c == LBRACKET_CHAR)
	{
		curtoken.tokentype=IT_LBRACKET;
		return (&curtoken);
	}

	if (c == RBRACKET_CHAR)
	{
		curtoken.tokentype=IT_RBRACKET;
		return (&curtoken);
	}

	if (c == '"')
	{
		int bit8=0;

		l=0;
		while ((c=READ()) != '"')
		{
			if (c == '\\')
				c=READ();
			if (c == '\r' || c == '\n')
			{
				UNREAD(c);
				curtoken.tokentype=IT_ERROR;
				return (&curtoken);
			}
			if (l >= BUFSIZ)
			{
				writes("* NO [ALERT] IMAP command too long.\r\n");
				curtoken.tokentype=IT_ERROR;
			}
			if (c & 0x80)
				bit8=1;
			appendch(c);
		}
		appendch(0);
		curtoken.tokentype=IT_QUOTED_STRING;

		/*
		 * Strings must be valid UTF-8. Shortcut check.
		 */

		if (bit8)
		{
			int errptr=0;

			unicode_convert_handle_t h=
				unicode_convert_init("utf-8",
						     unicode_u_ucs4_native,
						     ignore_output_func,
						     (void *)0);

			if (unicode_convert(h, curtoken.tokenbuf,
					    strlen(curtoken.tokenbuf)))
			{
				curtoken.tokentype=IT_ERROR;
			}

			if (unicode_convert_deinit(h, &errptr) || errptr)
			{
				curtoken.tokentype=IT_ERROR;
			}
		}
		return (&curtoken);
	}

	/*
	 * Parse LITERAL or LITERAL8
	 */
	curtoken.tokentype=IT_LITERAL_STRING_START;

	if (c == '~')
	{
		c=READ();
		if (c != '{')
		{
			curtoken.tokentype=IT_ERROR;
			return (&curtoken);
		}
		curtoken.tokentype=IT_LITERAL8_STRING_START;
	}

	if (c == '{')
	{
		curtoken.tokennum=0;
		while ((c=READ()) != '}')
		{
			if (!isdigit((int)(unsigned char)c))
			{
				UNREAD(c);
				curtoken.tokentype=IT_ERROR;
				return (&curtoken);
			}
			curtoken.tokennum = curtoken.tokennum*10 + (c-'0');
		}
		c=READ();
		if (c == '\r')
		{
			c=READ();
		}
		if (c != '\n')
		{
			curtoken.tokentype=IT_ERROR;
			return (&curtoken);
		}
		return (&curtoken);
	}

	l=0;
	if (c == '\\')
	{
		appendch(c);	/* Message flag */
		c=READ();
	}
	else if (isdigit(c))
	{
		curtoken.tokentype=IT_NUMBER;
		curtoken.tokennum=0;
		do
		{
			appendch(c);
			curtoken.tokennum = curtoken.tokennum*10 +
				(c-'0');
			c=READ();
		} while (isdigit( (int)(unsigned char)c));

		/* Could be stuff like mime.spec, so continue reading. */
	}

	while (c != '\r' && c != '\n'
	       && !isspace((int)(unsigned char)c)
	       && (((unsigned char)c) & 0x80) == 0
	       && c != '\\' && c != '"' && c != LPAREN_CHAR && c != RPAREN_CHAR
	       && c != '~'
	       && c != '{' && c != '}' && c != LBRACKET_CHAR && c != RBRACKET_CHAR)
	{
		curtoken.tokentype=IT_ATOM;
		max_atom_size = (ptr = getenv("MAX_ATOM_SIZE")) ? strtoull(ptr, 0, 0) : IT_MAX_ATOM_SIZE;
		if (l < max_atom_size)
		{
			if (touc)
				c=toupper(c);
			appendch(c);
		}
		else
		{
			write_error_exit("max atom size too small");
		}
		c=READ();
	}
	if (l == 0)
	{
		curtoken.tokentype=IT_ERROR;
		return (&curtoken);
	}
	appendch(0);
	UNREAD(c);

	if (strcmp(curtoken.tokenbuf, "NIL") == 0)
		curtoken.tokentype=IT_NIL;
	return (&curtoken);
}

static struct imaptoken *readtoken(int touc)
{
	struct imaptoken *tok=do_readtoken(touc);

	convert_literal_tokens(tok);
	return (tok);
}

/*
 * If a token is a LITERAL, read it and replace it with a QUOTED_STRING.
 */

void convert_literal_tokens(struct imaptoken *tok)
{
	unsigned long nbytes;
	if (tok->tokentype != IT_LITERAL_STRING_START &&
	    tok->tokentype != IT_LITERAL8_STRING_START)
		return;

	nbytes=curtoken.tokennum;

	if (nbytes > BUFSIZ)
	{
		writes("* NO [ALERT] IMAP command too long.\r\n");
		tok->tokentype=IT_ERROR;
	}
	else
	{
		unsigned long i;

		/*
		  RFC4466: In addition, the non-terminal "literal8" defined
		  in [BINARY] got extended to allow for non-synchronizing
		  literals if both [BINARY] and [LITERAL+] extensions are
		  supported by the server.

		*/

		writes("+ OK\r\n");
		writeflush();
		alloc_tokenbuf(nbytes+1);
		for (i=0; i<nbytes; i++)
			tok->tokenbuf[i]= READ();
		tok->tokenbuf[i]=0;
		tok->tokentype=IT_QUOTED_STRING;
	}
}

struct imaptoken *nexttoken(void)
{
	return (readtoken(1));
}

struct imaptoken *nexttoken_nouc(void)
{
	return (readtoken(0));
}

/* RFC 2060 sucks */

struct imaptoken *nexttoken_okbracket(void)
{
	struct imaptoken *t;

	LBRACKET_CHAR=RBRACKET_CHAR='\n';

	t=nexttoken();

	LBRACKET_CHAR='[';
	RBRACKET_CHAR=']';
	return (t);
}

struct imaptoken *nexttoken_nouc_okbracket(void)
{
	struct imaptoken *t;

	LBRACKET_CHAR=RBRACKET_CHAR='\n';

	t=nexttoken_nouc();

	LBRACKET_CHAR='[';
	RBRACKET_CHAR=']';
	return (t);
}

struct imaptoken *currenttoken(void)
{
	return (&curtoken);
}

struct imaptoken *nexttoken_noparseliteral(void)
{
	return (do_readtoken(0));
}

/* Read an IMAP literal string (or a portion of) */

void read_string(char **ptr, unsigned long *left, unsigned long cnt)
{
	char *cp;

	if (imap_readptrleft == 0)
	{
		/* Keep reading until we fill the buffer or until we've
		** read the entire string.
		*/
		if (!(cp = getenv("SOCKET_TIMEOUT")))
			read_timeout(SOCKET_TIMEOUT);
		else
			read_timeout(atoi(cp));
		imap_readptr=readbuf;
		while (imap_readptrleft < sizeof(readbuf) && imap_readptrleft < cnt)
			imap_readptrleft += doread(readbuf+imap_readptrleft,
						sizeof(readbuf)-imap_readptrleft);
	}

	if (cnt < imap_readptrleft)	/* Can satisfy fully from buffer */
	{
		*ptr=imap_readptr;
		*left=cnt;
		imap_readptr += cnt;
		imap_readptrleft -= cnt;
		return;
	}

	*ptr=imap_readptr;
	*left=imap_readptrleft;
	imap_readptrleft=0;
	return;
}

char *my_strdup(const char *s)
{
char	*q=strdup(s);

	if (!q)	write_error_exit("malloc");
	return (q);
}

int ismsgset(struct imaptoken *tok)
	 /* See if this token is a syntactically valid message set */
{

	if (tok->tokentype == IT_NUMBER)	return (1);
	if (tok->tokentype != IT_ATOM)		return (0);

	return ismsgset_str(tok->tokenbuf);
}

int ismsgset_str(const char *p)
{
	while (isdigit((int)(unsigned char)*p) || *p == '*')
	{
		if (*p == '0')	return (0);
		if (*p == '*')
			++p;
		else
			do
			{
				++p;
			} while (isdigit((int)(unsigned char)*p));

		if (*p == ':')
		{
			++p;
			if (!isdigit((int)(unsigned char)*p) &&
				*p != '*')
				return (0);
			if (*p == '0')	return (0);
			if (*p == '*')
				++p;
			else
				do
				{
					++p;
				} while (isdigit((int)(unsigned char)*p));
		}
		if (*p != ',')	break;
		++p;
	}
	if (*p)	return (0);
	return (1);
}
		
