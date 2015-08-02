/* memory allocating, unlimited strcat 
 * (C) 2004 by Matthias Andree, GNU GPL v2 */

#include "mxcat.h"
#include "system.h"
#include "xmalloc.h"
#include <stdarg.h>
#include <string.h>

char *mxcat(const char *first, ...) {
    va_list ap;
    size_t s;
    char *t, *r;

    va_start(ap, first);
    s = strlen(first);
    while ((t = va_arg(ap, char *))) {
	s += strlen(t);
    }
    va_end(ap);

    s++;
    r = xmalloc(s);
    va_start(ap, first);
    strlcpy(r, first, s);
    while ((t = va_arg(ap, char *))) {
	strlcat(r, t, s);
    }
    va_end(ap);

    return r;
}
