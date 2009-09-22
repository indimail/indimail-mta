/*
 * $Log: rcpthosts.c,v $
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
#include "variables.h"
#include "rcpthosts.h"

static int      flagrh = 0;
static stralloc rh = { 0 };
static struct constmap maprh;
static int      fdmrh;
static stralloc morercpthosts;

int
rcpthosts_init()
{

	flagrh = control_readfile(&rh, "rcpthosts", 0);
	if (flagrh != 1)
		return flagrh;
	if (!constmap_init(&maprh, rh.s, rh.len, 0))
		return flagrh = -1;
	if(!controldir)
	{
		if(!(controldir = env_get("CONTROLDIR")))
			controldir = "control";
	}
	if (!stralloc_copys(&morercpthosts, controldir))
		return(-1);
	if (morercpthosts.s[morercpthosts.len - 1] != '/' && !stralloc_cats(&morercpthosts, "/"))
		return(-1);
	if (!stralloc_cats(&morercpthosts, "morercpthosts.cdb"))
		return(-1);
	if (!stralloc_0(&morercpthosts))
		return(-1);
	if ((fdmrh = open_read(morercpthosts.s)) == -1)
	{
		if (errno != error_noent)
			return flagrh = -1;
	}
	return 0;
}

static stralloc host = { 0 };

int
rcpthosts(buf, len, nolocal)
	char           *buf;
	int             len, nolocal;
{
	int             j;

	if(nolocal == 0)
	{
		if (flagrh != 1)
			return 1;
		if((j = byte_rchr(buf, len, '@')) >= len)
			return 1;	/*- presumably envnoathost is acceptable */
		++j;
	} else
	{
		if (flagrh != 1)
			return 0;
		if((j = byte_rchr(buf, len, '@')) >= len)
			j = 0;
		else
			++j;
	}
	buf += j;
	len -= j;
	if (!stralloc_copyb(&host, buf, len))
		return -1;
	buf = host.s;
	case_lowerb(buf, len);
	for (j = 0; j < len; ++j)
	{
		if ((!j || (buf[j] == '.')) && constmap(&maprh, buf + j, len - j))
			return 1;
	}
	if (fdmrh != -1)
	{
		uint32          dlen;
		int             r;

		for (j = 0; j < len; ++j)
		{
			if (!j || (buf[j] == '.'))
			{
				if((r = cdb_seek(fdmrh, buf + j, len - j, &dlen)))
				{
					if (errno == error_ebadf) /*- oops fdmrh got closed */
					{
						if ((fdmrh = open_read(morercpthosts.s)) == -1)
							return r;
						if((r = cdb_seek(fdmrh, buf + j, len - j, &dlen)))
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
	static char    *x = "$Id: rcpthosts.c,v 1.8 2009-04-06 08:58:27+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
