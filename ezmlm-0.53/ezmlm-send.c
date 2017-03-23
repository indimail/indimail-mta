#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "stralloc.h"
#include "byte.h"
#include "subfd.h"
#include "strerr.h"
#include "error.h"
#include "qmail.h"
#include "env.h"
#include "lock.h"
#include "sig.h"
#include "open.h"
#include "getln.h"
#include "case.h"
#include "scan.h"
#include "str.h"
#include "fmt.h"
#include "readwrite.h"
#include "exit.h"
#include "substdio.h"
#include "getconf.h"
#include "constmap.h"

#define FATAL "ezmlm-send: fatal: "

void die_usage()
{
  strerr_die1x(100,"ezmlm-send: usage: ezmlm-send dir");
}
void die_nomem()
{
  strerr_die2x(111,FATAL,"out of memory");
}

char strnum[FMT_ULONG];

stralloc fnadir = {0};
stralloc fnaf = {0};
stralloc fnsub = {0};
stralloc line = {0};

int flagarchived;
int fdarchive;
substdio ssarchive;
char archivebuf[1024];

int flagsublist;
stralloc sublist = {0};
stralloc mailinglist = {0};
stralloc outlocal = {0};
stralloc outhost = {0};
stralloc headerremove = {0};
struct constmap headerremovemap;
stralloc headeradd = {0};

struct qmail qq;
substdio ssin;
char inbuf[1024];
substdio ssout;
char outbuf[1];

int mywrite(fd,buf,len)
int fd;
char *buf;
unsigned int len;
{
  qmail_put(&qq,buf,len);
  return len;
}

void die_archive()
{
  strerr_die4sys(111,FATAL,"unable to write to ",fnaf.s,": ");
}
void die_numnew()
{
  strerr_die2sys(111,FATAL,"unable to create numnew: ");
}

void put(buf,len) char *buf; int len;
{
  qmail_put(&qq,buf,len);
  if (flagarchived)
    if (substdio_put(&ssarchive,buf,len) == -1) die_archive();
}

void ezputs(buf) char *buf;
{
  qmail_puts(&qq,buf);
  if (flagarchived)
    if (substdio_puts(&ssarchive,buf) == -1) die_archive();
}

int sublistmatch(sender)
char *sender;
{
  int i;
  int j;

  j = str_len(sender);
  if (j < sublist.len) return 0;

  i = byte_rchr(sublist.s,sublist.len,'@');
  if (i == sublist.len) return 1;

  if (byte_diff(sublist.s,i,sender)) return 0;
  if (case_diffb(sublist.s + i,sublist.len - i,sender + j - (sublist.len - i)))
    return 0;

  return 1;
}

substdio ssnumnew;
char numnewbuf[16];
unsigned long msgnum;
stralloc num = {0};

char buf0[256];
substdio ss0 = SUBSTDIO_FDBUF(read,0,buf0,sizeof(buf0));

void numwrite()
{
  int fd;

  fd = open_trunc("numnew");
  if (fd == -1) die_numnew();
  substdio_fdbuf(&ssnumnew,write,fd,numnewbuf,sizeof(numnewbuf));
  if (substdio_put(&ssnumnew,strnum,fmt_ulong(strnum,msgnum)) == -1)
    die_numnew();
  if (substdio_puts(&ssnumnew,"\n") == -1) die_numnew();
  if (substdio_flush(&ssnumnew) == -1) die_numnew();
  if (fsync(fd) == -1) die_numnew();
  if (close(fd) == -1) die_numnew(); /* NFS stupidity */
  if (rename("numnew","num") == -1)
    strerr_die2sys(111,FATAL,"unable to move numnew to num: ");
}

stralloc mydtline = {0};

void main(argc,argv)
int argc;
char **argv;
{
  int fd;
  char *dir;
  int fdlock;
  char *sender;
  int flagmlwasthere;
  int match;
  int i;
  char ch;
  int flaginheader;
  int flagbadfield;

  umask(022);
  sig_pipeignore();

  dir = argv[1];
  if (!dir) die_usage();

  sender = env_get("SENDER");

  if (chdir(dir) == -1)
    strerr_die4sys(111,FATAL,"unable to switch to ",dir,": ");

  fdlock = open_append("lock");
  if (fdlock == -1)
    strerr_die4sys(111,FATAL,"unable to open ",dir,"/lock: ");
  if (lock_ex(fdlock) == -1)
    strerr_die4sys(111,FATAL,"unable to obtain ",dir,"/lock: ");

  if (qmail_open(&qq) == -1)
    strerr_die2sys(111,FATAL,"unable to run qmail-queue: ");

  flagarchived = getconf_line(&line,"archived",0,FATAL,dir);

  getconf_line(&num,"num",1,FATAL,dir);
  if (!stralloc_0(&num)) die_nomem();
  scan_ulong(num.s,&msgnum);
  ++msgnum;

  getconf_line(&outhost,"outhost",1,FATAL,dir);
  getconf_line(&outlocal,"outlocal",1,FATAL,dir);
  getconf_line(&mailinglist,"mailinglist",1,FATAL,dir);
  flagsublist = getconf_line(&sublist,"sublist",0,FATAL,dir);

  getconf(&headerremove,"headerremove",1,FATAL,dir);
  constmap_init(&headerremovemap,headerremove.s,headerremove.len,0);

  getconf(&headeradd,"headeradd",1,FATAL,dir);
  for (i = 0;i < headeradd.len;++i)
    if (!headeradd.s[i])
      headeradd.s[i] = '\n';

  if (!stralloc_copys(&mydtline,"Delivered-To: mailing list ")) die_nomem();
  if (!stralloc_catb(&mydtline,outlocal.s,outlocal.len)) die_nomem();
  if (!stralloc_cats(&mydtline,"@")) die_nomem();
  if (!stralloc_catb(&mydtline,outhost.s,outhost.len)) die_nomem();
  if (!stralloc_cats(&mydtline,"\n")) die_nomem();

  if (sender) {
    if (!*sender)
      strerr_die2x(100,FATAL,"I don't distribute bounce messages (#5.7.2)");
    if (str_equal(sender,"#@[]"))
      strerr_die2x(100,FATAL,"I don't distribute bounce messages (#5.7.2)");
    if (flagsublist)
      if (!sublistmatch(sender))
        strerr_die2x(100,FATAL,"this message is not from my parent list (#5.7.2)");
  }

  if (flagarchived) {
    if (!stralloc_copys(&fnadir,"archive/")) die_nomem();
    if (!stralloc_catb(&fnadir,strnum,fmt_ulong(strnum,msgnum / 100))) die_nomem();
    if (!stralloc_copy(&fnaf,&fnadir)) die_nomem();
    if (!stralloc_cats(&fnaf,"/")) die_nomem();
    if (!stralloc_catb(&fnaf,strnum,fmt_uint0(strnum,(unsigned int) (msgnum % 100),2))) die_nomem();
    if (!stralloc_0(&fnadir)) die_nomem();
    if (!stralloc_0(&fnaf)) die_nomem();

    if (mkdir(fnadir.s,0755) == -1)
      if (errno != error_exist)
	strerr_die4sys(111,FATAL,"unable to create ",fnadir.s,": ");
    fdarchive = open_trunc(fnaf.s);
    if (fdarchive == -1)
      strerr_die4sys(111,FATAL,"unable to write ",fnaf.s,": ");

    substdio_fdbuf(&ssarchive,write,fdarchive,archivebuf,sizeof(archivebuf));
  }

  if (!flagsublist) {
    ezputs("Mailing-List: ");
    put(mailinglist.s,mailinglist.len);
    ezputs("\n");
  }
  put(headeradd.s,headeradd.len);
  put(mydtline.s,mydtline.len);

  flagmlwasthere = 0;
  flaginheader = 1;
  flagbadfield = 0;

  for (;;) {
    if (getln(&ss0,&line,&match,'\n') == -1)
      strerr_die2sys(111,FATAL,"unable to read input: ");

    if (flaginheader && match) {
      if (line.len == 1)
	flaginheader = 0;
      if ((line.s[0] != ' ') && (line.s[0] != '\t')) {
	flagbadfield = 0;
	if (constmap(&headerremovemap,line.s,byte_chr(line.s,line.len,':')))
	  flagbadfield = 1;
	if (case_startb(line.s,line.len,"mailing-list:"))
	  flagmlwasthere = 1;
	if (line.len == mydtline.len)
	  if (!byte_diff(line.s,line.len,mydtline.s))
            strerr_die2x(100,FATAL,"this message is looping: it already has my Delivered-To line (#5.4.6)");
      }
    }

    if (!(flaginheader && flagbadfield))
      put(line.s,line.len);

    if (!match)
      break;
  }

  if (flagsublist)
    if (!flagmlwasthere)
      strerr_die2x(100,FATAL,"sublist messages must have Mailing-List (#5.7.2)");
  if (!flagsublist)
    if (flagmlwasthere)
      strerr_die2x(100,FATAL,"message already has Mailing-List (#5.7.2)");

  if (flagarchived) {
    if (substdio_flush(&ssarchive) == -1) die_archive();
    if (fsync(fdarchive) == -1) die_archive();
    if (fchmod(fdarchive,0744) == -1) die_archive();
    if (close(fdarchive) == -1) die_archive(); /* NFS stupidity */
  }

  numwrite();

  if (!stralloc_copy(&line,&outlocal)) die_nomem();
  if (!stralloc_cats(&line,"-return-")) die_nomem();
  if (!stralloc_catb(&line,strnum,fmt_ulong(strnum,msgnum))) die_nomem();
  if (!stralloc_cats(&line,"-@")) die_nomem();
  if (!stralloc_cat(&line,&outhost)) die_nomem();
  if (!stralloc_cats(&line,"-@[]")) die_nomem();
  if (!stralloc_0(&line)) die_nomem();

  qmail_from(&qq,line.s);

  for (i = 0;i < 53;++i) {
    ch = 64 + i;
    if (!stralloc_copys(&fnsub,"subscribers/")) die_nomem();
    if (!stralloc_catb(&fnsub,&ch,1)) strerr_die2x(111,FATAL,"out of memory");
    if (!stralloc_0(&fnsub)) strerr_die2x(111,FATAL,"out of memory");
    fd = open_read(fnsub.s);
    if (fd == -1) {
      if (errno != error_noent)
	strerr_die4sys(111,FATAL,"unable to read ",fnsub.s,": ");
    }
    else {
      substdio_fdbuf(&ssin,read,fd,inbuf,sizeof(inbuf));
      substdio_fdbuf(&ssout,mywrite,-1,outbuf,sizeof(outbuf));
      if (substdio_copy(&ssout,&ssin) != 0)
	strerr_die4sys(111,FATAL,"unable to read ",fnsub.s,": ");
      close(fd);
    }
  }

  switch(qmail_close(&qq)) {
    case 0:
      strnum[fmt_ulong(strnum,qmail_qp(&qq))] = 0;
      strerr_die2x(0,"ezmlm-send: info: qp ",strnum);
    default:
      --msgnum;
      numwrite();
      strerr_die2x(111,FATAL,"temporary qmail-queue error");
  }
}
