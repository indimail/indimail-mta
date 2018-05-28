#include <sys/types.h>
#include <unistd.h>
int main()
{
  gid_t x[4];
 
  x[0] = x[1] = 1;
  if (getgroups(1,x) == 0) if (setgroups(1,x) == -1) _exit(1);
 
  if (getgroups(1,x) == -1) _exit(1);
  if (x[1] != 1) _exit(1);
  x[1] = 2;
  if (getgroups(1,x) == -1) _exit(1);
  if (x[1] != 2) _exit(1);
  _exit(0);
}
