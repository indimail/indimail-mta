/*
 * $Log: load_shared.c,v $
 * Revision 1.2  2016-03-01 18:50:38+05:30  Cprogrammer
 * changed RTLD_GLOBAL to RTLD_LOCAL
 *
 * Revision 1.1  2016-02-08 21:27:51+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <unistd.h>
#include "strerr.h"
#include "str.h"
#ifdef LOAD_SHARED_OBJECTS
#include <dlfcn.h>
#endif

#define FATAL "tcpserver: fatal: "

#ifdef LOAD_SHARED_OBJECTS
void
load_shared(char *file, char **argv, char **envp)
{
	int             argc;
	int             (*func) (int, char **, char **);
	void           *handle;
	char           *error;
	char          **ptr;

	if (!str_end(file, ".so")) {
		if (!(handle = dlopen(file, RTLD_NOW|RTLD_LOCAL)))
			return;
		dlerror(); /*- man page told me to do this */
		func = dlsym(handle, "qmail_smtpd");
		if ((error = dlerror()))
			strerr_die(0, FATAL, "dlsym: qmail-smtpd.so: ", error, 0, 0, 0, (struct strerr *) 0);
		for (argc = 0,ptr = argv; *ptr; ptr++)
			argc++;
		(*func) (argc, argv, envp); /*- execute the function */
		if (dlclose(handle))
			strerr_die(0, FATAL, "dlclose: qmail-smtpd.so: ", error, 0, 0, 0, (struct strerr *) 0);
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
