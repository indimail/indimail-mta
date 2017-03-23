#include "stralloc.h"
#include "substdio.h"
#include "getln.h"
#include "str.h"
#include "strerr.h"
#include "error.h"
#include "readwrite.h"
#include "exit.h"
#include "open.h"

#define FATAL "ezmlm-list: fatal: "
void die_write()
{
  strerr_die2sys(111,FATAL,"unable to write: ");
}

char outbuf[1024];
substdio out = SUBSTDIO_FDBUF(write,1,outbuf,sizeof(outbuf));
char inbuf[1024];
substdio in;

stralloc line = {0};

char fn[14] = "subscribers/?";

void main(argc,argv)
int argc;
char **argv;
{
  char *dir;
  int fd;
  int match;

  dir = argv[1];
  if (!dir) strerr_die1x(100,"ezmlm-list: usage: ezmlm-list dir");

  if (chdir(dir) == -1)
    strerr_die4sys(111,FATAL,"unable to switch to ",dir,": ");

  for (fn[12] = 64;fn[12] < 64 + 53;++fn[12]) {
    fd = open_read(fn);
    if (fd == -1) {
      if (errno != error_noent)
	strerr_die4sys(111,FATAL,"unable to open ",fn,": ");
    }
    else {
      substdio_fdbuf(&in,read,fd,inbuf,sizeof(inbuf));
      for (;;) {
        if (getln(&in,&line,&match,'\0') == -1)
	  strerr_die4sys(111,FATAL,"unable to read ",fn,": ");
	if (!match) break;
	if (line.s[str_chr(line.s,'\n')])
	  strerr_die3x(111,FATAL,"newline in ",fn);
	if (substdio_puts(&out,line.s + 1)) die_write();
	if (substdio_put(&out,"\n",1) == -1) die_write();
      }
    }

  }

  if (substdio_flush(&out) == -1) die_write();
  _exit(0);
}
