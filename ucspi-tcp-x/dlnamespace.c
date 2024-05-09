/*
 * $Log: dlnamespace.c,v $
 * Revision 1.13  2024-05-09 22:55:54+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.12  2021-07-03 14:03:56+05:30  Cprogrammer
 * use Lmid_t data type for id instead of unsigned long
 *
 * Revision 1.11  2021-05-12 20:59:03+05:30  Cprogrammer
 * replaced pathexec_env() with upathexec_env()
 *
 * Revision 1.10  2020-09-16 20:47:52+05:30  Cprogrammer
 * fixed compiler warnings
 *
 * Revision 1.9  2020-08-04 22:27:54+05:30  Cprogrammer
 * removed compiler warning
 *
 * Revision 1.8  2020-08-03 17:21:41+05:30  Cprogrammer
 * use qmail library
 *
 * Revision 1.7  2017-12-25 15:20:38+05:30  Cprogrammer
 * added rcsid
 *
 * Revision 1.6  2017-04-22 12:11:14+05:30  Cprogrammer
 * use environment variable LMID or envp variable to figure out LMID when namespace variable is absent
 *
 * Revision 1.5  2017-04-12 10:18:56+05:30  Cprogrammer
 * auto configure for dlmopen()
 *
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
#define _GNU_SOURCE
#include <dlfcn.h>
#include "hasdlmopen.h"
#include <errno.h>
#ifdef HASDLMOPEN
#include <str.h>
#include <fmt.h>
#include <stralloc.h>
#include <scan.h>
#include <env.h>
#include <unistd.h>
#include "upathexec.h"

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
dlnamespace(const char *fn, char **envp, Lmid_t *id)
{
	char           *ptr, *cptr, *s;
	int             i, j;
	char            strnum[FMT_ULONG], env_str[FMT_ULONG + 5]; /*- LMID_NO */

	if (!id) {
		errno = EINVAL;
		return (-1);
	}
	/*- search for loaded module fn. each entry is separated by NULL character */
	if (!*id) {
		if (!namespace.len) {
			for (j = 1;;j++) {
				s = env_str;
				s += (i = fmt_str((char *) s, "LMID"));
				s += (i = fmt_uint((char *) s, j));
				*s++ = 0;
				if (!(s = env_get(env_str))) /*- no shared libs loaded/stored using dlmopen */
					break;
				if (!str_diff(fn, s)) {
					*id = j;
					return (1);
				}
			}
			for (j = 0;envp[j];j++) {
				if (str_diffn(envp[j], "LMID", 4))
					continue;
				for (s = envp[j]; *s; s++) {
					if (*s != '=')
						continue;
					*s = 0;
					if (str_diff(s + 1, fn)) {
						*s = '=';
						break;
					}
					ptr = envp[j] + 4;
					if (*ptr) {
						scan_long(ptr, id);
						*s = '=';
						return (1);
					} else
						*s = '=';
					break;
				} /*- for (s = envp[j]; *s; s++) */
			} /*- for (j = 0;envp[j];j++) { */
		}
		for (cptr = ptr = namespace.s, i = 0; i < namespace.len;ptr++, i++) {
			if (!*ptr) {
				cptr = ptr + 1;
				continue;
			} else
			if (*ptr == ':') {
				*ptr = 0;
				if (!str_diff(ptr + 1, fn)) {
					scan_long(cptr, id);
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
	s = env_str;
	s += (i = fmt_str((char *) s, "LMID"));
	s += (i = fmt_ulong((char *) s, *id));
	*s++ = 0;
	if (!upathexec_env(env_str, fn))
		return (-1);
	return (0);
}
#else
int
dlnamespace(const char *fn, unsigned long *id)
{
	if (!id) {
		errno = EINVAL;
		return (-1);
	}
	if (!*id)
		*id = 0;
	return (0);
}
#endif /*- ifdef HASDLMOPEN */
#endif /*- ifdef LOAD_SHARED_OBJECTS */

void
getversion_dlnamespace_c()
{
	const char     *x = "$Id: dlnamespace.c,v 1.13 2024-05-09 22:55:54+05:30 mbhangui Exp mbhangui $";
	if (x)
		x++;
}
