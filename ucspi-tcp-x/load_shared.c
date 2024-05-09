/*
 * $Log: load_shared.c,v $
 * Revision 1.19  2021-07-03 14:04:38+05:30  Cprogrammer
 * use Lmid_t data type for id instead of unsigned long
 *
 * Revision 1.18  2021-05-12 21:02:52+05:30  Cprogrammer
 * replaced pathexec with upathexec
 *
 * Revision 1.17  2020-10-01 14:35:33+05:30  Cprogrammer
 * Darwin Port
 *
 * Revision 1.16  2020-09-30 23:22:24+05:30  Cprogrammer
 * Darwin Port
 *
 * Revision 1.15  2020-09-16 20:49:20+05:30  Cprogrammer
 * fixed compiler warnings
 *
 * Revision 1.14  2020-08-03 17:24:54+05:30  Cprogrammer
 * use qmail library
 *
 * Revision 1.13  2020-06-08 22:48:20+05:30  Cprogrammer
 * quench compiler warning
 *
 * Revision 1.12  2017-12-25 15:21:10+05:30  Cprogrammer
 * ability to chose at runtime dlopen(), dlmopen()
 *
 * Revision 1.11  2017-12-17 14:41:38+05:30  Cprogrammer
 * use the last period in filename to determine shared lib name
 *
 * Revision 1.10  2017-04-22 12:13:31+05:30  Cprogrammer
 * use pathexec_dl() to execute dynamically loaded functions
 *
 * Revision 1.9  2017-04-16 19:54:12+05:30  Cprogrammer
 * display lmid only on reload
 *
 * Revision 1.8  2017-04-12 10:20:41+05:30  Cprogrammer
 * use #ifdef HASDLMOPEN to use dlmopen() function
 *
 * Revision 1.7  2017-04-05 03:12:39+05:30  Cprogrammer
 * changed dlopen() to dlmopen() for private namespace
 *
 * Revision 1.6  2017-03-30 23:01:06+05:30  Cprogrammer
 * use RTLD_DEEPBIND to hide symbols within a shared library.
 *
 * Revision 1.5  2016-05-23 04:42:46+05:30  Cprogrammer
 * added two arguments to strerr_die()
 *
 * Revision 1.4  2016-05-15 21:49:11+05:30  Cprogrammer
 * use the basename of the shared object as the function to execute
 *
 * Revision 1.3  2016-03-31 17:47:59+05:30  Cprogrammer
 * compile load_shared only if LOAD_SHARED_OBJECTS is defined
 *
 * Revision 1.2  2016-03-01 18:50:38+05:30  Cprogrammer
 * changed RTLD_GLOBAL to RTLD_LOCAL
 *
 * Revision 1.1  2016-02-08 21:27:51+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef LOAD_SHARED_OBJECTS
#include "hasdlmopen.h"
#ifdef HASDLMOPEN
#define _GNU_SOURCE
#include "dlnamespace.h"
#endif
#include <dlfcn.h>
#include <unistd.h>
#include <strerr.h>
#include <str.h>
#include <fmt.h>
#include <env.h>
#include <alloc.h>
#include "upathexec.h"

#define FATAL "tcpserver: fatal: "

#ifdef HASDLMOPEN
extern void    *tcdlmopen(Lmid_t, const char *, int);
#endif

void
load_shared(const char *file, char **argv, char **envp)
{
	int             argc, split, len;
	int             (*func) (int, char **, char **);
	void           *handle;
	char           *error, *fptr, *file_t;
	char          **ptr;
#ifdef HASDLMOPEN
	char            strnum[FMT_ULONG];
	int             i, loadedNew = 0, use_dlmopen;
	Lmid_t          lmid;
#endif

	if (!str_end(file, ".so")) {
#ifdef HASDLMOPEN
		i = lmid = 0;
		use_dlmopen = env_get("USE_DLMOPEN") ? 1 : 0;
		/*- get lmid */
		if (use_dlmopen && (i = dlnamespace(file, envp, &lmid)) < 0)
			strerr_die4x(111, FATAL, "dlnamespace: ", file, ": unable to store namespace");
		if (!i) { /*- No existing entry for lmid (loaded libs) found */
#ifdef RTLD_DEEPBIND
			if (!(handle = tcdlmopen(LM_ID_NEWLM, file, RTLD_NOW|RTLD_LOCAL|RTLD_DEEPBIND|RTLD_NODELETE))) {
#else
			if (!(handle = tcdlmopen(LM_ID_NEWLM, file, RTLD_NOW|RTLD_LOCAL|RTLD_NODELETE))) {
#endif
				strerr_die5x(111, FATAL, "tcdlmopen: ", file, ": ", dlerror());
				return;
			}
			if (use_dlmopen) {
				loadedNew = 1; /*- indicator for newly loaded shared lib */
				if (dlinfo(handle, RTLD_DI_LMID, &lmid) == -1)
					strerr_die5x(111, FATAL, "dlinfo: ", file, ": ", dlerror());
				/*- store the new lmid */
				if (dlnamespace(file, 0, &lmid) < 0)
					strerr_die4x(111, FATAL, "dlnamespace: ", file, ": unable to store namespace");
			}
		} else /*- loaded earlier so use RTLD_NOLOAD */
		if (!(handle = tcdlmopen(lmid, file, RTLD_NOW|RTLD_NOLOAD))) { /*- load it again (may have been dlclosed */
#ifdef RTLD_DEEPBIND
			if (!(handle = tcdlmopen(LM_ID_NEWLM, file, RTLD_NOW|RTLD_LOCAL|RTLD_DEEPBIND|RTLD_NODELETE))) {
#else
			if (!(handle = tcdlmopen(LM_ID_NEWLM, file, RTLD_NOW|RTLD_LOCAL|RTLD_NODELETE))) {
#endif
				strerr_die5x(111, FATAL, "tcdlmopen: ", file, ": ", dlerror());
				return;
			}
			if (use_dlmopen) {
				loadedNew = 1; /*- indicator for newly loaded shared lib */
				if (dlinfo(handle, RTLD_DI_LMID, &lmid) == -1)
					strerr_die5x(111, FATAL, "dlinfo: ", file, ": ", dlerror());
				/*- store the new lmid */
				if (dlnamespace(file, 0, &lmid) < 0)
					strerr_die4x(111, FATAL, "dlnamespace: ", file, ": unable to store namespace");
			}
		} else
			loadedNew = 0;
#else /*- HASDLMOPEN */
		if (!(handle = dlopen(file, RTLD_NOW|RTLD_NOLOAD))) {
#ifdef RTLD_DEEPBIND
			if (!(handle = dlopen(file, RTLD_NOW|RTLD_LOCAL|RTLD_DEEPBIND|RTLD_NODELETE))) {
#else
			if (!(handle = dlopen(file, RTLD_NOW|RTLD_LOCAL|RTLD_NODELETE))) {
#endif
				strerr_die5x(111, FATAL, "dlopen: ", file, ": ", dlerror());
				return;
			}
		}
#endif /*- ifdef HASDLMOPEN */
		dlerror(); /*- clear existing error */
		/*- use the basename of the shared object as the function to execute */
		if (!(file_t = (char *) alloc((len = str_len(file)) + 1)))
			strerr_die2x(111, FATAL, "dlopen: out of memory");
		str_copyb(file_t, file, len + 1);
		split = str_rchr(file_t, '.');
		if (split)
			file_t[split--] = 0;
		for (fptr = file_t + split; *fptr && *fptr != '/'; fptr--);
		if (*fptr == '/')
			fptr++;
#ifdef HASDLMOPEN
		if (use_dlmopen && loadedNew) {
			strnum[fmt_ulong(strnum, lmid)] = 0;
			strerr_warn4("tcpserver: ", fptr, ".so: link map ID: ", strnum, 0);
		}
#endif
		func = dlsym(handle, fptr);
		alloc_free(file_t);
		if ((error = dlerror()))
			strerr_die5x(111, FATAL, "dlsym: ", fptr, ": ", error);
		for (argc = 0,ptr = argv; *ptr; ptr++)
			argc++;
		pathexec_dl(argc, argv, envp, func);
		if (dlclose(handle)) /*- this will not unload the object due to RTLD_NODELETE */
			strerr_die5x(111, FATAL, "dlclose: ", fptr, ": ", dlerror());
		_exit(0);
	} else
		execve(file, argv, envp);
}
#else
#include <unistd.h>
void
load_shared(char *file, char **argv, char **envp)
{
	execve(file, argv, envp);
}
#endif

void
getversion_load_shared_c()
{
	const char     *x = "$Id: load_shared.c,v 1.19 2021-07-03 14:04:38+05:30 Cprogrammer Exp mbhangui $";
	if (x)
		x++;
}
