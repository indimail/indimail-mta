#include "substdio.h"
#include <sys/types.h>

void substdio_fdbuf(s,op,fd,buf,len)
register substdio *s;
register ssize_t (*op)();
register int fd;
register char *buf;
register int len;
{
  s->x = buf;
  s->fd = fd;
  s->op = op;
  s->p = 0;
  s->n = len;
}
