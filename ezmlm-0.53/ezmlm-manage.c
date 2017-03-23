#include <sys/types.h>
#include <sys/stat.h>
#include "error.h"
#include "open.h"
#include "stralloc.h"
#include "str.h"
#include "env.h"
#include "sig.h"
#include "slurp.h"
#include "getconf.h"
#include "strerr.h"
#include "byte.h"
#include "getln.h"
#include "case.h"
#include "substdio.h"
#include "qmail.h"
#include "readwrite.h"
#include "seek.h"
#include "quote.h"
#include "datetime.h"
#include "now.h"
#include "date822fmt.h"
#include "fmt.h"
#include "subscribe.h"
#include "scan.h"
#include "cookie.h"
#include "log.h"

#define FATAL "ezmlm-manage: fatal: "
void die_usage() { strerr_die1x(100,"ezmlm-manage: usage: ezmlm-manage dir"); }
void die_nomem() { strerr_die2x(111,FATAL,"out of memory"); }
void die_badaddr()
{
  strerr_die2x(100,FATAL,"I do not accept messages at this address (#5.1.1)");
}

stralloc inhost = {0};
stralloc outhost = {0};
stralloc inlocal = {0};
stralloc outlocal = {0};
stralloc key = {0};
stralloc mailinglist = {0};

datetime_sec when;
struct datetime dt;

char strnum[FMT_ULONG];
char date[DATE822FMT];
char hash[COOKIE];
datetime_sec hashdate;
stralloc target = {0};
stralloc confirm = {0};
stralloc line = {0};
stralloc quoted = {0};

int hashok(action)
char *action;
{
  char *x;
  unsigned long u;

  x = action + 4;
  x += scan_ulong(x,&u);
  hashdate = u;
  if (hashdate > when) return 0;
  if (hashdate < when - 1000000) return 0;

  u = hashdate;
  strnum[fmt_ulong(strnum,u)] = 0;
  cookie(hash,key.s,key.len,strnum,target.s,action + 1);

  if (*x == '.') ++x;
  if (str_len(x) != COOKIE) return 0;
  return byte_equal(hash,COOKIE,x);
}

struct qmail qq;
ssize_t qqwrite(fd,buf,len) int fd; char *buf; unsigned int len;
{
  qmail_put(&qq,buf,len);
  return len;
}
char qqbuf[1];
substdio ssqq = SUBSTDIO_FDBUF(qqwrite,-1,qqbuf,sizeof(qqbuf));

char inbuf[1024];
substdio ssin = SUBSTDIO_FDBUF(read,0,inbuf,sizeof(inbuf));
substdio ssin2 = SUBSTDIO_FDBUF(read,0,inbuf,sizeof(inbuf));

substdio sstext;
char textbuf[1024];

void copy(fn)
char *fn;
{
  int fd;
  int match;

  fd = open_read(fn);
  if (fd == -1)
    strerr_die4sys(111,FATAL,"unable to open ",fn,": ");

  substdio_fdbuf(&sstext,read,fd,textbuf,sizeof(textbuf));
  for (;;) {
    if (getln(&sstext,&line,&match,'\n') == -1)
      strerr_die4sys(111,FATAL,"unable to read ",fn,": ");

    if (match)
      if (line.s[0] == '!') {
	if (line.s[1] == 'R') {
	  qmail_puts(&qq,"   ");
	  qmail_puts(&qq,confirm.s);
	  qmail_puts(&qq,"\n");
	  continue;
	}
	if (line.s[1] == 'A') {
	  qmail_puts(&qq,"   ");
	  qmail_puts(&qq,target.s);
	  qmail_puts(&qq,"\n");
	  continue;
	}
      }

    qmail_put(&qq,line.s,line.len);

    if (!match)
      break;
  }

  close(fd);
}

stralloc mydtline = {0};

void main(argc,argv)
int argc;
char **argv;
{
  char *dir;
  char *sender;
  char *host;
  char *local;
  char *action;
  int fd;
  int i;
  int flagconfirm;
  int flaghashok;
  int flaggoodfield;
  int match;

  umask(022);
  sig_pipeignore();
  when = now();

  dir = argv[1];
  if (!dir) die_usage();

  sender = env_get("SENDER");
  if (!sender) strerr_die2x(100,FATAL,"SENDER not set");
  local = env_get("LOCAL");
  if (!local) strerr_die2x(100,FATAL,"LOCAL not set");
  host = env_get("HOST");
  if (!host) strerr_die2x(100,FATAL,"HOST not set");

  if (!*sender)
    strerr_die2x(100,FATAL,"I don't reply to bounce messages (#5.7.2)");
  if (!sender[str_chr(sender,'@')])
    strerr_die2x(100,FATAL,"I don't reply to senders without host names (#5.7.2)");
  if (str_equal(sender,"#@[]"))
    strerr_die2x(100,FATAL,"I don't reply to bounce messages (#5.7.2)");

  if (chdir(dir) == -1)
    strerr_die4sys(111,FATAL,"unable to switch to ",dir,": ");

  switch(slurp("key",&key,32)) {
    case -1:
      strerr_die4sys(111,FATAL,"unable to read ",dir,"/key: ");
    case 0:
      strerr_die3x(100,FATAL,dir,"/key does not exist");
  }
  getconf_line(&mailinglist,"mailinglist",1,FATAL,dir);
  getconf_line(&inhost,"inhost",1,FATAL,dir);
  getconf_line(&inlocal,"inlocal",1,FATAL,dir);
  getconf_line(&outhost,"outhost",1,FATAL,dir);
  getconf_line(&outlocal,"outlocal",1,FATAL,dir);

  if (inhost.len != str_len(host)) die_badaddr();
  if (case_diffb(inhost.s,inhost.len,host)) die_badaddr();
  if (inlocal.len > str_len(local)) die_badaddr();
  if (case_diffb(inlocal.s,inlocal.len,local)) die_badaddr();

  action = local + inlocal.len;

  switch(slurp("public",&line,1)) {
    case -1:
      strerr_die4sys(111,FATAL,"unable to read ",dir,"/public: ");
    case 0:
      strerr_die2x(100,FATAL,"sorry, I've been told to reject all requests (#5.7.2)");
  }

  if (!stralloc_copys(&target,sender)) die_nomem();
  if (action[0]) {
    i = 1 + str_chr(action + 1,'-');
    if (action[i]) {
      action[i] = 0;
      if (!stralloc_copys(&target,action + i + 1)) die_nomem();
      i = byte_rchr(target.s,target.len,'=');
      if (i < target.len)
	target.s[i] = '@';
    }
  }
  if (!stralloc_0(&target)) die_nomem();
  if (!stralloc_copys(&confirm,"")) die_nomem();

  if (qmail_open(&qq) == -1)
    strerr_die2sys(111,FATAL,"unable to run qmail-queue: ");

  qmail_puts(&qq,"Mailing-List: ");
  qmail_put(&qq,mailinglist.s,mailinglist.len);
  qmail_puts(&qq,"\nDate: ");
  datetime_tai(&dt,when);
  qmail_put(&qq,date,date822fmt(date,&dt));
  qmail_puts(&qq,"Message-ID: <");
  qmail_put(&qq,strnum,fmt_ulong(strnum,(unsigned long) when));
  qmail_puts(&qq,".");
  qmail_put(&qq,strnum,fmt_ulong(strnum,(unsigned long) getpid()));
  qmail_puts(&qq,".ezmlm@");
  qmail_put(&qq,outhost.s,outhost.len);
  qmail_puts(&qq,">\nFrom: ");
  if (!quote(&quoted,&outlocal)) die_nomem();
  qmail_put(&qq,quoted.s,quoted.len);
  qmail_puts(&qq,"-help@");
  qmail_put(&qq,outhost.s,outhost.len);
  qmail_puts(&qq,"\nTo: ");
  if (!quote2(&quoted,target.s)) die_nomem();
  qmail_put(&qq,quoted.s,quoted.len);
  qmail_puts(&qq,"\n");

  flaghashok = 1;
  if (str_start(action,"-sc.")) flaghashok = hashok(action);
  if (str_start(action,"-uc.")) flaghashok = hashok(action);

  flagconfirm = 0;
  if (str_equal(action,"-subscribe")) flagconfirm = 1;
  if (str_equal(action,"-unsubscribe")) flagconfirm = 1;
  if (!flaghashok) flagconfirm = 1;
  
  if (flagconfirm) {
    strnum[fmt_ulong(strnum,(unsigned long) when)] = 0;
    cookie(hash,key.s,key.len,strnum,target.s,action + 1);
    if (!stralloc_copy(&confirm,&outlocal)) die_nomem();
    if (!stralloc_cats(&confirm,"-")) die_nomem();
    if (!stralloc_catb(&confirm,action + 1,1)) die_nomem();
    if (!stralloc_cats(&confirm,"c.")) die_nomem();
    if (!stralloc_cats(&confirm,strnum)) die_nomem();
    if (!stralloc_cats(&confirm,".")) die_nomem();
    if (!stralloc_catb(&confirm,hash,COOKIE)) die_nomem();
    if (!stralloc_cats(&confirm,"-")) die_nomem();
    i = str_rchr(target.s,'@');
    if (!stralloc_catb(&confirm,target.s,i)) die_nomem();
    if (target.s[i]) {
      if (!stralloc_cats(&confirm,"=")) die_nomem();
      if (!stralloc_cats(&confirm,target.s + i + 1)) die_nomem();
    }
    if (!stralloc_cats(&confirm,"@")) die_nomem();
    if (!stralloc_cat(&confirm,&outhost)) die_nomem();
    if (!stralloc_0(&confirm)) die_nomem();

    qmail_puts(&qq,"Reply-To: ");
    if (!quote2(&quoted,confirm.s)) die_nomem();
    qmail_put(&qq,quoted.s,quoted.len);
    qmail_puts(&qq,"\n");
  }
  if (!stralloc_0(&confirm)) die_nomem();

  qmail_puts(&qq,"Subject: ezmlm response\n");

  if (!stralloc_copys(&mydtline,"Delivered-To: responder for ")) die_nomem();
  if (!stralloc_catb(&mydtline,outlocal.s,outlocal.len)) die_nomem();
  if (!stralloc_cats(&mydtline,"@")) die_nomem();
  if (!stralloc_catb(&mydtline,outhost.s,outhost.len)) die_nomem();
  if (!stralloc_cats(&mydtline,"\n")) die_nomem();

  qmail_put(&qq,mydtline.s,mydtline.len);

  flaggoodfield = 0;
  for (;;) {
    if (getln(&ssin,&line,&match,'\n') == -1)
      strerr_die2sys(111,FATAL,"unable to read input: ");
    if (!match) break;
    if (line.len == 1) break;
    if ((line.s[0] != ' ') && (line.s[0] != '\t')) {
      flaggoodfield = 0;
      if (case_startb(line.s,line.len,"mailing-list:"))
        strerr_die2x(100,FATAL,"incoming message has Mailing-List (#5.7.2)");
      if (line.len == mydtline.len)
	if (byte_equal(line.s,line.len,mydtline.s))
          strerr_die2x(100,FATAL,"this message is looping: it already has my Delivered-To line (#5.4.6)");
      if (case_startb(line.s,line.len,"delivered-to:"))
        flaggoodfield = 1;
      if (case_startb(line.s,line.len,"received:"))
        flaggoodfield = 1;
    }
    if (flaggoodfield)
      qmail_put(&qq,line.s,line.len);
  }
  if (seek_begin(0) == -1)
    strerr_die2sys(111,FATAL,"unable to seek input: ");

  qmail_puts(&qq,"\n");
  copy("text/top");
  if (str_equal(action,"-subscribe"))
    copy("text/sub-confirm");
  else if (str_equal(action,"-unsubscribe"))
    copy("text/unsub-confirm");
  else if (str_start(action,"-sc.")) {
    if (!flaghashok)
      copy("text/sub-bad");
    else
      switch(subscribe(target.s,1)) {
        case -1: strerr_die1(111,FATAL,&subscribe_err);
        case -2: strerr_die1(100,FATAL,&subscribe_err);
	case 1: ezlog("+",target.s); copy("text/sub-ok"); break;
	default: copy("text/sub-nop"); break;
      }
  }
  else if (str_start(action,"-uc.")) {
    if (!flaghashok)
      copy("text/unsub-bad");
    else
      switch(subscribe(target.s,0)) {
        case -1: strerr_die1(111,FATAL,&subscribe_err);
        case -2: strerr_die1(100,FATAL,&subscribe_err);
	case 1: ezlog("-",target.s); copy("text/unsub-ok"); break;
	default: copy("text/unsub-nop"); break;
      }
  }
  else if (str_start(action,"-get.")) {
    unsigned long u;
    struct stat st;
    char ch;
    int r;

    scan_ulong(action + 5,&u);

    if (!stralloc_copys(&line,"archive/")) die_nomem();
    if (!stralloc_catb(&line,strnum,fmt_ulong(strnum,u / 100))) die_nomem();
    if (!stralloc_cats(&line,"/")) die_nomem();
    if (!stralloc_catb(&line,strnum,fmt_uint0(strnum,(unsigned int) (u % 100),2))) die_nomem();
    if (!stralloc_0(&line)) die_nomem();

    fd = open_read(line.s);
    if (fd == -1)
      if (errno != error_noent)
	strerr_die4sys(111,FATAL,"unable to open ",line.s,": ");
      else
        copy("text/get-bad");
    else {
      if (fstat(fd,&st) == -1)
	copy("text/get-bad");
      else if (!(st.st_mode & 0100))
	copy("text/get-bad");
      else {
        substdio_fdbuf(&sstext,read,fd,textbuf,sizeof(textbuf));
	qmail_puts(&qq,"> ");
	for (;;) {
	  r = substdio_get(&sstext,&ch,1);
	  if (r == -1) strerr_die4sys(111,FATAL,"unable to read ",line.s,": ");
	  if (r == 0) break;
	  qmail_put(&qq,&ch,1);
	  if (ch == '\n') qmail_puts(&qq,"> ");
	}
	qmail_puts(&qq,"\n");
      }
      close(fd);
    }
  }
  else
    copy("text/help");

  copy("text/bottom");

  qmail_puts(&qq,"Return-Path: <");
  if (!quote2(&quoted,sender)) die_nomem();
  qmail_put(&qq,quoted.s,quoted.len);
  qmail_puts(&qq,">\n");
  if (substdio_copy(&ssqq,&ssin2) != 0)
    strerr_die2sys(111,FATAL,"unable to read input: ");

  if (!stralloc_copy(&line,&outlocal)) die_nomem();
  if (!stralloc_cats(&line,"-return-@")) die_nomem();
  if (!stralloc_cat(&line,&outhost)) die_nomem();
  if (!stralloc_0(&line)) die_nomem();
  qmail_from(&qq,line.s);

  qmail_to(&qq,target.s);

  switch(qmail_close(&qq)) {
    case 0:
      strnum[fmt_ulong(strnum,qmail_qp(&qq))] = 0;
      strerr_die2x(0,"ezmlm-manage: info: qp ",strnum);
    default:
      /* don't worry about undoing actions; everything is idempotent */
      strerr_die2x(111,FATAL,"temporary qmail-queue error");
  }
}
