/*
** Copyright 2011 Double Precision, Inc.  See COPYING for
** distribution information.
*/

#include "rfc2045_config.h"
#include	"rfc3676parser.h"
#include	<stdlib.h>
#include	<string.h>

#define NONFLOWED_WRAP_REDUCE	74

#define NONFLOWED_THRESHOLD_EXCEEDED	30


static void emit_line_begin(rfc3676_parser_t handle);

static void emit_line_contents(rfc3676_parser_t handle,
			       const char32_t *uc,
			       size_t cnt);

static void emit_line_flowed_wrap(rfc3676_parser_t handle);

static void emit_line_end(rfc3676_parser_t handle);


static void nonflowed_line_begin(rfc3676_parser_t handle);

static void nonflowed_line_contents(rfc3676_parser_t handle,
				    const char32_t *uc,
				    size_t cnt);

static void nonflowed_line_end(rfc3676_parser_t handle);

static int nonflowed_line_process(int linebreak_opportunity,
				  char32_t ch, void *dummy);

#define EMIT_LINE_BEGIN(h) do {			\
		(*(h)->line_begin_handler)(h);	\
	} while (0)

#define EMIT_LINE_CONTENTS(h, uc, cnt) do {			\
		(*(h)->line_content_handler)((h),(uc),(cnt));	\
	} while (0)

#define EMIT_LINE_END(h) do {			\
		(*(h)->line_end_handler)(h);	\
	} while (0)

struct rfc3676_parser_struct {

	struct rfc3676_parser_info info;
	unicode_convert_handle_t uhandle;

	int errflag;

	/* Receive raw text stream, converted to unicode */
	size_t (*line_handler)(rfc3676_parser_t,
			       const char32_t *ptr, size_t cnt);

	/*
	** Receive mostly raw text stream: CRs that precede an LF
	** are removed from the stream received by content_handler.
	*/
	size_t (*content_handler)(rfc3676_parser_t,
				  const char32_t *ptr, size_t cnt);

	size_t quote_level;
	size_t sig_block_index;

	/*
	** Flag: previous line ended in a flowed space, and the previous
	** line's quoting level was this.
	*/
	int has_previous_quote_level;
	size_t previous_quote_level;

	/*
	** Flag: current line was flowed into from a previous line with the
	** same quoting level.
	*/
	int was_previous_quote_level;

	/* A line has begun */
	void (*line_begin_handler)(rfc3676_parser_t handle);

	/* Content of this line */
	void (*line_content_handler)(rfc3676_parser_t handle,
				     const char32_t *uc,
				     size_t cnt);

	/* End of this line */
	void (*line_end_handler)(rfc3676_parser_t handle);


	/*
	** When non-flowed text is getting rewrapped, we utilize the services
	** of the unicode_lbc_info API.
	*/

	unicode_lbc_info_t lb;

	struct unicode_buf nonflowed_line;
	/* Collect unflowed line until it reaches the given size */

	struct unicode_buf nonflowed_next_word;
	/* Collects unicode stream until a linebreaking opportunity */

	size_t nonflowed_line_target_width;
	/* Targeted width of nonflowed lines */

	size_t nonflowed_line_width; /* Width of nonflowed_line */

	size_t nonflowed_next_word_width; /* Width of nonflowed_next_word */

	/* Current handle of non-flowd content. */
	void (*nonflowed_line_process)(struct rfc3676_parser_struct *handle,
				       int linebreak_opportunity,
				       char32_t ch,
				       size_t ch_width);

	void (*nonflowed_line_end)(struct rfc3676_parser_struct *handle);
};

static int parse_unicode(const char *, size_t, void *);

static size_t scan_crlf(rfc3676_parser_t handle,
			const char32_t *ptr, size_t cnt);

static size_t scan_crlf_seen_cr(rfc3676_parser_t handle,
				const char32_t *ptr, size_t cnt);

static size_t start_of_line(rfc3676_parser_t handle,
			    const char32_t *ptr, size_t cnt);

static size_t count_quote_level(rfc3676_parser_t handle,
				const char32_t *ptr, size_t cnt);

static size_t counted_quote_level(rfc3676_parser_t handle,
				  const char32_t *ptr, size_t cnt);

static size_t check_signature_block(rfc3676_parser_t handle,
				    const char32_t *ptr, size_t cnt);

static size_t start_content_line(rfc3676_parser_t handle,
				const char32_t *ptr, size_t cnt);

static size_t scan_content_line(rfc3676_parser_t handle,
				const char32_t *ptr, size_t cnt);

static size_t seen_sig_block(rfc3676_parser_t handle,
			     const char32_t *ptr, size_t cnt);

static size_t seen_notsig_block(rfc3676_parser_t handle,
				const char32_t *ptr, size_t cnt);

static size_t seen_content_sp(rfc3676_parser_t handle,
			      const char32_t *ptr, size_t cnt);


/*
** The top layer initializes the conversion to unicode.
*/

rfc3676_parser_t rfc3676parser_init(const struct rfc3676_parser_info *info)
{
	rfc3676_parser_t handle=
		(rfc3676_parser_t)calloc(1,
					 sizeof(struct rfc3676_parser_struct));

	if (!handle)
		return NULL;

	handle->info=*info;
	if ((handle->uhandle=unicode_convert_init(info->charset,
						    unicode_u_ucs4_native,
						    parse_unicode,
						    handle)) == NULL)
	{
		free(handle);
		return NULL;
	}

	if (!handle->info.isflowed)
		handle->info.isdelsp=0; /* Sanity check */

	handle->line_handler=scan_crlf;
	handle->content_handler=start_of_line;
	handle->has_previous_quote_level=0;
	handle->previous_quote_level=0;

	handle->line_begin_handler=emit_line_begin;
	handle->line_content_handler=emit_line_contents;
	handle->line_end_handler=emit_line_end;

	unicode_buf_init(&handle->nonflowed_line, (size_t)-1);
	unicode_buf_init(&handle->nonflowed_next_word, (size_t)-1);

	if (!handle->info.isflowed)
	{
		handle->line_begin_handler=nonflowed_line_begin;
		handle->line_content_handler=nonflowed_line_contents;
		handle->line_end_handler=nonflowed_line_end;
	}
	return handle;
}

int rfc3676parser(rfc3676_parser_t handle,
		  const char *txt,
		  size_t txt_cnt)
{
	if (handle->errflag)
		return handle->errflag; /* Error occured previously */

	/* Convert to unicode and invoke parse_unicode() */

	return unicode_convert(handle->uhandle, txt, txt_cnt);
}

/*
** Convert char stream from iconv into char32_ts, then pass them to the
** current handler, until all converted char32_ts are consumed.
*/

static int parse_unicode(const char *ucs4, size_t nbytes, void *arg)
{
	rfc3676_parser_t handle=(rfc3676_parser_t)arg;
	char32_t ucs4buf[128];
	const char32_t *p;

	/* Keep going until there's an error, or everything is consumed. */

	while (handle->errflag == 0 && nbytes)
	{
		/* Do it in pieces, using the temporary char32_t buffer */

		size_t cnt=nbytes;

		if (cnt > sizeof(ucs4buf))
			cnt=sizeof(ucs4buf);

		memcpy(ucs4buf, ucs4, cnt);

		ucs4 += cnt;
		nbytes -= cnt;

		cnt /= sizeof(char32_t);
		p=ucs4buf;

		/* Keep feeding it to the current handler */

		while (handle->errflag == 0 && cnt)
		{
			size_t n=(*handle->line_handler)(handle, p, cnt);

			if (handle->errflag == 0)
			{
				cnt -= n;
				p += n;
			}
		}
	}

	return handle->errflag;
}

int rfc3676parser_deinit(rfc3676_parser_t handle, int *errptr)
{
	/* Finish unicode conversion */

	int rc=unicode_convert_deinit(handle->uhandle, errptr);

	if (rc == 0)
		rc=handle->errflag;

	if (rc == 0)
	{
		(*handle->line_handler)(handle, NULL, 0);
		rc=handle->errflag;
	}

	if (handle->lb)
	{
		int rc2=unicode_lbc_end(handle->lb);

		if (rc2 && rc == 0)
			rc=rc2;
	}

	unicode_buf_deinit(&handle->nonflowed_line);
	unicode_buf_deinit(&handle->nonflowed_next_word);

	free(handle);
	return rc;
}

/*
** Look for a CR that might precede an LF.
*/

static size_t scan_crlf(rfc3676_parser_t handle,
			const char32_t *ptr, size_t cnt)
{
	size_t i;

	if (ptr == NULL)
	{
		if (handle->errflag == 0)
			(*handle->content_handler)(handle, NULL, 0);
		return 0;
	}

	for (i=0; ptr && i<cnt; ++i)
	{
		if (ptr[i] == '\r')
			break;
	}

	if (i)
	{
		size_t consumed=0;

		while (i && handle->errflag == 0)
		{
			size_t n=(*handle->content_handler)(handle, ptr, i);

			ptr += n;
			consumed += n;
			i -= n;
		}
		return consumed;
	}

	/* Consume the first character, the CR */

	handle->line_handler=scan_crlf_seen_cr;
	return 1;
}

/*
** Check the first character after a CR.
*/

static size_t scan_crlf_seen_cr(rfc3676_parser_t handle,
				const char32_t *ptr, size_t cnt)
{
	char32_t cr='\r';

	handle->line_handler=scan_crlf;

	if (ptr == NULL || *ptr != '\n')
	{
		/*
		** CR was not followed by a NL.
		** Restore it in the char stream.
		*/

		while (handle->errflag == 0)
			if ((*handle->content_handler)(handle, &cr, 1))
				break;
	}

	return scan_crlf(handle, ptr, cnt);
}

/*
** From this point on, CRLF are collapsed into NLs, so don't need to worry
** about them.
*/


/*
** Check for an EOF indication at the start of the line.
*/

static size_t start_of_line(rfc3676_parser_t handle,
			    const char32_t *ptr, size_t cnt)
{
	if (ptr == NULL)
	{
		if (handle->has_previous_quote_level)
			EMIT_LINE_END(handle); /* Last line was flowed */

		return cnt; /* EOF */
	}

	/* Begin counting the quote level */

	handle->content_handler=count_quote_level;
	handle->quote_level=0;
	return count_quote_level(handle, ptr, cnt);
}

/*
** Count leading > in flowed content.
*/

static size_t count_quote_level(rfc3676_parser_t handle,
				const char32_t *ptr, size_t cnt)
{
	size_t i;

	if (ptr == NULL) /* EOF, pretend that the quote level was counted */
		return (handle->content_handler=counted_quote_level)
			(handle, ptr, cnt);

	for (i=0; i<cnt; ++i)
	{
		if (ptr[i] != '>' || !handle->info.isflowed)
		{
			handle->content_handler=counted_quote_level;

			if (i == 0)
				return counted_quote_level(handle, ptr, cnt);
			break;
		}
		++handle->quote_level;
	}

	return i;
}

/*
** This line's quote level has now been counted.
*/

static size_t counted_quote_level(rfc3676_parser_t handle,
				  const char32_t *ptr, size_t cnt)
{
	handle->was_previous_quote_level=0;

	/*
	** If the previous line was flowed and this line has the same
	** quote level, make the flow official.
	*/

	if (handle->has_previous_quote_level &&
	    handle->quote_level == handle->previous_quote_level)
	{
		/* Remember that this line was flowed into */
		handle->was_previous_quote_level=1;
	}
	else
	{
		/*
		** If the previous line was flowed, but this line carries
		** a different quote level, force-terminate the previous
		** line, before beginning this line.
		*/
		if (handle->has_previous_quote_level)
			EMIT_LINE_END(handle);

		EMIT_LINE_BEGIN(handle);
	}

	handle->has_previous_quote_level=0;
	/* Assume this line won't be flowed, until shown otherwise */


	if (!handle->info.isflowed)
	{
		/*
		** No space-stuffing, or sig block checking, if this is not
		** flowed content.
		*/
		handle->content_handler=scan_content_line;
		return scan_content_line(handle, ptr, cnt);
	}


	handle->content_handler=start_content_line;

	if (ptr != NULL && *ptr == ' ')
		return 1; /* Remove stuffed space */

	return start_content_line(handle, ptr, cnt);
}

/*
** Minor deviation from RFC3676, but this fixes a lot of broken text.
**
** If the previous line was flowed, but this is an empty line (optionally
** space-stuffed), unflow the last line (make it fixed), and this becomes
** a fixed line too. Example:
**
** this is the last end of a paragraph[SPACE]
** [SPACE]
** This is the first line of the next paragraph.
**
** Strict RFC3676 rules will parse this as a flowed line, then a fixed line,
** resulting in no paragraph breaks.
*/

static size_t start_content_line(rfc3676_parser_t handle,
				const char32_t *ptr, size_t cnt)
{
	/*
	** We'll start scanning for the signature block, as soon as
	** this check is done.
	*/
	handle->content_handler=check_signature_block;
	handle->sig_block_index=0;

	if (ptr && *ptr == '\n' && handle->was_previous_quote_level)
	{
		EMIT_LINE_END(handle);
		EMIT_LINE_BEGIN(handle);
		handle->was_previous_quote_level=0;
	}

	return check_signature_block(handle, ptr, cnt);
}


static const char32_t sig_block[]={'-', '-', ' '};

/* Checking for a magical sig block */

static size_t check_signature_block(rfc3676_parser_t handle,
				    const char32_t *ptr, size_t cnt)
{
	if (ptr && *ptr == sig_block[handle->sig_block_index])
	{
		if (++handle->sig_block_index == sizeof(sig_block)
		    /sizeof(sig_block[0]))

			/* Well, it's there, but does a NL follow? */
			handle->content_handler=seen_sig_block;
		return 1;
	}

	return seen_notsig_block(handle, ptr, cnt);
}

static size_t seen_sig_block(rfc3676_parser_t handle,
			     const char32_t *ptr, size_t cnt)
{
	if (ptr == NULL || *ptr == '\n')
	{
		/*
		** If the previous line was flowed, the sig block is not
		** considered to be flowable-into content, so terminate
		** the previous line before emitting the sig block.
		*/

		if (handle->was_previous_quote_level)
		{
			EMIT_LINE_END(handle);
			EMIT_LINE_BEGIN(handle);
			handle->was_previous_quote_level=0;
		}

		/* Pass through the sig block */

		handle->content_handler=start_of_line;

		EMIT_LINE_CONTENTS(handle, sig_block,
				   sizeof(sig_block)/sizeof(sig_block[0]));
		EMIT_LINE_END(handle);
		return ptr ? 1:0;
	}

	return seen_notsig_block(handle, ptr, cnt);
}

/* This is not a sig block line */

static size_t seen_notsig_block(rfc3676_parser_t handle,
				 const char32_t *newptr, size_t newcnt)
{
	const char32_t *ptr;
	size_t i;

	if (handle->was_previous_quote_level)
		emit_line_flowed_wrap(handle);

	handle->content_handler=scan_content_line;

	ptr=sig_block;
	i=handle->sig_block_index;

	while (i && handle->errflag == 0)
	{
		size_t n=(*handle->content_handler)(handle, ptr, i);

		ptr += n;
		i -= n;
	}

	return (*handle->content_handler)(handle, newptr, newcnt);
}

/*
** Pass through the line, until encountering an NL, or a space in flowable
** content.
*/

static size_t scan_content_line(rfc3676_parser_t handle,
				const char32_t *ptr, size_t cnt)
{
	size_t i;

	for (i=0; ptr && i<cnt && ptr[i] != '\n' &&
		     (ptr[i] != ' ' || !handle->info.isflowed); ++i)
		;

	/* Pass through anything before the NL or potentially flowable SP */

 	if (i)
		EMIT_LINE_CONTENTS(handle, ptr, i);

	if (i)
		return i;

	if (ptr && ptr[i] == ' ')
	{
		handle->content_handler=seen_content_sp;
		return 1;
	}

	/* NL. This line does not flow */
	EMIT_LINE_END(handle);

	handle->content_handler=start_of_line;

	return ptr ? 1:0;
}

static size_t seen_content_sp(rfc3676_parser_t handle,
			      const char32_t *ptr, size_t cnt)
{
	char32_t sp=' ';

	handle->content_handler=scan_content_line;

	if (ptr == NULL || *ptr != '\n')
	{
		/*
		** SP was not followed by the NL. Pass through the space,
		** then resume scanning.
		*/
		EMIT_LINE_CONTENTS(handle, &sp, 1);
		return scan_content_line(handle, ptr, cnt);
	}

	/* NL after a SP -- flowed line */

	if (!handle->info.isdelsp)
		EMIT_LINE_CONTENTS(handle, &sp, 1);

	handle->has_previous_quote_level=1;
	handle->previous_quote_level=handle->quote_level;
	handle->content_handler=start_of_line;
	return ptr ? 1:0;
}

/**************************************************************************/

/*
** At this point, the processing has reduced to the following API:
**
** + begin logical line
**
** + contents of the logical line (multiple consecutive invocations)
**
** + the logical line has flowed onto the next physical line
**
** + end of logical line
**
** The third one, logical line flowed, is normally used for flowed text,
** by definition. But, it may also be get used if non-flowed text gets
** rewrapped when broken formatting is detected.
**
** Provide default implementations of the other three API calls that
** simply invoke the corresponding user callback.
*/

static void emit_line_begin(rfc3676_parser_t handle)
{
	if (handle->errflag == 0)
		handle->errflag=(*handle->info.line_begin)(handle->quote_level,
							   handle->info.arg);
}

static void emit_line_flowed_wrap(rfc3676_parser_t handle)
{
	if (handle->errflag == 0 && handle->info.line_flowed_notify)
		handle->errflag=(*handle->info.line_flowed_notify)
			(handle->info.arg);
}

static void emit_line_contents(rfc3676_parser_t handle,
			       const char32_t *uc,
			       size_t cnt)
{
	if (handle->errflag == 0 && cnt > 0)
		handle->errflag=(*handle->info.line_contents)
			(uc, cnt, handle->info.arg);
}

static void emit_line_end(rfc3676_parser_t handle)
{
	if (handle->errflag == 0)
		handle->errflag=(*handle->info.line_end)(handle->info.arg);
}

/*
** When processing a non-flowed text, handle broken mail formatters (I'm
** looking at you, Apple Mail) that spew out quoted-printable content with
** each decoded line forming a single paragraph. This is heuristically
** detected by looking for lines that exceed a wrapping threshold, then
** rewrapping them.
**
** Redefine the three line API calls to launder the logical line via
** the linebreak API.
*/

static void initial_nonflowed_line(rfc3676_parser_t handle,
				   int linebreak_opportunity,
				   char32_t ch,
				   size_t ch_width);

static void initial_nonflowed_end(rfc3676_parser_t handle);

static void begin_forced_rewrap(rfc3676_parser_t handle);

/*
** A non-flowed line begins. Initialize the linebreaking module.
*/
static void nonflowed_line_begin(rfc3676_parser_t handle)
{
	if (handle->lb)
	{
		/* Just in case */

		int rc=unicode_lbc_end(handle->lb);

		if (rc && handle->errflag == 0)
			handle->errflag=rc;
	}

	if ((handle->lb=unicode_lbc_init(nonflowed_line_process, handle))
	    == NULL)
	{
		if (handle->errflag == 0)
			handle->errflag=-1;
	}

	if (handle->lb)
		unicode_lbc_set_opts(handle->lb,
				     UNICODE_LB_OPT_PRBREAK
				     | UNICODE_LB_OPT_SYBREAK);

	unicode_buf_clear(&handle->nonflowed_line);
	unicode_buf_clear(&handle->nonflowed_next_word);

	handle->nonflowed_line_width=0;
	handle->nonflowed_next_word_width=0;

	handle->nonflowed_line_process=initial_nonflowed_line;
	handle->nonflowed_line_end=initial_nonflowed_end;
	emit_line_begin(handle); /* Fallthru - user callback */

	handle->nonflowed_line_target_width=
		handle->quote_level < NONFLOWED_WRAP_REDUCE - 20 ?
		NONFLOWED_WRAP_REDUCE - handle->quote_level:20;
}

/*
** Process contents of non-flowed lines. The contents are submitted to the
** linebreaking API.
*/

static void nonflowed_line_contents(rfc3676_parser_t handle,
				    const char32_t *uc,
				    size_t cnt)
{
	if (!handle->lb)
		return;

	while (cnt)
	{
		if (handle->errflag == 0)
			handle->errflag=unicode_lbc_next(handle->lb, *uc);

		++uc;
		--cnt;
	}
}

/*
** End of non-flowed content. Terminate the linebreaking API, then invoke
** the current end-of-line handler.
*/
static void nonflowed_line_end(rfc3676_parser_t handle)
{
	if (handle->lb)
	{
		int rc=unicode_lbc_end(handle->lb);

		if (rc && handle->errflag == 0)
			handle->errflag=rc;

		handle->lb=NULL;
	}

	(*handle->nonflowed_line_end)(handle);
	emit_line_end(handle); /* FALLTHRU */
}

/*
** Callback from the linebreaking API, gives us the next unicode character
** and its linebreak property. Look up the unicode character's width, then
** invoke the current handler.
*/
static int nonflowed_line_process(int linebreak_opportunity,
				  char32_t ch, void *dummy)
{
	rfc3676_parser_t handle=(rfc3676_parser_t)dummy;

	(*handle->nonflowed_line_process)(handle, linebreak_opportunity, ch,
					  unicode_wcwidth(ch));

	return 0;
}

/*
** Collecting initial nonflowed line.
*/

static void initial_nonflowed_line(rfc3676_parser_t handle,
				   int linebreak_opportunity,
				   char32_t ch,
				   size_t ch_width)
{
	/*
	** Collect words into nonflowed_line as long as it fits within the
	** targeted width.
	*/
	if (linebreak_opportunity != UNICODE_LB_NONE &&
	    handle->nonflowed_line_width + handle->nonflowed_next_word_width
	    <= handle->nonflowed_line_target_width)
	{
		unicode_buf_append_buf(&handle->nonflowed_line,
				       &handle->nonflowed_next_word);
		handle->nonflowed_line_width +=
			handle->nonflowed_next_word_width;

		unicode_buf_clear(&handle->nonflowed_next_word);
		handle->nonflowed_next_word_width=0;
	}

	/*
	** Add the character to the growing word.
	**
	** If the line's size now exceeds the target width by quite a bit,
	** we've had enough!
	*/

	unicode_buf_append(&handle->nonflowed_next_word, &ch, 1);
	handle->nonflowed_next_word_width += ch_width;

	if (handle->nonflowed_line_width + handle->nonflowed_next_word_width
	    > handle->nonflowed_line_target_width
	    + NONFLOWED_THRESHOLD_EXCEEDED)
		begin_forced_rewrap(handle);
}

/*
** End of line handler. The line did not reach its threshold, so output it.
*/
static void initial_nonflowed_end(rfc3676_parser_t handle)
{
	emit_line_contents(handle,
			   unicode_buf_ptr(&handle->nonflowed_line),
			   unicode_buf_len(&handle->nonflowed_line));

	emit_line_contents(handle,
			   unicode_buf_ptr(&handle->nonflowed_next_word),
			   unicode_buf_len(&handle->nonflowed_next_word));
}

/*
** Check for the abnormal situation where we're ready to wrap something but
** nonflowed_line is empty because all this text did not have a linebreaking
** opportunity.
*/

static void check_abnormal_line(rfc3676_parser_t handle)
{
	size_t n, i;
	const char32_t *p;

	if (unicode_buf_len(&handle->nonflowed_line) > 0)
		return;

	/* Extreme times call for extreme measures */

	n=unicode_buf_len(&handle->nonflowed_next_word);
	p=unicode_buf_ptr(&handle->nonflowed_next_word);

	for (i=n; i>0; --i)
	{
		if (i < n && unicode_grapheme_break(p[i-1], p[i]))
		{
			n=i;
			break;
		}
	}

	unicode_buf_append(&handle->nonflowed_line, p, n);
	unicode_buf_remove(&handle->nonflowed_next_word, 0, n);

	/*
	** Recalculate the width of the growing word, now.
	*/

	handle->nonflowed_next_word_width=0;
	p=unicode_buf_ptr(&handle->nonflowed_next_word);

	for (i=0; i<unicode_buf_len(&handle->nonflowed_next_word); ++i)
		handle->nonflowed_next_word_width +=
			unicode_wcwidth(p[i]);
}

/*
** We've decided that the line is too long, so begin rewrapping it.
*/

static void forced_rewrap_line(rfc3676_parser_t handle,
			       int linebreak_opportunity,
			       char32_t ch,
			       size_t ch_width);

static void forced_rewrap_end(rfc3676_parser_t handle);

/*
** Emit nonflowed_line as the rewrapped line. Clear the buffer.
*/
static void emit_rewrapped_line(rfc3676_parser_t handle)
{
	check_abnormal_line(handle);
	emit_line_contents(handle, unicode_buf_ptr(&handle->nonflowed_line),
			   unicode_buf_len(&handle->nonflowed_line));

	emit_line_flowed_wrap(handle);

	/* nonflowed_line is now empty */
	unicode_buf_clear(&handle->nonflowed_line);
	handle->nonflowed_line_width=0;
}

static void begin_forced_rewrap(rfc3676_parser_t handle)
{
	handle->nonflowed_line_process=forced_rewrap_line;
	handle->nonflowed_line_end=forced_rewrap_end;
	emit_rewrapped_line(handle);
}

static void forced_rewrap_line(rfc3676_parser_t handle,
			       int linebreak_opportunity,
			       char32_t ch,
			       size_t ch_width)
{
	if (linebreak_opportunity != UNICODE_LB_NONE)
	{
		/* Found a linebreaking opportunity */

		if (handle->nonflowed_line_width
		    + handle->nonflowed_next_word_width
		    > handle->nonflowed_line_target_width)
		{
			/* Accumulated word is too long */
			emit_rewrapped_line(handle);
		}

		unicode_buf_append_buf(&handle->nonflowed_line,
				       &handle->nonflowed_next_word);

		handle->nonflowed_line_width +=
			handle->nonflowed_next_word_width;
		unicode_buf_clear(&handle->nonflowed_next_word);
		handle->nonflowed_next_word_width=0;
	}

	/*
	** Check for another excessively long line.
	*/

	if (handle->nonflowed_line_width == 0 &&
	    handle->nonflowed_next_word_width + ch_width
	    > handle->nonflowed_line_target_width)
	{
		emit_rewrapped_line(handle);
	}

	unicode_buf_append(&handle->nonflowed_next_word, &ch, 1);
	handle->nonflowed_next_word_width += ch_width;
}

static void forced_rewrap_end(rfc3676_parser_t handle)
{
	initial_nonflowed_end(handle); /* Same logic, for now */
}
