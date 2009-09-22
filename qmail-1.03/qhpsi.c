/*
 * $Log: qhpsi.c,v $
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
#include "strerr.h"
#include "env.h"
#include "str.h"
#include "stralloc.h"
#include "auto_qmail.h"
#include "auto_uids.h"

#define FATAL "qhpsi: fatal: "

int             uidinit();

void
nomem(int flaglog)
{
	if (flaglog)
		strerr_die2sys(51, FATAL, "out of memory: ");
	_exit(51);
}

int
main(int argc, char **argv)
{
	int             childrc = -1, i, u, flaglog = 0;
	void           *handle;
	int             (*func) (char *);
	char           *messfn, *error, *queue_plugin, *plugindir;
	stralloc        plugin = { 0 };

	if (uidinit() == -1)
		_exit(67);
	if (env_get("DEBUG"))
		flaglog = 1;
	/*
	 * Set the real and effective user id to qscand to
	 * prevent rogue programs from creating mischief
	 */
	if (setreuid(auto_uidc, auto_uidc))
	{
		if (flaglog)
			strerr_die2sys(50, FATAL, "setreuid failed: ");
		_exit(50);
	}
	if (!str_diffn(argv[0], "plugin:", 7))
	{
		if (!(plugindir = env_get("PLUGINDIR")))
			plugindir = "plugins";
		if (plugindir[i = str_chr(plugindir, '/')])
			_exit(87);
		if (!(queue_plugin = env_get("QUEUE_PLUGIN")))
			queue_plugin = "virusscan";
		messfn = argv[0] + 7;
		for (u = 1; argv[u]; u++)
		{
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
			if (!(handle = dlopen(plugin.s, RTLD_LAZY|RTLD_GLOBAL)))
			{
				if (flaglog)
					strerr_die(57, FATAL, "dlopen: ", plugin.s, ": ", dlerror(), 0, 0, 0, (struct strerr *) 0);
				_exit(57);
			}
			dlerror(); /*- man page told me to do this */
			func = dlsym(handle, queue_plugin);
			if ((error = dlerror()))
			{
				if (flaglog)
					strerr_die(58, FATAL, "dlsym: ", plugin.s, ": ", error, 0, 0, 0, (struct strerr *) 0);
				_exit(58);
			}
			if ((childrc = (*func) (messfn)))
				break;
			if (dlclose(handle))
			{
				if (flaglog)
					strerr_die(59, FATAL, "dlclose: ", plugin.s, ": ", error, 0, 0, 0, (struct strerr *) 0);
				_exit(59);
			}
		}
		_exit(childrc);
	} else
	{
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
	static char    *x = "$Id: qhpsi.c,v 1.3 2005-04-27 17:16:03+05:30 Cprogrammer Stab mbhangui $";
	x++;
}
