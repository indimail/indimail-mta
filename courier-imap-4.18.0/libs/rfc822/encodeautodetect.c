/*
** Copyright 2003-2011 Double Precision, Inc.  See COPYING for
** distribution information.
*/

/*
*/
#include	"encode.h"
#include	<string.h>
#include	<stdlib.h>
#include	"unicode/courier-unicode.h"

static const char *libmail_encode_autodetect(int use7bit,
					     int (*func)(void *), void *arg,
					     int *binaryflag)
{
	int	l=0;
	int	longline=0;
	int c;

	size_t charcnt=0;
	size_t bit8cnt=0;

	if (binaryflag)
		*binaryflag=0;

	while ((c = (*func)(arg)) != EOF)
	{
		unsigned char ch= (unsigned char)c;

		++charcnt;

		++l;
		if (ch < 0x20 || ch >= 0x80)
		{
			if (ch != '\t' && ch != '\r' && ch != '\n')
			{
				++bit8cnt;
				l += 2;
			}
		}

		if (ch == 0)
		{
			if (binaryflag)
				*binaryflag=1;

			return "base64";
		}

		if (ch == '\n')	l=0;
		else if (l > 990)
		{
			longline=1;
		}

	}

	if (use7bit || longline)
	{
		if (bit8cnt > charcnt / 10)
			return "base64";

		return "quoted-printable";
	}

	return bit8cnt ? "8bit":"7bit";
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

const char *libmail_encode_autodetect_fp(FILE *fp, int use7bit,
					 int *binaryflag)
{
	return libmail_encode_autodetect_fpoff(fp, use7bit, 0, -1,
					       binaryflag);
}

const char *libmail_encode_autodetect_fpoff(FILE *fp, int use7bit,
					    off_t start_pos, off_t end_pos,
					    int *binaryflag)
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

	rc = libmail_encode_autodetect(use7bit, &read_file, &fi,
				       binaryflag);
  
	if (fseek(fp, orig_pos, SEEK_SET) == (off_t)-1)
		return NULL;
	return rc;
}

const char *libmail_encode_autodetect_buf(const char *str, int use7bit)
{
	return libmail_encode_autodetect(use7bit, &read_string, &str,
					 NULL);
}
