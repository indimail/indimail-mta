#include <unistd.h>
#include <pwd.h>
#include "strerr.h"
#include "buffer.h"
#include "exit.h"
#include "open.h"
#include "auto_home.h"
#include "generic-conf.h"

#define FATAL "curvedns-conf: fatal: "

void usage(void)
{
  strerr_die1x(100,"curvedns-conf: usage: curvedns-conf acct logacct /curvedns myip [myport targetip targetport]");
}

char *dir;
char *user;
char *loguser;
struct passwd *pw;
const char *myip, *myport, *targetip, *targetport;
int privatekeyfd;
char pbuf[64];
buffer sspbuf;

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
  myport = argv[5];
  if (!myport) {
    myport = "53";
    targetip = "127.0.0.1";
    targetport = "53";
  } else {
    targetip = argv[6];
    if (!targetip) {
      targetip = "127.0.0.1";
      targetport = "53";
    } else {
      targetport = argv[7];
      if (!targetport)
        targetport = "53";
    }
  }

  pw = getpwnam(loguser);
  if (!pw)
    strerr_die3x(111,FATAL,"unknown account ",loguser);

  if (chdir(pw->pw_dir) == -1)
    strerr_die4sys(111,FATAL,"unable to switch to ",pw->pw_dir,": ");
  privatekeyfd = open_read("curvedns.private.key");
  if (privatekeyfd == -1)
    strerr_die2sys(111,FATAL,"unable to open curvedns.private.key: ");
  init(dir,FATAL);
  makelog(loguser,pw->pw_uid,pw->pw_gid);

  makedir("env");
  perm(02755);
  start("env/CURVEDNS_PRIVATE_KEY_FILE"); outs(dir); outs("/private.key\n"); finish();
  perm(0600);
  start("env/LISTEN_IPS"); outs(myip); outs("\n"); finish();
  perm(0600);
  start("env/LISTEN_PORT"); outs(myport); outs("\n"); finish();
  perm(0600);
  start("env/TARGET_IP"); outs(targetip); outs("\n"); finish();
  perm(0600);
  start("env/TARGET_PORT"); outs(targetport); outs("\n"); finish();
  perm(0600);

  start("run");
  outs("#!/bin/sh\nexec 2>&1\nexec envdir ./env sh -c '\n  exec envuidgid "); outs(user);
  outs(" softlimit -o250 -d \"$DATALIMIT\" ");
  outs(auto_home); outs("/bin/curvedns $LISTEN_IPS $LISTEN_PORT $TARGET_IP $TARGET_PORT\n'\n"); finish();
  perm(0755);
  start("log/run");
  outs("#!/bin/sh\nexec setuidgid "); outs(loguser);
  outs(" multilog t ./main\n"); finish();
  start("private.key");
  buffer_init(&sspbuf,buffer_unixread,privatekeyfd,pbuf,sizeof pbuf);
  copyfrom(&sspbuf);
  finish();
  perm(0600);
  _exit(0);
}
