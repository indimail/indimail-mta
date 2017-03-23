#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "stralloc.h"
#include "str.h"
#include "open.h"
#include "lock.h"
#include "slurpclose.h"
#include "log.h"
#include "scan.h"
#include "env.h"
#include "sig.h"
#include "slurp.h"
#include "getconf.h"
#include "strerr.h"
#include "byte.h"
#include "case.h"
#include "getln.h"
#include "substdio.h"
#include "error.h"
#include "quote.h"
#include "readwrite.h"
#include "fmt.h"
#include "now.h"
#include "cookie.h"
#include "subscribe.h"
#include "issub.h"

#define FATAL "ezmlm-return: fatal: "
void die_usage() { strerr_die1x(100,"ezmlm-return: usage: ezmlm-return dir"); }
void die_nomem() { strerr_die2x(111,FATAL,"out of memory"); }
void die_badaddr()
{
  strerr_die2x(100,FATAL,"I do not accept messages at this address (#5.1.1)");
}
void die_trash()
{
  strerr_die1x(0,"ezmlm-return: info: trash address");
}

char outbuf[1024];
substdio ssout;
char inbuf[1024];
substdio ssin;

char strnum[FMT_ULONG];
char hash[COOKIE];
char hashcopy[COOKIE];
unsigned long cookiedate;
stralloc fndate = {0};
stralloc fndatenew = {0};
stralloc fnhash = {0};
stralloc fnhashnew = {0};

stralloc quoted = {0};
char *sender;

void die_hashnew()
{ strerr_die4sys(111,FATAL,"unable to write ",fnhashnew.s,": "); }
void die_datenew()
{ strerr_die4sys(111,FATAL,"unable to write ",fndatenew.s,": "); }
void die_msgin()
{ strerr_die2sys(111,FATAL,"unable to read input: "); }

void dowit(addr,when,bounce)
char *addr;
unsigned long when;
stralloc *bounce;
{
  int fd;

  if (!issub(addr)) return;

  if (!stralloc_copys(&fndate,"bounce/w")) die_nomem();
  if (!stralloc_catb(&fndate,strnum,fmt_ulong(strnum,when))) die_nomem();
  if (!stralloc_cats(&fndate,".")) die_nomem();
  if (!stralloc_catb(&fndate,strnum,fmt_ulong(strnum,(unsigned long) getpid()))) die_nomem();
  if (!stralloc_0(&fndate)) die_nomem();
  if (!stralloc_copy(&fndatenew,&fndate)) die_nomem();
  fndatenew.s[7] = 'W';

  fd = open_trunc(fndatenew.s);
  if (fd == -1) die_datenew();
  substdio_fdbuf(&ssout,write,fd,outbuf,sizeof(outbuf));
  if (substdio_puts(&ssout,addr) == -1) die_datenew();
  if (substdio_put(&ssout,"",1) == -1) die_datenew();
  if (substdio_puts(&ssout,"Return-Path: <") == -1) die_datenew();
  if (!quote2(&quoted,sender)) die_nomem();
  if (substdio_put(&ssout,quoted.s,quoted.len) == -1) die_datenew();
  if (substdio_puts(&ssout,">\n") == -1) die_datenew();
  if (substdio_put(&ssout,bounce->s,bounce->len) == -1) die_datenew();
  if (substdio_flush(&ssout) == -1) die_datenew();
  if (fsync(fd) == -1) die_datenew();
  if (close(fd) == -1) die_datenew(); /* NFS stupidity */

  if (rename(fndatenew.s,fndate.s) == -1)
    strerr_die6sys(111,FATAL,"unable to rename ",fndatenew.s," to ",fndate.s,": ");
}

void doit(addr,msgnum,when,bounce)
char *addr;
unsigned long msgnum;
unsigned long when;
stralloc *bounce;
{
  int fd;
  int fdnew;

  if (!issub(addr)) return;

  if (!stralloc_copys(&fndate,"bounce/d")) die_nomem();
  if (!stralloc_catb(&fndate,strnum,fmt_ulong(strnum,when))) die_nomem();
  if (!stralloc_cats(&fndate,".")) die_nomem();
  if (!stralloc_catb(&fndate,strnum,fmt_ulong(strnum,(unsigned long) getpid()))) die_nomem();
  if (!stralloc_0(&fndate)) die_nomem();
  if (!stralloc_copy(&fndatenew,&fndate)) die_nomem();
  fndatenew.s[7] = 'D';

  fd = open_trunc(fndatenew.s);
  if (fd == -1) die_datenew();
  substdio_fdbuf(&ssout,write,fd,outbuf,sizeof(outbuf));
  if (substdio_puts(&ssout,addr) == -1) die_datenew();
  if (substdio_put(&ssout,"",1) == -1) die_datenew();
  if (substdio_puts(&ssout,"Return-Path: <") == -1) die_datenew();
  if (!quote2(&quoted,sender)) die_nomem();
  if (substdio_put(&ssout,quoted.s,quoted.len) == -1) die_datenew();
  if (substdio_puts(&ssout,">\n") == -1) die_datenew();
  if (substdio_put(&ssout,bounce->s,bounce->len) == -1) die_datenew();
  if (substdio_flush(&ssout) == -1) die_datenew();
  if (fsync(fd) == -1) die_datenew();
  if (close(fd) == -1) die_datenew(); /* NFS stupidity */

  cookie(hash,"",0,"",addr,"");
  if (!stralloc_copys(&fnhash,"bounce/h")) die_nomem();
  if (!stralloc_catb(&fnhash,hash,COOKIE)) die_nomem();
  if (!stralloc_0(&fnhash)) die_nomem();
  if (!stralloc_copy(&fnhashnew,&fnhash)) die_nomem();
  fnhashnew.s[7] = 'H';

  fdnew = open_trunc(fnhashnew.s);
  if (fdnew == -1) die_hashnew();
  substdio_fdbuf(&ssout,write,fdnew,outbuf,sizeof(outbuf));

  fd = open_read(fnhash.s);
  if (fd == -1) {
    if (errno != error_noent)
      strerr_die4sys(111,FATAL,"unable to read ",fnhash.s,": ");
    if (rename(fndatenew.s,fndate.s) == -1)
      strerr_die6sys(111,FATAL,"unable to rename ",fndatenew.s," to ",fndate.s,": ");
  }
  else {
    substdio_fdbuf(&ssin,read,fd,inbuf,sizeof(inbuf));
    switch(substdio_copy(&ssout,&ssin)) {
      case -2: die_msgin();
      case -3: die_hashnew();
    }
    close(fd);
    if (unlink(fndatenew.s) == -1)
      strerr_die4sys(111,FATAL,"unable to unlink ",fndatenew.s,": ");
  }
  if (substdio_puts(&ssout,"   ") == -1) die_hashnew();
  if (substdio_put(&ssout,strnum,fmt_ulong(strnum,msgnum)) == -1) die_hashnew();
  if (substdio_puts(&ssout,"\n") == -1) die_hashnew();
  if (substdio_flush(&ssout) == -1) die_hashnew();
  if (fsync(fdnew) == -1) die_hashnew();
  if (close(fdnew) == -1) die_hashnew(); /* NFS stupidity */

  if (rename(fnhashnew.s,fnhash.s) == -1)
    strerr_die6sys(111,FATAL,"unable to rename ",fnhashnew.s," to ",fnhash.s,": ");
}

stralloc bounce = {0};
stralloc line = {0};
stralloc header = {0};
stralloc intro = {0};
stralloc failure = {0};
stralloc paragraph = {0};
int flaghaveheader;
int flaghaveintro;

stralloc key = {0};
stralloc inhost = {0};
stralloc outhost = {0};
stralloc inlocal = {0};
stralloc outlocal = {0};

char msginbuf[1024];
substdio ssmsgin;

void main(argc,argv)
int argc;
char **argv;
{
  char *dir;
  char *host;
  char *local;
  char *action;
  unsigned long msgnum;
  unsigned long cookiedate;
  unsigned long when;
  int match;
  int i;
  int fdlock;

  umask(022);
  sig_pipeignore();
  when = (unsigned long) now();

  dir = argv[1];
  if (!dir) die_usage();

  sender = env_get("SENDER");
  if (!sender) strerr_die2x(100,FATAL,"SENDER not set");
  local = env_get("LOCAL");
  if (!local) strerr_die2x(100,FATAL,"LOCAL not set");
  host = env_get("HOST");
  if (!host) strerr_die2x(100,FATAL,"HOST not set");

  if (chdir(dir) == -1)
    strerr_die4sys(111,FATAL,"unable to switch to ",dir,": ");

  switch(slurp("key",&key,32)) {
    case -1:
      strerr_die4sys(111,FATAL,"unable to read ",dir,"/key: ");
    case 0:
      strerr_die3x(100,FATAL,dir,"/key does not exist");
  }
  getconf_line(&inhost,"inhost",1,FATAL,dir);
  getconf_line(&inlocal,"inlocal",1,FATAL,dir);
  getconf_line(&outhost,"outhost",1,FATAL,dir);
  getconf_line(&outlocal,"outlocal",1,FATAL,dir);

  if (inhost.len != str_len(host)) die_badaddr();
  if (case_diffb(inhost.s,inhost.len,host)) die_badaddr();
  if (inlocal.len > str_len(local)) die_badaddr();
  if (case_diffb(inlocal.s,inlocal.len,local)) die_badaddr();

  action = local + inlocal.len;

  if (!str_start(action,"-return-")) die_badaddr();
  action += 8;

  if (!*action) die_trash();

  if (str_start(action,"probe-")) {
    action += 6;
    action += scan_ulong(action,&cookiedate);
    if (now() - cookiedate > 3000000) die_trash();
    if (*action++ != '.') die_trash();
    i = str_chr(action,'-');
    if (i != COOKIE) die_trash();
    byte_copy(hashcopy,COOKIE,action);
    action += COOKIE;
    if (*action++ != '-') die_trash();
    i = str_rchr(action,'=');
    if (!stralloc_copyb(&line,action,i)) die_nomem();
    if (action[i]) {
      if (!stralloc_cats(&line,"@")) die_nomem();
      if (!stralloc_cats(&line,action + i + 1)) die_nomem();
    }
    if (!stralloc_0(&line)) die_nomem();
    strnum[fmt_ulong(strnum,cookiedate)] = 0;
    cookie(hash,key.s,key.len,strnum,line.s,"P");
    if (byte_diff(hash,COOKIE,hashcopy)) die_trash();

    if (subscribe(line.s,0) == 1) ezlog("-probe",line.s);
    _exit(0);
  }

  fdlock = open_append("lockbounce");
  if (fdlock == -1)
    strerr_die4sys(111,FATAL,"unable to open ",dir,"/lockbounce: ");
  if (lock_ex(fdlock) == -1)
    strerr_die4sys(111,FATAL,"unable to lock ",dir,"/lockbounce: ");

  if (str_start(action,"warn-")) {
    action += 5;
    action += scan_ulong(action,&cookiedate);
    if (now() - cookiedate > 3000000) die_trash();
    if (*action++ != '.') die_trash();
    i = str_chr(action,'-');
    if (i != COOKIE) die_trash();
    byte_copy(hashcopy,COOKIE,action);
    action += COOKIE;
    if (*action++ != '-') die_trash();
    i = str_rchr(action,'=');
    if (!stralloc_copyb(&line,action,i)) die_nomem();
    if (action[i]) {
      if (!stralloc_cats(&line,"@")) die_nomem();
      if (!stralloc_cats(&line,action + i + 1)) die_nomem();
    }
    if (!stralloc_0(&line)) die_nomem();
    strnum[fmt_ulong(strnum,cookiedate)] = 0;
    cookie(hash,key.s,key.len,strnum,line.s,"W");
    if (byte_diff(hash,COOKIE,hashcopy)) die_trash();

    if (slurpclose(0,&bounce,1024) == -1) die_msgin();
    dowit(line.s,when,&bounce);
    _exit(0);
  }

  action += scan_ulong(action,&msgnum);
  if (*action != '-') die_badaddr();
  ++action;

  if (*action) {
    if (slurpclose(0,&bounce,1024) == -1) die_msgin();

    i = str_rchr(action,'=');
    if (!stralloc_copyb(&line,action,i)) die_nomem();
    if (action[i]) {
      if (!stralloc_cats(&line,"@")) die_nomem();
      if (!stralloc_cats(&line,action + i + 1)) die_nomem();
    }
    if (!stralloc_0(&line)) die_nomem();
    doit(line.s,msgnum,when,&bounce);
    _exit(0);
  }

  /* pre-VERP bounce, in QSBMF format */

  substdio_fdbuf(&ssmsgin,read,0,msginbuf,sizeof(msginbuf));

  flaghaveheader = 0;
  flaghaveintro = 0;

  for (;;) {
    if (!stralloc_copys(&paragraph,"")) die_nomem();
    for (;;) {
      if (getln(&ssmsgin,&line,&match,'\n') == -1) die_msgin();
      if (!match) die_trash();
      if (!stralloc_cat(&paragraph,&line)) die_nomem();
      if (line.len <= 1) break;
    }

    if (!flaghaveheader) {
      if (!stralloc_copy(&header,&paragraph)) die_nomem();
      flaghaveheader = 1;
      continue;
    }

    if (!flaghaveintro) {
      if (paragraph.len < 15) die_trash();
      if (str_diffn(paragraph.s,"Hi. This is the",15)) die_trash();
      if (!stralloc_copy(&intro,&paragraph)) die_nomem();
      flaghaveintro = 1;
      continue;
    }

    if (paragraph.s[0] == '-')
      break;

    if (paragraph.s[0] == '<') {
      if (!stralloc_copy(&failure,&paragraph)) die_nomem();

      if (!stralloc_copy(&bounce,&header)) die_nomem();
      if (!stralloc_cat(&bounce,&intro)) die_nomem();
      if (!stralloc_cat(&bounce,&failure)) die_nomem();

      i = byte_chr(failure.s,failure.len,'\n');
      if (i < 3) die_trash();

      if (!stralloc_copyb(&line,failure.s + 1,i - 3)) die_nomem();
      if (byte_chr(line.s,line.len,'\0') == line.len) {
        if (!stralloc_0(&line)) die_nomem();
        doit(line.s,msgnum,when,&bounce);
      }
    }
  }

  _exit(0);
}
