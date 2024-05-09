/*
 * $Log: rcpthosts.c,v $
 * Revision 1.11  2022-12-24 22:33:27+05:30  Cprogrammer
 * converted function prototypes to ansic
 *
 * Revision 1.10  2016-05-17 19:44:58+05:30  Cprogrammer
 * use auto_control, set by conf-control to set control directory
 *
 * Revision 1.9  2011-01-14 22:23:01+05:30  Cprogrammer
 * set flagrh to -1 for all errors
 *
 * Revision 1.8  2009-04-06 08:58:27+05:30  Cprogrammer
 * reopen morercpthosts if closed
 *
 * Revision 1.7  2004-10-22 20:29:52+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.6  2003-12-20 01:48:56+05:30  Cprogrammer
 * use stralloc for preparing control file
 *
 * Revision 1.5  2003-10-23 01:27:21+05:30  Cprogrammer
 * fixed compilation warnings
 *
 * Revision 1.4  2003-09-27 21:07:49+05:30  Cprogrammer
 * morercpthosts was not read correctly
 * added nolocal flag to allow parameter without '@' sign
 *
 */
#include "cdb.h"
#include "byte.h"
#include "case.h"
#include "open.h"
#include "fmt.h"
#include "error.h"
#include "control.h"
#include "constmap.h"
#include "stralloc.h"
#include "env.h"
#include "auto_control.h"
#include "variables.h"
#include "rcpthosts.h"

static int      flagrh = 0;
static stralloc rh = { 0 };
static struct constmap maprh;
static int      fdmrh = -1;
static stralloc morercpthosts;

int
rcpthosts_init()
{

	if ((flagrh = control_readfile(&rh, "rcpthosts", 0)) != 1)
		return flagrh;
	if (!constmap_init(&maprh, rh.s, rh.len, 0))
		return flagrh = -1;
	if (!controldir) {
		if (!(controldir = env_get("CONTROLDIR")))
			controldir = auto_control;
	}
	if (!stralloc_copys(&morercpthosts, controldir))
		return (flagrh = -1);
	if (morercpthosts.s[morercpthosts.len - 1] != '/' && !stralloc_cats(&morercpthosts, "/"))
		return (flagrh = -1);
	if (!stralloc_cats(&morercpthosts, "morercpthosts.cdb"))
		return (flagrh = -1);
	if (!stralloc_0(&morercpthosts))
		return (flagrh = -1);
	if (fdmrh == -1 && (fdmrh = open_read(morercpthosts.s)) == -1) {
		if (errno != error_noent)
			return flagrh = -1;
	}
	return 0;
}

static stralloc host = { 0 };

int
rcpthosts(const char *buf, int len, int nolocal)
{
	int             j;
	char           *b;

	if (nolocal == 0) {
		if (flagrh != 1)
			return 1;
		if ((j = byte_rchr(buf, len, '@')) >= len)
			return 1; /*- presumably envnoathost is acceptable */
		++j;
	} else {
		if (flagrh != 1)
			return 0;
		if ((j = byte_rchr(buf, len, '@')) >= len)
			j = 0;
		else
			++j;
	}
	buf += j;
	len -= j;
	if (!stralloc_copyb(&host, buf, len))
		return -1;
	b = host.s;
	case_lowerb(b, len);
	for (j = 0; j < len; ++j) {
		if ((!j || (b[j] == '.')) && constmap(&maprh, b + j, len - j))
			return 1;
	}
	if (fdmrh != -1) {
		uint32          dlen;
		int             r;

		for (j = 0; j < len; ++j) {
			if (!j || (b[j] == '.')) {
				if ((r = cdb_seek(fdmrh, b + j, len - j, &dlen))) {
					if (errno == error_ebadf) { /*- oops fdmrh got closed */
						if ((fdmrh = open_read(morercpthosts.s)) == -1)
							return r;
						if ((r = cdb_seek(fdmrh, b + j, len - j, &dlen)))
							return r;
					} else
						return r;
				}
			}
		}
	}
	return 0;
}

void
getversion_rcpthosts_c()
{
	const char     *x = "$Id: rcpthosts.c,v 1.11 2022-12-24 22:33:27+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
