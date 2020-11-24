/*
 * $Log: checkaddr.c,v $
 * Revision 1.3  2020-11-24 13:44:22+05:30  Cprogrammer
 * removed exit.h
 *
 * Revision 1.2  2004-10-22 20:23:50+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.1  2004-07-17 21:25:33+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <unistd.h>
#include "substdio.h"
#include "strerr.h"
#include "getln.h"
#include "mess822.h"
#include "case.h"
#include "env.h"

#define FATAL "checkaddr: fatal: "

void nomem()
{
  strerr_die2x(111,FATAL,"out of memory");
}

stralloc addrlist = {0};
int match;
static char     ssinbuf[1024];
static substdio ssin = SUBSTDIO_FDBUF(read, 0, ssinbuf, sizeof ssinbuf);

char *recipient;
char **recips;

void check(addr)
char *addr;
{
  int i;

  if (recipient)
    if (case_equals(addr,recipient)) _exit(0);

  if (!recipient)
    for (i = 0;recips[i];++i)
      if (case_equals(addr,recips[i])) _exit(0);
}

int main(argc,argv)
int argc;
char **argv;
{
  recipient = env_get("RECIPIENT");
  recips = argv + 1;
  if (*recips) recipient = 0;

	for (;;) {
		if (getln(&ssin,&addrlist,&match,'\0') == -1)
			strerr_die2sys(111,FATAL,"unable to read input: ");
		if (!match) break;
		if (addrlist.s[0] == '+')
			check(addrlist.s + 1);
	}

	_exit(100);
}

void
getversion_checkaddr_c()
{
	static char    *x = "$Id: checkaddr.c,v 1.3 2020-11-24 13:44:22+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
