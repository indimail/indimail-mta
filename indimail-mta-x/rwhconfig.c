/*
 * $Log: rwhconfig.c,v $
 * Revision 1.6  2021-06-12 19:30:49+05:30  Cprogrammer
 * removed #include "auto_qmail.h"
 *
 * Revision 1.5  2020-04-30 18:10:08+05:30  Cprogrammer
 * define rwhconfig_err variable locally
 *
 * Revision 1.4  2016-05-17 19:44:58+05:30  Cprogrammer
 * use auto_control, set by conf-control to set control directory
 *
 * Revision 1.3  2010-07-21 09:02:42+05:30  Cprogrammer
 * use CONTROLDIR environment variable instead of a hardcoded control directory
 *
 * Revision 1.2  2004-10-22 20:30:04+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.1  2004-06-16 01:20:00+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "rewritehost.h"
#include "stralloc.h"
#include "sconfig.h"
#include "strerr.h"
#include "rwhconfig.h"
#include "auto_control.h"
#include "variables.h"
#include "env.h"
#include <unistd.h>

static struct strerr rwhconfig_err;
static stralloc tmp = { 0 }, fn = {0};
static config_str me = CONFIG_STR;
static config_str defaultdomain = CONFIG_STR;
static config_str defaulthost = CONFIG_STR;
static config_str plusdomain = CONFIG_STR;
static config_str idhost = CONFIG_STR;

static int
my_config_read(config_str *c, const char *fname, int line)
{
	if (!controldir) {
		if (!(controldir = env_get("CONTROLDIR")))
			controldir = auto_control;
	}
	if (!stralloc_copys(&fn, controldir) ||
			!stralloc_cats(&fn, "/") ||
			!stralloc_cats(&fn, fname) ||
			!stralloc_0(&fn))
		STRERR_SYS(-1, rwhconfig_err, 0)
	if ((line ? config_readline : config_readfile) (c, fn.s) == -1)
		STRERR_SYS3(-1, rwhconfig_err, "unable to read ", fn.s, ": ")
	return (0);
}

int
rwhconfig(config_str *rewrite, stralloc *idappend)
{
	int             i;

	if ((i = my_config_read(&me, "me", 1)))
		return (i);
	if (!config(rewrite)) {
		if ((i = my_config_read(rewrite, "rewrite", 0)))
			return (i);
		if (!config(rewrite)) {
			if (config_env(&defaulthost, "QMAILDEFAULTHOST") == -1)
				goto nomem;
			if ((i = my_config_read(&defaulthost, "defaulthost", 1)))
				return (i);
			if (config_copy(&defaulthost, &me) == -1 ||
					config_default(&defaulthost, "DEFAULTHOST") == -1)
				goto nomem;

			if (config_env(&defaultdomain, "QMAILDEFAULTDOMAIN") == -1)
				goto nomem;
			if ((i = my_config_read(&defaultdomain, "defaultdomain", 1)))
				return (i);
			if (config_copy(&defaultdomain, &me) == -1 ||
					config_default(&defaultdomain, "DEFAULTDOMAIN") == -1)
				goto nomem;

			if (config_env(&plusdomain, "QMAILPLUSDOMAIN") == -1)
				goto nomem;
			if ((i = my_config_read(&plusdomain, "plusdomain", 1)))
				return (i);
			if (config_copy(&plusdomain, &me) == -1 ||
					config_default(&plusdomain, "PLUSDOMAIN") == -1)
				goto nomem;

			if (!stralloc_copys(config_data(rewrite), "*.:") ||
					!stralloc_0(config_data(rewrite)) ||
					!stralloc_cats(config_data(rewrite), "=:") ||
					!stralloc_cat(config_data(rewrite), config_data(&defaulthost)) ||
					!stralloc_0(config_data(rewrite)) ||
					!stralloc_cats(config_data(rewrite), "*+:.") ||
					!stralloc_cat(config_data(rewrite), config_data(&plusdomain)) ||
					!stralloc_0(config_data(rewrite)) ||
					!stralloc_cats(config_data(rewrite), "?:.") ||
					!stralloc_cat(config_data(rewrite), config_data(&defaultdomain)) ||
					!stralloc_0(config_data(rewrite)))
				goto nomem;
		}
	}
	if (config_env(&idhost, "QMAILIDHOST") == -1)
		goto nomem;
	if ((i = my_config_read(&idhost, "idhost", 1)))
		return (i);
	if (config_copy(&idhost, &me) == -1 ||
			config_default(&idhost, "IDHOST") == -1)
		goto nomem;

	if (!rewritehost(&tmp, config_data(&idhost)->s, config_data(&idhost)->len, config_data(rewrite)))
		goto nomem;

	if (!stralloc_copys(idappend, ".") ||
			!stralloc_catint(idappend, (int) getpid()) ||
			!stralloc_cats(idappend, ".qmail@") ||
			!stralloc_cat(idappend, &tmp))
		goto nomem;

	return 0;

nomem:
	STRERR_SYS(-1, rwhconfig_err, 0)
}

void
getversion_rwhconfig_c()
{
	const char     *x = "$Id: rwhconfig.c,v 1.6 2021-06-12 19:30:49+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
