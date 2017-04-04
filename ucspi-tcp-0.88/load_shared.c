/*
 * $Log: load_shared.c,v $
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
#define _GNU_SOURCE
#include <unistd.h>
#include "dlnamespace.h"
#include "strerr.h"
#include "str.h"
#include "fmt.h"
#include <link.h>
#include <dlfcn.h>

#define FATAL "tcpserver: fatal: "

void
load_shared(char *file, char **argv, char **envp)
{
	int             i, argc, split, loaded = 0;
	int             (*func) (int, char **, char **);
	void           *handle;
	char           *error, *fptr;
	char          **ptr;
	char            strnum[FMT_ULONG];
	Lmid_t          lmid;

	loaded = 0;
	if (!str_end(file, ".so")) {
		lmid = 0;
		if ((i = dlnamespace(file, (unsigned long *) &lmid)) < 0)
			strerr_die(111, FATAL, "dlnamespace: ", file, ": ", 0, 0, 0, 0, (struct strerr *) 0);
		if (!(handle = dlmopen(lmid, file, RTLD_NOW|RTLD_NOLOAD))) {
#ifdef RTLD_DEEPBIND
			if (!(handle = dlmopen(LM_ID_NEWLM, file, RTLD_NOW|RTLD_LOCAL|RTLD_DEEPBIND|RTLD_NODELETE))) {
#else
			if (!(handle = dlmopen(LM_ID_NEWLM, file, RTLD_NOW|RTLD_LOCAL|RTLD_NODELETE))) {
#endif
				strerr_die(111, FATAL, "dlmopen: ", file, ": ", dlerror(), 0, 0, 0, (struct strerr *) 0);
				return;
			} 
			loaded = 1;
			if (dlinfo(handle, RTLD_DI_LMID, &lmid) == -1)
				strerr_die(111, FATAL, "dlinfo: ", file, ": ", dlerror(), 0, 0, 0, (struct strerr *) 0);
			if (dlnamespace(file, (unsigned long *) &lmid) < 0)
				strerr_die(111, FATAL, "dlnamespace: ", file, ": unable to store namespace", 0, 0, 0, 0, (struct strerr *) 0);
		}
		dlerror(); /*- clear existing error */
		/*- use the basename of the shared object as the function to execute */
		split = str_chr(file, '.');
		if (split)
			file[split--] = 0;
		for (fptr = file + split;*fptr && *fptr != '/';fptr--);
		if (*fptr == '/')
			fptr++;
		if (loaded) {
			strnum[fmt_ulong(strnum, lmid)] = 0;
			strerr_warn4("tcpserver: ", "load_shared", ".so: link map ID: ", strnum, 0);
		}
		func = dlsym(handle, fptr);
		if ((error = dlerror()))
			strerr_die(111, FATAL, "dlsym: ", fptr, ": ", error, 0, 0, 0, (struct strerr *) 0);
		for (argc = 0,ptr = argv; *ptr; ptr++)
			argc++;
		(*func) (argc, argv, envp); /*- execute the function */
		if (dlclose(handle)) /*- this will not unload the object due to RTLD_NODELETE */
			strerr_die(111, FATAL, "dlclose: ", fptr, ": ", dlerror(), 0, 0, 0, (struct strerr *) 0);
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
