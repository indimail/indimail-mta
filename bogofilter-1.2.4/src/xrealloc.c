/* $Id: xrealloc.c 5649 2005-03-16 00:52:05Z relson $ */

/*
* NAME:
*    xrealloc.c -- front-end to standard heap manipulation routines, with error checking.
*
* AUTHOR:
*    Gyepi Sam <gyepi@praxis-sw.com>
*
*/

#include "config.h"

#include "xmalloc.h"

void
*xrealloc(void *ptr, size_t size){
   ptr = bf_realloc(ptr, size);
   if (ptr == NULL && size == 0)
       ptr = bf_calloc(1, 1);
   if (ptr == NULL)
       xmem_error("xrealloc");
   return ptr;
}
