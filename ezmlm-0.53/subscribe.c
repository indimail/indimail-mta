#include <stdio.h>
#include "stralloc.h"
#include "getln.h"
#include "readwrite.h"
#include "substdio.h"
#include "strerr.h"
#include "open.h"
#include "str.h"
#include "case.h"
#include "byte.h"
#include "lock.h"
#include "error.h"
#include "uint32.h"
#include "subscribe.h"

static stralloc addr = {0};
static stralloc line = {0};
static stralloc fnnew = {0};
static stralloc fn = {0};

static int fd;
static substdio ss;
static char ssbuf[256];
static int fdnew;
static substdio ssnew;
static char ssnewbuf[256];

static int doit(userhost,flagadd)
char *userhost;
int flagadd;
{
  int j;
  uint32 h;
  char ch;
  int match;
  int flagwasthere;

  if (userhost[str_chr(userhost,'\n')]) return -8;
  if (!stralloc_copys(&addr,"T")) return -2;
  if (!stralloc_cats(&addr,userhost)) return -2;
  if (addr.len > 401) return -7;

  j = byte_rchr(addr.s,addr.len,'@');
  if (j == addr.len) return -6;
  case_lowerb(addr.s + j + 1,addr.len - j - 1);

  h = 5381;
  for (j = 0;j < addr.len;++j)
    h = (h + (h << 5)) ^ (uint32) (unsigned char) addr.s[j];
  ch = 64 + (h % 53);

  if (!stralloc_0(&addr)) return -2;

  if (!stralloc_copys(&fn,"subscribers/")) return -2;
  if (!stralloc_catb(&fn,&ch,1)) return -2;
  if (!stralloc_copy(&fnnew,&fn)) return -2;
  if (!stralloc_cats(&fnnew,"n")) return -2;
  if (!stralloc_0(&fnnew)) return -2;
  if (!stralloc_0(&fn)) return -2;

  fdnew = open_trunc(fnnew.s);
  if (fdnew == -1) return -4;
  substdio_fdbuf(&ssnew,write,fdnew,ssnewbuf,sizeof(ssnewbuf));

  flagwasthere = 0;

  fd = open_read(fn.s);
  if (fd == -1) {
    if (errno != error_noent) { close(fdnew); return -3; }
  }
  else {
    substdio_fdbuf(&ss,read,fd,ssbuf,sizeof(ssbuf));

    for (;;) {
      if (getln(&ss,&line,&match,'\0') == -1) { close(fd); close(fdnew); return -3; }
      if (!match) break;
      if (line.len == addr.len)
        if (!byte_diff(line.s,line.len,addr.s)) {
	  flagwasthere = 1;
	  if (!flagadd)
	    continue;
	}
      if (substdio_bput(&ssnew,line.s,line.len) == -1) { close(fd); close(fdnew); return -4; }
    }

    close(fd);
  }

  if (flagadd && !flagwasthere)
    if (substdio_bput(&ssnew,addr.s,addr.len) == -1) { close(fdnew); return -4; }

  if (substdio_flush(&ssnew) == -1) { close(fdnew); return -4; }
  if (fsync(fdnew) == -1) { close(fdnew); return -4; }
  close(fdnew);

  if (rename(fnnew.s,fn.s) == -1) return -5;
  return flagadd ^ flagwasthere;
}

struct strerr subscribe_err;

int subscribe(userhost,flagadd)
char *userhost;
int flagadd;
{
  int fdlock;
  int r;

  fdlock = open_append("lock");
  if (fdlock == -1)
    STRERR_SYS(-1,subscribe_err,"unable to open lock: ")
  if (lock_ex(fdlock) == -1) {
    close(fdlock);
    STRERR_SYS(-1,subscribe_err,"unable to obtain lock: ")
  }

  r = doit(userhost,flagadd);
  close(fdlock);

  if (r == -2) STRERR(-1,subscribe_err,"out of memory")
  if (r == -3) STRERR_SYS3(-1,subscribe_err,"unable to read ",fn.s,": ")
  if (r == -4) STRERR_SYS3(-1,subscribe_err,"unable to write ",fnnew.s,": ")
  if (r == -5) STRERR_SYS3(-1,subscribe_err,"unable to move temporary file to ",fn.s,": ")
  if (r == -6) STRERR(-2,subscribe_err,"address does not contain @")
  if (r == -7) STRERR(-2,subscribe_err,"address is too long")
  if (r == -8) STRERR(-2,subscribe_err,"address contains newline")

  return r;
}
