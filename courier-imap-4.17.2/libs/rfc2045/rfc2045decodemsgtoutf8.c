/*
** Copyright 2010-2014 Double Precision, Inc.  See COPYING for
** distribution information.
*/

#include	"rfc2045.h"
#include	"rfc822/rfc822.h"
#include	<ctype.h>
#include	<unistd.h>
#include	<string.h>



struct doconvtoutf8_info {

	struct rfc2045_decodemsgtoutf8_cb *callback;

	int err_flag;
};

static void doconvtoutf8_write(const char *p, size_t n,
			       void *void_arg)
{
	struct doconvtoutf8_info *ptr=
		(struct doconvtoutf8_info *)void_arg;

	if (n && ptr->err_flag == 0)
		ptr->err_flag=(*ptr->callback->output_func)
			(p, n, ptr->callback->arg);
}

static void doconvtoutf8_write_noeol(const char *p, size_t n,
				     void *void_arg)
{
	while (n)
	{
		size_t i;

		if (*p == '\n')
		{
			doconvtoutf8_write(" ", 1, void_arg);
			++p;
			--n;
			continue;
		}

		for (i=0; i<n; ++i)
			if (p[i] == '\n')
				break;
		doconvtoutf8_write(p, i, void_arg);
		p += i;
		n -= i;
	}
}

static void doconvtoutf8_error(const char *p, int n,
			       void *void_arg)
{
	struct doconvtoutf8_info *ptr=(struct doconvtoutf8_info *)void_arg;

	ptr->err_flag= -1;
}

static int doconvtoutf8_rfc822hdr(const char *header,
				  const char *value,
				  struct rfc2045_decodemsgtoutf8_cb *callback)
{
	struct doconvtoutf8_info info;

	info.err_flag=0;
	info.callback=callback;

	if (callback->headerfilter_func &&
	    (*callback->headerfilter_func)(header, value, callback->arg) == 0)
		return 0;

	if ((callback->flags & RFC2045_DECODEMSG_NOHEADERNAME) == 0)
	{
		doconvtoutf8_write(header, strlen(header), &info);
		doconvtoutf8_write(": ", 2, &info);
	}
	rfc822_display_hdrvalue(header, value, "utf-8", 
				doconvtoutf8_write_noeol,
				doconvtoutf8_error,
				&info);
	doconvtoutf8_write("\n", 1, &info);
	if (callback->headerdone_func && info.err_flag == 0)
	{
		int rc=(*callback->headerdone_func)(header, callback->arg);

		if (rc)
			info.err_flag=rc;
	}

	return info.err_flag;
}

static int decode_handler(const char *p, size_t n, void *voidarg)
{
	const struct doconvtoutf8_info *ptr=
		(const struct doconvtoutf8_info *)voidarg;

	int rc=0;

	if (n)
		rc=(*ptr->callback->output_func)(p, n, ptr->callback->arg);

	return rc;
}

int rfc2045_decodemsgtoutf8(struct rfc2045src *src,
			    struct rfc2045 *p,
			    struct rfc2045_decodemsgtoutf8_cb *callback)
{
	struct rfc2045headerinfo *hi;
	int rc;

	hi=rfc2045header_start(src, p);

	if (hi)
	{
		char *header;
		char *value;

		while (rfc2045header_get(hi, &header, &value,
					 RFC2045H_NOLC |
					 RFC2045H_KEEPNL) == 0 && header)
		{
			if (callback->flags & RFC2045_DECODEMSG_NOHEADERS)
				continue;

			if (doconvtoutf8_rfc822hdr(header, value,
						   callback) < 0)
				return -1;
		}
		rfc2045header_end(hi);
	}

	if (p->firstpart)
	{
		for (p=p->firstpart; p; p=p->next)
		{
			if (!p->isdummy)
			{
				if ((rc=rfc2045_decodemsgtoutf8(src, p,
								callback))
				    != 0)
					return rc;
			}
		}
	}
	else
	{
		const char *content_type;
		const char *transfer_encoding;
		const char *charset;
		struct doconvtoutf8_info info;

		info.callback=callback;

		rfc2045_mimeinfo(p, &content_type, &transfer_encoding,
				 &charset);

		if ((strncmp(content_type, "text/", 5) == 0 ||
		     strncmp(content_type, "message/", 8) == 0) &&
		    (callback->flags & RFC2045_DECODEMSG_NOBODY) == 0 &&
		    (rc=rfc2045_decodetextmimesection(src, p, "utf-8", NULL,
						      decode_handler,
						      &info)) != 0)
			return rc;
	}
	return 0;
}
