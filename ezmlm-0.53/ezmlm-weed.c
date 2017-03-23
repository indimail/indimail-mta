#include "stralloc.h"
#include "str.h"
#include "byte.h"
#include "readwrite.h"
#include "substdio.h"
#include "getln.h"
#include "strerr.h"

char buf0[256];
substdio ss0 = SUBSTDIO_FDBUF(read,0,buf0,sizeof(buf0));

#define FATAL "ezmlm-weed: fatal: "

void get(sa)
stralloc *sa;
{
  int match;
  if (getln(&ss0,sa,&match,'\n') == -1)
    strerr_die2sys(111,FATAL,"unable to read input: ");
  if (!match) _exit(0);
}

stralloc line = {0};
stralloc line1 = {0};
stralloc line2 = {0};
stralloc line3 = {0};
stralloc line4 = {0};
stralloc line5 = {0};
stralloc line6 = {0};
stralloc line7 = {0};
stralloc line8 = {0};

char warn1[] = "    **********************************************";
char warn2[] = "    **      THIS IS A WARNING MESSAGE ONLY      **";
char warn3[] = "    **  YOU DO NOT NEED TO RESEND YOUR MESSAGE  **";
char warn4[] = "    **********************************************";

int flagmds = 0;
int flagsw = 0;
int flagsr = 0;
int flagas = 0;
int flagbw = 0;

void main()
{
  int match;

  for (;;) {
    get(&line);
    if (line.len == 1) break;

    if (stralloc_starts(&line,"Subject: success notice"))
      _exit(99);
    if (stralloc_starts(&line,"Subject: deferral notice"))
      _exit(99);

    if (stralloc_starts(&line,"From: Mail Delivery Subsystem <MAILER-DAEMON@"))
      flagmds = 1;
    if (stralloc_starts(&line,"Subject: Warning: could not send message"))
      flagsw = 1;
    if (stralloc_starts(&line,"Subject: Returned mail: warning: cannot send message"))
      flagsr = 1;
    if (stralloc_starts(&line,"Auto-Submitted: auto-generated (warning"))
      flagas = 1;
  }

  get(&line1);
  get(&line2);
  get(&line3);
  get(&line4);
  get(&line5);
  get(&line6);
  get(&line7);
  get(&line8);

  if (stralloc_starts(&line1,"This is a MIME-encapsulated message"))
  if (stralloc_starts(&line3,"--"))
  if (stralloc_starts(&line5,warn1))
  if (stralloc_starts(&line6,warn2))
  if (stralloc_starts(&line7,warn3))
  if (stralloc_starts(&line8,warn4))
    flagbw = 1;

  if (stralloc_starts(&line1,warn1))
  if (stralloc_starts(&line2,warn2))
  if (stralloc_starts(&line3,warn3))
  if (stralloc_starts(&line4,warn4))
    flagbw = 1;

  if (flagmds && flagsw && flagas && flagbw) _exit(99);
  if (flagmds && flagsr && flagbw) _exit(99);

  _exit(0);
}
