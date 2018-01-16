#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "strerr.h"
#include "buffer.h"
#include "stralloc.h"
#include "open.h"
#include "str.h"
#include "generic-conf.h"

static const char *fatal;
static const char *dir;
static const char *fn;
static stralloc target;

static int fd;
static char buf[1024];
static buffer ss;

void init(const char *d,const char *f)
{
  dir = d;
  fatal = f;
  umask(022);
  if (mkdir(dir,0700) == -1)
    strerr_die4sys(111,fatal,"unable to create ",dir,": ");
  if (chmod(dir,03755) == -1)
    strerr_die4sys(111,fatal,"unable to set mode of ",dir,": ");
  if (chdir(dir) == -1)
    strerr_die4sys(111,fatal,"unable to switch to ",dir,": ");
}

void failmem()
{
  strerr_die2sys(111,fatal,"out of memory: ");
}

void fail(void)
{
  strerr_die6sys(111,fatal,"unable to create ",dir,"/",fn,": ");
}

void faill(const char *s, const char *t)
{
  strerr_die6sys(111,fatal,"unable to link ",s," to ",t,": ");
}

void makedir(const char *s)
{
  fn = s;
  if (mkdir(fn,0700) == -1) fail();
}

void makelink(const char *s, const char *sdir, const char *t)
{
  if (!stralloc_copys(&target, sdir)) failmem();
  if (!stralloc_catb(&target, "/", 1)) failmem();
  if (!stralloc_cats(&target, t)) failmem();
  if (!stralloc_0(&target)) failmem();
  if (access(target.s, F_OK) && mkdir(target.s,0700) == -1) fail();
  if (access(s, F_OK) && symlink(target.s,s) == -1) faill(s,t);
  fn = target.s;
}

void start(const char *s)
{
  fn = s;
  fd = open_trunc(fn);
  if (fd == -1) fail();
  buffer_init(&ss,buffer_unixwrite,fd,buf,sizeof buf);
}

void outs(const char *s)
{
  if (buffer_puts(&ss,s) == -1) fail();
}

void out(const char *s,unsigned int len)
{
  if (buffer_put(&ss,s,len) == -1) fail();
}

void copyfrom(buffer *b)
{
  if (buffer_copy(&ss,b) < 0) fail();
}

void finish(void)
{
  if (buffer_flush(&ss) == -1) fail();
  if (fsync(fd) == -1) fail();
  close(fd);
}

void perm(mode_t mode)
{
  if (chmod(fn,mode) == -1) fail();
}

void owner(uid_t uid,gid_t gid)
{
  if (chown(fn,uid,gid) == -1) fail();
}

void makelog(const char *sdir, const char *logdir, const char *user,uid_t uid,gid_t gid)
{
  int i;

  makedir("log");
  perm(02755);
  if (sdir) {
    i = str_rchr(sdir,'/');
    makelink("log/main", logdir, i ? sdir + i + 1 : sdir);
  } else
    makedir("log/main");
  owner(uid,gid);
  perm(02755);
  start("log/status");
  finish();
  owner(uid,gid);
  perm(0644);

  start("log/run");
  outs("#!/bin/sh\nexec");
  outs(" setuidgid "); outs(user);
  outs(" multilog t ./main\n");
  finish();
  perm(0755);
}
