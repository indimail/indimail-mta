#include "strerr.h"
#include "case.h"
#include "substdio.h"
#include "readwrite.h"
#include "stralloc.h"
#include "getln.h"
#include "sgetopt.h"

int flagrejectcommands = 1;
int flagneedsubject = 1;

int flaghavesubject = 0;
int flaghavecommand = 0;

char buf0[256];
substdio ss0 = SUBSTDIO_FDBUF(read,0,buf0,sizeof(buf0));
stralloc line = {0};

void main(argc,argv)
int argc;
char **argv;
{
  int opt;
  char *x;
  int len;
  int match;

  while ((opt = getopt(argc,argv,"cCsS")) != opteof)
    switch(opt) {
      case 'c': flagrejectcommands = 1; break;
      case 'C': flagrejectcommands = 0; break;
      case 's': flagneedsubject = 1; break;
      case 'S': flagneedsubject = 0; break;
      default:
	strerr_die1x(100,"ezmlm-reject: usage: ezmlm-reject [ -cCsS ]");
    }

  for (;;) {
    if (getln(&ss0,&line,&match,'\n') == -1)
      strerr_die2sys(111,"ezmlm-reject: fatal: ","unable to read input: ");
    if (!match) break;
    if (line.len == 1) break;

    x = line.s; len = line.len - 1;
    while (len && ((x[len - 1] == ' ') || (x[len - 1] == '\t'))) --len;

    if (case_startb(x,len,"subject:")) {
      x += 8; len -= 8;
      while (len && ((*x == ' ') || (*x == '\t'))) { ++x; --len; }
      if (len) {
        flaghavesubject = 1;

        if (len == 4)
          if (!case_diffb("help",4,x))
            flaghavecommand = 1;

        if (len == 9)
          if (!case_diffb("subscribe",9,x))
            flaghavecommand = 1;

        if (len == 11)
          if (!case_diffb("unsubscribe",11,x))
            flaghavecommand = 1;
      }
    }
  }

  if (flagneedsubject && !flaghavesubject)
    strerr_die1x(100,"\
ezmlm-reject: fatal: I need a nonempty Subject line in every message.\n\
If you are trying to subscribe or unsubscribe, WRONG ADDRESS!\n\
Do not send administrative requests to the mailing list.\n\
Send an empty message to ...-help@... for automated assistance.");

  if (flagrejectcommands && flaghavecommand)
    strerr_die1x(100,"\
ezmlm-reject: fatal: Your Subject line looks like a command word.\n\
If you are trying to subscribe or unsubscribe, WRONG ADDRESS!\n\
Do not send administrative requests to the mailing list.\n\
Send an empty message to ...-help@... for automated assistance.");

  _exit(0);
}
