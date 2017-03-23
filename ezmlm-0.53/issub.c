#include "stralloc.h"
#include "getln.h"
#include "readwrite.h"
#include "substdio.h"
#include "open.h"
#include "byte.h"
#include "case.h"
#include "lock.h"
#include "error.h"
#include "issub.h"
#include "uint32.h"

static stralloc addr = {0};
static stralloc line = {0};
static stralloc fn = {0};
static int fd;
static substdio ss;
static char ssbuf[256];

static int doit(userhost)
char *userhost;
{
  int j;
  uint32 h;
  char ch;
  int match;

  if (!stralloc_copys(&addr,"T")) return -2;
  if (!stralloc_cats(&addr,userhost)) return -2;

  j = byte_rchr(addr.s,addr.len,'@');
  if (j == addr.len) return 0;
  case_lowerb(addr.s + j + 1,addr.len - j - 1);

  h = 5381;
  for (j = 0;j < addr.len;++j)
    h = (h + (h << 5)) ^ (uint32) (unsigned char) addr.s[j];
  ch = 64 + (h % 53);

  if (!stralloc_0(&addr)) return -2;

  if (!stralloc_copys(&fn,"subscribers/")) return -2;
  if (!stralloc_catb(&fn,&ch,1)) return -2;
  if (!stralloc_0(&fn)) return -2;

  fd = open_read(fn.s);
  if (fd == -1) {
    if (errno != error_noent) return -3;
    return 0;
  }
  substdio_fdbuf(&ss,read,fd,ssbuf,sizeof(ssbuf));

  for (;;) {
    if (getln(&ss,&line,&match,'\0') == -1) { close(fd); return -3; }
    if (!match) break;
    if (line.len == addr.len)
      if (!byte_diff(line.s,line.len,addr.s)) { close(fd); return 1; }
  }

  close(fd);
  return 0;
}

struct strerr issub_err;

int issub(userhost)
char *userhost;
{
  int fdlock;
  int r;

  fdlock = open_append("lock");
  if (fdlock == -1)
    STRERR_SYS(-1,issub_err,"unable to open lock: ")
  if (lock_ex(fdlock) == -1) {
    close(fdlock);
    STRERR_SYS(-1,issub_err,"unable to obtain lock: ")
  }

  r = doit(userhost);
  close(fdlock);

  if (r == -2) STRERR(-1,issub_err,"out of memory")
  if (r == -3) STRERR_SYS3(-1,issub_err,"unable to read ",fn.s,": ")

  return r;
}
