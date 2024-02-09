#include <unistd.h>

void main()
{

  unlinkat(-1, "test", 0);
}
