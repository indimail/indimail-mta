/*
 * $Log: qhpsi.c,v $
 * Revision 1.10  2021-08-29 23:27:08+05:30  Cprogrammer
 * define functions as noreturn
 *
 * Revision 1.9  2021-06-27 10:37:09+05:30  Cprogrammer
 * uidnit new argument to disable/enable error on missing uids
 *
 * Revision 1.8  2021-06-24 12:16:52+05:30  Cprogrammer
 * use uidinit function proto from auto_uids.h
 *
 * Revision 1.7  2019-07-18 10:48:00+05:30  Cprogrammer
 * use strerr_die?x macro instead of strerr_die() function
 *
 * Revision 1.6  2017-05-04 20:20:22+05:30  Cprogrammer
 * close passwd, group database
 *
 * Revision 1.5  2010-07-18 19:17:27+05:30  Cprogrammer
 * renamed QUEUE_PLUGIN to QUEUE_PLUGIN_SYMB
 *
 * Revision 1.4  2009-12-09 23:57:04+05:30  Cprogrammer
 * additional closeflag argument to uidinit()
 *
 * Revision 1.3  2005-04-27 17:16:03+05:30  Cprogrammer
 * prevent path to be given in plugindir
 *
 * Revision 1.2  2005-04-26 23:28:59+05:30  Cprogrammer
 * use uidinit() to set uid/gid to qscand
 *
 * Revision 1.1  2005-04-25 22:48:02+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <unistd.h>
#include <dlfcn.h>
#include <strerr.h>
#include <env.h>
#include <str.h>
#include <stralloc.h>
#include <noreturn.h>
#include "auto_qmail.h"
#include "auto_uids.h"

#define FATAL "qhpsi: fatal: "

no_return void
nomem(int flaglog)
{
	if (flaglog)
		strerr_die2x(51, FATAL, "out of memory");
	_exit(51);
}

int
main(int argc, char **argv)
{
	int             childrc = -1, i, u, flaglog = 0;
	void           *handle;
	int             (*func) (char *);
	char           *messfn, *error, *queue_plugin_symbol, *plugindir;
	stralloc        plugin = { 0 };

	if (uidinit(1, 1) == -1)
		_exit(67);
	if (env_get("DEBUG"))
		flaglog = 1;
	/*
	 * Set the real and effective user id to qscand to
	 * prevent rogue programs from creating mischief
	 */
	if (setreuid(auto_uidc, auto_uidc)) {
		if (flaglog)
			strerr_die2sys(50, FATAL, "setreuid failed: ");
		_exit(50);
	}
	if (!str_diffn(argv[0], "plugin:", 7)) {
		if (!(plugindir = env_get("PLUGINDIR")))
			plugindir = "plugins";
		if (plugindir[i = str_chr(plugindir, '/')])
			_exit(87);
		if (!(queue_plugin_symbol = env_get("QUEUE_PLUGIN_SYMB")))
			queue_plugin_symbol = "virusscan";
		messfn = argv[0] + 7;
		for (u = 1; argv[u]; u++) {
			/* 
			 * silently ignore plugins containing path
			 */
			if (argv[u][i = str_chr(argv[u], '/')])
				_exit(87);
			/*
			 * Load the plugin with the full path of the shared
			 * library
			 */
			if (!stralloc_copys(&plugin, auto_qmail))
				nomem(flaglog);
			if (!stralloc_append(&plugin, "/"))
				nomem(flaglog);
			if (!stralloc_cats(&plugin, plugindir))
				nomem(flaglog);
			if (!stralloc_append(&plugin, "/"))
				nomem(flaglog);
			if (!stralloc_cats(&plugin, argv[u]))
				nomem(flaglog);
			if (!stralloc_0(&plugin))
				nomem(flaglog);
			if (!(handle = dlopen(plugin.s, RTLD_LAZY|RTLD_GLOBAL))) {
				if (flaglog)
					strerr_die5x(57, FATAL, "dlopen: ", plugin.s, ": ", dlerror());
				_exit(57);
			}
			dlerror(); /*- man page told me to do this */
			func = dlsym(handle, queue_plugin_symbol);
			if ((error = dlerror())) {
				if (flaglog)
					strerr_die5x(58, FATAL, "dlsym: ", plugin.s, ": ", error);
				_exit(58);
			}
			childrc = (*func) (messfn); /*- execute the function */
			if (dlclose(handle)) {
				if (flaglog)
					strerr_die5x(59, FATAL, "dlclose: ", plugin.s, ": ", error);
				_exit(59);
			}
			if (childrc)
				break;
		}
		_exit(childrc);
	} else {
		if (*argv[0] != '/' && *argv[0] != '.')
			execvp(*argv, argv);
		else
			execv(*argv, argv);
		if (flaglog)
			strerr_die2sys(75, FATAL, "execv failed: ");
		_exit(75);
	}
	/*- Not reached */
}

void
getversion_qmail_qhpsi_c()
{
	static char    *x = "$Id: qhpsi.c,v 1.10 2021-08-29 23:27:08+05:30 Cprogrammer Exp mbhangui $";
	x++;
}
