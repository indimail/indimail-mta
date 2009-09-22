/*
 * $Log: rwhconfig.c,v $
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
#include <unistd.h>
#include "auto_qmail.h"

struct strerr   rwhconfig_err;

static stralloc tmp = { 0 };
static config_str me = CONFIG_STR;
static config_str defaultdomain = CONFIG_STR;
static config_str defaulthost = CONFIG_STR;
static config_str plusdomain = CONFIG_STR;
static config_str idhost = CONFIG_STR;

int
rwhconfig(rewrite, idappend)
	config_str     *rewrite;
	stralloc       *idappend;
{
	if (config_readline(&me, "control/me") == -1)
		STRERR_SYS3(-1, rwhconfig_err, "unable to read ", auto_qmail, "/control/me: ")
	if (!config(rewrite))
	{
		if (config_readfile(rewrite, "control/rewrite") == -1)
			STRERR_SYS3(-1, rwhconfig_err, "unable to read ", auto_qmail, "/control/rewrite: ")
		if (!config(rewrite))
		{
			if (config_env(&defaulthost, "QMAILDEFAULTHOST") == -1)
				goto nomem;
			if (config_readline(&defaulthost, "control/defaulthost") == -1)
				STRERR_SYS3(-1, rwhconfig_err, "unable to read ", auto_qmail, "/control/defaulthost: ")
			if (config_copy(&defaulthost, &me) == -1)
				goto nomem;
			if (config_default(&defaulthost, "DEFAULTHOST") == -1)
				goto nomem;

			if (config_env(&defaultdomain, "QMAILDEFAULTDOMAIN") == -1)
				goto nomem;
			if (config_readline(&defaultdomain, "control/defaultdomain") == -1)
				STRERR_SYS3(-1, rwhconfig_err, "unable to read ", auto_qmail, "/control/defaultdomain: ")
			if (config_copy(&defaultdomain, &me) == -1)
				goto nomem;
			if (config_default(&defaultdomain, "DEFAULTDOMAIN") == -1)
				goto nomem;

			if (config_env(&plusdomain, "QMAILPLUSDOMAIN") == -1)
				goto nomem;
			if (config_readline(&plusdomain, "control/plusdomain") == -1)
				STRERR_SYS3(-1, rwhconfig_err, "unable to read ", auto_qmail, "/control/plusdomain: ")
			if (config_copy(&plusdomain, &me) == -1)
				goto nomem;
			if (config_default(&plusdomain, "PLUSDOMAIN") == -1)
				goto nomem;

			if (!stralloc_copys(config_data(rewrite), "*.:"))
				goto nomem;
			if (!stralloc_0(config_data(rewrite)))
				goto nomem;
			if (!stralloc_cats(config_data(rewrite), "=:"))
				goto nomem;
			if (!stralloc_cat(config_data(rewrite), config_data(&defaulthost)))
				goto nomem;
			if (!stralloc_0(config_data(rewrite)))
				goto nomem;
			if (!stralloc_cats(config_data(rewrite), "*+:."))
				goto nomem;
			if (!stralloc_cat(config_data(rewrite), config_data(&plusdomain)))
				goto nomem;
			if (!stralloc_0(config_data(rewrite)))
				goto nomem;
			if (!stralloc_cats(config_data(rewrite), "?:."))
				goto nomem;
			if (!stralloc_cat(config_data(rewrite), config_data(&defaultdomain)))
				goto nomem;
			if (!stralloc_0(config_data(rewrite)))
				goto nomem;
		}
	}
	if (config_env(&idhost, "QMAILIDHOST") == -1)
		goto nomem;
	if (config_readline(&idhost, "control/idhost") == -1)
		STRERR_SYS3(-1, rwhconfig_err, "unable to read ", auto_qmail, "/control/idhost: ")
	if (config_copy(&idhost, &me) == -1)
		goto nomem;
	if (config_default(&idhost, "IDHOST") == -1)
		goto nomem;

	if (!rewritehost(&tmp, config_data(&idhost)->s, config_data(&idhost)->len, config_data(rewrite)))
		goto nomem;

	if (!stralloc_copys(idappend, "."))
		goto nomem;
	if (!stralloc_catint(idappend, (int) getpid()))
		goto nomem;
	if (!stralloc_cats(idappend, ".qmail@"))
		goto nomem;
	if (!stralloc_cat(idappend, &tmp))
		goto nomem;

	return 0;

nomem:
	STRERR_SYS(-1, rwhconfig_err, 0)
}

void
getversion_rwhconfig_c()
{
	static char    *x = "$Id: rwhconfig.c,v 1.2 2004-10-22 20:30:04+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
