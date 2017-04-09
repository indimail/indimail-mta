#include <sys/types.h>
#include <sys/time.h>
#include <utime.h>
#include "scan.h"
#include "exit.h"

char *fn;

char *ustr;
unsigned long u;

int main(int argc,char **argv)
{
  struct utimbuf ut;

  if (!(fn = argv[1]))
    _exit(100);

  if (!(ustr = argv[2]))
    _exit(100);
  scan_ulong(ustr,&u);

  ut.actime = ut.modtime = u;
  if (utime(fn,&ut) == -1)
    _exit(111);
  _exit(0);
}
