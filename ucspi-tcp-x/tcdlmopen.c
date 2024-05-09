/*
 * $Log: tcdlmopen.c,v $
 * Revision 1.4  2024-05-09 22:55:54+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.3  2020-09-16 20:50:12+05:30  Cprogrammer
 * fixed compiler warnings
 *
 * Revision 1.2  2020-08-03 17:27:31+05:30  Cprogrammer
 * use qmail library
 *
 * Revision 1.1  2017-12-25 15:19:19+05:30  Cprogrammer
 * Initial revision
 *
 */

#ifdef LOAD_SHARED_OBJECTS
#include "hasdlmopen.h"
#ifdef HASDLMOPEN
#define _GNU_SOURCE
#endif
#include <dlfcn.h>
#ifdef HASDLMOPEN
#include "dlnamespace.h"
#endif
#include <env.h>

#ifdef HASDLMOPEN
void           *
tcdlmopen(Lmid_t lmid, const char *filename, int flags)
{
	return (env_get("USE_DLMOPEN") ? dlmopen(lmid, filename, flags) : dlopen(filename, flags));
}
#else
void           *
tcdlmopen(int lmid, const char *filename, int flags)
{
	return (dlopen(filename, flags));
}
#endif

#endif /*- ifdef LOAD_SHARED_OBJECTS */

extern void write(int, char *, int);
void
getversion_tcdlmopen_c()
{
	const char     *x = "$Id: tcdlmopen.c,v 1.4 2024-05-09 22:55:54+05:30 mbhangui Exp mbhangui $";
	if (x)
		x++;
}
