#include "substdio.h"
#include "readwrite.h"
#include "exit.h"

char buf1[256];
substdio ss1 = SUBSTDIO_FDBUF(write,1,buf1,sizeof(buf1));

void ezputs(s)
char *s;
{
  if (substdio_puts(&ss1,s) == -1) _exit(111);
}

void main(argc,argv)
int argc;
char **argv;
{
  char *name;
  char *value;
  unsigned char ch;
  char octal[4];

  name = argv[1];
  if (!name) _exit(100);
  value = argv[2];
  if (!value) _exit(100);

  ezputs("char ");
  ezputs(name);
  ezputs("[] = \"\\\n");

  while (ch = *value++) {
    ezputs("\\");
    octal[3] = 0;
    octal[2] = '0' + (ch & 7); ch >>= 3;
    octal[1] = '0' + (ch & 7); ch >>= 3;
    octal[0] = '0' + (ch & 7);
    ezputs(octal);
  }

  ezputs("\\\n\";\n");
  if (substdio_flush(&ss1) == -1) _exit(111);
  _exit(0);
}
