#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include "sgetopt.h"
#include "stralloc.h"
#include "strerr.h"
#include "exit.h"
#include "readwrite.h"
#include "open.h"
#include "substdio.h"
#include "str.h"
#include "auto_bin.h"

#define FATAL "ezmlm-make: fatal: "

void die_usage()
{
  strerr_die1x(100,"ezmlm-make: usage: ezmlm-make [ -aApP ] dir dot local host");
}
void die_relative()
{
  strerr_die2x(100,FATAL,"dir must start with slash");
}
void die_newline()
{
  strerr_die2x(100,FATAL,"newlines not allowed");
}
void die_quote()
{
  strerr_die2x(100,FATAL,"quotes not allowed");
}
void die_nomem()
{
  strerr_die2x(111,FATAL,"out of memory");
}

stralloc key = {0};
struct timeval tv;

void keyadd(u)
unsigned long u;
{
  char ch;
  ch = u; if (!stralloc_append(&key,&ch)) die_nomem(); u >>= 8;
  ch = u; if (!stralloc_append(&key,&ch)) die_nomem(); u >>= 8;
  ch = u; if (!stralloc_append(&key,&ch)) die_nomem(); u >>= 8;
  ch = u; if (!stralloc_append(&key,&ch)) die_nomem();
}

void keyaddtime()
{
  gettimeofday(&tv,(struct timezone *) 0);
  keyadd(tv.tv_usec);
}

char *dir;
char *dot;
char *local;
char *host;

stralloc dotplus = {0};
stralloc dirplus = {0};

void dirplusmake(slash)
char *slash;
{
  if (!stralloc_copys(&dirplus,dir)) die_nomem();
  if (!stralloc_cats(&dirplus,slash)) die_nomem();
  if (!stralloc_0(&dirplus)) die_nomem();
}

void linkdotdir(dash,slash)
char *dash;
char *slash;
{
  if (!stralloc_copys(&dotplus,dot)) die_nomem();
  if (!stralloc_cats(&dotplus,dash)) die_nomem();
  if (!stralloc_0(&dotplus)) die_nomem();
  dirplusmake(slash);
  if (symlink(dirplus.s,dotplus.s) == -1)
    strerr_die4sys(111,FATAL,"unable to create ",dotplus.s,": ");
  keyaddtime();
}

void dcreate(slash)
char *slash;
{
  dirplusmake(slash);
  if (mkdir(dirplus.s,0755) == -1)
    strerr_die4sys(111,FATAL,"unable to create ",dirplus.s,": ");
  keyaddtime();
}

substdio ss;
char ssbuf[SUBSTDIO_OUTSIZE];

void ezfopen(slash)
char *slash;
{
  int fd;

  dirplusmake(slash);
  fd = open_trunc(dirplus.s);
  if (fd == -1)
    strerr_die4sys(111,FATAL,"unable to create ",dirplus.s,": ");

  substdio_fdbuf(&ss,write,fd,ssbuf,sizeof(ssbuf));
}

void ezfput(buf,len)
char *buf;
unsigned int len;
{
  if (substdio_bput(&ss,buf,len) == -1)
    strerr_die4sys(111,FATAL,"unable to write to ",dirplus.s,": ");
}
void ezfputs(buf)
char *buf;
{
  if (substdio_bputs(&ss,buf) == -1)
    strerr_die4sys(111,FATAL,"unable to write to ",dirplus.s,": ");
}

void ezfclose()
{
  if (substdio_flush(&ss) == -1)
    strerr_die4sys(111,FATAL,"unable to write to ",dirplus.s,": ");
  if (fsync(ss.fd) == -1)
    strerr_die4sys(111,FATAL,"unable to write to ",dirplus.s,": ");
  if (close(ss.fd) == -1) /* NFS stupidity */
    strerr_die4sys(111,FATAL,"unable to write to ",dirplus.s,": ");
  keyaddtime();
}

void main(argc,argv)
int argc;
char **argv;
{
  int opt;
  int flagarchived;
  int flagpublic;

  keyadd(getpid());
  keyadd(getppid());
  keyadd(getuid());
  keyadd(getgid());
  gettimeofday(&tv,(struct timezone *) 0);
  keyadd(tv.tv_sec);

  umask(077);

  flagarchived = 1;
  flagpublic = 1;

  while ((opt = getopt(argc,argv,"aApP")) != opteof)
    switch(opt) {
      case 'a': flagarchived = 1; break;
      case 'A': flagarchived = 0; break;
      case 'p': flagpublic = 1; break;
      case 'P': flagpublic = 0; break;
      default:
	die_usage();
    }
  argv += optind;

  if (!(dir = *argv++)) die_usage();
  if (!(dot = *argv++)) die_usage();
  if (!(local = *argv++)) die_usage();
  if (!(host = *argv++)) die_usage();

  if (dir[0] != '/') die_relative();
  if (dir[str_chr(dir,'\'')]) die_quote();
  if (dir[str_chr(dir,'\n')]) die_newline();
  if (local[str_chr(local,'\n')]) die_newline();
  if (host[str_chr(host,'\n')]) die_newline();

  dcreate("");
  dcreate("/archive");
  dcreate("/subscribers");
  dcreate("/bounce");
  dcreate("/text");


  linkdotdir("-owner","/owner");
  linkdotdir("-default","/manager");
  linkdotdir("-return-default","/bouncer");
  linkdotdir("","/editor");

  ezfopen("/lock"); ezfclose();
  ezfopen("/lockbounce"); ezfclose();
  if (flagpublic) {
    ezfopen("/public"); ezfclose();
  }
  if (flagarchived) {
    ezfopen("/archived"); ezfclose();
  }
  ezfopen("/num"); ezfputs("0\n"); ezfclose();
  ezfopen("/inhost"); ezfputs(host); ezfputs("\n"); ezfclose();
  ezfopen("/outhost"); ezfputs(host); ezfputs("\n"); ezfclose();
  ezfopen("/inlocal"); ezfputs(local); ezfputs("\n"); ezfclose();
  ezfopen("/outlocal"); ezfputs(local); ezfputs("\n"); ezfclose();

  ezfopen("/mailinglist");
  ezfputs("contact ");
  ezfputs(local); ezfputs("-help@"); ezfputs(host); ezfputs("; run by ezmlm\n");
  ezfclose();

  ezfopen("/owner");
  ezfputs(dir); ezfputs("/Mailbox\n");
  ezfputs("|"); ezfputs(auto_bin); ezfputs("/ezmlm-warn '"); ezfputs(dir);
  ezfputs("' || exit 0\n");
  ezfclose();

  ezfopen("/manager");
  ezfputs("|"); ezfputs(auto_bin); ezfputs("/ezmlm-manage '"); ezfputs(dir); ezfputs("'\n");
  ezfputs("|"); ezfputs(auto_bin); ezfputs("/ezmlm-warn '"); ezfputs(dir);
  ezfputs("' || exit 0\n");
  ezfclose();

  ezfopen("/editor");
  ezfputs("|"); ezfputs(auto_bin); ezfputs("/ezmlm-reject\n");
  ezfputs("|"); ezfputs(auto_bin); ezfputs("/ezmlm-send '"); ezfputs(dir); ezfputs("'\n");
  ezfputs("|"); ezfputs(auto_bin); ezfputs("/ezmlm-warn '"); ezfputs(dir);
  ezfputs("' || exit 0\n");
  ezfclose();

  ezfopen("/bouncer");
  ezfputs("|"); ezfputs(auto_bin); ezfputs("/ezmlm-warn '"); ezfputs(dir);
  ezfputs("' || exit 0\n");
  ezfputs("|"); ezfputs(auto_bin); ezfputs("/ezmlm-weed\n");
  ezfputs("|"); ezfputs(auto_bin); ezfputs("/ezmlm-return '"); ezfputs(dir); ezfputs("'\n");
  ezfclose();

  ezfopen("/headerremove");
  ezfputs("\
return-path\n\
return-receipt-to\n\
content-length\n\
");
  ezfclose();

  ezfopen("/headeradd");
  ezfclose();


  ezfopen("/text/top");
  ezfputs("Hi! This is the ezmlm program. I'm managing the\n");
  ezfputs(local); ezfputs("@"); ezfputs(host); ezfputs(" mailing list.\n\n");
  ezfclose();

  ezfopen("/text/bottom");
  ezfputs("\n--- Here are the ezmlm command addresses.\n\
\n\
I can handle administrative requests automatically.\n\
Just send an empty note to any of these addresses:\n\n   <");
  ezfputs(local); ezfputs("-subscribe@"); ezfputs(host); ezfputs(">:\n");
  ezfputs("   Receive future messages sent to the mailing list.\n\n   <");
  ezfputs(local); ezfputs("-unsubscribe@"); ezfputs(host); ezfputs(">:\n");
  ezfputs("   Stop receiving messages.\n\n   <");
  ezfputs(local); ezfputs("-get.12345@"); ezfputs(host); ezfputs(">:\n");
  ezfputs("   Retrieve a copy of message 12345 from the archive.\n\
\n\
DO NOT SEND ADMINISTRATIVE REQUESTS TO THE MAILING LIST!\n\
If you do, I won't see them, and subscribers will yell at you.\n\
\n\
To specify God@heaven.af.mil as your subscription address, send mail\n\
to <");
  ezfputs(local); ezfputs("-subscribe-God=heaven.af.mil@"); ezfputs(host);
  ezfputs(">.\n\
I'll send a confirmation message to that address; when you receive that\n\
message, simply reply to it to complete your subscription.\n\
\n");
  ezfputs("\n--- Below this line is a copy of the request I received.\n\n");
  ezfclose();

  ezfopen("/text/sub-confirm");
  ezfputs("To confirm that you would like\n\
\n\
!A\n\
\n\
added to this mailing list, please send an empty reply to this address:\n\
\n\
!R\n\
\n\
Your mailer should have a Reply feature that uses this address automatically.\n\
\n\
This confirmation serves two purposes. First, it verifies that I am able\n\
to get mail through to you. Second, it protects you in case someone\n\
forges a subscription request in your name.\n\
\n");
  ezfclose();

  ezfopen("/text/unsub-confirm");
  ezfputs("To confirm that you would like\n\
\n\
!A\n\
\n\
removed from this mailing list, please send an empty reply to this address:\n\
\n\
!R\n\
\n\
Your mailer should have a Reply feature that uses this address automatically.\n\
\n\
I haven't checked whether your address is currently on the mailing list.\n\
To see what address you used to subscribe, look at the messages you are\n\
receiving from the mailing list. Each message has your address hidden\n\
inside its return path; for example, God@heaven.af.mil receives messages\n\
with return path ...-God=heaven.af.mil.\n\
\n");
  ezfclose();

  ezfopen("/text/sub-ok");
  ezfputs("Acknowledgment: I have added the address\n\
\n\
!A\n\
\n\
to this mailing list.\n\
\n");
  ezfclose();

  ezfopen("/text/unsub-ok");
  ezfputs("Acknowledgment: I have removed the address\n\
\n\
!A\n\
\n\
from this mailing list.\n\
\n");
  ezfclose();

  ezfopen("/text/sub-nop");
  ezfputs("Acknowledgment: The address\n\
\n\
!A\n\
\n\
is on this mailing list.\n\
\n");
  ezfclose();

  ezfopen("/text/unsub-nop");
  ezfputs("Acknowledgment: The address\n\
\n\
!A\n\
\n\
is not on this mailing list.\n\
\n");
  ezfclose();

  ezfopen("/text/sub-bad");
  ezfputs("Oops, that confirmation number appears to be invalid.\n\
\n\
The most common reason for invalid numbers is expiration. I have to\n\
receive confirmation of each request within ten days.\n\
\n\
I've set up a new confirmation number. To confirm that you would like\n\
\n\
!A\n\
\n\
added to this mailing list, please send an empty reply to this address:\n\
\n\
!R\n\
\n\
Sorry for the trouble.\n\
\n");
  ezfclose();

  ezfopen("/text/unsub-bad");
  ezfputs("Oops, that confirmation number appears to be invalid.\n\
\n\
The most common reason for invalid numbers is expiration. I have to\n\
receive confirmation of each request within ten days.\n\
\n\
I've set up a new confirmation number. To confirm that you would like\n\
\n\
!A\n\
\n\
removed from this mailing list, please send an empty reply to this address:\n\
\n\
!R\n\
\n\
Sorry for the trouble.\n\
\n");
  ezfclose();

  ezfopen("/text/get-bad");
  ezfputs("Sorry, I don't see that message.\n\n");
  ezfclose();

  ezfopen("/text/bounce-bottom");
  ezfputs("\n\
--- Below this line is a copy of the bounce message I received.\n\n");
  ezfclose();

  ezfopen("/text/bounce-warn");
  ezfputs("\n\
Messages to you seem to have been bouncing. I've attached a copy of\n\
the first bounce message I received.\n\
\n\
If this message bounces too, I will send you a probe. If the probe bounces,\n\
I will remove your address from the mailing list, without further notice.\n\
\n");
  ezfclose();

  ezfopen("/text/bounce-probe");
  ezfputs("\n\
Messages to you seem to have been bouncing. I sent you a warning\n\
message, but it bounced. I've attached a copy of the bounce message.\n\
\n\
This is a probe to check whether your address is reachable. If this\n\
probe bounces, I will remove your address from the mailing list, without\n\
further notice.\n\
\n");
  ezfclose();

  ezfopen("/text/bounce-num");
  ezfputs("\n\
I've kept a list of which messages bounced from your address. Copies of\n\
these messages may be in the archive. To get message 12345 from the\n\
archive, send an empty note to ");
  ezfputs(local); ezfputs("-get.12345@"); ezfputs(host); ezfputs(".\n\
Here are the message numbers:\n\
\n");
  ezfclose();

  ezfopen("/text/help");
  ezfputs("\
This is a generic help message. The message I received wasn't sent to\n\
any of my command addresses.\n\
\n");
  ezfclose();

  ezfopen("/key");
  ezfput(key.s,key.len);
  ezfclose();

  _exit(0);
}
