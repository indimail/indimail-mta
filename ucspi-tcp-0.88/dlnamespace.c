/*
 * $Log: dlnamespace.c,v $
 * Revision 1.4  2017-04-06 17:24:35+05:30  Cprogrammer
 * added comments/description
 *
 * Revision 1.3  2017-04-05 04:36:25+05:30  Cprogrammer
 * replace ':' character after str comparision
 *
 * Revision 1.2  2017-04-05 04:05:14+05:30  Cprogrammer
 * conditional compilation of dlnamespace()
 *
 * Revision 1.1  2017-04-05 03:08:25+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef LOAD_SHARED_OBJECTS
#include <errno.h>
#include "str.h"
#include "fmt.h"
#include "stralloc.h"
#include "scan.h"

static stralloc namespace = {0};

/*
 * dlnamespace has two modes of operations - search & store
 * Search - return value
 *  1 - Entry found
 *  0 - No Entry found
 * Store - return value
 *  0 - successful
 * -1 - ENOMEM failure
 */
int
dlnamespace(char *fn, unsigned long *id)
{
	char           *ptr, *cptr;
	int             i;
	char            strnum[FMT_ULONG];

	if (!id) {
		errno = EINVAL;
		return (-1);
	}
	/*- search for loaded module fn. each entry is separated by NULL character */
	if (!*id) {
		for (cptr = ptr = namespace.s, i = 0; i < namespace.len;ptr++, i++) {
			if (!*ptr) {
				cptr = ptr + 1;
				continue;
			} else
			if (*ptr == ':') {
				*ptr = 0;
				if (!str_diff(ptr + 1, fn)) {
					scan_ulong(cptr, id);
					*ptr = ':';
					return (1);
				} else
					*ptr = ':';
			}
		}
		return (0);
	}
	/*- 
	 * store operation
	 * append/store the new id in the form
	 * id1:filename1^@id2:filename2^@
	 */
	strnum[fmt_ulong(strnum, *id)] = 0;
	if (!stralloc_cats(&namespace, strnum))
		return (-1);
	else
	if (!stralloc_append(&namespace, ":"))
		return (-1);
	else
	if (!stralloc_cats(&namespace, fn))
		return (-1);
	else
	if (!stralloc_0(&namespace))
		return (-1);
	return (0);
}
#endif
