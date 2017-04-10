#include <sys/types.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <unistd.h>
#include "strerr.h"
#include "error.h"
#include "exit.h"
#include "stralloc.h"
#include "str.h"

extern void hier();

#define FATAL "instcheck: fatal: "
#define WARNING "instcheck: warning: "
const char     *mandir = "/usr/share/man";
stralloc        tfile = {0};

void
l(home, file, target)
	char           *home;
	char           *file;
	char           *target;
{
  char buf[MAXPATHLEN];
  if (chdir(home) == -1)
    strerr_die4sys(111,FATAL,"unable to switch to ",home,": ");
  if (readlink(file, buf, sizeof(buf)) == -1)
    strerr_die6sys(111,FATAL,"unable to readlink ",home, "/", file,": ");
}

void perm(prefix1,prefix2,prefix3,file,type,uid,gid,mode)
char *prefix1;
char *prefix2;
char *prefix3;
char *file;
int type;
int uid;
int gid;
int mode;
{
  struct stat st;

  if (stat(file,&st) == -1) {
    if (errno == error_noent) {
      if (!str_diffn(prefix2, "man", 3)) {/*- check for .gz extension */
		if (!stralloc_copys(&tfile, file))
          strerr_die2sys(111, FATAL,"out of memory: ");
		if (!stralloc_catb(&tfile, ".gz", 4))
          strerr_die2sys(111, FATAL,"out of memory: ");
        if (stat(tfile.s,&st) == -1) {
          if (errno == error_noent) {
            strerr_warn6(WARNING,prefix1,prefix2,prefix3,file," does not exist",0);
          } else
            strerr_warn4(WARNING,"unable to stat .../",file,": ",&strerr_sys);
        }
      } else
        strerr_warn6(WARNING,prefix1,prefix2,prefix3,file," does not exist",0);
    } else
      strerr_warn4(WARNING,"unable to stat .../",file,": ",&strerr_sys);
    return;
  }

  if ((uid != -1) && (st.st_uid != uid))
    strerr_warn6(WARNING,prefix1,prefix2,prefix3,file," has wrong owner",0);
  if ((gid != -1) && (st.st_gid != gid))
    strerr_warn6(WARNING,prefix1,prefix2,prefix3,file," has wrong group",0);
  if ((mode != -1) && ((st.st_mode & 07777) != mode))
    strerr_warn6(WARNING,prefix1,prefix2,prefix3,file," has wrong permissions",0);
  if ((mode != -1) && ((st.st_mode & S_IFMT) != type))
    strerr_warn6(WARNING,prefix1,prefix2,prefix3,file," has wrong type",0);
}

void h(home,uid,gid,mode)
char *home;
int uid;
int gid;
int mode;
{
  perm("","","",home,S_IFDIR,uid,gid,mode);
}

void d(home,subdir,uid,gid,mode)
char *home;
char *subdir;
int uid;
int gid;
int mode;
{
  if (chdir(home) == -1)
    strerr_die4sys(111,FATAL,"unable to switch to ",home,": ");
  perm("",home,"/",subdir,S_IFDIR,uid,gid,mode);
}

void p(home,fifo,uid,gid,mode)
char *home;
char *fifo;
int uid;
int gid;
int mode;
{
  if (chdir(home) == -1)
    strerr_die4sys(111,FATAL,"unable to switch to ",home,": ");
  perm("",home,"/",fifo,S_IFIFO,uid,gid,mode);
}

void c(home,subdir,file,uid,gid,mode)
char *home;
char *subdir;
char *file;
int uid;
int gid;
int mode;
{
  if (chdir(home) == -1)
    strerr_die4sys(111,FATAL,"unable to switch to ",home,": ");
  if (chdir(subdir) == -1)
    strerr_die6sys(111,FATAL,"unable to switch to ",home,"/",subdir,": ");
  perm(".../",subdir,"/",file,S_IFREG,uid,gid,mode);
}

void z(home,file,len,uid,gid,mode)
char *home;
char *file;
int len;
int uid;
int gid;
int mode;
{
  if (chdir(home) == -1)
    strerr_die4sys(111,FATAL,"unable to switch to ",home,": ");
  perm("",home,"/",file,S_IFREG,uid,gid,mode);
}

int main()
{
  hier(0);
  _exit(0);
}
