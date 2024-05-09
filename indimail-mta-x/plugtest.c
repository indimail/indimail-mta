/*
 * $Log: plugtest.c,v $
 * Revision 1.8  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.7  2021-08-29 23:27:08+05:30  Cprogrammer
 * define functions as noreturn
 *
 * Revision 1.6  2021-05-26 10:43:34+05:30  Cprogrammer
 * handle access() error other than ENOENT
 *
 * Revision 1.5  2017-12-26 21:57:28+05:30  Cprogrammer
 * BUGFIX - fixed wrong copy of plugindir
 *
 * Revision 1.4  2017-12-26 15:24:45+05:30  Cprogrammer
 * use auto_prefix for plugin directory & make PLUGINDIR path absolute
 *
 * Revision 1.3  2011-07-08 13:47:27+05:30  Cprogrammer
 * added setting of authenticated, relayclient variables
 *
 * Revision 1.2  2011-04-19 20:51:24+05:30  Cprogrammer
 * added checking of arguments
 *
 * Revision 1.1  2011-04-18 22:15:35+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <unistd.h>
#include <dlfcn.h>
#include <sgetopt.h>
#include <alloc.h>
#include <str.h>
#include <error.h>
#include <strerr.h>
#include <env.h>
#include <stralloc.h>
#include <fmt.h>
#include <subfd.h>
#include <noreturn.h>
#include "auto_qmail.h"
#include "auto_prefix.h"
#include "smtp_plugin.h"

#define FATAL "plugtest: fatal: "

static int       authenticated;
const char      *relayclient;
static PLUGIN  **plug = (PLUGIN **) 0;
static void    **handle;

void
out(const char *str)
{
	if (!str || !*str)
		return;
	if (substdio_puts(subfdout, str) == -1)
		strerr_die2sys(111, FATAL, "write: ");
	return;
}

void
flush()
{
	if (substdio_flush(subfdout) == -1)
		strerr_die2sys(111, FATAL, "write: ");
	return;
}

no_return void
die_nomem()
{
	substdio_flush(subfdout);
	substdio_puts(subfderr, "plugtest: out of memory\n");
	substdio_flush(subfderr);
	_exit(1);
}

no_return void
die_plugin(const char *arg1, const char *arg2, const char *arg3, const char *arg4)
{
	substdio_flush(subfdout);
	if (arg1)
		substdio_puts(subfderr, arg1);
	if (arg2)
		substdio_puts(subfderr, arg2);
	if (arg3)
		substdio_puts(subfderr, arg3);
	if (arg4)
		substdio_puts(subfderr, arg4);
	substdio_puts(subfderr, "\n");
	substdio_flush(subfderr);
	_exit(1);
}

void
load_plugin(const char *library, const char *plugin_symb, int j)
{
	PLUGIN         *(*func) (void);
	char           *error;
	char            strnum[FMT_ULONG];

	out("plugin ");
	strnum[fmt_ulong(strnum, j)] = 0;
	out(strnum);
	out(": ");
	out(library);
	out("\n");
	flush();
	if (!(handle[j] = dlopen(library, RTLD_NOW|RTLD_GLOBAL)))
		die_plugin("plugtest: dlopen failed for ", library, ": ", dlerror());
	dlerror(); /*- man page told me to do this */
	*(void **) (&func) = dlsym(handle[j], plugin_symb);
	if ((error = dlerror()))
		die_plugin("plugtest: dlsym ", plugin_symb, " failed: ", error);
	/*- execute the function */
	if (!(plug[j] = (*func) ())) /*- this function returns a pointer to PLUGIN */
		die_plugin("plugtest: function ", plugin_symb, " failed", 0);
	return;
}

const char     *usage =
				"usage: plugtest -MTD -l localip -r remoteip  -R remotehost -m mailfrom recipient ...]\n"
				"        -M test mail plugin\n"
				"        -T test rcpt plugin\n"
				"        -D test data plugin";

int
main(int argc, char **argv)
{
	const char      *localip, *remoteip, *remotehost, *mailfrom,
					*plugindir, *start_plugin, *plugin_symb;
	char           *mesg;
	char          **argv_ptr;
	char            strnum[FMT_ULONG];
	int             opt, i, j, len, mail_opt, rcpt_opt, data_opt, plugin_count, status;
	stralloc        plugin = { 0 };

	mail_opt = rcpt_opt = data_opt = 0;
	localip = remoteip = remotehost = mailfrom = (char *) 0;
	while ((opt = getopt(argc, argv, "MTDiIr:R:l:m:")) != opteof) {
		switch (opt) {
		case 'i':
			authenticated = 1;
		case 'I':
			relayclient = "";
		case 'r':
			remoteip = optarg;
			break;
		case 'R':
			remotehost = optarg;
			break;
		case 'l':
			localip = optarg;
			break;
		case 'm':
			mailfrom = optarg;
			break;
		case 'M':
			mail_opt = 1;
			break;
		case 'T':
			rcpt_opt = 1;
			break;
		case 'D':
			data_opt = 1;
			break;
		}
	}
	argc -= optind;
	argv += optind;
	if (!mail_opt && !rcpt_opt && !data_opt)
		strerr_die1x(100, usage);
	if (mail_opt || rcpt_opt || data_opt) {
		if (!remoteip) {
			out("remoteip not specified\n");
			flush();
			_exit(100);
		}
		if (!mailfrom) {
			out("mailfrom not specified\n");
			flush();
			_exit(100);
		}
	}
	if ((rcpt_opt || data_opt) && !argc) {
		out("recipients not specified\n");
		flush();
		_exit(100);
	}
	if (data_opt) {
		if (!remoteip) {
			out("localip not specified\n");
			flush();
			_exit(100);
		}
	}
	if (chdir(auto_qmail) == -1)
		strerr_die2sys(111, FATAL, auto_qmail);
	if (!(plugin_symb = env_get("SMTP_PLUGIN_SYMB")))
		plugin_symb = "plugin_init";

	if (!(plugindir = env_get("PLUGINDIR"))) {
		if (!stralloc_copys(&plugin, auto_prefix))
			die_nomem();
		if (!stralloc_catb(&plugin, "/lib/indimail/plugins/", 22))
			die_nomem();
	} else {
		if (*plugindir != '/')
			die_plugin(plugindir, "plugindir must have an absolute path", 0, 0);
		if (!stralloc_copys(&plugin, plugindir))
			die_nomem();
		if (!stralloc_append(&plugin, "/"))
			die_nomem();
	}

	if (!(start_plugin = env_get("SMTP_PLUGIN")))
		start_plugin = "smtpd-plugin.so";
	if (!stralloc_cats(&plugin, start_plugin))
		die_nomem();
	if (!stralloc_0(&plugin))
		die_nomem();

	len = plugin.len;
	/*- figure out plugin count */
	for (i = plugin_count = 0;;plugin_count++) {
		if (!plugin_count) {
			if (access(plugin.s, R_OK)) {
				if (errno != error_noent)
					strerr_die3sys(111, FATAL, plugin.s, ": ");
				plugin.len -= 4;
				strnum[fmt_ulong(strnum, i++)] = 0;
				if (!stralloc_catb(&plugin, strnum, 1))
					die_nomem();
				if (!stralloc_cats(&plugin, ".so"))
					die_nomem();
				if (!stralloc_0(&plugin))
					die_nomem();
				if (access(plugin.s, R_OK)) {
					if (errno != error_noent)
						strerr_die3sys(111, FATAL, plugin.s, ": ");
					out("plugtest: no plugins found\n");
					return (0);
				}
			}
		} else {
			plugin.len = len - 4;
			strnum[fmt_ulong(strnum, i++)] = 0;
			if (!stralloc_catb(&plugin, strnum, 1))
				die_nomem();
			if (!stralloc_cats(&plugin, ".so"))
				die_nomem();
			if (!stralloc_0(&plugin))
				die_nomem();
			if (access(plugin.s, R_OK))
				break;
		}
	}
	if (!(handle = (void **) alloc(sizeof(void *) * plugin_count)))
		die_nomem();
	if (!(plug = (PLUGIN **) alloc(sizeof(PLUGIN *) * plugin_count)))
		die_nomem();
	plugin.len = len - 4;
	if (!stralloc_cats(&plugin, ".so"))
		die_nomem();
	if (!stralloc_0(&plugin))
		die_nomem();
	for (i = j = 0;i < plugin_count;) {
		if (!j) {
			if (access(plugin.s, R_OK)) { /*- smtpd-plugin.so */
				plugin.len -= 4;
				strnum[fmt_ulong(strnum, i)] = 0;
				if (!stralloc_catb(&plugin, strnum, 1))
					die_nomem();
				if (!stralloc_cats(&plugin, ".so"))
					die_nomem();
				if (!stralloc_0(&plugin))
					die_nomem();
				if (access(plugin.s, R_OK)) { /*- smtpd-plugin0.so */
					out("plugtest: no plugins found\n");
					return (0);
				}
				load_plugin(plugin.s, plugin_symb, j++);
				i++;
			} else
				load_plugin(plugin.s, plugin_symb, j++);
		} else { /*- smtpd-plugin1.so, smtpd-plugin2.so, ... */
			plugin.len = len - 4;
			strnum[fmt_ulong(strnum, i)] = 0;
			if (!stralloc_catb(&plugin, strnum, 1))
				die_nomem();
			if (!stralloc_cats(&plugin, ".so"))
				die_nomem();
			if (!stralloc_0(&plugin))
				die_nomem();
			if (access(plugin.s, R_OK))
				break;
			load_plugin(plugin.s, plugin_symb, j++);
			i++;
		}
	}

	argv_ptr = argv;
	for (status = i = 0;i < plugin_count;i++) {
		if (!plug[i]) {
			out("plugin ");
			strnum[fmt_ulong(strnum, i)] = 0;
			out(strnum);
			out(": no plugin function returned\n");
			continue;
		}
		if (mail_opt) {
			if (!plug[i]->mail_func) {
				out("plugin ");
				strnum[fmt_ulong(strnum, i)] = 0;
				out(strnum);
				out(": no mail plugin returned\n");
			} else
			if (plug[i]->mail_func(remoteip, mailfrom, &mesg)) {
				out("plugin ");
				strnum[fmt_ulong(strnum, i)] = 0;
				out(strnum);
				out(": mail plugin returned non-zero: ");
				out(mesg);
				status = 1;
			} else {
				out("plugin ");
				strnum[fmt_ulong(strnum, i)] = 0;
				out(strnum);
				out(": from plugin success\n");
			}
		}
		if (rcpt_opt) {
			if (!plug[i]->rcpt_func) {
				out("plugin ");
				strnum[fmt_ulong(strnum, i)] = 0;
				out(strnum);
				out(": no rcpt plugin returned\n");
			} else {
				while (*argv_ptr) {
					if (plug[i]->rcpt_func(remoteip, mailfrom, *argv_ptr++, &mesg)) {
						out("plugin ");
						strnum[fmt_ulong(strnum, i)] = 0;
						out(strnum);
						out(": rcpt plugin returned non-zero: ");
						out(mesg);
						status = 1;
					} else {
						out("plugin ");
						strnum[fmt_ulong(strnum, i)] = 0;
						out(strnum);
						out(": rcpt plugin success\n");
					}
				}
			}
		}
		if (data_opt) {
			if (!plug[i]->data_func) {
				out("plugin ");
				strnum[fmt_ulong(strnum, i)] = 0;
				out(strnum);
				out(": no data plugin returned\n");
			}
			else {
				while (*argv_ptr) {
					if (plug[i]->data_func(localip, remoteip, mailfrom, *argv_ptr++, &mesg)) {
						out("plugin ");
						strnum[fmt_ulong(strnum, i)] = 0;
						out(strnum);
						out(": data plugin returned non-zero: ");
						out(mesg);
						status = 1;
					} else {
						out("plugin ");
						strnum[fmt_ulong(strnum, i)] = 0;
						out(strnum);
						out(": data plugin success\n");
					}
				}
			}
		}
	} /*- for (i = 0;i < plugin_count;i++) */
	for (i = 0;i < plugin_count;i++)
		dlclose(handle[i]);
	out(status ? "reject\n" : "accept\n");
	flush();
	return (status);
}

void
getversion_plugtest_c()
{
	const char     *x = "$Id: plugtest.c,v 1.8 2024-05-09 22:03:17+05:30 mbhangui Exp mbhangui $";

	x++;
}
