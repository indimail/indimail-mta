#include "dkim.h"

int main()
{
#ifdef HASDKIM
DKIMContext     ctxt;
DKIMSignOptions opts = { 0 };
  DKIMSignInit(&ctxt, &opts);
#else
  :
#endif
  return(0);
}
