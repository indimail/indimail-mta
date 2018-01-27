#include <stdio.h>
#include <string.h>
#include "eps_content.h"

struct _content_t content_prefs[] = {
	{NULL, CON_UNKNOWN},
	{"text/", CON_TEXT},
	{"multipart/", CON_MULTI},
	{"message/", CON_MESSAGE},
	{NULL, CON_UNKNOWN}
};

struct _content_t encoding_prefs[] = {
	{NULL, ENC_UNKNOWN},
	{"7bit", ENC_7BIT},
	{"base64", ENC_BASE64},
	{"8bit", ENC_8BIT},
	{"quoted-printable", ENC_QP},
	{"raw", ENC_RAW},
	{NULL, ENC_UNKNOWN}
};

struct _content_t disposition_prefs[] = {
	{NULL, DIS_INLINE},
	{"attachment", DIS_ATTACH},
	{"inline", DIS_INLINE},
	{"formdata", DIS_FORMDATA},
	{NULL, DIS_INLINE}
};

int
content_parse(char *d, char type)
{
	int             i = 0;
	struct _content_t *c = NULL;

	if (type == TYP_CON)
		c = content_prefs;

	else
	if (type == TYP_ENC)
		c = encoding_prefs;

	else
	if (type == TYP_DIS)
		c = disposition_prefs;

	else
		return 0;

	/*
	 * Header data is empty, so we
	 * just return the default type
	 * which is always the first
	 * in the array.
	 */
	if (d == NULL)
		return c[0].type;

	for (i = 1; c[i].data; i++)
	{
		if (c[i].data)
		{
			if (!(strncasecmp(c[i].data, d, strlen(c[i].data))))
				return c[i].type;
		}
	}

	/*
	 * Return our default encoding type
	 * since we didnt find a match.
	 */

	return c[0].type;
}
