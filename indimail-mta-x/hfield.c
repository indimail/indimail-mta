/*
 * $Log: hfield.c,v $
 * Revision 1.5  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.4  2004-10-22 20:25:43+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.3  2004-07-17 21:19:06+05:30  Cprogrammer
 * added RCS log
 *
 */
#include "hfield.h"

const char     *(hname[]) =
{
	"unknown-header",
	"sender",
	"from",
	"reply-to",
	"to",
	"cc",
	"bcc",
	"date",
	"message-id",
	"subject",
	"resent-sender",
	"resent-from",
	"resent-reply-to",
	"resent-to",
	"resent-cc",
	"resent-bcc",
	"resent-date",
	"resent-message-id",
	"return-receipt-to",
	"errors-to",
	"apparently-to",
	"received",
	"return-path",
	"delivered-to",
	"content-length",
	"content-type",
	"content-transfer-encoding",
	"notice-requested-upon-delivery-to",
	"mail-followup-to",
	0
};

static int
hmatch(const char *s, int len, const char *t)
{
	int             i;
	char            ch;

	for (i = 0; (ch = t[i]); ++i) {
		if (i >= len)
			return 0;
		if (ch != s[i]) {
			if (ch == '-')
				return 0;
			if (ch - 32 != s[i])
				return 0;
		}
	}
	for (;;) {
		if (i >= len)
			return 0;
		ch = s[i];
		if (ch == ':')
			return 1;
		if ((ch != ' ') && (ch != '\t'))
			return 0;
		++i;
	}
}

int
hfield_known(const char *s, int len)
{
	int             i;
	const char     *t;

	for (i = 1; (t = hname[i]); ++i)
		if (hmatch(s, len, t))
			return i;
	return 0;
}

int
hfield_valid(const char *s, int len)
{
	int             i, j;
	char            ch;

	for (j = 0; j < len; ++j)
		if (s[j] == ':')
			break;
	if (j >= len)
		return 0;
	while (j)
	{
		ch = s[j - 1];
		if ((ch != ' ') && (ch != '\t'))
			break;
		--j;
	}
	if (!j)
		return 0;
	for (i = 0; i < j; ++i)
	{
		ch = s[i];
		if (ch <= 32)
			return 0;
		if (ch >= 127)
			return 0;
	}
	return 1;
}

unsigned int
hfield_skipname(const char *s, int len)
{
	int             i;
	char            ch;

	for (i = 0; i < len; ++i)
		if (s[i] == ':')
			break;
	if (i < len)
		++i;
	while (i < len)
	{
		ch = s[i];
		if ((ch != '\t') && (ch != '\n') && (ch != '\r') && (ch != ' '))
			break;
		++i;
	}
	return i;
}

void
getversion_hfield_c()
{
	const char     *x = "$Id: hfield.c,v 1.5 2024-05-09 22:03:17+05:30 mbhangui Exp mbhangui $";

	x++;
}
