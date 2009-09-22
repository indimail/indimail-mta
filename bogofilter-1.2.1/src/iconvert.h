/* $Id: iconvert.h 6127 2005-06-25 23:23:35Z relson $ */

/*****************************************************************************

NAME:
   iconvert.h -- declarations for iconvert.c

AUTHOR:
   David Relson <relson@osagesoftware.com>

******************************************************************************/

#ifndef ICONVERT_H
#define ICONVERT_H

#include <iconv.h>

extern void iconvert(buff_t *src, buff_t *dst);
extern void iconvert_cd(iconv_t cd, buff_t *src, buff_t *dst);

#endif
