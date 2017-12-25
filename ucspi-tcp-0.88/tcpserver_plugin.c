/*
 *  $Log: tcpserver_plugin.c,v $
 *  Revision 1.10  2017-12-25 15:21:45+05:30  Cprogrammer
 *  ability to chose at runtime dlopen(), dlmopen()
 *
 *  Revision 1.9  2017-12-17 19:12:31+05:30  Cprogrammer
 *  use the last period in filename to determine shared lib name
 *
 *  Revision 1.8  2017-04-22 11:54:07+05:30  Cprogrammer
 *  added new argument, environ list to dlnamespace()
 *
 *  Revision 1.7  2017-04-12 13:45:35+05:30  Cprogrammer
 *  made PLUGINn_dir env variable independent of PLUGINn_init
 *
 *  Revision 1.6  2017-04-12 12:31:02+05:30  Cprogrammer
 *  fixed env_str formatting
 *
 *  Revision 1.5  2017-04-12 10:20:13+05:30  Cprogrammer
 *  use #ifdef HASDLMOPEN to use dlmopen() function
 *
 *  Revision 1.4  2017-04-05 03:13:55+05:30  Cprogrammer
 *  changed dlopen() to dlmopen() for private namespace
 *
 *  Revision 1.3  2016-05-23 04:44:17+05:30  Cprogrammer
 *  added two arguments to strerr_die()
 *
 *  Revision 1.2  2016-05-16 21:17:01+05:30  Cprogrammer
 *  added reload_flag for use in sighup handler
 *
 *  Revision 1.1  2016-05-15 22:37:54+05:30  Cprogrammer
 *  Initial revision
 *
 */

#ifndef	lint
static char     sccsid[] = "$Id: tcpserver_plugin.c,v 1.10 2017-12-25 15:21:45+05:30 Cprogrammer Exp mbhangui $";
#endif

#define FATAL "tcpserver: fatal: "

#ifdef LOAD_SHARED_OBJECTS
#include "hasdlmopen.h"
#ifdef HASDLMOPEN
#define _GNU_SOURCE
#endif
#include <unistd.h>
#include "stralloc.h"
#ifdef HASDLMOPEN
#include "dlnamespace.h"
#endif
#include "strerr.h"
#include "str.h"
#include "env.h"
#include "scan.h"
#include "fmt.h"
#include <link.h>
#ifndef HASDLMOPEN
#include <dlfcn.h>
#endif

struct stralloc shared_objfn = {0};

#ifdef HASDLMOPEN
extern void    *tcdlmopen(Lmid_t, char *, int);
#endif

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
#ifdef HASDLMOPEN
	char            strnum[FMT_ULONG];
	Lmid_t          lmid;
	int             use_dlmopen;
#endif

#ifdef HASDLMOPEN
	use_dlmopen = env_get("USE_DLMOPEN") ? 1 : 0;
#endif
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
#ifdef HASDLMOPEN
		if (reload_flag) {
#ifdef RTLD_DEEPBIND
			if (!(handle = tcdlmopen(LM_ID_NEWLM, shared_objfn.s, RTLD_NOW|RTLD_LOCAL|RTLD_DEEPBIND|RTLD_NODELETE))) {
#else
			if (!(handle = tcdlmopen(LM_ID_NEWLM, shared_objfn.s, RTLD_NOW|RTLD_LOCAL|RTLD_NODELETE))) {
#endif
				strerr_die(111, FATAL, "tcdlmopen: ", shared_objfn.s, ": ", dlerror(), 0, 0, 0, (struct strerr *) 0);
				return (1);
			} else
			if (use_dlmopen) {
				if (dlinfo(handle, RTLD_DI_LMID, &lmid) == -1)
					strerr_die(111, FATAL, "dlinfo: ", shared_objfn.s, ": ", dlerror(), 0, 0, 0, (struct strerr *) 0);
				/*- store the new lmid */
				if (dlnamespace(shared_objfn.s, 0, (unsigned long *) &lmid) < 0)
					strerr_die(111, FATAL, "dlnamespace: ", shared_objfn.s, ": unable to store namespace", 0, 0, 0, 0, (struct strerr *) 0);
				/*- display the new lmid in tcpserver log */
				strnum[fmt_ulong(strnum, lmid)] = 0;
				i = str_rchr(shared_objfn.s, '.');
				if (i)
					shared_objfn.s[i--] = 0;
				for (c = shared_objfn.s + i;*c && *c != '/';c--);
				if (*c == '/')
					c++;
				strerr_warn4("tcpserver: ", c, ".so: link map ID: ", strnum, 0);
			}
		} else {
			/*- get the old lmid for this shared object */
			lmid = 0;
			if (use_dlmopen) {
				if ((i = dlnamespace(shared_objfn.s, 0, (unsigned long *) &lmid)) < 0)
					strerr_die(111, FATAL, "dlnamespace: ", shared_objfn.s, ": ", 0, 0, 0, 0, (struct strerr *) 0);
				else
				if (!i)
					strerr_die(111, FATAL, "dlnamespace: ", shared_objfn.s, ": unable to get namespace", 0, 0, 0, 0, (struct strerr *) 0);
			}
			if (!(handle = tcdlmopen(lmid, shared_objfn.s, RTLD_NOW|RTLD_NOLOAD))) {
				strerr_die(111, FATAL, "tcdlmopen: ", shared_objfn.s, ": ", dlerror(), 0, 0, 0, (struct strerr *) 0);
				return (1);
			}
		}
#else /*- #ifdef HASDLMOPEN */
		if (reload_flag) {
#ifdef RTLD_DEEPBIND
			if (!(handle = dlopen(shared_objfn.s, RTLD_NOW|RTLD_LOCAL|RTLD_DEEPBIND|RTLD_NODELETE))) {
#else
			if (!(handle = dlopen(shared_objfn.s, RTLD_NOW|RTLD_LOCAL|RTLD_NODELETE))) {
#endif
				strerr_die(111, FATAL, "dlopen: ", shared_objfn.s, ": ", dlerror(), 0, 0, 0, (struct strerr *) 0);
				return (1);
			}
		} else {
			if (!(handle = dlopen(shared_objfn.s, RTLD_NOW|RTLD_NOLOAD))) {
				strerr_die(111, FATAL, "dlopen: ", shared_objfn.s, ": ", dlerror(), 0, 0, 0, (struct strerr *) 0);
				return (1);
			}
		}
#endif /*- ifdef HASDLMOPEN */
		/*- change to dir defined by PLUGIN<num>_dir */
		s = env_str;
		s += (i = fmt_str((char *) s, "PLUGIN"));
		s += (i = fmt_uint((char *) s, plugin_no));
		s += (i = fmt_str((char *) s, "_dir"));
		*s++ = 0;
		if ((s = env_get(env_str)) && chdir(s))
			strerr_die(111, FATAL, "chdir: ", s, ": ", 0, 0, 0, 0, (struct strerr *) 0);

		/*- execute function defined by PLUGIN<num>_init */
		s = env_str;
		s += (i = fmt_str((char *) s, "PLUGIN"));
		s += (i = fmt_uint((char *) s, plugin_no));
		s += (i = fmt_str((char *) s, "_init"));
		*s++ = 0;
		dlerror(); /*- clear existing error */
		if ((func_name = env_get(env_str))) {
			func = dlsym(handle, func_name);
			if ((error = dlerror()))
				strerr_die(111, FATAL, "dlsym: ", func_name, ": ", error, 0, 0, 0, (struct strerr *) 0);
			(*func) (!reload_flag); /*- execute the function */
		}
	} /*- for (ptr1 = envp; *ptr1; ptr1++) { */
	return (0);
}
#else
#include <unistd.h>
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
