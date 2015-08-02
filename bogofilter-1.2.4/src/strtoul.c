/* strtoul.c - replacement strtoul */
/* (C) 2004 by Matthias Andree. License: GNU GPL v2. */

#include <errno.h>
#include <limits.h>

unsigned long strtoul(const char *str, char **endptr, int base)
{
    unsigned long ret = 0;
    if (base != 10) { errno = EINVAL; return 0ul; }

    while (*str >= '0' && *str <= '9')
    {
	if (ret * 10 < ret) goto ovl;
	ret *= 10;
	if (ret + (*str - '0') < ret) goto ovl;
	ret += (*str - '0');
	str++;
    }
    if (endptr) *endptr = (char *)str;
    return 0;
ovl:
    errno = ERANGE;
    return ULONG_MAX;
}
