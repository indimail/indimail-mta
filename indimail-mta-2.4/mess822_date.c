/*
 * $Log: mess822_date.c,v $
 * Revision 1.2  2004-10-22 20:27:27+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.1  2004-01-04 23:17:09+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "mess822.h"
#include "stralloc.h"

static char    *montab[12] = {
	"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

int
mess822_date(out, when)
	stralloc       *out;
	mess822_time   *when;
{
	long            i;

	if (!stralloc_copys(out, ""))
		return 0;
	if (!when->known)
		return 1;
	if (!stralloc_catint(out, when->ct.date.day))
		return 0;
	if (!stralloc_cats(out, " "))
		return 0;
	i = when->ct.date.month - 1;
	if (i < 0)
		i = 0;
	if (i > 11)
		i = 11;
	if (!stralloc_cats(out, montab[i]))
		return 0;
	if (!stralloc_cats(out, " "))
		return 0;
	if (!stralloc_catlong(out, when->ct.date.year))
		return 0;
	if (!stralloc_cats(out, " "))
		return 0;
	if (!stralloc_catint0(out, when->ct.hour, 2))
		return 0;
	if (!stralloc_cats(out, ":"))
		return 0;
	if (!stralloc_catint0(out, when->ct.minute, 2))
		return 0;
	if (!stralloc_cats(out, ":"))
		return 0;
	if (!stralloc_catint0(out, when->ct.second, 2))
		return 0;
	if (when->known == 1)
	{
		if (!stralloc_cats(out, " -0000"))
			return 0;
	} else
	{
		i = when->ct.offset;
		if (i < 0)
		{
			if (!stralloc_cats(out, " -"))
				return 0;
			i = -i;
		} else
		{
			if (!stralloc_cats(out, " +"))
				return 0;
		}
		if (!stralloc_catint0(out, (int) (i / 60), 2))
			return 0;
		if (!stralloc_catint0(out, (int) (i % 60), 2))
			return 0;
	}
	return 1;
}

void
getversion_mess822_date_c()
{
	static char    *x = "$Id: mess822_date.c,v 1.2 2004-10-22 20:27:27+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
