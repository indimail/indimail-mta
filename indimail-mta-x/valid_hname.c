/*
 * $Id: valid_hname.c,v 1.1 2023-12-03 12:17:05+05:30 Cprogrammer Exp mbhangui $
 */
#include <ctype.h>
#include <str.h>
#include "valid_hname.h"

/*
 * valid_hostname() scrutinizes a hostname: the name should
 * be no longer than MAX_HNAME_LEN characters, should
 * contain only letters, digits, dots and hyphens, no adjacent
 * dots, no leading or trailing dots or hyphens, no labels
 * longer than MAX_LABEL_LEN characters, and it should not
 * be all numeric.
 *
 * Author(S)
 *      Wietse Venema
 *      IBM T.J. Watson Research
 *      P.O. Box 704
 *      Yorktown Heights, NY 10598, USA
 */
int
valid_hname(char *name)
{
	const char     *cp;
	int             label_length = 0, ch, non_numeric = 0;

	if (!name || !*name)
		return (0);
	/*
	 * Find bad characters or label lengths. Find adjacent delimiters.
	 */
	for (cp = name; (ch = *(unsigned char *) cp) != 0; cp++) {
		if (isalnum(ch) || ch == '_') {
			label_length++;
			if (label_length > MAX_LABEL_LEN)
				return (0);
			if (!isdigit(ch))
				non_numeric = 1;
		} else
		if (ch == '.') { /*- cannot end with dot */
			if (label_length == 0 || cp[1] == 0)
				return (0);
			label_length = 0;
		} else
		if (ch == '-') {
			non_numeric = 1;
			label_length++;
			/*- last char of a label cannot be - */
			if (label_length == 1 || cp[1] == 0 || cp[1] == '.')
				return (0);
		} else
			return (0);
	}
	if (!non_numeric)
		return (0);
	if (cp - name > MAX_HNAME_LEN)
		return (0);
	return (1);
}

/*
 * $Log: valid_hname.c,v $
 * Revision 1.1  2023-12-03 12:17:05+05:30  Cprogrammer
 * Initial revision
 *
 */
