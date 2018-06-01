/*
 * $Log: mess822_when.c,v $
 * Revision 1.3  2011-05-07 15:58:11+05:30  Cprogrammer
 * added headers for str & case function prototypes
 *
 * Revision 1.2  2004-10-22 20:27:32+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.1  2004-01-04 23:18:19+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "mess822.h"
#include "scan.h"
#include "str.h"
#include "case.h"

static stralloc tokens = { 0 };
static struct caltime ct;

int
mess822_when(out, in)
	mess822_time   *out;
	char           *in;
{
	int             state;
	int             pos;
	int             flagnumeric;
	long            number;
	char           *x;
	int             known;

	if (!mess822_token(&tokens, in))
		return 0;
	for (pos = tokens.len; pos > 0; --pos)
		if (pos >= 3)
			if (!tokens.s[pos - 1] && !tokens.s[pos - 3])
				if ((tokens.s[pos - 2] == ';') || (tokens.s[pos - 2] == ','))
					break;
  /*-          29 Aug 1996 14 : 04 : 52 CDT   */
  /*- states: 1  2   3    4  5 6  7 8  9   10 */
	state = 1;
	known = 0;
	while (pos < tokens.len)
	{
		x = tokens.s + pos;
		pos += str_len(x) + 1;
		if (*x == ':')
		{
			if (state == 5)
				state = 6;
			if (state == 7)
				state = 8;
		}
		if (*x == '=')
		{
			++x;
			flagnumeric = !x[scan_long(x, &number)];
			switch (state)
			{
			case 1:
				if (!flagnumeric)
					return 1;
				ct.date.day = number;
				break;
			case 2:
				if (!case_diffs(x, "Jan"))
				{
					ct.date.month = 1;
					break;
				}
				if (!case_diffs(x, "Feb"))
				{
					ct.date.month = 2;
					break;
				}
				if (!case_diffs(x, "Mar"))
				{
					ct.date.month = 3;
					break;
				}
				if (!case_diffs(x, "Apr"))
				{
					ct.date.month = 4;
					break;
				}
				if (!case_diffs(x, "May"))
				{
					ct.date.month = 5;
					break;
				}
				if (!case_diffs(x, "Jun"))
				{
					ct.date.month = 6;
					break;
				}
				if (!case_diffs(x, "Jul"))
				{
					ct.date.month = 7;
					break;
				}
				if (!case_diffs(x, "Aug"))
				{
					ct.date.month = 8;
					break;
				}
				if (!case_diffs(x, "Sep"))
				{
					ct.date.month = 9;
					break;
				}
				if (!case_diffs(x, "Oct"))
				{
					ct.date.month = 10;
					break;
				}
				if (!case_diffs(x, "Nov"))
				{
					ct.date.month = 11;
					break;
				}
				if (!case_diffs(x, "Dec"))
				{
					ct.date.month = 12;
					break;
				}
				return 1;
			case 3:
				if (!flagnumeric)
					return 1;
				if (number < 50)
					number += 2000;
				if (number < 999)
					number += 1900;
				ct.date.year = number;
				break;
			case 4:
				if (!flagnumeric)
					return 1;
				ct.hour = number;
				break;
			case 5:
				return 1;
			case 6:
				if (!flagnumeric)
					return 1;
				ct.minute = number;
				break;
			case 8:
				if (!flagnumeric)
					return 1;
				ct.second = number;
				break;
			case 7:
				ct.second = 0;
				state = 9;
			case 9:
				known = 2;
				if (flagnumeric)
				{	/*- happiness */
					if (number >= 0)
						ct.offset = (number / 100) * 60 + (number % 100);
					else
						ct.offset = -(((-number) / 100) * 60 + ((-number) % 100));
					if (!case_diffs(x, "-0000"))
						known = 1;
					break;
				}
				if (!case_diffs(x, "UT"))
				{
					ct.offset = 0;
					break;
				}
				if (!case_diffs(x, "GMT"))
				{
					ct.offset = 0;
					break;
				}
				/*
				 * XXX: GMT+nnnn? 
				 */
				if (!case_diffs(x, "BST"))
				{
					ct.offset = 60;
					break;
				}
				if (!case_diffs(x, "CDT"))
				{
					ct.offset = -300;
					break;
				}
				if (!case_diffs(x, "CET"))
				{
					ct.offset = 60;
					break;
				}
				if (!case_diffs(x, "CST"))
				{
					ct.offset = -360;
					break;
				}
				if (!case_diffs(x, "EDT"))
				{
					ct.offset = -240;
					break;
				}
				if (!case_diffs(x, "EET"))
				{
					ct.offset = 120;
					break;
				}
				if (!case_diffs(x, "EST"))
				{
					ct.offset = -300;
					break;
				}
				if (!case_diffs(x, "HKT"))
				{
					ct.offset = 480;
					break;
				}
				if (!case_diffs(x, "IST"))
				{
					ct.offset = 120;
					break;
				}
				if (!case_diffs(x, "JST"))
				{
					ct.offset = 540;
					break;
				}
				if (!case_diffs(x, "MDT"))
				{
					ct.offset = -360;
					break;
				}
				if (!case_diffs(x, "MET"))
				{
					ct.offset = 60;
					break;
				}
				if (!case_diffs(x, "METDST"))
				{
					ct.offset = 120;
					break;
				}
				if (!case_diffs(x, "MST"))
				{
					ct.offset = -420;
					break;
				}
				if (!case_diffs(x, "PDT"))
				{
					ct.offset = -420;
					break;
				}
				if (!case_diffs(x, "PST"))
				{
					ct.offset = -480;
					break;
				}
				return 1;
			case 10:
				if (!case_diffs(x, "DST"))
					ct.offset += 60;
				break;
			}
			if (state < 10)
				++state;
		}
	}
	if (known)
	{
		out->known = known;
		out->ct = ct;
	}
	return 1;
}

void
getversion_mess822_when_c()
{
	static char    *x = "$Id: mess822_when.c,v 1.3 2011-05-07 15:58:11+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
