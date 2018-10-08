/*
** Copyright 2018 Double Precision, Inc.
** See COPYING for distribution information.
*/

#if	HAVE_CONFIG_H
#include	"config.h"
#endif
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<errno.h>
#include	<ctype.h>
#if	HAVE_UNISTD_H
#include	<unistd.h>
#endif

#include	"maildirinfo.h"
#include	"unicode/courier-unicode.h"


/* Count the size of the converted filename or foldername string */
static void count_utf8(const char *str, size_t n, void *arg)
{
	*(size_t *)arg += n;
}

/* Save the converted filename or foldername string */
static void save_utf8(const char *str, size_t n, void *arg)
{
	char **p=(char **)arg;

	memcpy( (*p), str, n);
	*p += n;
}

/*
** Split a string at periods, convert each split range between charsets.
*/
static int foldername_filename_convert(const char *src_chset,
				       const char *dst_chset,
				       const char *foldername,
				       void (*closure)(const char *, size_t,
						       void *),
				       void *arg)
{
	unicode_convert_handle_t h;
	char *cbufptr;
	size_t cbufsize;
	size_t l;
	int conversion_error;

	while (*foldername)
	{
		if (*foldername == '.')
		{
			(*closure)(foldername, 1, arg);
			++foldername;
			continue;
		}

		h=unicode_convert_tocbuf_init(src_chset,
					      dst_chset,
					      &cbufptr,
					      &cbufsize,
					      0);
		if (!h)
		{
			errno=EINVAL;
			return -1;
		}

		for (l=0; foldername[l] && foldername[l] != '.'; ++l)
			;

		unicode_convert(h, foldername, l);

		if (unicode_convert_deinit(h, &conversion_error) == 0)
		{
			closure(cbufptr, cbufsize, arg);
			free(cbufptr);
			if (conversion_error)
			{
				errno=EILSEQ;
				return -1;
			}
		}
		else
			return -1;
		foldername += l;
	}
	return 0;
}

/* Convert either a foldername or a filename to the other */

static char *foldername_filename_convert_tobuf(const char *src_chset,
					       const char *dst_chset,
					       const char *foldername)
{
	char *utf8, *p;
	size_t l=1;

	if (foldername_filename_convert(src_chset,
					dst_chset,
					foldername, count_utf8, &l))
		return NULL;

	utf8=malloc(l+1);

	if (!utf8)
		return NULL;

	p=utf8;
	if (foldername_filename_convert(src_chset,
					dst_chset,
					foldername, save_utf8, &p))
	{
		free(utf8);
		return NULL;
	}

	*p=0;

	return utf8;
}

char *imap_foldername_to_filename(int utf8_format, const char *foldername)
{
	if (utf8_format)
		return strdup(foldername);

	return foldername_filename_convert_tobuf
		(unicode_x_imap_modutf7,
		 "utf-8",
		 foldername);
}

char *imap_filename_to_foldername(int utf8_format, const char *filename)
{
	if (utf8_format)
		return strdup(filename);

	return foldername_filename_convert_tobuf
		("utf-8",
		 unicode_x_imap_modutf7,
		 filename);
}
