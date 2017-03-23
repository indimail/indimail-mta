#include <unistd.h>
#include "strerr.h"
#include "subscribe.h"
#include "log.h"

#define FATAL "ezmlm-unsub: fatal: "
#define WARNING "ezmlm-unsub: warning: "

void main(argc,argv)
int argc;
char **argv;
{
  char *dir;
  char *addr;

  dir = argv[1];
  if (!dir)
    strerr_die1x(100,"ezmlm-unsub: usage: ezmlm-unsub dir box@domain ...");
  if (chdir(dir) == -1)
    strerr_die4sys(111,FATAL,"unable to switch to ",dir,": ");

  argv += 2;
  while (addr = *argv++)
    switch(subscribe(addr,0)) {
      case -1:
	strerr_die1(111,FATAL,&subscribe_err);
      case -2:
	strerr_warn4(WARNING,"cannot unsubscribe ",addr,": ",&subscribe_err);
	break;
      case 1:
	ezlog("-manual",addr);
    }

  _exit(0);
}
