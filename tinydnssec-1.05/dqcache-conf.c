#include <unistd.h>
#include <pwd.h>
#include "strerr.h"
#include "error.h"
#include "buffer.h"
#include "exit.h"
#include "open.h"
#include "auto_home.h"
#include "auto_sysconfdir.h"
#include "generic-conf.h"

#define FATAL "dqcache-conf: fatal: "

void usage(void)
{
  strerr_die1x(100,"dqcache-conf: usage: dqcache-conf acct logacct /dqcache myip");
}

int fdrootservers;
char rootserversbuf[64];
buffer ssrootservers;

char *dir;
char *user;
char *loguser;
struct passwd *pw;
char *myip;

int main(int argc,char **argv)
{
  user = argv[1];
  if (!user) usage();
  loguser = argv[2];
  if (!loguser) usage();
  dir = argv[3];
  if (!dir) usage();
  if (dir[0] != '/') usage();
  myip = argv[4];
  if (!myip) usage();

  pw = getpwnam(loguser);
  if (!pw)
    strerr_die3x(111,FATAL,"unknown account ",loguser);

  if (chdir(auto_sysconfdir) == -1)
    strerr_die4sys(111,FATAL,"unable to switch to ",auto_sysconfdir,": ");
  fdrootservers = open_read("dnsroots.local");
  if (fdrootservers == -1) {
    if (errno != error_noent)
      strerr_die2sys(111,FATAL,"unable to open dnsroots.local: ");
    fdrootservers = open_read("dnsroots.global");
    if (fdrootservers == -1)
      strerr_die2sys(111,FATAL,"unable to open dnsroots.global: ");
  }

  init(dir,FATAL);
  makelog(dir,pw->pw_dir,loguser,pw->pw_uid,pw->pw_gid);

  makedir("env");
  perm(02755);
  start("env/ROOT"); outs(dir); outs("/root\n"); finish();
  perm(0644);
  start("env/IP"); outs(myip); outs("\n"); finish();
  perm(0644);
  start("env/CACHESIZE"); outs("10000000"); outs("\n"); finish();
  perm(0644);

  start("run");
  outs("#!/bin/sh\nexec 2>&1\nexec envuidgid "); outs(user);
  outs(" envdir ./env softlimit -d30000000 ");
  outs(auto_home); outs("/bin/dqcache\n");
  finish();
  perm(0755);
  makedir("root");
  perm(02755);
  makedir("root/ip");
  perm(02755);
  start("root/ip/127.0.0.1"); finish();
  perm(0600);
  makedir("root/servers");
  perm(02755);
  start("root/servers/@");
  buffer_init(&ssrootservers,buffer_unixread,fdrootservers,rootserversbuf,sizeof rootserversbuf);
  copyfrom(&ssrootservers);
  finish();
  perm(0644);

  _exit(0);
}
