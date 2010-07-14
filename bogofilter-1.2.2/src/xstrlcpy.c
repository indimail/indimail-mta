#include "config.h"
#include "xstrlcpy.h"
#include "xmalloc.h"
#include "system.h"

size_t xstrlcpy(char *d, const char *s, size_t l) {
    size_t w = strlcpy(d, s, l);
    if (w >= l) xmem_toolong("xstrlcpy");
    return w;
}
