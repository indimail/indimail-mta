/*
** Copyright 2000-2011 Double Precision, Inc.  See COPYING for
** distribution information.
*/

#include "rfc2045_config.h"
#include	"rfc2045.h"
#include	"unicode/courier-unicode.h"
#include	<stdio.h>
#include	<unistd.h>
#include	<iconv.h>
#include	<errno.h>

/*
** Call rfc2045_decodemimesection, expecting textual content.  Convert
** textual content to local character set, if possible.  This is implemented
** by saving the real callback function, then calling rfc2045_decodemimesection
** and specifying our own callback function, which does the conversion, then
** calls the original callback function.  Neat, eh?
*/

static int myhandler(const char *, size_t, void *);

int rfc2045_decodetextmimesection(struct rfc2045src *src,
				  struct rfc2045 *rfc,
				  const char *mychset,
				  int *conv_err,
				  int (*handler)(const char *,
						 size_t, void *),
				  void *voidarg)
{
	const char *dummy;
	const char *src_chset;

	unicode_convert_handle_t ci;
	int rc;
	int dummy_flag;

	if (!conv_err)
		conv_err= &dummy_flag;

	rfc2045_mimeinfo(rfc, &dummy, &dummy, &src_chset);

	*conv_err=0;

	if ((ci=unicode_convert_init(src_chset, mychset, handler, voidarg))
	    == NULL)
	{
		*conv_err=1;
		return -1;
	}

	rc=rfc2045_decodemimesection(src, rfc, &myhandler, &ci);

	dummy_flag=0;
	if (unicode_convert_deinit(ci, &dummy_flag))
		rc= -1;

	if (dummy_flag)
		*conv_err=1;
	return (rc);
}

static int myhandler(const char *cp, size_t cnt, void *voidarg)
{
	return unicode_convert(*(unicode_convert_handle_t *)
				 voidarg, cp, cnt);
}
