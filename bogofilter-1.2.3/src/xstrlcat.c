#include "config.h"
#include "xstrlcat.h"
#include "xmalloc.h"
#include "system.h"

size_t xstrlcat(char *d, const char *s, size_t l) {
    size_t w = strlcat(d, s, l);
    if (w >= l) xmem_toolong("xstrlcat");
    return w;
}
