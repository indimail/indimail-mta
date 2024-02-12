#include <fcntl.h>
#include <stdio.h>

void main()
{
  renameat(0, "test", 0, "test");
}
