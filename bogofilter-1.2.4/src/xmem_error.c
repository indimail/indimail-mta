/* $Id: xmem_error.c 4050 2004-02-03 00:59:57Z relson $ */

#include <stdio.h>

#include "xmalloc.h"

/*@noreturn@*/
void xmem_error(const char *a)
{
    (void)fprintf(stderr, "%s: Out of memory\n", a);
    abort();
}

/*@noreturn@*/
void xmem_toolong(const char *a)
{
    (void)fprintf(stderr, "%s: String too long\n", a);
    abort();
}
