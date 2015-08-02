/* (C) 2002 by Matthias Andree, redistributable according to the terms
 * of the GNU General Public License, v2.
 *
 * $Id: xstrdup.c 4184 2004-02-25 04:51:58Z relson $
 *
 */

#include <string.h>
#include "xmalloc.h"
#include "xstrdup.h"

char *xstrdup(const char *s) {
    size_t l = strlen(s) + 1;
    char *t = xmalloc(l);
    memcpy(t, s, l);
    return t;
}
