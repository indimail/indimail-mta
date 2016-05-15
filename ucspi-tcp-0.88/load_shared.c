/*
 * $Log: load_shared.c,v $
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
#include <unistd.h>
#ifdef LOAD_SHARED_OBJECTS
#include "strerr.h"
#include "str.h"
#include <dlfcn.h>

#define FATAL "tcpserver: fatal: "

void
load_shared(char *file, char **argv, char **envp)
{
	int             argc, split;
	int             (*func) (int, char **, char **);
	void           *handle;
	char           *error, *fptr;
	char          **ptr;

	if (!str_end(file, ".so")) {
		if (!(handle = dlopen(file, RTLD_NOW|RTLD_NOLOAD))) {
			if (!(handle = dlopen(file, RTLD_NOW|RTLD_LOCAL|RTLD_NODELETE))) {
				strerr_die(111, FATAL, "dlopen2: ", dlerror(), 0, 0, 0, (struct strerr *) 0);
				return;
			}
		}
		dlerror(); /*- clear existing error */
		/*- use the basename of the shared object as the function to execute */
		split = str_chr(file, '.');
		if (split)
			file[split--] = 0;
		for (fptr = file + split;*fptr && *fptr != '/';fptr--);
		if (*fptr == '/')
			fptr++;
		func = dlsym(handle, fptr);
		if ((error = dlerror()))
			strerr_die(111, FATAL, "dlsym: ", fptr, ": ", error, 0, (struct strerr *) 0);
		for (argc = 0,ptr = argv; *ptr; ptr++)
			argc++;
		(*func) (argc, argv, envp); /*- execute the function */
		if (dlclose(handle)) /*- this will not unload the object due to RTLD_NODELETE */
			strerr_die(111, FATAL, "dlclose: ", fptr, ": ", error, 0, (struct strerr *) 0);
		_exit(0);
	} else
		execve(file, argv, envp);
}
#else
void
load_shared(char *file, char **argv, char **envp)
{
	execve(file, argv, envp);
}
#endif
