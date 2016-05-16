/*
 *  $Log: tcpserver_plugin.c,v $
 *  Revision 1.2  2016-05-16 21:17:01+05:30  Cprogrammer
 *  added reload_flag for use in sighup handler
 *
 *  Revision 1.1  2016-05-15 22:37:54+05:30  Cprogrammer
 *  Initial revision
 *
 */
#include <unistd.h>
#include "stralloc.h"
#include "strerr.h"
#include "str.h"
#include "env.h"
#include "scan.h"
#include "fmt.h"
#include <dlfcn.h>

#ifndef	lint
static char     sccsid[] = "$Id: tcpserver_plugin.c,v 1.2 2016-05-16 21:17:01+05:30 Cprogrammer Exp mbhangui $";
#endif

#define FATAL "tcpserver: fatal: "

#ifdef LOAD_SHARED_OBJECTS
struct stralloc shared_objfn = {0};

int
tcpserver_plugin(char **envp, int reload_flag)
{
	char            env_str[FMT_ULONG + 12];
	void           *handle;
	unsigned int    plugin_no;
	int             (*func) (int);
	int             i;
	char           *error, *c, *func_name, *s;
	char          **ptr1;

	for (ptr1 = envp; *ptr1; ptr1++) {
		if (!str_start(*ptr1, "PLUGIN"))
			continue;
		c = *ptr1 + 6;
		if (!(i = scan_uint(c, &plugin_no))) /*- no number following PLUGIN */
			continue;
		if (*(c + i) != '=') /*- anthing other than PLUGIN<num> */
			continue;
		for (c = *ptr1;*c && *c != '=';c++);
		if (!stralloc_copys(&shared_objfn, c + 1))
			strerr_die2x(111, FATAL, "out of memory");
		if (!stralloc_0(&shared_objfn))
			strerr_die2x(111, FATAL, "out of memory");
		if (reload_flag) {
			if (!(handle = dlopen(shared_objfn.s, RTLD_NOW|RTLD_LOCAL|RTLD_NODELETE))) {
				strerr_die(111, FATAL, "dlopen: ", shared_objfn.s, ": ", dlerror(), 0, (struct strerr *) 0);
				return (1);
			}
		} else {
			if (!(handle = dlopen(shared_objfn.s, RTLD_NOW|RTLD_NOLOAD))) {
				strerr_die(111, FATAL, "dlopen: ", shared_objfn.s, ": ", dlerror(), 0, (struct strerr *) 0);
				return (1);
			}
		}
		/*- execute function defined by PLUGIN<num>_init */
		s = env_str;
		s += (i = fmt_str((char *) s, "PLUGIN"));
		s += (i = fmt_uint((char *) s, plugin_no));
		s += (i = fmt_str((char *) s, "_init"));
		*s++ = 0;
		dlerror();
		if ((func_name = env_get(env_str))) {
			func = dlsym(handle, func_name);
			if ((error = dlerror()))
				strerr_die(111, FATAL, "dlsym: ", func_name, ": ", error, 0, (struct strerr *) 0);
			/*- change to dir defined by PLUGIN<num>_dir */
			s = env_str;
			s += (i = fmt_str((char *) s, "PLUGIN"));
			s += (i = fmt_uint((char *) s, plugin_no));
			s += (i = fmt_str((char *) s, "_dir"));
			*s++ = 0;
			if ((s = env_get(env_str)) && chdir(s))
				strerr_die(111, FATAL, "chdir: ", s, ": ", 0, 0, (struct strerr *) 0);
			(*func) (!reload_flag); /*- execute the function */
		}
	} /*- for (ptr1 = envp; *ptr1; ptr1++) { */
	return (0);
}
#else
int
tcpserver_plugin(char **envp)
{
	return (0);
}
#endif

void
getversion_tcpserver_plugin_c()
{
	write(1, sccsid, 0);
}
