#include "scan.h"

unsigned int scan_u32(const char *s,uint32 *u)
{
  unsigned long l;
  unsigned int r=scan_ulong(s,&l);
  if ((uint32)l != l) return 0;
  if (r) *u=l;
  return r;
}

