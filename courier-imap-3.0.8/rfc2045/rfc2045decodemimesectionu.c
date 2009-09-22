/*
** Copyright 2000 Double Precision, Inc.  See COPYING for
** distribution information.
*/

#include "rfc2045_config.h"
#include	"rfc2045.h"
#include	<stdio.h>
#include	<unistd.h>

#if HAVE_UNICODE

#include	"unicode/unicode.h"

/*
** Call rfc2045_decodemimesection, expecting textual content.  Convert
** textual content to local character set, if possible.  This is implemented
** by saving the real callback function, then calling rfc2045_decodemimesection
** and specifying our own callback function, which does the conversion, then
** calls the original callback function.  Neat, eh?
*/

struct convert_info {
	const struct unicode_info *src_chset;
	const struct unicode_info *dst_chset;
	int (*orig_handler)(const char *, size_t, void *);
	void *orig_voidarg;

	char *buffer;
	size_t bufsize;
	size_t bufcnt;
} ;

static int myhandler(const char *, size_t, void *);
static int txtflush(struct convert_info *, size_t);

int rfc2045_decodetextmimesection(int fd,
				  struct rfc2045 *rfc,
				  const char *mychset,
				  int (*handler)(const char *,
						 size_t, void *),
				  void *voidarg)
{
	const char *dummy;
	const char *src_chset;

	struct convert_info ci;
	int rc;

	rfc2045_mimeinfo(rfc, &dummy, &dummy, &src_chset);

	ci.src_chset=unicode_find(src_chset);
	ci.dst_chset=unicode_find(mychset);

	if (!ci.src_chset || !ci.dst_chset ||
	    strcmp(ci.src_chset->chset, ci.dst_chset->chset) == 0)
	{
		return (rfc2045_decodemimesection(fd, rfc, handler, voidarg));

		/*
		** Fallback to non-decoded extraction in the event that
		** we do not have unicode information for either character
		** set, or if there's no conversion needed.
		*/
	}

	ci.orig_handler=handler;
	ci.orig_voidarg=voidarg;
	ci.buffer=NULL;
	ci.bufsize=0;
	ci.bufcnt=0;

	rc=rfc2045_decodemimesection(fd, rfc, &myhandler, &ci);

	if (rc == 0)
		rc=txtflush(&ci, ci.bufcnt);	/* Any remainder */

	if (ci.buffer)
		free(ci.buffer);
	return (rc);
}

static int myhandler(const char *cp, size_t cnt, void *voidarg)
{
	struct convert_info *ci=(struct convert_info *)voidarg;
	size_t i;
	int rc;

	/* Do conversion in medium-sized pieces (we may get a large chunk) */

	while (cnt)
	{
		int n=1024;

		if (n > cnt)
			n=cnt;

		if (n + ci->bufcnt >= ci->bufsize)
			/* Make sure there's always an extra byte */
		{
			size_t newsize=ci->bufsize + 2048;
			char *newptr= ci->buffer ? realloc(ci->buffer, newsize)
				: malloc(newsize);

			if (!newptr)
				return (-1);
			ci->buffer=newptr;
			ci->bufsize=newsize;
		}
		memcpy(ci->buffer + ci->bufcnt, cp, n);
		ci->buffer[ci->bufcnt += n]=0; /* Prevent mem debuggers from
						  whining */

		cp += n;
		cnt -= n;

		/* Now, figure out what to convert.  We must avoid cutting
		** up multibyte characters, so we look for something safe
		** to break on, which would be a space or a newline.
		*/

		for (i=ci->bufcnt; i; --i)
		{
			switch (ci->buffer[i-1]) {
			case '\n':
			case ' ':
				break;
			default:
				continue;
			}
			break;
		}

		if (i)
		{
			rc=txtflush(ci, i);
			if (rc)
				return (rc);
		}
	}
	return (0);
}

static int txtflush(struct convert_info *ci, size_t n)
{
	int rc;
	char *ptr;
	char save_char;
	size_t i;

	if (n == 0)
		return (0);

	save_char=ci->buffer[n];

	/* We are assured there's always a spare byte at the end, see above */

	ci->buffer[n]=0;

	ptr=unicode_xconvert(ci->buffer, ci->src_chset, ci->dst_chset);
	ci->buffer[n]=save_char;

	i=0;
	while (n < ci->bufcnt)
		ci->buffer[i++]=ci->buffer[n++];
	ci->bufcnt=i;

	if (!ptr)
		return (-1);

	rc= (*ci->orig_handler)(ptr, strlen(ptr), ci->orig_voidarg);
	free(ptr);
	return (rc);
}

#else

int rfc2045_decodetextmimesection(int fd,
				  struct rfc2045 *rfc,
				  const char *mychset,
				  int (*handler)(const char *,
						 size_t, void *),
				  void *voidarg)
{
	return (rfc2045_decodemimesection(fd, rfc, handler, voidarg));
}
#endif
