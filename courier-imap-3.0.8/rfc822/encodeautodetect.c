/*
** Copyright 2003-2009 Double Precision, Inc.  See COPYING for
** distribution information.
*/

/*
** $Id: encodeautodetect.c,v 1.2 2009/11/08 18:14:47 mrsam Exp $
*/
#include	"encode.h"
#include	<string.h>
#include	<stdlib.h>
#include	"../unicode/unicode.h"

static const char *libmail_encode_autodetect(const char *charset, 
					     int (*func)(void *), void *arg)
{
	const char *encoding="7bit";
	int	l=0;
	int	longline=0;
	int c;
	const struct unicode_info *ci = unicode_find(charset);

	while ((c = (*func)(arg)) != EOF)
	{
		unsigned char ch= (unsigned char)c;

		if (ch >= 0x80)
		{

			if (!charset || !*charset)
				encoding="8bit";
			else if (ci && ci->flags & UNICODE_BODY_QUOPRI)
				encoding="quoted-printable";
			else if (!ci || ci->flags & UNICODE_BODY_BASE64)
				encoding="base64";
			else
				encoding="8bit";
		}

		if (ch < 0x20 &&
		    ch != '\t' && ch != '\r' && ch != '\n')
		{
			if (!charset || !*charset)
				;
			else if (ci && ci->flags & UNICODE_BODY_QUOPRI)
				encoding="quoted-printable";
			else if (!ci || ci->flags & UNICODE_BODY_BASE64)
				encoding="base64";
                }

		if (ch == 0)
			return "base64";

		if (ch == '\n')	l=0;
		else if (++l > 990)
		{
			longline=1;
			if (ci && ci->flags & UNICODE_BODY_QUOPRI)
				encoding="quoted-printable";
		}

	}

	if (longline)
	{
		if (ci && ci->flags & UNICODE_BODY_QUOPRI)
			encoding="quoted-printable";
		else
			encoding="base64";
	}
	return encoding;
}

struct file_info {
	FILE *fp;
	off_t pos;
	off_t end;
};

static int read_file(void *arg)
{
int c;
struct file_info *fi = (struct file_info *)arg;
	if (fi->end >= 0 && fi->pos > fi->end)
		return EOF;
	c = getc(fi->fp);
	fi->pos++;
	return c;
}

static int read_string(void * arg)
{
int c;
unsigned char **strp = (unsigned char **)arg;
	if (**strp == 0)
		return EOF;
	c = (int)**strp;
	(*strp)++;
	return c;
}

const char *libmail_encode_autodetect_fp(FILE *fp, int okQp)
{
	if (okQp)
		return libmail_encode_autodetect_fppos(fp, "ISO-8859-1", 0, -1);
	else
		return libmail_encode_autodetect_fppos(fp, NULL, 0, -1);
}

const char *libmail_encode_autodetect_fppos(FILE *fp, const char *charset,
					    off_t start_pos, off_t end_pos)
{
struct file_info fi;
off_t orig_pos = ftell(fp);
off_t pos = orig_pos;
const char *rc;

	if (start_pos >= 0)
	{
		if (fseek(fp, start_pos, SEEK_SET) == (off_t)-1)
			return NULL;
		else
			pos = start_pos;
	}

	fi.fp = fp;
	fi.pos = pos;
	fi.end = end_pos;
	rc = libmail_encode_autodetect(charset, &read_file, &fi);
  
	if (fseek(fp, orig_pos, SEEK_SET) == (off_t)-1)
		return NULL;
	return rc;
}

const char *libmail_encode_autodetect_str(const char *str, const char *charset)
{
	return libmail_encode_autodetect(charset, &read_string, &str);
}
