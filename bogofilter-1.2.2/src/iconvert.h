/* $Id: iconvert.h 6888 2010-03-14 16:24:40Z m-a $ */

/*****************************************************************************

NAME:
   iconvert.h -- declarations for iconvert.c

AUTHOR:
   David Relson <relson@osagesoftware.com>

******************************************************************************/

#ifndef ICONVERT_H
#define ICONVERT_H

#include "config.h"

#include <iconv.h>

extern void iconvert(buff_t *restrict src, buff_t *restrict dst);
extern void iconvert_cd(iconv_t cd, buff_t *restrict src, buff_t *restrict dst);

#endif
