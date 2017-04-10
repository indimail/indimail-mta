#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "buffer.h"
#include "stralloc.h"
#include "strerr.h"
#include "error.h"
#include "open.h"
#include "exit.h"
#include "sgetopt.h"

extern void hier(char *);

char           *destdir = 0, *mandir = 0;
const char     *usage = "usage: setup -d destdir [-m mandir] [instdir]";
stralloc        tmpdir = {0};
stralloc        dirbuf = { 0 };
stralloc        dird = { 0 };

#define FATAL "setup: fatal: "

int fdsourcedir = -1;

void
l(home, file, target)
	char           *home;
	char           *file;
	char           *target;
{
	if (destdir) {
		if (!stralloc_copys(&tmpdir, destdir))
			strerr_die2sys(111, FATAL, "out of memory: ");
		if (!stralloc_cats(&tmpdir, home))
			strerr_die2sys(111, FATAL, "out of memory: ");
	} else
	if (!stralloc_copys(&tmpdir, home))
		strerr_die2sys(111, FATAL, "out of memory: ");
	if (!stralloc_0(&tmpdir))
		strerr_die2sys(111, FATAL, "out of memory: ");
	if (chdir(tmpdir.s) == -1)
		strerr_die4sys(111, FATAL, "unable to switch to ", tmpdir.s, ": ");

	if (!stralloc_copys(&dird, target))
		strerr_die2sys(111, FATAL, "out of memory: ");
	else
	if (!stralloc_append(&dird, "/"))
		strerr_die2sys(111, FATAL, "out of memory: ");
	if (!stralloc_cats(&dird, file))
		strerr_die2sys(111, FATAL, "out of memory: ");
	else
	if (!stralloc_append(&dird, "/"))
		strerr_die2sys(111, FATAL, "out of memory: ");
	else
	if (!stralloc_0(&dird))
		strerr_die2sys(111, FATAL, "out of memory: ");

	if (symlink(dird.s, file) == -1 && errno != error_exist)
		strerr_die6sys(111, FATAL, "unable to symlink ", file, " to ", dird.s, ": ");
}

int
myr_mkdir(home, mode)
	char           *home;
	int             mode;
{
	char           *ptr;
	int             i;

	if (!stralloc_copys(&dirbuf, home))
		strerr_die2sys(111, FATAL, "out of memory: ");
	if (!stralloc_0(&dirbuf))
		strerr_die2sys(111, FATAL, "out of memory: ");
	for (ptr = dirbuf.s + 1;*ptr;ptr++) {
		if (*ptr == '/') {
			*ptr = 0;
			if (access(dirbuf.s, F_OK) && (i = mkdir(dirbuf.s, mode)) == -1)
				return (i);
			*ptr = '/';
		}
	}
	return (mkdir(dirbuf.s, mode));
}

void h(home,uid,gid,mode)
char *home;
int uid;
int gid;
int mode;
{
  if (destdir) {
    if (!stralloc_copys(&tmpdir, destdir))
      strerr_die2sys(111,FATAL,"out of memory");
    if (!stralloc_cats(&tmpdir, home))
      strerr_die2sys(111,FATAL,"out of memory");
  } else
  if (!stralloc_copys(&tmpdir, home))
    strerr_die2sys(111,FATAL,"out of memory");
  if (!stralloc_0(&tmpdir))
    strerr_die2sys(111,FATAL,"out of memory");
  if (myr_mkdir(tmpdir.s,0700) == -1)
    if (errno != error_exist)
      strerr_die4sys(111,FATAL,"h(): unable to mkdir ",tmpdir.s,": ");
  if (chown(tmpdir.s,uid,gid) == -1)
    strerr_die4sys(111,FATAL,"unable to chown ",tmpdir.s,": ");
  if (chmod(tmpdir.s,mode) == -1)
    strerr_die4sys(111,FATAL,"unable to chmod ",tmpdir.s,": ");
}

void d(home,subdir,uid,gid,mode)
char *home;
char *subdir;
int uid;
int gid;
int mode;
{
  if (destdir) {
    if (!stralloc_copys(&tmpdir, destdir))
      strerr_die2sys(111,FATAL,"out of memory");
    if (!stralloc_cats(&tmpdir, home))
      strerr_die2sys(111,FATAL,"out of memory");
  } else
  if (!stralloc_copys(&tmpdir, home))
    strerr_die2sys(111,FATAL,"out of memory");
  if (!stralloc_0(&tmpdir))
    strerr_die2sys(111,FATAL,"out of memory");

  if (chdir(tmpdir.s) == -1)
    strerr_die4sys(111,FATAL,"unable to switch to ",tmpdir.s,": ");
  if (mkdir(subdir,0700) == -1)
    if (errno != error_exist)
      strerr_die6sys(111,FATAL,"d(): unable to mkdir ",tmpdir.s,"/",subdir,": ");
  if (chown(subdir,uid,gid) == -1)
    strerr_die6sys(111,FATAL,"unable to chown ",tmpdir.s,"/",subdir,": ");
  if (chmod(subdir,mode) == -1)
    strerr_die6sys(111,FATAL,"unable to chmod ",tmpdir.s,"/",subdir,": ");
}

char inbuf[BUFFER_INSIZE];
char outbuf[BUFFER_OUTSIZE];
buffer ssin;
buffer ssout;

void c(home,subdir,file,uid,gid,mode)
char *home;
char *subdir;
char *file;
int uid;
int gid;
int mode;
{
  int fdin;
  int fdout;

  if (fchdir(fdsourcedir) == -1)
    strerr_die2sys(111,FATAL,"unable to switch back to source directory: ");

  fdin = open_read(file);
  if (fdin == -1)
    strerr_die4sys(111,FATAL,"unable to read ",file,": ");
  buffer_init(&ssin,buffer_unixread,fdin,inbuf,sizeof inbuf);

  if (destdir) {
    if (!stralloc_copys(&tmpdir, destdir))
      strerr_die2sys(111,FATAL,"out of memory");
    if (!stralloc_cats(&tmpdir, home))
      strerr_die2sys(111,FATAL,"out of memory");
  } else
  if (!stralloc_copys(&tmpdir, home))
    strerr_die2sys(111,FATAL,"out of memory");
  if (!stralloc_0(&tmpdir))
    strerr_die2sys(111,FATAL,"out of memory");

  if (chdir(tmpdir.s) == -1)
    strerr_die4sys(111,FATAL,"unable to switch to ",tmpdir.s,": ");
  if (chdir(subdir) == -1)
    strerr_die6sys(111,FATAL,"unable to switch to ",tmpdir.s,"/",subdir,": ");

  fdout = open_trunc(file);
  if (fdout == -1)
    strerr_die6sys(111,FATAL,"unable to write .../",subdir,"/",file,": ");
  buffer_init(&ssout,buffer_unixwrite,fdout,outbuf,sizeof outbuf);

  switch(buffer_copy(&ssout,&ssin)) {
    case -2:
      strerr_die4sys(111,FATAL,"unable to read ",file,": ");
    case -3:
      strerr_die6sys(111,FATAL,"unable to write .../",subdir,"/",file,": ");
  }

  close(fdin);
  if (buffer_flush(&ssout) == -1)
    strerr_die6sys(111,FATAL,"unable to write .../",subdir,"/",file,": ");
  if (fsync(fdout) == -1)
    strerr_die6sys(111,FATAL,"unable to write .../",subdir,"/",file,": ");
  if (close(fdout) == -1) /* NFS silliness */
    strerr_die6sys(111,FATAL,"unable to write .../",subdir,"/",file,": ");

  if (chown(file,uid,gid) == -1)
    strerr_die6sys(111,FATAL,"unable to chown .../",subdir,"/",file,": ");
  if (chmod(file,mode) == -1)
    strerr_die6sys(111,FATAL,"unable to chmod .../",subdir,"/",file,": ");
}

void z(home,subdir,file,len,uid,gid,mode)
char *home;
char *subdir;
char *file;
int len;
int uid;
int gid;
int mode;
{
  int fdout;

  if (destdir) {
    if (!stralloc_copys(&tmpdir, destdir))
      strerr_die2sys(111,FATAL,"out of memory");
    if (!stralloc_cats(&tmpdir, home))
      strerr_die2sys(111,FATAL,"out of memory");
  } else
  if (!stralloc_copys(&tmpdir, home))
    strerr_die2sys(111,FATAL,"out of memory");
  if (!stralloc_0(&tmpdir))
    strerr_die2sys(111,FATAL,"out of memory");

  if (chdir(tmpdir.s) == -1)
    strerr_die4sys(111,FATAL,"unable to switch to ",tmpdir.s,": ");
  if (chdir(subdir) == -1)
    strerr_die6sys(111,FATAL,"unable to switch to ",tmpdir.s,"/",subdir,": ");

  fdout = open_trunc(file);
  if (fdout == -1)
    strerr_die6sys(111,FATAL,"unable to write .../",subdir,"/",file,": ");
  buffer_init(&ssout,buffer_unixwrite,fdout,outbuf,sizeof outbuf);

  while (len-- > 0)
    if (buffer_put(&ssout,"",1) == -1)
      strerr_die6sys(111,FATAL,"unable to write .../",subdir,"/",file,": ");

  if (buffer_flush(&ssout) == -1)
    strerr_die6sys(111,FATAL,"unable to write .../",subdir,"/",file,": ");
  if (fsync(fdout) == -1)
    strerr_die6sys(111,FATAL,"unable to write .../",subdir,"/",file,": ");
  if (close(fdout) == -1) /* NFS silliness */
    strerr_die6sys(111,FATAL,"unable to write .../",subdir,"/",file,": ");

  if (chown(file,uid,gid) == -1)
    strerr_die6sys(111,FATAL,"unable to chown .../",subdir,"/",file,": ");
  if (chmod(file,mode) == -1)
    strerr_die6sys(111,FATAL,"unable to chmod .../",subdir,"/",file,": ");
}

int main(int argc, char **argv)
{
  int             opt;

  fdsourcedir = open_read(".");
  if (fdsourcedir == -1)
    strerr_die2sys(111,FATAL,"unable to open current directory: ");
  while ((opt = getopt(argc,argv,"d:m:")) != opteof) {
		switch (opt) {
		case 'd':
			destdir = optarg;
			break;
		case 'm':
			mandir = optarg;
			break;
		default:
			strerr_die1x(100, usage);
		}
  }

  umask(077);
  if (optind + 1 != argc)
    hier(0);
  else
    hier(argv[optind++]);
  _exit(0);
}
