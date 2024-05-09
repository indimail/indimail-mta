/*
 * $Id: qhpsi.c,v 1.13 2024-05-09 22:03:17+05:30 mbhangui Exp mbhangui $
 */
#include <unistd.h>
#include <dlfcn.h>
#include <strerr.h>
#include <env.h>
#include <str.h>
#include <stralloc.h>
#include <noreturn.h>
#include "qmail.h"
#include "auto_qmail.h"
#include "auto_uids.h"

#define FATAL "qhpsi: fatal: "

no_return void
nomem(int flaglog)
{
	if (flaglog)
		strerr_die2x(QQ_OUT_OF_MEMORY, FATAL, "out of memory");
	_exit(QQ_OUT_OF_MEMORY);
}

int
main(int argc, char **argv)
{
	int             childrc = -1, i, u, flaglog = 0;
	void           *handle;
	int             (*func) (const char *);
	const char     *messfn, *error, *queue_plugin_symbol, *plugindir;
	stralloc        plugin = { 0 };

	if (uidinit(1, 1) == -1)
		_exit(QQ_GET_UID_GID);
	if (env_get("DEBUG"))
		flaglog = 1;
	/*
	 * Set the real and effective user id to qscand to
	 * prevent rogue programs from creating mischief
	 */
	if (setreuid(auto_uidv, auto_uidv)) {
		if (flaglog)
			strerr_die2sys(QQ_VIRUS_SCANNER_PRIV, FATAL, "setreuid failed: ");
		_exit(QQ_VIRUS_SCANNER_PRIV);
	}
	if (!str_diffn(argv[0], "plugin:", 7)) {
		if (!(plugindir = env_get("PLUGINDIR")))
			plugindir = "plugins";
		if (plugindir[i = str_chr(plugindir, '/')])
			_exit(QQ_SYSTEM_MISCONFIG);
		if (!(queue_plugin_symbol = env_get("QUEUE_PLUGIN_SYMB")))
			queue_plugin_symbol = "virusscan";
		messfn = argv[0] + 7;
		for (u = 1; argv[u]; u++) {
			/*
			 * silently ignore plugins containing path
			 */
			if (argv[u][i = str_chr(argv[u], '/')])
				_exit(QQ_SYSTEM_MISCONFIG);
			/*
			 * Load the plugin with the full path of the shared
			 * library
			 */
			if (!stralloc_copys(&plugin, auto_qmail) ||
					!stralloc_append(&plugin, "/") ||
					!stralloc_cats(&plugin, plugindir) ||
					!stralloc_append(&plugin, "/") ||
					!stralloc_cats(&plugin, argv[u]) ||
					!stralloc_0(&plugin))
				nomem(flaglog);
			if (!(handle = dlopen(plugin.s, RTLD_LAZY|RTLD_GLOBAL))) {
				if (flaglog)
					strerr_die5x(QQ_OPEN_SHARED_OBJ, FATAL, "dlopen: ", plugin.s, ": ", dlerror());
				_exit(QQ_OPEN_SHARED_OBJ);
			}
			dlerror(); /*- man page told me to do this */
			func = dlsym(handle, queue_plugin_symbol);
			if ((error = dlerror())) {
				if (flaglog)
					strerr_die5x(QQ_RESOLVE_SHARED_SYM, FATAL, "dlsym: ", plugin.s, ": ", error);
				_exit(QQ_RESOLVE_SHARED_SYM);
			}
			childrc = (*func) (messfn); /*- execute the function */
			if (dlclose(handle)) {
				if (flaglog)
					strerr_die5x(QQ_CLOSE_SHARED_OBJ, FATAL, "dlclose: ", plugin.s, ": ", error);
				_exit(QQ_CLOSE_SHARED_OBJ);
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
			strerr_die2sys(QQ_EXEC_FAILED, FATAL, "execv failed: ");
		_exit(QQ_EXEC_FAILED);
	}
	/*- Not reached */
}

void
getversion_qmail_qhpsi_c()
{
	const char     *x = "$Id: qhpsi.c,v 1.13 2024-05-09 22:03:17+05:30 mbhangui Exp mbhangui $";
	x++;
}

/*
 * $Log: qhpsi.c,v $
 * Revision 1.13  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.12  2023-10-27 16:11:40+05:30  Cprogrammer
 * replace hard-coded exit values with constants from qmail.h
 *
 * Revision 1.11  2023-02-14 08:39:26+05:30  Cprogrammer
 * renamed auto_uidc to auto_uidv
 *
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
