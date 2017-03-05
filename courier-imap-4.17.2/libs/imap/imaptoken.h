#ifndef	imaptoken_h
#define	imaptoken_h

/*
** Copyright 1998 - 2003 Double Precision, Inc.
** See COPYING for distribution information.
*/

#include	<stdio.h>
#include	<stdlib.h>
#include	<time.h>


struct imaptoken {
	short	tokentype;
	unsigned long tokennum;
	char *tokenbuf;
	size_t	tokenbuf_size;
	} ;

#define	IT_ATOM			0
#define	IT_NUMBER		1
#define	IT_QUOTED_STRING	2
#define	IT_LITERAL_STRING_START	3
#define	IT_LPAREN		4
#define	IT_RPAREN		5
#define	IT_NIL			6
#define	IT_ERROR		7
#define	IT_EOL			8
#define	IT_LBRACKET		9
#define	IT_RBRACKET		10

struct imaptoken *nexttoken(void);
struct imaptoken *currenttoken(void);
struct imaptoken *nexttoken_nouc(void);
struct imaptoken *nexttoken_noparseliteral(void);
struct imaptoken *nexttoken_okbracket(void);
struct imaptoken *nexttoken_nouc_okbracket(void);

int ismsgset(struct imaptoken *);
	/* See if this token is a syntactically valid message set */
int ismsgset_str(const char *);
	/* Ditto */

void read_timeout(time_t);
void read_eol();
void unread(int);

extern size_t doread(char *buf, size_t bufsiz);
extern char *imap_readptr;
extern size_t imap_readptrleft;
extern void readfill();
extern void readflush();

#define	READ()	( imap_readptrleft ? \
	(--imap_readptrleft, (int)(unsigned char)*imap_readptr++): \
	(readfill(), --imap_readptrleft, (int)(unsigned char)*imap_readptr++))

void read_string(char **, unsigned long *, unsigned long);

/* Flags */

struct imapflags {
	char	seen;
	char	answered;
	char	deleted;
	char	flagged;
	char	drafts;
	char	recent;
	} ;

struct imapkeywords {
	struct imapflags flags;

	struct imapkeyword *first_keyword, *last_keyword;
};

/* ATOMS have the following maximum length */

#define	IT_MAX_ATOM_SIZE	16384

char *my_strdup(const char *s);


/* SMAP */

void smap_readline(char *buffer, size_t bufsize);

#endif
