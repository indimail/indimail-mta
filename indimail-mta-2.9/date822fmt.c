/*
 * $Log: date822fmt.c,v $
 * Revision 1.5  2011-02-12 15:44:47+05:30  Cprogrammer
 * added display of week days
 *
 * Revision 1.4  2004-10-22 20:24:13+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.3  2004-07-17 21:18:11+05:30  Cprogrammer
 * added RCS log
 *
 */
#include <time.h>
#include "datetime.h"
#include "fmt.h"
#include "date822fmt.h"

static char    *montab[12] = {
	"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};
static char    *daytab[7] = {
	"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
};

unsigned int
date822fmt(s, dt)
	char           *s;
	struct datetime *dt;
{
	unsigned int    i;
	unsigned int    len;
	time_t          now;
	datetime_sec    utc;
	datetime_sec    local;
	struct tm      *tm;
	struct datetime new_dt;
	int             minutes;

	utc = datetime_untai(dt);
	now = (time_t) utc;
	tm = localtime(&now);
	new_dt.year = tm->tm_year;
	new_dt.mon = tm->tm_mon;
	new_dt.mday = tm->tm_mday;
	new_dt.wday = tm->tm_wday;
	new_dt.hour = tm->tm_hour;
	new_dt.min = tm->tm_min;
	new_dt.sec = tm->tm_sec;
	local = datetime_untai(&new_dt);
	len = 0;

	i = fmt_str(s, daytab[new_dt.wday]);
	len += i;
	if (s)
		s += i;
	i = fmt_str(s, ", ");
	len += i;
	if (s)
		s += i;

	i = fmt_uint(s, new_dt.mday);
	len += i;
	if (s)
		s += i;
	i = fmt_str(s, " ");
	len += i;
	if (s)
		s += i;
	i = fmt_str(s, montab[new_dt.mon]);
	len += i;
	if (s)
		s += i;
	i = fmt_str(s, " ");
	len += i;
	if (s)
		s += i;
	i = fmt_uint(s, new_dt.year + 1900);
	len += i;
	if (s)
		s += i;
	i = fmt_str(s, " ");
	len += i;
	if (s)
		s += i;
	i = fmt_uint0(s, new_dt.hour, 2);
	len += i;
	if (s)
		s += i;
	i = fmt_str(s, ":");
	len += i;
	if (s)
		s += i;
	i = fmt_uint0(s, new_dt.min, 2);
	len += i;
	if (s)
		s += i;
	i = fmt_str(s, ":");
	len += i;
	if (s)
		s += i;
	i = fmt_uint0(s, new_dt.sec, 2);
	len += i;
	if (s)
		s += i;
	if (local < utc)
	{
		minutes = (utc - local + 30) / 60;
		i = fmt_str(s, " -");
		len += i;
		if (s)
			s += i;
		i = fmt_uint0(s, minutes / 60, 2);
		len += i;
		if (s)
			s += i;
		i = fmt_uint0(s, minutes % 60, 2);
		len += i;
		if (s)
			s += i;
	} else
	{
		minutes = (local - utc + 30) / 60;
		i = fmt_str(s, " +");
		len += i;
		if (s)
			s += i;
		i = fmt_uint0(s, minutes / 60, 2);
		len += i;
		if (s)
			s += i;
		i = fmt_uint0(s, minutes % 60, 2);
		len += i;
		if (s)
			s += i;
	}

	i = fmt_str(s, "\n");
	len += i;
	if (s)
		s += i;
	return len;
}

void
getversion_date822fmt_c()
{
	static char    *x = "$Id: date822fmt.c,v 1.5 2011-02-12 15:44:47+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
