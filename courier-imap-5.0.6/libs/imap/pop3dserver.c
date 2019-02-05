/*
** Copyright 1998 - 2010 Double Precision, Inc.
** See COPYING for distribution information.
**
** $Id: pop3dserver.c,v 1.46 2010/07/11 13:36:23 mrsam Exp $
*/

#if	HAVE_CONFIG_H
#include	"config.h"
#endif
#include	<sys/types.h>
#if HAVE_DIRENT_H
#include <dirent.h>
#define NAMLEN(dirent) strlen((dirent)->d_name)
#else
#define dirent direct
#define NAMLEN(dirent) (dirent)->d_namlen
#if HAVE_SYS_NDIR_H
#include <sys/ndir.h>
#endif
#if HAVE_SYS_DIR_H
#include <sys/dir.h>
#endif
#if HAVE_NDIR_H
#include <ndir.h>
#endif
#endif
#include	<sys/types.h>
#if	HAVE_SYS_STAT_H
#include	<sys/stat.h>
#endif
#if TIME_WITH_SYS_TIME
#include <sys/time.h>
#include <time.h>
#else
#if HAVE_SYS_TIME_H
#include <sys/time.h>
#else
#include <time.h>
#endif
#endif
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#if	HAVE_UNISTD_H
#include	<unistd.h>
#endif
#include	<signal.h>
#include	<errno.h>
#include	"authlib/auth.h"
#include	"authlib/authmod.h"
#include	"numlib/numlib.h"
#include	"maildir/config.h"
#include	"maildir/maildirmisc.h"
#include	"maildir/maildirquota.h"
#include	"maildir/maildirgetquota.h"
#include	"maildir/maildircreate.h"
#include	"maildir/loginexec.h"
#include	"rfc2045/rfc2045.h"

#define POP3DLIST "courierpop3dsizelist"
#define LISTVERSION 3

extern void pop3dcapa();
static void acctout(const char *disc);
void rfc2045_error(const char *p)
{
	if (write(2, p, strlen(p)) < 0)
		_exit(1);
	_exit(0);
}

static const char *authaddr, *remoteip, *remoteport;

struct msglist {
	struct msglist *next;
	char *filename;
	int isdeleted;
	int isnew;
	int isutf8;
	off_t size;

	struct {
		unsigned long uidv;
		unsigned long n;
	} uid;
};

static struct msglist *msglist_l;
static struct msglist **msglist_a;
static unsigned msglist_cnt;

static struct stat enomem_stat;
static int enomem_1msg;
int utf8_enabled;
int savesizes=0;

/*
** When a disk error occurs while saving an updated courierpop3dsizelist
** file, proceed to recover as follows:
**
** If there's at least one existing message that's found in the old
** courierpop3dsizelist, then ignore all new messages that were not found
** in the old courierpop3dsizelist, and a new uid/uidv was assigned to them.
** Therefore, the client doesn't see any new messages.  Hopefully, after
** at least one existing message gets deleted, there'll be room to create
** a new courierpop3dsizelist file next time, and record new messages.
**
** If none of the messages in the maildir could be found in the
** courierpop3dsizelist, take the first message in the maildir only, and
** use a UIDL that's derived from the message's dev_t/ino_t.  The client
** will see that message only.  After deleting it, hopefully a new
** courierpop3dsizelist file could be written out next time.
*/


static unsigned long top_count=0;
static unsigned long retr_count=0;

static unsigned long bytes_sent_count=0;
static unsigned long bytes_received_count=0;

static unsigned long uidv=0;

static time_t start_time;

/*
** The RFC is pretty strict in stating that octet size must count the CR
** in the CRLF endofline.
*/

static void calcsize(struct msglist *m)
{
	FILE	*f=fopen(m->filename, "r");
	struct rfc2045 *p=rfc2045_fromfp(f);

	m->size=p->nlines + p->endpos;

	clearerr(f);
	if (m->size > 0 && fseek(f, -1, SEEK_SET) == 0 && getc(f) != '\n')
		m->size+=2; /* We'll add an extra CRLF ourselves */

	m->isutf8=0;
	if (p->rfcviolation & RFC2045_ERR8BITHEADER)
		m->isutf8=1;
	rfc2045_free(p);
	fclose(f);
}

static FILE *
openpop3dlist()
{
	int tries;
	FILE *fp;

	tries = 0;
	do {
		fp = fopen(POP3DLIST, "r");
		if (fp != NULL)
			return (fp);
		if (errno != ESTALE) {
			if (errno != ENOENT)
				perror("failed to open " POP3DLIST " file");
			return (NULL);
		}
		++tries;
	} while (tries < 3); /* somewhat arbitrary */
	fprintf(stderr, "failed to open pop3dlist file after retries\n");
	return NULL;
}

/*
** Read courierpop3dsizelist
*/

static int cmpmsgs(const void *a, const void *b);

static struct msglist **readpop3dlist(unsigned long *uid)
{
	struct msglist **a;
	struct msglist *list=NULL;
	size_t mcnt=0;
	char linebuf[2048];
	FILE *fp=openpop3dlist();
	size_t i;
	int vernum=0;
	unsigned long size;

	uidv=time(NULL);

	if (fp == NULL ||
	    fgets(linebuf, sizeof(linebuf)-1, fp) == NULL ||
	    linebuf[0] != '/' || sscanf(linebuf+1, "%d %lu %lu", &vernum,
					uid, &uidv)
	    < 2 || (vernum != LISTVERSION &&
		    vernum != (LISTVERSION-1)))
	{
		if (fp)
			fclose(fp);
		fp=NULL;
	}
	else
	{
		if (vernum != LISTVERSION)
			savesizes=1;
	}

	if (fp)
	{
		struct msglist *m;

		char *p, *q;

		size_t n=0;
		int ch;

		while ((ch=getc(fp)) != EOF)
		{
			if (ch != '\n')
			{
				if (n < sizeof(linebuf)-20)
					linebuf[n++]=ch;
				continue;
			}
			linebuf[n]=0;
			n=0;

			if ((p=strrchr(linebuf, ' ')) == NULL)
				continue;
			*p=0;
			if ((q=strrchr(linebuf, ' ')) == NULL)
				continue;
			*p=' ';
			p=q;
			*p++=0;

			if (linebuf[0] == 0)
				continue;

			if ((m=(struct msglist *)malloc(sizeof(struct
							       msglist))) == 0)
			{
				perror("malloc");
				exit(1);
			}

			if ((m->filename=strdup(linebuf)) == NULL)
			{
				perror("malloc");
				exit(1);
			}

			// Converting from LISTVERSION 2, assuming no UTF-8.
			// We have extra room at the end.
			strcat(p, ":0");

			if (sscanf(p, "%lu %lu:%lu:%d", &size,
				   &m->uid.n, &m->uid.uidv,
				   &m->isutf8) == 4)
			{
				m->size=size;
				m->next=list;
				list=m;
				++mcnt;
			} else
			{
				free(m->filename);
				free(m);
			}
		}
		fclose(fp);
	}
	if ((a=(struct msglist **)malloc((mcnt+1)
					 *sizeof(struct msglist *))) == 0)
	{
		perror("malloc");
		exit(1);
	}

	for (i=0; list; list=list->next)
		a[i++]=list;

	a[i]=NULL;
	qsort(a, i, sizeof(*a), cmpmsgs);

	return a;
}

static int savepop3dlist(struct msglist **a, size_t cnt,
			  unsigned long uid)
{
	FILE *fp;
	size_t i;

	struct maildir_tmpcreate_info createInfo;

	maildir_tmpcreate_init(&createInfo);

	createInfo.uniq="pop3";
	createInfo.doordie=1;

	if ((fp=maildir_tmpcreate_fp(&createInfo)) == NULL)
	{
		maildir_tmpcreate_free(&createInfo);
		return -1;
	}

	fprintf(fp, "/%d %lu %lu\n", LISTVERSION, uid, uidv);

	for (i=0; i<cnt; i++)
	{
		char *p=a[i]->filename;
		char *q;

		if ((q=strrchr(p, '/')) != NULL)
			p=q+1;

		fprintf(fp, "%s %lu %lu:%lu:%d\n", p, (unsigned long)a[i]->size,
			a[i]->uid.n, a[i]->uid.uidv, a[i]->isutf8);
	}

	if (fflush(fp) || ferror(fp))
	{
		fclose(fp);
		unlink(createInfo.tmpname);
		maildir_tmpcreate_free(&createInfo);
		return -1;
	}

	if (fclose(fp) ||
	    rename(createInfo.tmpname, POP3DLIST) < 0)
	{
		unlink(createInfo.tmpname);
		maildir_tmpcreate_free(&createInfo);
		return -1;
	}

	maildir_tmpcreate_free(&createInfo);
	return 0;
}

/* Scan cur, and pick up all messages contained therein */

static int scancur()
{
DIR	*dirp;
struct	dirent *de;
struct	msglist *m;

	if ((dirp=opendir("cur")) == 0)
	{
		perror("scancur opendir(\"cur\")");
		return 1;
	}

	while ((de=readdir(dirp)) != 0)
	{
		if ( de->d_name[0] == '.' )	continue;

		if ((m=(struct msglist *)malloc(sizeof(struct msglist))) == 0)
		{
			perror("malloc");
			exit(1);
		}
		if ((m->filename=(char *)malloc(strlen(de->d_name)+5)) == 0)
		{
			free( (char *)m);
			perror("malloc");
			exit(1);
		}
		strcat(strcpy(m->filename, "cur/"), de->d_name);
		m->isdeleted=0;
		m->next=msglist_l;
		msglist_l=m;
		msglist_cnt++;
	}
	closedir(dirp);
	return 0;
}

/*
** When sorting messages, sort on the arrival date - the first part of the
** name of the file in the maildir is the timestamp.
*/

static int cmpmsgs(const void *a, const void *b)
{
	const char *aname=(*(struct msglist **)a)->filename;
	const char *bname=(*(struct msglist **)b)->filename;
	const char *ap=strrchr(aname, '/');
	const char *bp=strrchr(bname, '/');
	long	na, nb;

	if (ap)
		++ap;
	else
		ap=aname;

	if (bp)
		++bp;
	else
		bp=bname;

	na=atol(ap);
	nb=atol(bp);

	if (na < nb)	return (-1);
	if (na > nb)	return (1);

	while (*ap || *bp)
	{
		if (*ap == MDIRSEP[0] && *bp == MDIRSEP[0])
			break;

		if (*ap < *bp)
			return -1;
		if (*ap > *bp)
			return 1;
		++ap;
		++bp;
	}

	return 0;
}

static void sortmsgs()
{
	size_t i, n;
	struct msglist *m;
	struct msglist **prev_list;

	unsigned long nextuid;

	if (msglist_cnt == 0)	return;

	if ((msglist_a=(struct msglist **)malloc(
			msglist_cnt*sizeof(struct msglist *))) == 0)
	{
		perror("malloc");
		msglist_cnt=0;
		return;
	}

	for (i=0, m=msglist_l; m; m=m->next, i++)
	{
		m->isnew=0;
		msglist_a[i]=m;
	}
	qsort(msglist_a, msglist_cnt, sizeof(*msglist_a), cmpmsgs);

	nextuid=1;

	prev_list=readpop3dlist(&nextuid);

	n=0;

	for (i=0; i<msglist_cnt; i++)
	{
		while (prev_list[n] &&
		       cmpmsgs(&prev_list[n], &msglist_a[i]) < 0)
		{
			++n;
			savesizes=1;
		}

		if (prev_list[n] &&
		    cmpmsgs(&prev_list[n], &msglist_a[i]) == 0)
		{
			msglist_a[i]->size=prev_list[n]->size;
			msglist_a[i]->uid=prev_list[n]->uid;
			msglist_a[i]->isutf8=prev_list[n]->isutf8;
			n++;
		}
		else
		{
			msglist_a[i]->uid.n=nextuid++;
			msglist_a[i]->uid.uidv=uidv;
			msglist_a[i]->isnew=1;

			calcsize(msglist_a[i]);
			savesizes=1;
		}
	}

	if (prev_list[n])
		savesizes=1;

	for (i=0; prev_list[i]; i++)
	{
		free(prev_list[i]->filename);
		free(prev_list[i]);
	}

	free(prev_list);

	if (savesizes && savepop3dlist(msglist_a, msglist_cnt, nextuid) < 0)
	{
		fprintf(stderr, "ERR: Error while saving courierpop3dsizelist"
			", user=%s\n", authaddr);

		for (i=n=0; i<msglist_cnt; i++)
		{
			if (msglist_a[i]->isnew)
				continue;

			msglist_a[n]=msglist_a[i];
			++n;
		}

		if (n == 0 && n < msglist_cnt &&
		    stat(msglist_a[0]->filename, &enomem_stat) == 0)
		{
			enomem_1msg=1;
			++n;
		}
		msglist_cnt=n;

	}
}

static void mkupper(char *p)
{
	while (*p)
	{
		*p=toupper(*p);
		p++;
	}
}

static void cleanup()
{
	unsigned i;
	const char *cp=getenv("POP3_LOG_DELETIONS");
	int log_deletions= cp && *cp != '0';

	int64_t deleted_bytes=0;
	int64_t deleted_messages=0;

	for (i=0; i<msglist_cnt; i++)
		if (msglist_a[i]->isdeleted)
		{
			unsigned long un=0;

			const char *filename=msglist_a[i]->filename;

			if (maildirquota_countfile(filename))
			{
				if (maildir_parsequota(filename, &un))
				{
					struct stat stat_buf;

					if (stat(filename, &stat_buf) == 0)
						un=stat_buf.st_size;
				}
			}

			if (log_deletions)
				fprintf(stderr, "INFO: DELETED, user=%s, ip=[%s], filename=%s\n",
					getenv("AUTHENTICATED"),
					getenv("TCPREMOTEIP"),
					msglist_a[i]->filename);

			if (unlink(msglist_a[i]->filename))
				un=0;

			if (un)
			{
				deleted_bytes -= un;
				deleted_messages -= 1;
			}
		}

	if (deleted_messages < 0)
		maildir_quota_deleted(".", deleted_bytes, deleted_messages);

	return;
}

#define printed(c) do {int cnt=(c); if (cnt > 0)			\
					   bytes_sent_count += cnt;	\
	} while(0)

#define printchar(ch) do { putchar((ch)); printed(1); } while(0);

/* POP3 STAT */

static void do_stat()
{
off_t	n=0;
unsigned i, j;
char	buf[NUMBUFSIZE];

	j=0;
	for (i=0; i<msglist_cnt; i++)
	{
		if (msglist_a[i]->isdeleted)	continue;
		n += msglist_a[i]->size;
		++j;
	}

	printed(printf("+OK %u %s\r\n", j, libmail_str_off_t(n, buf)));
	fflush(stdout);
}

static unsigned getmsgnum(const char *p)
{
unsigned i;

	if (!p || (i=atoi(p)) > msglist_cnt || i == 0 ||
		msglist_a[i-1]->isdeleted)
	{
		printed(printf("-ERR Invalid message number.\r\n"));
		fflush(stdout);
		return (0);
	}
	return (i);
}

/* POP3 LIST */

static void do_list(const char *msgnum)
{
unsigned i;
char	buf[NUMBUFSIZE];

	if (msgnum)
	{
		if ((i=getmsgnum(msgnum)) != 0)
		{
			printed(printf("+OK %u %s\r\n", i,
				       libmail_str_off_t(msglist_a[i-1]->size,
							 buf)));
			fflush(stdout);
		}
		return;
	}

	printed(printf("+OK POP3 clients that break here, they violate STD53.\r\n"));
	for (i=0; i<msglist_cnt; i++)
	{
		if (msglist_a[i]->isdeleted)	continue;
		printed(printf("%u %s\r\n", i+1, libmail_str_off_t(msglist_a[i]->size, buf)));
	}
	printed(printf(".\r\n"));
	fflush(stdout);
}

/* RETR and TOP POP3 commands */

#define MIMEWRAPTXT							\
	"From: Mail Delivery Subsystem <postmaster>\n"			\
	"Subject: Cannot display Unicode content\n"			\
	"Mime-Version: 1.0\n"						\
	"Content-Type: multipart/mixed; boundary=\"%s\"\n"		\
	"\n\n\n--%s\n"							\
	"Content-Type: text/plain\n\n"					\
	"This E-mail message was determined to be Unicode-formatted\n"	\
	"but your E-mail reader does not support Unicode E-mail.\n\n"	\
	"Please use an E-mail reader that supports POP3 with UTF-8\n"	\
	"(see https://tools.ietf.org/html/rfc6856.html).\n\n"		\
	"This can also happen when the sender's E-mail program does not\n" \
	"correctly format the sent message.\n\n"			\
	"The original message is included as a separate attachment\n"	\
	"so that it can be downloaded manually.\n\n"			\
	"--%s\n"							\
	"Content-Type: message/global\n\n"

struct retr_source {
	char *wrapheader;
	FILE *f;
	char *wrapfooter;
};

static int get_retr_source(struct retr_source *s)
{
	int c=(unsigned char)*s->wrapheader;

	if (c)
	{
		++s->wrapheader;
		return c;
	}

	if (s->f)
	{
		int c=getc(s->f);

		if (c >= 0)
			return c;
		s->f=0;
	}

	if (*s->wrapfooter)
	{
		int c=(unsigned char)*s->wrapfooter;

		++s->wrapfooter;
		return c;
	}
	return -1;
}

static void do_retr(unsigned i, unsigned *lptr)
{
	FILE	*f;
	char	*p;
	int	c, lastc='\n';
	int	inheader=1;
	char	buf[NUMBUFSIZE];
	unsigned long *cntr;
	char boundary[256];
	char wrapheader[sizeof(MIMEWRAPTXT)+768];
	char wrapfooter[512];

	struct retr_source rs;

	wrapheader[0]=0;
	wrapfooter[0]=0;

	p = getenv("ENABLE_UTF8_COMPLIANCE");
	if ((p && *p) && msglist_a[i]->isutf8 && !utf8_enabled)
	{
		sprintf(boundary, "=_%d-%d", (int)getpid(), (int)time(NULL));

		sprintf(wrapheader, MIMEWRAPTXT, boundary, boundary, boundary);
		sprintf(wrapfooter, "\n--%s--\n", boundary);
	}

	f=fopen(msglist_a[i]->filename, "r");

	if (!f)
	{
		printed(printf("-ERR Can't open the message file - it's gone!\r\n"));
		fflush(stdout);
		return;
	}
	printed(printf( (lptr ? "+OK headers follow.\r\n":"+OK %s octets follow.\r\n"),
			libmail_str_off_t(msglist_a[i]->size +
					  strlen(wrapheader) +
					  strlen(wrapfooter),
					  buf)));

	cntr= &retr_count;
	if (lptr)
		cntr= &top_count;

	rs.wrapheader=wrapheader;
	rs.f=f;
	rs.wrapfooter=wrapfooter;

	for (lastc=0; (c=get_retr_source(&rs)) >= 0; lastc=c)
	{
		if (lastc == '\n')
		{
			if (lptr)
			{
				if (inheader)
				{
					if (c == '\n')	inheader=0;
				}
				else if ( (*lptr)-- == 0)	break;
			}

			if (c == '.')
				printchar('.');
		}
		if (c == '\n')	printchar('\r');
		printchar(c);
		++*cntr;
	}
	if (ferror(f)) {
		/* Oops! All we can do is drop the connection */
		fprintf(stderr, "ERR: I/O error while reading message file %s: %s\n",
			msglist_a[i]->filename, strerror(errno));
		acctout("INFO: I/O error disconnect");
		exit(1);
	}
	if (lastc != '\n')	printed(printf("\r\n"));
	printed(printf(".\r\n"));
	fflush(stdout);
	fclose(f);
	if (lptr)	return;

	if ((p=strchr(msglist_a[i]->filename, MDIRSEP[0])) != 0 &&
		(p[1] != '2' || p[2] != ',' || strchr(p, 'S') != 0))
		return;

	if ((p=malloc(strlen(msglist_a[i]->filename)+5)) == 0)
		return;

	strcpy(p, msglist_a[i]->filename);
	if (strchr(p, MDIRSEP[0]) == 0)	strcat(p, MDIRSEP "2,");
	strcat(p, "S");

	if (lptr	/* Don't mark as seen for TOP */
	    || rename(msglist_a[i]->filename, p))
	{
		free(p);
		return;
	}
	free(msglist_a[i]->filename);
	msglist_a[i]->filename=p;
}

/*
** The UIDL of the message is really just its filename, up to the first MDIRSEP character
*/

static void print_uidl(unsigned i)
{
	const char *p;

	if (enomem_1msg)
		/* Error recovery - out of disk space, see comments
		** at the beginning of this file.
		*/
	{
		char dev_buf[NUMBUFSIZE];
		char ino_buf[NUMBUFSIZE];
		char mtime_buf[NUMBUFSIZE];

		printed(printf("ENOMEM-%s-%s-%s\r\n",
			       libmail_strh_time_t(enomem_stat.st_mtime, mtime_buf),
			       libmail_strh_dev_t(enomem_stat.st_dev, dev_buf),
			       libmail_strh_ino_t(enomem_stat.st_ino, ino_buf))
			);
		return;
	}

	if (msglist_a[i]->uid.n != 0)
	{
		/* VERSION 1 and VERSION 2 UIDL */

		printed(printf((msglist_a[i]->uid.uidv ?
				"UID%lu-%lu\r\n":"UID%lu\r\n"),
			       msglist_a[i]->uid.n, msglist_a[i]->uid.uidv));
		return;
	}

	/* VERSION 0 UIDL */

	p=strchr(msglist_a[i]->filename, '/')+1;

	while (*p && *p != MDIRSEP[0])
	{
		if (*p < 0x21 || *p > 0x7E || *p == '\'' || *p == '"' ||
			*p == '+')
			printed(printf("+%02X", (int)(unsigned char)*p));
		else
			printchar(*p);
		++p;
	}
	printed(printf("\r\n"));
}

static void do_uidl(const char *msgnum)
{
unsigned i;

	if (msgnum)
	{
		if ((i=getmsgnum(msgnum)) != 0)
		{
			printed(printf("+OK %u ", i));
			print_uidl(i-1);
			fflush(stdout);
		}
		return;
	}
	printed(printf("+OK\r\n"));
	for (i=0; i<msglist_cnt; i++)
	{
		if (msglist_a[i]->isdeleted)	continue;
		printed(printf("%u ", i+1));
		print_uidl(i);
	}
	printed(printf(".\r\n"));
	fflush(stdout);
}

static void acctout(const char *disc)
{
	static const char msg2[]=", user=";
	static const char msg3[]=", ip=[";
	static const char msgport[]="], port=[";
	static const char msg4[]="], top=";
	static const char msg5[]=", retr=";
	static const char msg6[]=", time=";
	static const char msg7[]=", stls=1";
	static const char msgAR[]=", rcvd=";
	static const char msgAS[]=", sent=";

	char num1[NUMBUFSIZE];
	char num2[NUMBUFSIZE];
	char num3[NUMBUFSIZE];
	char numAR[NUMBUFSIZE];
	char numAS[NUMBUFSIZE];

	char *p;
	const char *q;

	libmail_str_size_t(top_count, num1);
	libmail_str_size_t(retr_count, num2);
	libmail_str_time_t(time(NULL)-start_time, num3);
	libmail_str_size_t(bytes_received_count, numAR);
	libmail_str_size_t(bytes_sent_count, numAS);

	p=malloc(strlen(authaddr)+strlen(remoteip)+strlen(remoteport)+strlen(disc)+
		 strlen(num1)+strlen(num2)+strlen(num3)+
		 strlen(numAR)+strlen(numAS)+200);	/* Should be enough */

	strcpy(p, disc);
	strcat(p, msg2);
	strcat(p, authaddr);
	strcat(p, msg3);
	strcat(p, remoteip);
	strcat(p, msgport);
	strcat(p, remoteport);
	strcat(p, msg4);
	strcat(p, num1);
	strcat(p, msg5);
	strcat(p, num2);
	strcat(p, msgAR);
	strcat(p, numAR);
	strcat(p, msgAS);
	strcat(p, numAS);
	strcat(p, msg6);
	strcat(p, num3);

	if ((q=getenv("POP3_TLS")) && atoi(q))
		strcat(p, msg7);

	strcat(p, "\n");
	if (write(2, p, strlen(p)) < 0)
		; /* make gcc shut up */
	free(p);
}

static RETSIGTYPE bye(int signum)
{
	acctout("INFO: TIMEOUT");
	exit(0);
#if	RETSIGTYPE != void
	return (0);
#endif
}

static void loop()
{
char	buf[BUFSIZ];
char	*p;
int	c;

	signal(SIGALRM, bye);
	while (alarm(300), fgets(buf, sizeof(buf), stdin))
	{
		bytes_received_count += strlen(buf);

		alarm(0);
		if ((p=strchr(buf, '\n')) != 0)
			*p=0;
		else while ((c=getc(stdin)) >= 0 && c != '\n')
			;
		p=strtok(buf, " \t\r");
		if (!p)	p="";

		mkupper(p);
		if (strcmp(p, "QUIT") == 0)
		{
			printed(printf("+OK Phir Kab Miloge?\r\n"));
			fflush(stdout);
			cleanup();
			acctout("INFO: LOGOUT");
			return;
		}

		if (strcmp(p, "STAT") == 0)
		{
			do_stat();
			continue;
		}

		if (strcmp(p, "LIST") == 0)
		{
			do_list(strtok(NULL, " \t\r"));
			continue;
		}

		if (strcmp(p, "RETR") == 0)
		{
		unsigned	i;

			if ((i=getmsgnum(strtok(NULL, " \t\r"))) == 0)
				continue;

			do_retr(i-1, 0);
			continue;
		}

		if (strcmp(p, "CAPA") == 0)
		{
			pop3dcapa();
			continue;
		}

		if (strcmp(p, "DELE") == 0)
		{
		unsigned	i;

			if ((i=getmsgnum(strtok(NULL, " \t\r"))) == 0)
				continue;

			msglist_a[i-1]->isdeleted=1;
			printed(printf("+OK Deleted.\r\n"));
			fflush(stdout);
			continue;
		}

		if (strcmp(p, "NOOP") == 0)
		{
			printed(printf("+OK NOOP.\r\n"));
			fflush(stdout);
			continue;
		}

		if (strcmp(p, "RSET") == 0)
		{
		unsigned i;

			for (i=0; i<msglist_cnt; i++)
				msglist_a[i]->isdeleted=0;
			printed(printf("+OK Resurrected.\r\n"));
			fflush(stdout);
			continue;
		}

		if (strcmp(p, "TOP") == 0)
		{
		unsigned	i, j;
		const	char *q;

			if ((i=getmsgnum(strtok(NULL, " \t\r"))) == 0)
				continue;

			q=strtok(NULL, " \t\r");

			if (!q)	goto error;

			j=atoi(q);
			do_retr(i-1, &j);
			continue;
		}

		if (strcmp(p, "UIDL") == 0)
		{
			do_uidl(strtok(NULL, " \t\r"));
			continue;
		}

		if (strcmp(p, "UTF8") == 0)
		{
			/* XXX workaround for MS Outlook */
			utf8_enabled=1;
			printed(printf("+OK UTF8 enabled\r\n"));
			fflush(stdout);
			continue;
		}

error:
		printed(printf("-ERR Invalid command.\r\n"));
		fflush(stdout);
	}
	acctout("INFO: DISCONNECTED");
}

/* Like every good Maildir reader, we purge the tmp subdirectory */

static void purgetmp()
{
DIR	*p=opendir("tmp");
time_t	t;
struct	dirent *de;
struct	stat	stat_buf;
char	*n;

	if (!p)	return;
	time (&t);
	t -= 48L * 60L * 60L;

	while ((de=readdir(p)) != 0)
	{
		if (de->d_name[0] == '.')	continue;
		n=malloc(strlen(de->d_name)+5);
		if (!n)	continue;
		strcat(strcpy(n, "tmp/"), de->d_name);
		if (stat(n, &stat_buf) == 0 && stat_buf.st_mtime < t)
			unlink(n);
		free(n);
	}
	closedir(p);
}


#include	<unistd.h>

int main(int argc, char **argv)
{
char   *p, *protocol;
uid_t  euid, uid;

#ifdef HAVE_SETVBUF_IOLBF
	setvbuf(stderr, NULL, _IOLBF, BUFSIZ);
#endif
	if (!(euid = geteuid()))
	{
		if (euid != (uid = getuid()))
		{
			if (setuid(uid))
			{
				fprintf(stderr, "imapd: setuid: %s\n", strerror(errno));
				exit(1);
			}
		}
	}
	authmodclient();
	time(&start_time);
	if ((authaddr=getenv("AUTHADDR")) == NULL ||
	    *authaddr == 0)
	{
		authaddr=getenv("AUTHENTICATED");
		if (authaddr == NULL || *authaddr == 0)
			authaddr="nobody";
	}
	if ((remoteip=getenv("TCPREMOTEIP")) == NULL)
	{
		fprintf(stderr, "ERROR: Required environment variables not initialized.\n");
		fflush(stderr);
		exit(1);
	}

	utf8_enabled=1; /* Until proven otherwise */

	{
		const char *utf8=getenv("UTF8");

		if (utf8)
			utf8_enabled=atoi(utf8);
	}

	if ((remoteport=getenv("TCPREMOTEPORT")) == NULL)
		remoteport="0";

	protocol = getenv("PROTOCOL");
	if (!protocol || !*protocol)
		protocol = "POP3";

	{
	struct	stat	buf;

		if ( stat(".", &buf) < 0 || buf.st_mode & S_ISVTX)
		{
			fprintf(stderr, "INFO: LOCKED, user=%s, ip=[%s], port=[%s]\n",
							authaddr, remoteip, remoteport);
			printed(printf("-ERR Your account is temporarily unavailable (+t bit set on home directory).\r\n"));
			exit(0);
		}
	}

	if ((p = getenv("MAILDIR")) != 0 && *p)
	{
		if (chdir(p))
		{
			printed(printf("-ERR Maildir: %s\r\n", strerror(errno)));
			fprintf(stderr, "chdir %s: %s\n", p, strerror(errno));
			exit(1);
		}
	} else
	if (argc > 1 && chdir((p = argv[1])))
	{
		printed(printf("-ERR chdir Maildir (%s): %s\r\n", p, strerror(errno)));
		fprintf(stderr, "chdir %s: %s\n", p, strerror(errno));
		exit(1);
	}
	maildir_loginexec();
	p = authgetoptionenv("disablepop3");
	if (p && atoi(p))
	{
		printed(printf("-ERR POP3 access disabled for this account.\r\n"));
		fflush(stdout);
		exit(1);
	}
	free(p);
	p = authgetoptionenv("disableinsecurepop3");
	if (p && atoi(p))
	{
		free (p);
	    if (!(p = getenv("POP3_TLS")) || !atoi(p))
		{
			printed(printf("-ERR POP3 access disabled via insecure connection.\r\n"));
			fflush(stdout);
			exit(1);
		}
	} else
	if (p)
		free(p);
	fprintf(stderr, "INFO: LOGIN, user=%s, ip=[%s], port=[%s], protocol=%s\n",
		authaddr, remoteip, remoteport, protocol);
	fflush(stderr);
	msglist_cnt=0;
	msglist_l=0;
	msglist_a=0;
	purgetmp();
	maildir_getnew(".", INBOX, NULL, NULL);
	if (scancur())
	{
		printed(printf("-ERR Maildir invalid (no 'cur' directory)\r\n"));
		return (0);
	}
	sortmsgs();
	printed(printf("+OK logged in.\r\n"));
	fflush(stdout);
	loop();
	return (0);
}
